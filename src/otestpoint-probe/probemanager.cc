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
#include <Python.h>
#include "pythonprobeadapter.h"
#include "probemanager.h"
#include "pluginprobeadapter.h"
#include "libotestpoint.pb.h"
#include "probereport.pb.h"
#include "probeserviceimpl.h"

#include "otestpoint/toolkit/exception.h"
#include "otestpoint/toolkit/transaction.h"
#include "otestpoint/toolkit/log/clientbuilder.h"

#include <zmq.h>
#include <cstring>
#include <sys/timerfd.h>
#include <chrono>
#include <sstream>

std::int64_t scheduleNextProbe(int iFd,int iDelta);

OpenTestPoint::ProbeManager::ProbeManager(const std::string & sStatusEndpoint,
                                          const std::string & sNodeId,
                                          ProbeIndex probeIndex,
                                          const uuid_t & uuid,
                                          std::uint16_t u16ProbeRate):
  sNodeId_{sNodeId},
  probeIndex_{probeIndex},
  u16ProbeRate_{u16ProbeRate},
  pContext_{},
  pServer_{}
{
  uuid_copy(uuid_,uuid);

  Toolkit::Log::ClientBuilder logClientBuilder{};

  std::stringstream ssLabel{};
  ssLabel<<"otestpoint-probe/"<<sNodeId<<"/"<<probeIndex;

  auto pLogClient = logClientBuilder.buildClient(ssLabel.str());

  pProbeService_.reset(new ProbeServiceImpl{pLogClient});

  pContext_ = zmq_ctx_new();

  if(!pContext_)
    {
      throw Toolkit::Exception{"Error creating new messaging context: %s",
          zmq_strerror(errno)};
    }

  pServer_ = zmq_socket(pContext_,ZMQ_REP);

  if(!pServer_)
    {
      zmq_ctx_destroy(pContext_);
      throw Toolkit::Exception{"Error creating messaging socket: %s",
          zmq_strerror(errno)};
    }

  if(zmq_bind(pServer_,"tcp://127.0.0.1:*") < 0)
    {
      zmq_close(pServer_);
      zmq_ctx_destroy(pContext_);
      throw Toolkit::Exception{"Error binding messaging socket: %s",
          zmq_strerror(errno)};
    }

  char buf[1024];
  size_t len{sizeof(buf)};

  if(zmq_getsockopt(pServer_,ZMQ_LAST_ENDPOINT,buf,&len))
    {
      throw Toolkit::Exception{"unable to determine last pub endpoint : %s",
          zmq_strerror(errno)};
    }

  std::string sProbeControlEndpoint{buf};

  pPublisher_ = zmq_socket(pContext_,ZMQ_PUB);

  if(!pPublisher_)
    {
      zmq_close(pServer_);
      zmq_ctx_destroy(pContext_);
      throw Toolkit::Exception{"Error creating publish socket: %s",
          zmq_strerror(errno)};
    }

  if(zmq_bind(pPublisher_,"tcp://127.0.0.1:*") < 0)
    {
      zmq_close(pPublisher_);
      zmq_close(pServer_);
      zmq_ctx_destroy(pContext_);
      throw Toolkit::Exception{"Error binding publish socket: %s",
          zmq_strerror(errno)};
    }

  if(zmq_getsockopt(pPublisher_,ZMQ_LAST_ENDPOINT,buf,&len))
    {
      throw Toolkit::Exception{"unable to determine last pub endpoint: %s",
          zmq_strerror(errno)};
    }

  std::string sProbePublishEndpoint{buf};

  void * pStatusSocket{};

  if((pStatusSocket = zmq_socket(pContext_,ZMQ_REQ)) == nullptr)
    {
      throw Toolkit::Exception{"unable to open status socket: %s",
          zmq_strerror(errno)};
    }

  if(zmq_connect(pStatusSocket,sStatusEndpoint.c_str()) < 0)
    {
      zmq_close(pStatusSocket);

      throw Toolkit::Exception{"unable to connect to probe status endpoint: %s ",
          zmq_strerror(errno)};
    }

  if(!Toolkit::transaction<OpenTestPoint::ProbeStatusReport,
     OpenTestPoint::ProbeStatusResponse>
     (pStatusSocket,
      OpenTestPoint::ProbeStatusReport::TYPE_READY,
      std::chrono::seconds{5},
      [pLogClient,
       sProbeControlEndpoint,
       sProbePublishEndpoint](OpenTestPoint::ProbeStatusReport & report)
      {
        auto pReady = report.mutable_ready();

        pReady->set_logcontrol(pLogClient->getControlEndpoint());
        pReady->set_logpublish(pLogClient->getPublishEndpoint());
        pReady->set_probecontrol(sProbeControlEndpoint);
        pReady->set_probepublish(sProbePublishEndpoint);
      }))
    {
      throw Toolkit::Exception{"unable to singnal ready: %s ",
          zmq_strerror(errno)};
    }

  zmq_close(pStatusSocket);
}

OpenTestPoint::ProbeManager::~ProbeManager()
{
  zmq_close(pPublisher_);
  zmq_close(pServer_);
  zmq_ctx_destroy(pContext_);
}

void OpenTestPoint::ProbeManager::run()
{
  try
    {
      std::chrono::high_resolution_clock::time_point absTimeout{};

      int iFd{};
      std::int64_t i64Timestamp{};

      // create an interval timer with CLOCK_MONOTONIC
      if((iFd = timerfd_create(CLOCK_REALTIME,0)) < 0)
        {
          throw Toolkit::Exception{"unable to create timer"};
        }

      bool bRun{true};

      while(bRun)
        {
          zmq_pollitem_t items[] =
            {
              {pServer_,0,ZMQ_POLLIN,0},
              {nullptr,iFd,ZMQ_POLLIN,0},
            };

          int rc = zmq_poll(items, 2, -1);

          if(rc == -1)
            {
              continue;
            }

          if(items[0].revents & ZMQ_POLLIN)
            {
              zmq_msg_t message;

              zmq_msg_init(&message);

              int size = zmq_msg_recv(&message,pServer_, 0);

              OpenTestPoint::ProbeRequest request{};

              if(size > 0)
                {
                  if(!request.ParseFromArray(zmq_msg_data(&message),size))
                    {
                      Toolkit::sendFailureResponse<OpenTestPoint::ProbeResponse>(pServer_,
                                                                                 "unknown message");
                      break;
                    }
                }
              else
                {
                  zmq_msg_close(&message);
                  break;
                }

              zmq_msg_close(&message);

              switch(request.type())
                {
                case OpenTestPoint::ProbeRequest::TYPE_CREATE:

                  if(request.has_create())
                    {
                      auto & create = request.create();

                      switch(create.type())
                        {
                        case OpenTestPoint::ProbeRequest::Create::TYPE_PLUGIN:
                          if(create.has_plugin())
                            {
                              try
                                {
                                  OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pProbeService_,
                                                                       "/manager creating probe from plugin %s",
                                                                       create.plugin().name().c_str());

                                  pProbePlugin_.reset(new PluginProbeAdapter{probeIndex_,
                                        create.plugin().name(),
                                        pProbeService_.get()});

                                  pProbePlugin_->setProbeService(pProbeService_.get());

                                  Toolkit::sendSuccessResponse<OpenTestPoint::ProbeResponse>(pServer_);
                                }
                              catch(Toolkit::Exception & exp)
                                {
                                  Toolkit::sendFailureResponse<OpenTestPoint::ProbeResponse>(pServer_,
                                                                                             "%s",
                                                                                             exp.what());
                                  //throw;
                                }
                            }
                          else
                            {
                              Toolkit::sendFailureResponse<OpenTestPoint::ProbeResponse>(pServer_,
                                                                                         "malformed create message");

                              break;
                            }

                          break;

                        case  OpenTestPoint::ProbeRequest::Create::TYPE_PYTHON:

                          if(create.has_python())
                            {
                              try
                                {
                                  OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pProbeService_,
                                                                       "/manager creating probe from python class %s.%s",
                                                                       create.python().module().c_str(),
                                                                       create.python().class_().c_str());

                                  pProbePlugin_.reset(new PythonProbeAdapter{probeIndex_,
                                        create.python().module(),
                                        create.python().class_(),
                                        pProbeService_.get()});

                                  pProbePlugin_->setProbeService(pProbeService_.get());

                                  Toolkit::sendSuccessResponse<OpenTestPoint::ProbeResponse>(pServer_);
                                }
                              catch(Toolkit::Exception & exp)
                                {
                                  OPENTESTPOINT_PROBESERVICE_LOG_ABORT(pProbeService_,"/manager %s",exp.what());

                                  Toolkit::sendFailureResponse<OpenTestPoint::ProbeResponse>(pServer_,
                                                                                             "%s",
                                                                                             exp.what());
                                }
                            }
                          else
                            {
                              Toolkit::sendFailureResponse<OpenTestPoint::ProbeResponse>(pServer_,
                                                                                         "malformed create message");

                              break;
                            }
                          break;

                        default:
                          break;

                        }
                    }
                  break;

                case OpenTestPoint::ProbeRequest::TYPE_INITIALIZE:

                  OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pProbeService_,
                                                       "/manager initialize");

                  if(pProbePlugin_)
                    {
                      if(request.has_initialize())
                        {
                          auto & initialize = request.initialize();

                          ProbeNames names{};

                          try
                            {
                              if(initialize.has_configuration())
                                {
                                  names = pProbePlugin_->initialize(initialize.configuration());
                                }
                              else
                                {
                                  names = pProbePlugin_->initialize();
                                }

                              OPENTESTPOINT_PROBESERVICE_LOG_FN_INFO(pProbeService_,
                                                                     [names]()
                                                                     {
                                                                       std::list<std::string> output;

                                                                       for(const auto & name : names)
                                                                         {
                                                                           output.push_back(name);
                                                                         }

                                                                       return output;
                                                                     },
                                                                     "/manager available probes:");

                              OpenTestPoint::ProbeResponse response{};

                              response.set_type(OpenTestPoint::ProbeResponse::TYPE_INITIALIZE);

                              auto pInitialize = response.mutable_initialize();

                              for(const auto & name : names)
                                {
                                  pInitialize->add_names(name + "." + sNodeId_);
                                }

                              std::string sSerialization;

                              if(!response.SerializeToString(&sSerialization))
                                {
                                  throw Toolkit::Exception{"unable to serialize message"};
                                }

                              if(zmq_send(pServer_,sSerialization.c_str(),sSerialization.size(),0) < 0)
                                {
                                  throw Toolkit::Exception{"unable to send message: %s",zmq_strerror(errno)};
                                }

                            }
                          catch(Toolkit::Exception & exp)
                            {
                              Toolkit::sendFailureResponse<OpenTestPoint::ProbeResponse>(pServer_,
                                                                                         "%s",
                                                                                         exp.what());
                            }
                        }
                      else
                        {
                          Toolkit::sendFailureResponse<OpenTestPoint::ProbeResponse>(pServer_,
                                                                                     "malformed create message");
                        }
                    }
                  else
                    {
                      Toolkit::sendFailureResponse<OpenTestPoint::ProbeResponse>(pServer_,
                                                                                 "probe not created");
                    }
                  break;

                case OpenTestPoint::ProbeRequest::TYPE_START:

                  OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pProbeService_,"/manager start");

                  if(pProbePlugin_)
                    {
                      try
                        {
                          pProbePlugin_->start();

                          Toolkit::sendSuccessResponse<OpenTestPoint::ProbeResponse>(pServer_);

                          OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pProbeService_,"/manager start success");

                          i64Timestamp = scheduleNextProbe(iFd,u16ProbeRate_);
                        }
                      catch(Toolkit::Exception & exp)
                        {
                          Toolkit::sendFailureResponse<OpenTestPoint::ProbeResponse>(pServer_,
                                                                                     "%s",
                                                                                     exp.what());
                        }
                    }
                  else
                    {
                      Toolkit::sendFailureResponse<OpenTestPoint::ProbeResponse>(pServer_,
                                                                                 "probe not initialized");
                    }

                  break;

                case OpenTestPoint::ProbeRequest::TYPE_STOP:

                  OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pProbeService_,"/manager stop");

                  if(pProbePlugin_)
                    {
                      try
                        {
                          pProbePlugin_->stop();

                          Toolkit::sendSuccessResponse<OpenTestPoint::ProbeResponse>(pServer_);
                        }
                      catch(Toolkit::Exception & exp)
                        {
                          Toolkit::sendFailureResponse<OpenTestPoint::ProbeResponse>(pServer_,
                                                                                     "%s",
                                                                                     exp.what());
                        }
                    }
                  else
                    {
                      Toolkit::sendFailureResponse<OpenTestPoint::ProbeResponse>(pServer_,
                                                                                 "manager probe not running");

                    }

                  break;

                case OpenTestPoint::ProbeRequest::TYPE_DESTROY:

                  OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pProbeService_,"/manager destroy");

                  try
                    {
                      if(pProbePlugin_)
                        {
                          pProbePlugin_->destroy();
                        }
                    }
                  catch(Toolkit::Exception & exp)
                    {
                      OPENTESTPOINT_PROBESERVICE_LOG_ERROR(pProbeService_,"/manager %s",exp.what());
                    }

                  Toolkit::sendSuccessResponse<OpenTestPoint::ProbeResponse>(pServer_);

                  bRun = false;
                  break;

                default:
                  Toolkit::sendFailureResponse<OpenTestPoint::ProbeResponse>(pServer_,
                                                                             "unknown message type");
                  //throw Toolkit::Exception{"unknown message type"};
                }
            }

          if(items[1].revents & ZMQ_POLLIN)
            {
              std::uint64_t u64Expired{};

              // wait for an interval timer to expire
              if(read(iFd,&u64Expired,sizeof(u64Expired)) > 0)
                {
                  try
                    {
                      auto info = pProbePlugin_->probe();

                      for(const auto & entry  : info)
                        {
                          std::string sTopic{std::get<0>(entry) +"." + sNodeId_};
                          const std::string & sData(std::get<1>(entry));

                          OpenTestPoint::ProbeReport report{};

                          report.set_index(probeIndex_);
                          report.set_tag(sNodeId_);
                          report.set_uuid(reinterpret_cast<const char *>(uuid_),sizeof(uuid_));
                          report.set_timestamp(i64Timestamp);
                          report.set_type(OpenTestPoint::ProbeReport::TYPE_DATA);

                          auto pData = report.mutable_data();

                          pData->set_name(std::get<2>(entry));
                          pData->set_module(std::get<3>(entry));
                          pData->set_version(std::get<4>(entry));
                          pData->set_blob(sData.c_str(),sData.length());

                          std::string sReport{};

                          if(report.SerializeToString(&sReport))
                            {
                              OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pProbeService_,
                                                                   "/manager sending %s",
                                                                   sTopic.c_str());

                              zmq_send(pPublisher_,sTopic.c_str(),sTopic.length(),ZMQ_SNDMORE);
                              zmq_send(pPublisher_,sReport.c_str(),sReport.length(),0);
                            }
                          else
                            {
                              OPENTESTPOINT_PROBESERVICE_LOG_ERROR(pProbeService_,
                                                                   "/manager probe report serialization error");
                            }
                        }
                    }
                  catch(std::exception & exp)
                    {
                      OPENTESTPOINT_PROBESERVICE_LOG_ERROR(pProbeService_,
                                                           "/manager probe error: %s",
                                                           exp.what());
                    }

                  i64Timestamp = scheduleNextProbe(iFd,u16ProbeRate_);
                }
            }
        }
    }
  catch(std::exception & exp)
    {
      OPENTESTPOINT_PROBESERVICE_LOG_ABORT(pProbeService_,
                                           "/manager thread aborting: %s",
                                           exp.what());
    }

  OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pProbeService_,"/manager thread exiting");
}


std::int64_t scheduleNextProbe(int iFd,int iDelta)
{
  timespec ts;

  clock_gettime(CLOCK_REALTIME,&ts);

  ts.tv_sec += iDelta - ts.tv_sec % iDelta;

  ts.tv_nsec = 0;

  // schedule the interval timer
  itimerspec spec{{0,0},ts};

  timerfd_settime(iFd,TFD_TIMER_ABSTIME,&spec,nullptr);

  return ts.tv_sec;
}
