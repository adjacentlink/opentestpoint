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
#include "otestpoint/toolkit/raiipython.h"
#include "otestpoint/toolkit/exception.h"
#include "otestpoint/toolkit/pythonutils.h"
#include "pythonprobeadapter.h"

OpenTestPoint::PythonProbeAdapter::PythonProbeAdapter(ProbeIndex probeIndex,
                                                      const std::string & sModule,
                                                      const std::string & sClass,
                                                      ProbeService * pProbeService):
  ProbePlugin{probeIndex}
{
  // new reference
  pModule_.reset(PyImport_ImportModuleNoBlock(sModule.c_str()));

  if(!pModule_)
    {
      throw Toolkit::Exception{"Unable to load module: %s",sModule.c_str()};
    }

  // borrowed reference
  PyObject * pTestPointModuleDict =
    PyModule_GetDict(pModule_.get());

  if(!pTestPointModuleDict)
    {
      throw Toolkit::Exception{"Unable to locate module dictionary: %s",sModule.c_str()};
    }

  // borrowed reference
  PyObject * pProbeClass =
    PyDict_GetItemString(pTestPointModuleDict, sClass.c_str());

  if(!pProbeClass)
    {
      throw Toolkit::Exception{"Unable to locate probe class %s in module %s",
                                 sClass.c_str(),
                                 sModule.c_str()};
    }

  // new reference
  pProbe_.reset(PyObject_CallObject(pProbeClass,nullptr));

  if(!pProbe_)
    {
      throw Toolkit::Exception{"Unable create an instance of %s",sClass.c_str()};
    }

  // new refernce
  Toolkit::RAIIPyObject pToolkitLoggerModule{PyImport_ImportModuleNoBlock("otestpoint.toolkit.logger")};

  if(!pToolkitLoggerModule)
    {
      throw Toolkit::Exception{"Unable to load module: otestpoint.toolkit.logger"};
    }

  // borrowed reference
  PyObject * pToolkitLoggerModuleDict =
    PyModule_GetDict(pToolkitLoggerModule.get());

  if(!pToolkitLoggerModuleDict)
    {
      throw Toolkit::Exception{"Unable to locate module dictionary: otestpoint.toolkit.logger"};
    }

  // borrowed reference
  PyObject * pLoggerClass =
    PyDict_GetItemString(pToolkitLoggerModuleDict, "Logger");

  if(!pLoggerClass)
    {
      throw Toolkit::Exception{"Unable to locate Logger class in module otestpoint.toolkit.logger"};
    }

  PyObject * pArgs = Py_BuildValue("(O)",
                                   reinterpret_cast<PyObject *>(pProbeService->logClient()));

  // new reference
  Toolkit::RAIIPyObject pLogger =
    PyObject_CallObject(pLoggerClass,pArgs);

  Py_DECREF(pArgs);

  if(!pLogger)
    {
      throw Toolkit::Exception{"Unable to create instance of Logger"};
    }


  PyObject_SetAttrString(pProbe_.get(),"_logger", pLogger.release());

}

OpenTestPoint::PythonProbeAdapter::~PythonProbeAdapter()
{}


OpenTestPoint::ProbeNames
OpenTestPoint::PythonProbeAdapter::initialize(const std::string & sConfigurationFile)
{
  ProbeNames probeNames{};

  Toolkit::RAIIPyObject pReturn{};

  if(sConfigurationFile.empty())
    {
      // new object
      pReturn.reset(PyObject_CallMethod(pProbe_.get(),
                                        const_cast<char *>("initialize"),
                                        nullptr));
    }
  else
    {
      // new object
      Toolkit::RAIIPyObject pConfiguration{PyString_FromString(sConfigurationFile.c_str())};

      // new object
      pReturn.reset(PyObject_CallMethod(pProbe_.get(),
                                        const_cast<char *>("initialize"),
                                        const_cast<char *>("N"),
                                        pConfiguration.release()));
    }

  if(!pReturn)
    {
      throw Toolkit::PythonUtils::makeExceptionFromTrace();
    }

  Toolkit::RAIIPyObject pProbeNamesTuple{};

  if(PyList_Check(pReturn.get()))
    {
      pProbeNamesTuple.reset(PyList_AsTuple(pReturn.get()));
    }
  else if(PyTuple_Check(pReturn.get()))
    {
      pProbeNamesTuple.swap(pReturn);
    }
  else
    {
      throw Toolkit::Exception("invalid initialize method return format");
    }

  Py_ssize_t items{PyTuple_Size(pProbeNamesTuple.get())};

  for(Py_ssize_t i = 0; i < items; ++i)
    {
      // borrowed reference
      PyObject * pItem{PyTuple_GetItem(pProbeNamesTuple.get(), i)};

      if(PyString_Check(pItem))
        {
          probeNames.push_back(PyCompat_PyString_AsString(pItem));
        }
      else
        {
          throw Toolkit::Exception("invalid probe name must be a string");
        }
    }

  return probeNames;
}

void OpenTestPoint::PythonProbeAdapter::start()
{
  // new object
  Toolkit::RAIIPyObject pReturn{PyObject_CallMethod(pProbe_.get(),const_cast<char *>("start"),nullptr)};

  if(!pReturn)
    {
      throw Toolkit::PythonUtils::makeExceptionFromTrace();
    }
}

void OpenTestPoint:: PythonProbeAdapter::stop()
{
  // new object
  Toolkit::RAIIPyObject pReturn{PyObject_CallMethod(pProbe_.get(),const_cast<char *>("stop"),nullptr)};

  if(!pReturn)
    {
      throw Toolkit::PythonUtils::makeExceptionFromTrace();
    }
}

void OpenTestPoint::PythonProbeAdapter::destroy()
{
  // new object
  Toolkit::RAIIPyObject pReturn{PyObject_CallMethod(pProbe_.get(),const_cast<char *>("destroy"),nullptr)};

  if(!pReturn)
    {
      throw Toolkit::PythonUtils::makeExceptionFromTrace();
    }
}

OpenTestPoint::ProbeData
OpenTestPoint::PythonProbeAdapter::probe()
{
  ProbeData probeData{};

  // new object
  Toolkit::RAIIPyObject pReturn{PyObject_CallMethod(pProbe_.get(),const_cast<char *>("probe"),nullptr)};

  if(!pReturn)
    {
      throw Toolkit::PythonUtils::makeExceptionFromTrace();
    }

  Toolkit::RAIIPyObject pProbeDataTuple{};

  if(PyList_Check(pReturn.get()))
    {
      pProbeDataTuple.reset(PyList_AsTuple(pReturn.get()));
    }
  else if(PyTuple_Check(pReturn.get()))
    {
      pProbeDataTuple.swap(pReturn);
    }
  else
    {
      throw Toolkit::Exception("invalid probe method return format");
    }

  Py_ssize_t items{PyTuple_Size(pProbeDataTuple.get())};

  for(Py_ssize_t i = 0; i < items; ++i)
    {
      // borrowed reference
      PyObject * pItem{PyTuple_GetItem(pProbeDataTuple.get(), i)};

      if(!PyTuple_Check(pItem))
        {
          throw Toolkit::Exception("invalid probe return entry must be a tuple");
        }

      char * pzProbeName{};
      char * pzProbeData{};
      char * pzMessageName{};
      char * pzMessageModule{};
      int iProbeNameSize{};
      int iProbeDataSize{};
      int iMessageNameSize{};
      int iMessageModuleSize{};
      std::uint32_t u32Version{};

      if(!(PyArg_ParseTuple(pItem,
                            "s#s#s#s#I",
                            &pzProbeName,
                            &iProbeNameSize,
                            &pzProbeData,
                            &iProbeDataSize,
                            &pzMessageName,
                            &iMessageNameSize,
                            &pzMessageModule,
                            &iMessageModuleSize,
                            &u32Version)))
        {
          throw Toolkit::Exception("invalid probe return entry must be a tuple of 2 strings and int");
        }

      probeData.push_back(std::make_tuple(std::string(pzProbeName,iProbeNameSize),
                                          std::string(pzProbeData,iProbeDataSize),
                                          std::string(pzMessageName,iMessageNameSize),
                                          std::string(pzMessageModule,iMessageModuleSize),
                                          u32Version));
    }


  return probeData;
}
