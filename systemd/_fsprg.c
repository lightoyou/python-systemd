
#include <Python.h>
#include <alloca.h>
#include <systemd/sd-messages.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <gcrypt.h>
#include "fsprg.h"
#include "util.h"

static void printkey(void *key, size_t key_size) {
        size_t i;
        for(i = 0; i < key_size; i++)
                printf("%02x ", ((uint8_t*)key)[i]);
        printf("\n");
}

static uint8_t * parse_verification_key(const char *key) {
        size_t seed_size = FSPRG_RECOMMENDED_SEEDLEN;
        uint8_t seed[seed_size];
        size_t c;
        const char *k;
        int r;
        unsigned long long start, interval;

        if (!seed)
                return -ENOMEM;
        k = key;
       
        for (c = 0; c < seed_size; c++) {
              
                int x, y;

                while (*k == '-')
                        k++;

                x = unhexchar(*k);
                if (x < 0) {
                        return -EINVAL;
                }
                k++;
                y = unhexchar(*k);
                if (y < 0) {
                        return -EINVAL;
                }
                k++;

                seed[c] = (uint8_t) (x * 16 + y);
        }

        if (*k != '/') {
                return -EINVAL;
        }
        k++;

        r = sscanf(k, "%llx-%llx", &start, &interval);
        if (r != 2) {
                return -EINVAL;
        }
        printkey(seed, seed_size);
        return seed;
}


static PyObject* evolve(PyObject *self, PyObject *args) {
       size_t state_size = FSPRG_stateinbytes(FSPRG_RECOMMENDED_SECPAR);
       uint8_t state[state_size];
       Py_ssize_t n;
       size_t i;
       
       PyObject *pItem;
       PyObject *pList;
       PyObject* pystate = PyList_New(state_size);
       PyObject* value;
       if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &pList)) {
               PyErr_SetString(PyExc_TypeError, "parameter must be a list.");
               return NULL;
        }
        // Recover state from pyObject
        n = PyList_Size(pList);
        for (i=0; i<n; i++) {
                pItem = PyList_GetItem(pList, i);
                state[i] = (uint8_t)PyLong_AsUnsignedLong(pItem);
                if(!PyLong_Check(pItem)) {
                        PyErr_SetString(PyExc_TypeError, "list items must be integers.");
                        return NULL;
                }
        }
       FSPRG_Evolve(state);

       for(i = 0; i < state_size; i++)
        {
            value = PyLong_FromUnsignedLong((uint8_t*)state[i]);
            PyList_SetItem(pystate,i,value);
        }

       return pystate;
}

static PyObject* get_epoch(PyObject *self, PyObject *args) {
        
       uint64_t epoch;

       size_t state_size = FSPRG_stateinbytes(FSPRG_RECOMMENDED_SECPAR);
       uint8_t state[state_size];
       Py_ssize_t n;
       size_t i;
       
       PyObject *pItem;
       PyObject *pList;

       if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &pList)) {
               return NULL;
        }
         
        n = PyList_Size(pList);
        for (i=0; i<n; i++) {
                pItem = PyList_GetItem(pList, i);
                state[i] = (uint8_t)PyLong_AsUnsignedLong(pItem);
                if(!PyLong_Check(pItem)) {
                        PyErr_SetString(PyExc_TypeError, "list items must be integers.");
                        return NULL;
                }
        }

       epoch = FSPRG_GetEpoch(state);
       
       printf("epoch_{%8llu}\n", (unsigned long long)epoch);
       Py_RETURN_NONE;
}

static PyObject* seek(PyObject *self, PyObject *args) {
       PyObject *pState; // list
       PyObject *pItem; // pState object
       PyObject *pGoal; // int
       PyObject *pSeed; // str
   
       Py_ssize_t n;

       size_t state_size = FSPRG_stateinbytes(FSPRG_RECOMMENDED_SECPAR);
       uint8_t state[state_size];
       PyObject* pystate = PyList_New(state_size);
       PyObject* value;
       size_t i;
       
       size_t seed_size = FSPRG_RECOMMENDED_SEEDLEN;
       uint8_t seed[seed_size];
       uint64_t epoch;
       
       uint8_t msk_size = FSPRG_mskinbytes(FSPRG_RECOMMENDED_SECPAR);
       uint8_t msk[msk_size];

       if (!PyArg_ParseTuple(args, "O!Ks", &PyList_Type, &pState, &pGoal, &pSeed)) {
               PyErr_SetString(PyExc_TypeError, "wrong parameter.");
               return NULL;
        }

        // Recover State from Py object
        n = PyList_Size(pState);
        for (i=0; i<n; i++) {
                pItem = PyList_GetItem(pState, i);
                state[i] = (uint8_t)PyLong_AsUnsignedLong(pItem);
                if(!PyLong_Check(pItem)) {
                        PyErr_SetString(PyExc_TypeError, "list items must be integers.");
                        return NULL;
                }
        }

        printf("State");
        for(i = 0; i < state_size; i++)
                printf("%u ", ((uint8_t*)state)[i]);
        printf("\n");
        printf("Seed : %s\n", pSeed);
        *seed = parse_verification_key((const char * )pSeed);
     
        printf("Goal : %8llu\n", pGoal);

       
        FSPRG_GenMK(msk, NULL, seed, seed_size, FSPRG_RECOMMENDED_SECPAR);
        FSPRG_Seek(state, (uint64_t)pGoal, msk, seed, seed_size);



        for(i = 0; i < state_size; i++)
        {
            value = PyLong_FromUnsignedLong((uint8_t*)state[i]);
            PyList_SetItem(pystate,i,value);
        }

       return pystate;

}



static PyObject* get_key(PyObject *self, PyObject *args) {

       uint8_t key[256 / 8];/* Let's pass 256 bit from FSPRG to HMAC */

       size_t state_size = FSPRG_stateinbytes(FSPRG_RECOMMENDED_SECPAR);
       uint8_t state[state_size];
       Py_ssize_t n;
       size_t i;
       PyObject *pItem;
       PyObject *pList;
       if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &pList)) {
               PyErr_SetString(PyExc_TypeError, "parameter must be a list.");
               return NULL;
        }
        // Recover State from Py object
        n = PyList_Size(pList);
        for (i=0; i<n; i++) {
                pItem = PyList_GetItem(pList, i);
                state[i] = (uint8_t)PyLong_AsUnsignedLong(pItem);
                if(!PyLong_Check(pItem)) {
                        PyErr_SetString(PyExc_TypeError, "list items must be integers.");
                        return NULL;
                }
        }

        FSPRG_GetKey(state, key, sizeof(key), 0);
        printkey(key, sizeof(key));
        Py_RETURN_NONE;
}


/*
static PyObject* setup_keys(PyObject *self, PyObject *args) {

        size_t mpk_size = FSPRG_mskinbytes(FSPRG_RECOMMENDED_SECPAR);
        size_t seed_size = FSPRG_RECOMMENDED_SEEDLEN;
        size_t state_size = FSPRG_stateinbytes(FSPRG_RECOMMENDED_SECPAR);
        ssize_t l;
        uint8_t *mpk, *seed, *state;
        sd_id128_t machine;
        int r;
       // int fd = -1;
        uint64_t n;

        static usec_t arg_interval = DEFAULT_FSS_INTERVAL_USEC;


        p = gcry_check_version("1.4.5");
        assert(p);
        gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
        gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);


        r = sd_id128_get_machine(&machine);
    
        mpk = alloca(mpk_size);
        seed = alloca(seed_size);
        state = alloca(state_size);
        
        gcry_randomize(seed, seed_size, GCRY_STRONG_RANDOM);

        //fd = open("/dev/random", O_RDONLY|O_CLOEXEC|O_NOCTTY);
        //l = loop_read(fd, seed, seed_size, true);

        printf("Generating master keys (this may take some time)..."); fflush(stdout);
        FSPRG_GenMK(NULL, mpk, seed, seed_size, FSPRG_RECOMMENDED_SECPAR);
        printf("Generating the first state"); fflush(stdout);
        FSPRG_GenState0(state, mpk, seed, seed_size);
        printkey(state, state_size);

        n = now(CLOCK_REALTIME);
        n /= arg_interval;
        
        //safe_close(fd);

        for (size_t i = 0; i < seed_size; i++) {
                if (i > 0 && i % 3 == 0)
                        putchar('-');
                printf("%02x", ((uint8_t*) seed)[i]);
        }

        printf("/%llx-%llx\n", (unsigned long long) n, (unsigned long long) arg_interval);
        Py_RETURN_NONE;

}

*/


static PyMethodDef methods[] = {
        { "get_key",  get_key, METH_VARARGS, NULL },
        { "get_epoch",  get_epoch, METH_VARARGS, NULL },
        { "evolve",  evolve, METH_VARARGS, NULL },
        { "seek",  seek, METH_VARARGS, NULL },
        {}        /* Sentinel */
};

static struct PyModuleDef module = {
        PyModuleDef_HEAD_INIT,
        .m_name = "_fsprg", /* name of module */
        .m_doc = NULL, /* module documentation, may be NULL */
        .m_size = 0, /* size of per-interpreter state of the module */
        .m_methods = methods,
};

PyMODINIT_FUNC PyInit__fsprg(void) {
        PyObject *m;
        m = PyModule_Create(&module);
        if (!m)
                return NULL;
        return m;
}
