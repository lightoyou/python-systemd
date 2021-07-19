
#include <Python.h>

#include <alloca.h>
#include <systemd/sd-messages.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "fsprg.h"
#include "util.h"



static PyObject*  journal_file_fss_load(PyObject *self, PyObject *args) {
        int r, fd = -1;
        char *p = NULL;
        struct stat st;
        FSSHeader *m = NULL;
        sd_id128_t machine;
        JournalFile f;

        uint8_t key[256/8];

        r = sd_id128_get_machine(&machine);


        if (r < 0)
                return r;

        if (asprintf(&p, "/var/log/journal/" SD_ID128_FORMAT_STR "/fss",
                     SD_ID128_FORMAT_VAL(machine)) < 0)
                return -ENOMEM;

        fd = open(p, O_RDWR|O_CLOEXEC|O_NOCTTY, 0600);
        if (fstat(fd, &st) < 0) {
                r = -errno;
                goto finish;
        }

        if (st.st_size < (off_t) sizeof(FSSHeader)) {
                r = -ENODATA;
                goto finish;
        }

        m = mmap(NULL, PAGE_ALIGN(sizeof(FSSHeader)), PROT_READ, MAP_SHARED, fd, 0);
        if (m == MAP_FAILED) {
                m = NULL;
                r = -errno;
                goto finish;
        }

        if (memcmp(m->signature, FSS_HEADER_SIGNATURE, 8) != 0) {
                r = -EBADMSG;
                goto finish;
        }

        if (m->incompatible_flags != 0) {
                r = -EPROTONOSUPPORT;
                goto finish;
        }

        if (le64toh(m->header_size) < sizeof(FSSHeader)) {
                r = -EBADMSG;
                goto finish;
        }

        if (le64toh(m->fsprg_state_size) != FSPRG_stateinbytes(le16toh(m->fsprg_secpar))) {
                r = -EBADMSG;
                goto finish;
        }

        f.fss_file_size = le64toh(m->header_size) + le64toh(m->fsprg_state_size);
        if ((uint64_t) st.st_size < f.fss_file_size) {
                r = -ENODATA;
                goto finish;
        }

        if (!sd_id128_equal(machine, m->machine_id)) {
                r = -EHOSTDOWN;
                goto finish;
        }

        if (le64toh(m->start_usec) <= 0 ||
            le64toh(m->interval_usec) <= 0) {
                r = -EBADMSG;
                goto finish;
        }

        f.fss_file = mmap(NULL, PAGE_ALIGN(f.fss_file_size), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        if (f.fss_file == MAP_FAILED) {
                f.fss_file = NULL;
                r = -errno;
                goto finish;
        }

        f.fss_start_usec = le64toh(f.fss_file->start_usec);
        f.fss_interval_usec = le64toh(f.fss_file->interval_usec);

        f.fsprg_state = (uint8_t*) f.fss_file + le64toh(f.fss_file->header_size);
        f.fsprg_state_size = le64toh(f.fss_file->fsprg_state_size);
      
       
        printf("%" PRIu8 "\n",htole64(FSPRG_GetEpoch(f.fsprg_state)));

        printf("%" PRIx8 "\n",f.fsprg_state);


        FSPRG_GetKey(f.fsprg_state, key, sizeof(key), 0);

        for (size_t i = 0; i < (256/8); i++)
        {
        printf ("KEY[%zu] = %#" PRIx8 "\n", i, key[i]); 
        }

        r = 0;

finish:
        if (m)
                munmap(m, PAGE_ALIGN(sizeof(FSSHeader)));

        safe_close(fd);
        free(p);
        Py_RETURN_NONE;
}


static PyObject* setup_keys(PyObject *self, PyObject *args) {

        size_t mpk_size, seed_size, state_size;
        ssize_t l;
        uint8_t *mpk, *seed, *state;
        sd_id128_t machine;
        int r;
        int fd = -1;
        uint64_t n;

        static usec_t arg_interval = DEFAULT_FSS_INTERVAL_USEC;

        r = sd_id128_get_machine(&machine);
    
        mpk_size = FSPRG_mskinbytes(FSPRG_RECOMMENDED_SECPAR);
        mpk = alloca(mpk_size);

        seed_size = FSPRG_RECOMMENDED_SEEDLEN;
        seed = alloca(seed_size);

        state_size = FSPRG_stateinbytes(FSPRG_RECOMMENDED_SECPAR);
        state = alloca(state_size);
        
        fd = open("/dev/random", O_RDONLY|O_CLOEXEC|O_NOCTTY);
        
        l = loop_read(fd, seed, seed_size, true);

        FSPRG_GenMK(NULL, mpk, seed, seed_size, FSPRG_RECOMMENDED_SECPAR);
        FSPRG_GenState0(state, mpk, seed, seed_size);

        n = now(CLOCK_REALTIME);
        n /= arg_interval;
        safe_close(fd);

        for (size_t i = 0; i < seed_size; i++) {
                if (i > 0 && i % 3 == 0)
                        putchar('-');
                printf("%02x", ((uint8_t*) seed)[i]);
        }

        printf("/%llx-%llx\n", (unsigned long long) n, (unsigned long long) arg_interval);
        Py_RETURN_NONE;

}

static PyMethodDef methods[] = {
        { "setup_keys",  setup_keys, METH_VARARGS, NULL },
        { "journal_file_fss_load",  journal_file_fss_load, METH_VARARGS, NULL },
        {}        /* Sentinel */
};


static struct PyModuleDef module = {
        PyModuleDef_HEAD_INIT,
        .m_name = "_fsprg", /* name of module */
        .m_doc = NULL, /* module documentation, may be NULL */
        .m_size = 0, /* size of per-interpreter state of the module */
        .m_methods = methods,
};

DISABLE_WARNING_MISSING_PROTOTYPES;
PyMODINIT_FUNC PyInit__fsprg(void) {
        PyObject *m;

        m = PyModule_Create(&module);
        if (!m)
                return NULL;
        return m;
}