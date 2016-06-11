/*
 * Copyright (c) 2014-2016 - Adjacent Link LLC, Bridgewater, New Jersey
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

#include "probecontainer.h"
#include "otestpoint/toolkit/exception.h"
#include "otestpoint/toolkit/transaction.h"
#include "otestpoint/toolkit/servicesingleton.h"

#include <memory>
#include <zmq.h>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>

#include "libotestpoint.pb.h"


#ifndef HAVE_SECURE_GETENV
#  ifdef HAVE___SECURE_GETENV
#    define secure_getenv __secure_getenv
#  else
#    error neither secure_getenv nor __secure_getenv is available
#  endif
#endif


OpenTestPoint::ProbeContainer::ProbeContainer(const uuid_t & uuid,
                                              const std::string & sNodeId,
                                              ProbeIndex probeIndex,
                                              const std::string & sPlugin,
                                              const std::chrono::seconds & probeRate,
                                              const std::chrono::seconds & commTimeout):
  Probe{sNodeId,
    probeIndex},
  pid_{},
  commTimeout_{commTimeout},
  bFailure_{}
{
  init(uuid,
       sNodeId,
       probeIndex,
       probeRate);

  OPENTESTPOINT_TOOLKIT_LOG_FN_DEBUG(logIdentifierCallable_,
                                     "creating probe plugin %s rate: %zd threshold: %zd",
                                     sPlugin.c_str(),
                                     probeRate.count(),
                                     commTimeout_.count());

  try
    {
      if(!Toolkit::transaction<OpenTestPoint::ProbeRequest,
         OpenTestPoint::ProbeResponse>
         (pClient_.get(),
          OpenTestPoint::ProbeRequest::TYPE_CREATE,
          commTimeout_,
          [sPlugin,probeIndex](OpenTestPoint::ProbeRequest & request)
          {
            auto pCreate = request.mutable_create();

            pCreate->set_type(OpenTestPoint::ProbeRequest::Create::TYPE_PLUGIN);

            auto pPlugin = pCreate->mutable_plugin();

            pPlugin->set_name(sPlugin.c_str());
          }))
        {
          OPENTESTPOINT_TOOLKIT_LOG_FN_ERROR(logIdentifierCallable_,
                                             "communication timeout while creating"
                                             " probe plugin %s",
                                             sPlugin.c_str());

          bFailure_ = true;
        }
    }
  catch(Toolkit::Exception & exp)
    {
      OPENTESTPOINT_TOOLKIT_LOG_FN_ERROR(logIdentifierCallable_,
                                         "%s",
                                         exp.what());

      bFailure_ = true;
    }
}

OpenTestPoint::ProbeContainer::ProbeContainer(const uuid_t & uuid,
                                              const std::string & sNodeId,
                                              ProbeIndex probeIndex,
                                              const std::string & sPythonModule,
                                              const std::string & sPythonClass,
                                              const std::chrono::seconds & probeRate,
                                              const std::chrono::seconds & commTimeout):
  Probe{sNodeId,
    probeIndex},
  pid_{},
  commTimeout_{commTimeout},
  bFailure_{}
{
  init(uuid,
       sNodeId,
       probeIndex,
       probeRate);

  OPENTESTPOINT_TOOLKIT_LOG_FN_DEBUG(logIdentifierCallable_,
                                     "creating python probe %s.%s rate: %zd threshold: %zd",
                                     sPythonModule.c_str(),
                                     sPythonClass.c_str(),
                                     probeRate.count(),
                                     commTimeout_.count());
  try
    {
      if(!Toolkit::transaction<OpenTestPoint::ProbeRequest,
         OpenTestPoint::ProbeResponse>
         (pClient_.get(),
          OpenTestPoint::ProbeRequest::TYPE_CREATE,
          commTimeout_,
          [sPythonModule,sPythonClass,probeIndex](OpenTestPoint::ProbeRequest & request)
          {
            auto pCreate = request.mutable_create();

            pCreate->set_type(OpenTestPoint::ProbeRequest::Create::TYPE_PYTHON);

            auto pPython = pCreate->mutable_python();

            pPython->set_module(sPythonModule);

            pPython->set_class_(sPythonClass);
          }))
        {
          OPENTESTPOINT_TOOLKIT_LOG_FN_ERROR(logIdentifierCallable_,
                                             "communication timeout while creating"
                                             " python probe %s.%s",
                                             sPythonModule.c_str(),
                                             sPythonClass.c_str());

          bFailure_ = true;
        }
    }
  catch(Toolkit::Exception & exp)
    {
      OPENTESTPOINT_TOOLKIT_LOG_FN_ERROR(logIdentifierCallable_,
                                         "%s",
                                         exp.what());

      bFailure_ = true;
    }
}


void OpenTestPoint::ProbeContainer::init(const uuid_t & uuid,
                                         const std::string & sNodeId,
                                         ProbeIndex probeIndex,
                                         const std::chrono::seconds & probeRate)

{
  uuid_copy(uuid_,uuid);

  logIdentifierCallable_ =
    [sNodeId,probeIndex]()->std::list<std::string>
    {
      std::stringstream ssLabel{};
      ssLabel<<'/'<<sNodeId<<'/'<<probeIndex<<"/container";
      return {ssLabel.str()};
    };

  pContext_.reset(zmq_ctx_new());

  if(!pContext_)
    {
      throw Toolkit::Exception{"Error creating new messaging context: %s",
          zmq_strerror(errno)};
    }

  Toolkit::RAIIZMQSocket pStatusSocket{zmq_socket(pContext_.get(),ZMQ_REP)};

  if(!pStatusSocket)
    {
      throw Toolkit::Exception{"unable to create status socket: %s ",
          zmq_strerror(errno)};
    }

  if(zmq_bind(pStatusSocket.get(),"tcp://127.0.0.1:*") < 0)
    {
      throw Toolkit::Exception{"unable to bind status socket: %s ",
          zmq_strerror(errno)};
    }

  char buf[1024];
  size_t len{sizeof(buf)};

  if(zmq_getsockopt(pStatusSocket.get(),ZMQ_LAST_ENDPOINT,buf,&len))
    {
      throw Toolkit::Exception{"unable to determine status socket endpoint : %s",
          zmq_strerror(errno)};
    }

  std::string sPublishEndpoint{buf};

  pid_ = fork();

  switch(pid_)
    {
    case 0:
      // child
      {
        std::string sPythonPathEnv{"PYTHONPATH="};

        const char * pzPYTHON_PATH=secure_getenv("PYTHONPATH");

        std::string sLDLibraryPathEnv{"LD_LIBRARY_PATH="};

        const char * pzLD_LIBRARY_PATH=secure_getenv("LD_LIBRARY_PATH");

        std::string sNodeIdEnv{"nodeid="};
        sNodeIdEnv.append(sNodeId);

        std::string sProbeIndexEnv{"probeindex="};
        sProbeIndexEnv.append(std::to_string(probeIndex));

        std::string sProbeRateEnv{"proberate="};
        sProbeRateEnv.append(std::to_string(probeRate.count()));

        std::string sStatusEnv{"status="};
        sStatusEnv.append(buf);

        char buf[37]; // 36-byte string (plus tailing '\0')
        uuid_unparse(uuid,buf);
        std::string sUUIDEnv{"uuid="};
        sUUIDEnv.append(buf);

        if(pzPYTHON_PATH)
          {
            sPythonPathEnv.append(pzPYTHON_PATH);
          }

        if(pzLD_LIBRARY_PATH)
          {
            sLDLibraryPathEnv.append(pzLD_LIBRARY_PATH);
          }

        const char * const argv[] = {"otestpoint-probe",0};

        const char * const envp[] =
          {
            sNodeIdEnv.c_str(),
            sPythonPathEnv.c_str(),
            sLDLibraryPathEnv.c_str(),
            sProbeIndexEnv.c_str(),
            sProbeRateEnv.c_str(),
            sUUIDEnv.c_str(),
            sStatusEnv.c_str(),
            0};

        if(execvpe("otestpoint-probe",
                   const_cast<char **>(argv),
                   const_cast<char **>(&envp[0])) < 0)
          {
            std::cerr<<"unable to start probe: "<<strerror(errno)<<std::endl;
          }

        exit(1);

      }
      break;

    case -1:
      // error
      throw Toolkit::Exception{"unable to start probe: %s ",strerror(errno)};
      break;

    default:
      //parent
      zmq_msg_t message;

      zmq_msg_init(&message);

      zmq_msg_recv(&message,pStatusSocket.get(),0);

      OpenTestPoint::ProbeStatusReport report{};

      if(!report.ParseFromArray(zmq_msg_data(&message),zmq_msg_size(&message)))
        {
          zmq_msg_close(&message);
          throw Toolkit::Exception{"unable to deserialize transaction"};
        }

      zmq_msg_close(&message);

      if(report.type() == OpenTestPoint::ProbeStatusReport::TYPE_READY)
        {
          sLogControlEndpoint_ = report.ready().logcontrol();
          sLogPublishEndpoint_ = report.ready().logpublish();

          sProbeControlEndpoint_ = report.ready().probecontrol();
          sProbePublishEndpoint_ = report.ready().probepublish();

          if(!(pClient_ = zmq_socket(pContext_.get(),ZMQ_REQ)))
            {
              throw Toolkit::Exception{"unable to create probe endpoint: %s ",
                  zmq_strerror(errno)};
            }

          if(zmq_connect(pClient_.get(),sProbeControlEndpoint_.c_str()) < 0)
            {
              throw Toolkit::Exception{"unable to connect to probe endpoint: %s ",
                  zmq_strerror(errno)};
            }
        }

      OpenTestPoint::ProbeStatusResponse response{};

      response.set_type(OpenTestPoint::ProbeStatusResponse::TYPE_SUCCESS);

      std::string sSerialization{};

      response.SerializeToString(&sSerialization);

      zmq_send(pStatusSocket.get(),sSerialization.c_str(),sSerialization.size(),0);

      break;
    }
}

OpenTestPoint::ProbeContainer::~ProbeContainer()
{
  if(pid_)
    {
      OPENTESTPOINT_TOOLKIT_LOG_FN_ERROR(logIdentifierCallable_,
                                         "sending kill to %d",pid_);

      kill(pid_,SIGTERM);
    }
}

OpenTestPoint::ProbeNames
OpenTestPoint::ProbeContainer::initialize(const std::string & sConfigurationFile)
{
  ProbeNames probeNames;

  if(!bFailure_)
    {
      OPENTESTPOINT_TOOLKIT_LOG_FN_DEBUG(logIdentifierCallable_,"sending initialize");

      try
        {
          if(!Toolkit::transaction<OpenTestPoint::ProbeRequest,
             OpenTestPoint::ProbeResponse>
             (pClient_.get(),
              OpenTestPoint::ProbeRequest::TYPE_INITIALIZE,
              commTimeout_,
              [sConfigurationFile](OpenTestPoint::ProbeRequest & request)
              {
                auto pInitialize = request.mutable_initialize();

                pInitialize->set_configuration(sConfigurationFile);
              },
              [&probeNames](OpenTestPoint::ProbeResponse & response)
              {
                const auto & initialize = response.initialize();

                for(int i = 0; i < initialize.names_size(); ++i)
                  {
                    probeNames.push_back(initialize.names(i));
                  }
              }))
            {
              OPENTESTPOINT_TOOLKIT_LOG_FN_ERROR(logIdentifierCallable_,
                                                 "initialize communication timeout");

              bFailure_ = true;
            }
        }
      catch(Toolkit::Exception & exp)
        {
          OPENTESTPOINT_TOOLKIT_LOG_FN_ERROR(logIdentifierCallable_,
                                             "%s",
                                             exp.what());

          bFailure_ = true;
        }
    }

  return probeNames;
}


void OpenTestPoint::ProbeContainer::start()
{
  if(!bFailure_)
    {
      OPENTESTPOINT_TOOLKIT_LOG_FN_DEBUG(logIdentifierCallable_,"sending start");

      try
        {
          if(!Toolkit::transaction<OpenTestPoint::ProbeRequest,
             OpenTestPoint::ProbeResponse>
             (pClient_.get(),
              OpenTestPoint::ProbeRequest::TYPE_START,
              commTimeout_))
            {
              OPENTESTPOINT_TOOLKIT_LOG_FN_ERROR(logIdentifierCallable_,
                                                 "start communication timeout");

              bFailure_ = true;
            }
        }
      catch(Toolkit::Exception & exp)
        {
          OPENTESTPOINT_TOOLKIT_LOG_FN_ERROR(logIdentifierCallable_,
                                             "%s",
                                             exp.what());

          bFailure_ = true;
        }
    }
}

void OpenTestPoint::ProbeContainer::stop()
{
  if(!bFailure_)
    {
      OPENTESTPOINT_TOOLKIT_LOG_FN_DEBUG(logIdentifierCallable_,"sending stop");

      try
        {
          if(!Toolkit::transaction<OpenTestPoint::ProbeRequest,
             OpenTestPoint::ProbeResponse>
             (pClient_.get(),
              OpenTestPoint::ProbeRequest::TYPE_STOP,
              commTimeout_))
            {
              OPENTESTPOINT_TOOLKIT_LOG_FN_ERROR(logIdentifierCallable_,
                                                 "stop communication timeout");

              bFailure_ = true;
            }
        }
      catch(Toolkit::Exception & exp)
        {
          OPENTESTPOINT_TOOLKIT_LOG_FN_ERROR(logIdentifierCallable_,
                                             "%s",
                                             exp.what());

          bFailure_ = true;
        }
    }
}

void OpenTestPoint::ProbeContainer::destroy()
{
  OPENTESTPOINT_TOOLKIT_LOG_FN_DEBUG(logIdentifierCallable_,"sending destroy");

  try
    {
      if(!Toolkit::transaction<OpenTestPoint::ProbeRequest,
         OpenTestPoint::ProbeResponse>
         (pClient_.get(),
          OpenTestPoint::ProbeRequest::TYPE_DESTROY,
          commTimeout_))
        {
          OPENTESTPOINT_TOOLKIT_LOG_FN_ERROR(logIdentifierCallable_,
                                             "destroy communication timeout");

          bFailure_ = true;
        }
      else
        {
          pid_ = 0;
        }
    }
  catch(Toolkit::Exception & exp)
    {
      OPENTESTPOINT_TOOLKIT_LOG_FN_ERROR(logIdentifierCallable_,
                                         "%s",
                                         exp.what());

      bFailure_ = true;
    }
}
