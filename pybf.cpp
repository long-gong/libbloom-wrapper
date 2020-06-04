#include "bloom.h"
#include <Python.h>

static char bloom_doc[] = "Python wrapping for libbloom, a simple and small "
                          "bloom filter implementation in C";

/*
 * Exported Methods declarations
 */
static PyObject *wrapper_init(PyObject *self, PyObject *args);
static PyObject *wrapper_check(PyObject *self, PyObject *args);
static PyObject *wrapper_add(PyObject *self, PyObject *args);
static PyObject *wrapper_check_str(PyObject *self, PyObject *args);
static PyObject *wrapper_add_str(PyObject *self, PyObject *args);
static PyObject *wrapper_free(PyObject *self, PyObject *args);
static PyObject *wrapper_print(PyObject *self, PyObject *args);
static PyObject *wrapper_size(PyObject *self, PyObject *args);
static PyObject *wrapper_num_hash(PyObject *self, PyObject *args);
static PyObject *wrapper_error(PyObject *self, PyObject *args);
static PyObject *wrapper_set_seed(PyObject *self, PyObject *args);

static PyObject *wrapper_num_set_bits(PyObject *self, PyObject *args);
static PyObject *wrapper_fpr(PyObject *self, PyObject *args);

/*
 * Methods exported by this module
 */
static PyMethodDef bloom_methods[] = {
    {"init", wrapper_init, METH_VARARGS},
    {"check", wrapper_check, METH_VARARGS},
    {"add", wrapper_add, METH_VARARGS},
    {"check_str", wrapper_check_str, METH_VARARGS},
    {"add_str", wrapper_add_str, METH_VARARGS},
    {"free", wrapper_free, METH_VARARGS},
    {"size", wrapper_size, METH_VARARGS},
    {"num_hash", wrapper_num_hash, METH_VARARGS},
    {"error", wrapper_error, METH_VARARGS},
    {"print", wrapper_print, METH_VARARGS},
    {"set_seed", wrapper_set_seed, METH_VARARGS},
    {"num_set_bits", wrapper_num_set_bits, METH_VARARGS},
    {"fpr", wrapper_fpr, METH_VARARGS},
    {NULL, NULL} /* Sentinel */
};

#if PY_MAJOR_VERSION >= 3
/* ! the struct name must respect the format: <name of the module> followed by
 * string "module"*/
static struct PyModuleDef bloommodule = {
    PyModuleDef_HEAD_INIT, "bloom", /* name of module */
    bloom_doc,                      /* module documentation, may be NULL */
    -1, /* size of per-interpreter state of the module,
           or -1 if the module keeps state in global variables. */
    bloom_methods};
#endif

/*
 * Module initialization
 */
#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC PyInit_bloom(void)
#else
PyMODINIT_FUNC initbloom(void)
#endif
{
#if PY_MAJOR_VERSION >= 3
  return PyModule_Create(&bloommodule);
#else
  (void)Py_InitModule("bloom", bloom_methods);
#endif
}

/*
 * Methods definition
 */

PyObject *wrapper_init(PyObject *self, PyObject *args) {
  int items = 0;
  double error = 0.0;
  if (!PyArg_ParseTuple(args, "id", &items, &error))
    Py_RETURN_NONE;

  struct bloom *bf = new bloom();
  bloom_init(bf, items, error);
  return Py_BuildValue("l", bf);
}

static PyObject *wrapper_check(PyObject *self, PyObject *args) {
  size_t key;
  int len;
  struct bloom *bf;
  int ret;
  if (!PyArg_ParseTuple(args, "lk", &bf, &key))
    Py_RETURN_NONE;

  len = sizeof(key);
  ret = bloom_check(bf, &key, len);

  return Py_BuildValue("i", ret);
}

static PyObject *wrapper_check_str(PyObject *self, PyObject *args) {
  int len;
  struct bloom *bf;
  int ret;
  char *str;
  if (!PyArg_ParseTuple(args, "lsi", &bf, &str, &len))
    Py_RETURN_NONE;
  ret = bloom_check(bf, str, len);

  return Py_BuildValue("i", ret);
}

static PyObject *wrapper_add(PyObject *self, PyObject *args) {
  size_t key;
  int len;
  struct bloom *bf;
  int ret;
  if (!PyArg_ParseTuple(args, "lk", &bf, &key))
    Py_RETURN_NONE;

  len = sizeof(key);
  ret = bloom_add(bf, &key, len);
  return Py_BuildValue("i", ret);
}

static PyObject *wrapper_add_str(PyObject *self, PyObject *args) {
  int len;
  struct bloom *bf;
  int ret;
  char *str;
  if (!PyArg_ParseTuple(args, "lsi", &bf, &str, &len))
    Py_RETURN_NONE;
  ret = bloom_add(bf, str, len);
  return Py_BuildValue("i", ret);
}

static PyObject *wrapper_size(PyObject *self, PyObject *args) {
  struct bloom *bf;
  if (!PyArg_ParseTuple(args, "l", &bf))
    Py_RETURN_NONE;
  int ret = bf->bits;
  return Py_BuildValue("i", ret);
}

static PyObject *wrapper_num_hash(PyObject *self, PyObject *args) {
  struct bloom *bf;
  if (!PyArg_ParseTuple(args, "l", &bf))
    Py_RETURN_NONE;

  int ret = bf->hashes;
  return Py_BuildValue("i", ret);
}

static PyObject *wrapper_error(PyObject *self, PyObject *args) {
  struct bloom *bf;
  if (!PyArg_ParseTuple(args, "l", &bf))
    Py_RETURN_NONE;

  int ret = bf->error;
  return Py_BuildValue("d", ret);
}

static PyObject *wrapper_free(PyObject *self, PyObject *args) {
  struct bloom *bf;
  if (!PyArg_ParseTuple(args, "l", &bf))
    Py_RETURN_NONE;

  bloom_free(bf);
  free(bf);
  return Py_BuildValue("");
}

static PyObject *wrapper_print(PyObject *self, PyObject *args) {
  struct bloom *bf;
  if (!PyArg_ParseTuple(args, "l", &bf))
    Py_RETURN_NONE;

  bloom_print(bf);

  return Py_BuildValue("");
}

static PyObject *wrapper_set_seed(PyObject *self, PyObject *args) {
  struct bloom *bf;
  unsigned int seed;
  if (!PyArg_ParseTuple(args, "lI", &bf, &seed))
    Py_RETURN_NONE;

  bf->hashSeed = seed;

  return Py_BuildValue("");
}

static PyObject *wrapper_num_set_bits(PyObject *self, PyObject *args) {
#ifdef COUNTING_SET_BITS_ON
  struct bloom *bf;
  if (!PyArg_ParseTuple(args, "l", &bf))
    Py_RETURN_NONE;

  return Py_BuildValue("I", bf->num_set_bits);
#else
  Py_RETURN_NONE;
#endif
}

static PyObject *wrapper_fpr(PyObject *self, PyObject *args) {
#ifdef COUNTING_SET_BITS_ON
  struct bloom *bf;
  if (!PyArg_ParseTuple(args, "l", &bf))
    Py_RETURN_NONE;

  double fpr = pow((double)bf->num_set_bits / bf.bits, bf.hashes);
  return Py_BuildValue("d", fpr);
#else
  Py_RETURN_NONE;
#endif
}