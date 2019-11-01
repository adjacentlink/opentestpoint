/*
 * Copyright (c) 2018,2019 - Adjacent Link LLC, Bridgewater, New Jersey
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Adjacent Link LLC nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * See toplevel COPYING for more information.
 */

#ifndef PYTHONCOMPAT_HEADER_
#define PYTHONCOMPAT_HEADER_

#include <Python.h>
#include <structmember.h>
#include <string>

#if PY_MAJOR_VERSION > 2
#define PyInt_Check PyLong_Check
#define PyInt_AsUnsignedLongLongMask PyLong_AsUnsignedLongLongMask
#define PyInt_FromLong PyLong_FromLong

#define PyString_Check PyUnicode_Check
#define PyString_FromString PyUnicode_FromString
#define PyString_FromFormat PyUnicode_FromFormat

#define PyCompat_Py_TYPE_Free(self)                                     \
  reinterpret_cast<PyObject*>(self)->ob_type->tp_free(reinterpret_cast<PyObject*>(self))

#define PyCompat_PyTypeObject_HEAD_INIT(name)   \
  PyVarObject_HEAD_INIT(NULL, 0)                \
  name,

inline std::string PyCompat_PyString_AsString(PyObject * pString)
{
  PyObject * pTemp{PyUnicode_AsEncodedString(pString,
                                             "UTF-8",
                                             "strict")};
  std::string s{PyBytes_AS_STRING(pTemp)};
  Py_DECREF(pTemp);
  return s;
}

#else

#define PyCompat_Py_TYPE_Free(self)                             \
  self->ob_type->tp_free(reinterpret_cast<PyObject*>(self))

#define PyCompat_PyTypeObject_HEAD_INIT(name)   \
  PyObject_HEAD_INIT(NULL)                      \
  0,                                            \
    name,

inline std::string PyCompat_PyString_AsString(PyObject * pString)
{
  return PyString_AsString(pString);
}

#endif

#endif //PYTHONCOMPAT_HEADER_
