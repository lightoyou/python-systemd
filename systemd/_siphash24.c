#include <Python.h>


#include "siphash24.h"

static void do_siphash24(const uint8_t *in, size_t len, const uint8_t *key) {
    uint64_t out;
    out = siphash24(in, len, key);
    printf("0x%hx", out);
    
}

static PyObject* generate_siphash24(PyObject *self, PyObject *args) {
    const uint8_t in[15]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e };
    const uint8_t key[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

    uint8_t in_buf[20];                  
    memcpy(in_buf, in, sizeof(in));
    do_siphash24(in_buf, sizeof(in), key);
    Py_RETURN_NONE;


}

static PyMethodDef methods[] = {
        { "siphash24",  generate_siphash24, METH_VARARGS, NULL },
        {}        /* Sentinel */
};



static struct PyModuleDef module = {
        PyModuleDef_HEAD_INIT,
        .m_name = "_siphash24", /* name of module */
        .m_doc = NULL, /* module documentation, may be NULL */
        .m_size = 0, /* size of per-interpreter state of the module */
        .m_methods = methods,
};

DISABLE_WARNING_MISSING_PROTOTYPES;
PyMODINIT_FUNC PyInit__siphash24(void) {
        PyObject *m;

        m = PyModule_Create(&module);
        if (!m)
                return NULL;
        return m;
}