/*-*- Mode: C; c-basic-offset: 8; indent-tabs-mode: nil -*-*/

/***

  Copyright 2012 David Strauss <david@davidstrauss.net>

  python-systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  python-systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with python-systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#include <Python.h>

#include <alloca.h>
#include <systemd/sd-messages.h>
#include <fcntl.h>
#include "fsprg.h"
#include "util.h"


static PyObject* setup_keys(PyObject *self, PyObject *args) {

        size_t mpk_size, seed_size, state_size;
        ssize_t l;
        uint8_t *mpk, *seed, *state;
        sd_id128_t machine;
        int r;
        int fd = -1;
        uint64_t n;

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


        safe_close(fd);

        for (size_t i = 0; i < seed_size; i++) {
                if (i > 0 && i % 3 == 0)
                        putchar('-');
                printf("%02x", ((uint8_t*) seed)[i]);
        }
        Py_RETURN_NONE;


}

static PyMethodDef methods[] = {
        { "setup_keys",  setup_keys, METH_VARARGS, NULL },
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