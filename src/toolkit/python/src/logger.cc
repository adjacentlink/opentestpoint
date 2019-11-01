/*
 * Copyright (c) 2014-2016,2019 - Adjacent Link LLC, Bridgewater,
 * New Jersey
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

#include "otestpoint/toolkit/pycompat.h"
#include "otestpoint/toolkit/log/client.h"
#include "otestpoint/toolkit/log/clientbuilder.h"
#include "otestpoint/toolkit/exception.h"
#include <cstdlib>
#include <cstring>

typedef struct {
  PyObject_HEAD
  /* Type-specific fields go here. */
  OpenTestPoint::Toolkit::Log::Client * pLogClient;
} Logger;


/* module functions */
static void
Logger_dealloc(Logger * self)
{
  delete self->pLogClient;
  PyCompat_Py_TYPE_Free(self);
}

static PyObject *
Logger_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  Logger * self{};

  self = reinterpret_cast<Logger *>(type->tp_alloc(type, 0));

  if(self != nullptr)
    {
      if(kwds != nullptr)
        {
          OpenTestPoint::Toolkit::Log::ClientBuilder builder{};

          // borrowed reference
          PyObject * pLabel{PyDict_GetItemString(kwds,"label")};

          if(!pLabel)
            {
              PyErr_SetString(PyExc_RuntimeError,"missing required keyword: label");
              Py_DECREF(self);
              return nullptr;
            }

          try
            {
              self->pLogClient = builder.buildClient(PyCompat_PyString_AsString(pLabel));
            }
          catch(OpenTestPoint::Toolkit::Exception & exp)
            {
              PyErr_SetString(PyExc_RuntimeError,exp.what());
              Py_DECREF(self);
              return nullptr;
            }
        }
      else
        {
          PyObject * pLogger{};

          if(!PyArg_ParseTuple(args,"O",&pLogger))
            {
              PyErr_SetString(PyExc_RuntimeError,"Missing Toolkit::LogClient instance");
              Py_DECREF(self);
              return nullptr;
            }

          self->pLogClient = reinterpret_cast<OpenTestPoint::Toolkit::Log::Client *>(pLogger);
        }
    }

  return reinterpret_cast<PyObject *>(self);
}


PyDoc_STRVAR(Logger_log_doc,
             "log(level,message)\n\n"
             "Publish log message at a specified level.\n\n"
             "level     - Log level of message\n\n"
             "message   - Message string to publish.\n\n"
             "Log message strings that contain a / as the first\n"
             "charater in the first word will be appended to the log\n"
             "topic."
             );

static PyObject * Logger_log(PyObject * self, PyObject * args)
{
  Logger * pLogger{reinterpret_cast<Logger *>(self)};

  PyObject * pReturn{};

  std::string sError{};

  int iLevel{};

  const char * pzMessage{};

  if(!PyArg_ParseTuple(args,"is",&iLevel,&pzMessage))
    {
      return nullptr;
    }

  if(iLevel < 0 || iLevel > 4)
    {
      PyErr_SetString(PyExc_ValueError,"invalid log level");
      return nullptr;
    }

  Py_BEGIN_ALLOW_THREADS;

  try
    {
      pLogger->pLogClient->log(static_cast<OpenTestPoint::Toolkit::Log::Level>(iLevel),
                               "%s",
                               pzMessage);
    }
  catch(OpenTestPoint::Toolkit::Exception & exp)
    {
      sError = exp.what();
    }

  Py_END_ALLOW_THREADS;

  if(sError.empty())
    {
      Py_INCREF(Py_None);

      pReturn = Py_None;
    }
  else
    {
      PyErr_SetString(PyExc_RuntimeError,sError.c_str());
    }

  return pReturn;
}


static PyMethodDef Logger_methods[] =
  {
   {
    "log",
    (PyCFunction)Logger_log,
    METH_VARARGS,
    Logger_log_doc,
   },
   {nullptr,nullptr,0,nullptr}
  };

PyDoc_STRVAR(Logger_type_doc,
             "OpenTestPoint Toolkit Log Client extension\n\n"
             "SYNOPSIS\n\n"
             "import opentestpoint.toolkit.logger as Logger\n\n"
             "logger = Logger(label='probe',\n"
             "                control='tcp://127.0.0.1:9901',\n"
             "                publish='tcp://127.0.0.1:9902')\n\n"
             "logger.log(Logger.DEBUG_LEVEL,'/TimeOfDay start')\n\n"
             );

static PyTypeObject LoggerType =
  {
   PyCompat_PyTypeObject_HEAD_INIT("Logger")
   sizeof(Logger),                   /*tp_basicsize*/
   0,                                /*tp_itemsize*/
   (destructor)Logger_dealloc,       /*tp_dealloc*/
   0,                                /*tp_print*/
   0,                                /*tp_getattr*/
   0,                                /*tp_setattr*/
   0,                                /*tp_compare*/
   0,                                /*tp_repr*/
   0,                                /*tp_as_number*/
   0,                                /*tp_as_sequence*/
   0,                                /*tp_as_mapping*/
   0,                                /*tp_hash */
   0,                                /*tp_call*/
   0,                                /*tp_str*/
   0,                                /*tp_getattro*/
   0,                                /*tp_setattro*/
   0,                                /*tp_as_buffer*/
   Py_TPFLAGS_DEFAULT,               /*tp_flags*/
   Logger_type_doc,                  /*tp_doc*/
   0,                                /*tp_traverse*/
   0,                                /*tp_clear*/
   0,                                /*tp_richcompare*/
   0,                                /*tp_weaklistoffset*/
   0,                                /*tp_iter*/
   0,                                /*tp_iternext*/
   Logger_methods,                   /*tp_methods*/
   0,                                /*tp_members*/
   0,                                /*tp_getset*/
   0,                                /*tp_base*/
   0,                                /*tp_dict*/
   0,                                /*tp_descr_get*/
   0,                                /*tp_descr_set*/
   0,                                /*tp_dictoffset*/
   0,                                /*tp_init*/
   0,                                /*tp_alloc*/
   Logger_new,                       /*tp_new*/
  };



#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

PyDoc_STRVAR(Logger_module_doc,
             "OpenTestPoint Toolkit Logger allows multiple clients to send log\n"
             " messages via TCP connections to a single server.\n\n"
             );

#if PY_MAJOR_VERSION > 2

static PyModuleDef loggermodule =
  {
   PyModuleDef_HEAD_INIT,
   "otestpoint.toolkit.logger",
   Logger_module_doc,
   -1,
   nullptr,
   nullptr,
   nullptr,
   nullptr,
   nullptr
  };

PyMODINIT_FUNC
PyInit_logger(void)
{
  PyObject * m{};

  if(PyType_Ready(&LoggerType) < 0)
    {
      return nullptr;
    }

  m = PyModule_Create(&loggermodule);

  if(m == nullptr)
    {
      return nullptr;
    }

  PyModule_AddIntConstant(m,
                          "NOLOG_LEVEL",
                          static_cast<int>(OpenTestPoint::Toolkit::Log::Level::NOLOG_LEVEL));

  PyModule_AddIntConstant(m,
                          "ABORT_LEVEL",
                          static_cast<int>(OpenTestPoint::Toolkit::Log::Level::ABORT_LEVEL));

  PyModule_AddIntConstant(m,
                          "ERROR_LEVEL",
                          static_cast<int>(OpenTestPoint::Toolkit::Log::Level::ERROR_LEVEL));

  PyModule_AddIntConstant(m,
                          "INFO_LEVEL",
                          static_cast<int>(OpenTestPoint::Toolkit::Log::Level::INFO_LEVEL));

  PyModule_AddIntConstant(m,
                          "DEBUG_LEVEL",
                          static_cast<int>(OpenTestPoint::Toolkit::Log::Level::DEBUG_LEVEL));

  Py_INCREF(&LoggerType);

  PyModule_AddObject(m,"Logger",(PyObject *)&LoggerType);

  PyEval_InitThreads();

  return m;
}

#else

static struct PyMethodDef Logger_module_methods [] =
  {
   {nullptr}
  };

PyMODINIT_FUNC
initlogger()
{
  PyObject * m{};

  if(PyType_Ready(&LoggerType) < 0)
    {
      return;
    }

  if((m = Py_InitModule3("otestpoint.toolkit.logger",
                         Logger_module_methods,
                         Logger_module_doc
                         )) == nullptr)
    {
      return;
    }

  PyModule_AddIntConstant(m,
                          "NOLOG_LEVEL",
                          static_cast<int>(OpenTestPoint::Toolkit::Log::Level::NOLOG_LEVEL));

  PyModule_AddIntConstant(m,
                          "ABORT_LEVEL",
                          static_cast<int>(OpenTestPoint::Toolkit::Log::Level::ABORT_LEVEL));

  PyModule_AddIntConstant(m,
                          "ERROR_LEVEL",
                          static_cast<int>(OpenTestPoint::Toolkit::Log::Level::ERROR_LEVEL));

  PyModule_AddIntConstant(m,
                          "INFO_LEVEL",
                          static_cast<int>(OpenTestPoint::Toolkit::Log::Level::INFO_LEVEL));


  PyModule_AddIntConstant(m,
                          "DEBUG_LEVEL",
                          static_cast<int>(OpenTestPoint::Toolkit::Log::Level::DEBUG_LEVEL));

  Py_INCREF(&LoggerType);

  PyModule_AddObject(m,"Logger",(PyObject *)&LoggerType);

  PyEval_InitThreads();
}

#endif
