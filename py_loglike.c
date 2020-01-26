#include <Python.h>
#include <numpy/arrayobject.h>
#include "py_module_defs.h"
#include "loglike.h"

static PyObject *PgError;

/* Docstrings */
static char module_docstring[] =
    "This module provides an interface for calculating chi-squared using C.";
static char chi2_docstring[] =
    "Calculate the chi-squared of some data given a model.";

/* Available functions: forward declaration */
static PyObject* Py_loglike(PyObject *self, PyObject *args);

/* Module specification */
static PyMethodDef __module_methods__[] = 
{
    {"loglike", Py_loglike, METH_VARARGS, chi2_docstring},
    {NULL, NULL, 0, NULL}
};
static PyMethodDef module_methods[] = {

  {"loglike", (PyCFunction) Py_loglike, METH_VARARGS | METH_KEYWORDS,
   "Calculate log-likelihood"},
  
  {NULL, NULL, 0, NULL}        /* Sentinel */
};
#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "chi2.chi2",     /* m_name */
        "chi2 test",  /* m_doc */
        -1,                  /* m_size */
        module_methods,       /* m_methods */
        NULL,                /* m_reload */
        NULL,                /* m_traverse */
        NULL,                /* m_clear */
        NULL,                /* m_free */
};
#endif

MOD_INIT(chi2)
{

    PyObject *module = NULL;
    #if PY_MAJOR_VERSION >= 3
      module = PyModule_Create (&moduledef);
    #else
      module = Py_InitModule ("py_loglike test", module_methods);
    #endif

    if (module == NULL)
    {
      return MOD_ERROR_VAL;
    }
    import_array ();

    #if PY_MAJOR_VERSION < 3
      PyModule_AddStringConstant (module,
                "__doc__",
                "chi^2 test (C) functions");
    #endif

    PgError = PyErr_NewException ("chi2.error", NULL, NULL);
    Py_INCREF (PgError);
    PyModule_AddObject (module, "error", PgError);

    #if PY_MAJOR_VERSION >= 3
      return module;
    #endif
}

static PyObject* Py_loglike(PyObject *self, PyObject *args)
{
    double  alpha;
    PyObject *x_obj;
    PyObject *y_obj;
    PyObject *yerr_obj;

    /* Parse the input tuple */
    if (!PyArg_ParseTuple(args, "ddOOO", &alpha, &x_obj, &y_obj,
                                         &yerr_obj))
        return NULL;

    /* Interpret the input objects as numpy arrays. */
    PyObject *x_array = PyArray_FROM_OTF(x_obj, NPY_DOUBLE, NPY_IN_ARRAY);
    PyObject *y_array = PyArray_FROM_OTF(y_obj, NPY_DOUBLE, NPY_IN_ARRAY);
    PyObject *yerr_array = PyArray_FROM_OTF(yerr_obj, NPY_DOUBLE,
                                            NPY_IN_ARRAY);

    /* If that didn't work, throw an exception. */
    if (x_array == NULL || y_array == NULL || yerr_array == NULL) 
    {
        Py_XDECREF(x_array);
        Py_XDECREF(y_array);
        Py_XDECREF(yerr_array);
        return NULL;
    }

    /* How many data points are there? */
    int N = (int)PyArray_DIM(x_array, 0);

    /* Get pointers to the data as C-types. */
    double *x    = (double*)PyArray_DATA(x_array);
    double *y    = (double*)PyArray_DATA(y_array);
    double *yerr = (double*)PyArray_DATA(yerr_array);

    /* Call the external C function to compute the chi-squared. */
    double value = loglike(N, x, y, yerr);

    /* Clean up. */
    Py_DECREF(x_array);
    Py_DECREF(y_array);
    Py_DECREF(yerr_array);

    if (value < 0.0) {
        PyErr_SetString(PyExc_RuntimeError,
                    "Chi-squared returned an impossible value.");
        return NULL;
    }

    /* Build the output tuple */
    PyObject *ret = Py_BuildValue("d", value);
    return ret;
}