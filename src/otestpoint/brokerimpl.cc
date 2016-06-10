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

#include "brokerimpl.h"
#include "otestpoint/toolkit/exception.h"
#include "otestpoint/toolkit/transaction.h"
#include "otestpoint/toolkit/log/clientbuilder.h"

#include "broker.pb.h"
#include "libotestpoint.pb.h"
#include "discovery.pb.h"

#include <zmq.h>
#include <vector>
#include <set>
#include <iostream>

std::tuple<std::set<std::string>,bool>
discoverRemote(const std::string & sDiscoveryEndpoint,
               void * pContext,
               OpenTestPoint::Toolkit::Log::Client * pLogClient);

OpenTestPoint::BrokerImpl::BrokerImpl(Toolkit::Log::Service & logService,
                                      Toolkit::Log::Client & logClient,
                                      const std::string & sServiceEndpoint,
                                      const std::string & sPublishEndpoint):
  logService_(logService),
  logClient_(logClient)
{
  pContext_.reset(zmq_ctx_new());

  if(!pContext_)
    {
      throw Toolkit::Exception{"Error creating new broker messaging context: %s",
          zmq_strerror(errno)};
    }

  pInternalSocket_.reset(zmq_socket(pContext_.get(),ZMQ_PAIR));

  if(!pInternalSocket_)
    {
      throw Toolkit::Exception{"unable to create new messaging socket: %s ",
          zmq_strerror(errno)};
    }

  if(zmq_bind(pInternalSocket_.get(),"inproc://broker") < 0)
    {
      throw Toolkit::Exception{"unable to connect to broker endpoint:  %s ",
          zmq_strerror(errno)};
    }

  thread_ = std::move(std::thread(&BrokerImpl::process,
                                  this,
                                  sServiceEndpoint,
                                  sPublishEndpoint));

  if(!Toolkit::transaction<OpenTestPoint::BrokerCommand,
     OpenTestPoint::BrokerResponse>
     (pInternalSocket_.get(),
      OpenTestPoint::BrokerCommand::TYPE_READY,
      std::chrono::seconds{5},
      [this](OpenTestPoint::BrokerCommand &){},
      [this](OpenTestPoint::BrokerResponse & response)
      {
        const auto & ready = response.ready();
        logService_.add(ready.logcontrol(),ready.logpublish());
      }))
    {
      thread_.join();
      throw Toolkit::Exception{"unable to verify processing thread creation"};
    }
}

OpenTestPoint::BrokerImpl::~BrokerImpl()
{
  if(Toolkit::transaction<OpenTestPoint::BrokerCommand,
     OpenTestPoint::BrokerResponse>
     (pInternalSocket_.get(),
      OpenTestPoint::BrokerCommand::TYPE_END,
      std::chrono::seconds{5}))
    {
      thread_.join();
    }
}

void OpenTestPoint::BrokerImpl::initialize(const std::string &)
{
  logClient_.log(Toolkit::Log::Level::DEBUG_LEVEL,"/broker initialize");
}

void OpenTestPoint::BrokerImpl::start()
{
  logClient_.log(Toolkit::Log::Level::DEBUG_LEVEL,"/broker start");
}

void OpenTestPoint::BrokerImpl::stop()
{
  logClient_.log(Toolkit::Log::Level::DEBUG_LEVEL,"/broker stop");
}

void OpenTestPoint::BrokerImpl::destroy()
{
  logClient_.log(Toolkit::Log::Level::DEBUG_LEVEL,"/broker destroy");
}

void OpenTestPoint::BrokerImpl::add(const std::string & sDiscoveryEndpoint,
                                    const std::string & sPublishEndpoint)
{
  if(!Toolkit::transaction<OpenTestPoint::BrokerCommand,
     OpenTestPoint::BrokerResponse>
     (pInternalSocket_.get(),
      OpenTestPoint::BrokerCommand::TYPE_ADD,
      std::chrono::seconds{5},
      [sDiscoveryEndpoint,
       sPublishEndpoint](OpenTestPoint::BrokerCommand & command)
      {
        auto pAdd = command.mutable_add();

        pAdd->set_discovery(sDiscoveryEndpoint);

        pAdd->set_publish(sPublishEndpoint);
      }))
    {
      logClient_.log(Toolkit::Log::Level::DEBUG_LEVEL,
                     "/broker timeout while adding: %s",
                     sDiscoveryEndpoint.c_str());
    }
}

void OpenTestPoint::BrokerImpl::process(const std::string & sServiceEndpoint,
                                        const std::string & sPublishEndpoint)
{
  std::vector<std::string> discoveryEndpoints{};

  Toolkit::Log::ClientBuilder logClientBuilder{};

  std::unique_ptr<Toolkit::Log::Client>
    pLogClient{logClientBuilder.buildClient("testpoint-broker/broker/processor")};

  try
    {
      Toolkit::RAIIZMQSocket pInternalSocket{zmq_socket(pContext_.get(),ZMQ_PAIR)};

      if(!pInternalSocket)
        {
          throw Toolkit::Exception{"unable to create new messaging socket: %s",
              zmq_strerror(errno)};
        }

      if(zmq_connect(pInternalSocket.get(),"inproc://broker") < 0)
        {
          throw Toolkit::Exception{"unable to connect to broker endpoint:  %s",
              zmq_strerror(errno)};
        }

      Toolkit::RAIIZMQSocket pXPubSocket{zmq_socket(pContext_.get(),ZMQ_XPUB)};

      if(!pXPubSocket)
        {
          throw Toolkit::Exception{"unable to create new xpub socket: %s",
              zmq_strerror(errno)};
        }

      int iIPv4Only = 0;

      if(zmq_setsockopt(pXPubSocket.get(),ZMQ_IPV4ONLY,&iIPv4Only,sizeof(iIPv4Only)))
        {
          throw Toolkit::Exception{"unable to disable IPv4 only on xpub endpoint: %s",
              zmq_strerror(errno)};
        }

      if(zmq_bind(pXPubSocket.get(),sPublishEndpoint.c_str()) < 0)
        {
          throw Toolkit::Exception{"unable to bind xpub endpoint:  %s",
              zmq_strerror(errno)};
        }

      Toolkit::RAIIZMQSocket pDiscoverySocket{zmq_socket(pContext_.get(),ZMQ_REP)};

      if(!pDiscoverySocket)
        {
          throw Toolkit::Exception{"unable to create new service socket: %s",
              zmq_strerror(errno)};
        }

      iIPv4Only = 0;

      if(zmq_setsockopt(pDiscoverySocket.get(),ZMQ_IPV4ONLY,&iIPv4Only,sizeof(iIPv4Only)))
        {
          throw Toolkit::Exception{"unable to disable IPv4 only on service endpoint: %s",
              zmq_strerror(errno)};
        }

      if(zmq_bind(pDiscoverySocket.get(),sServiceEndpoint.c_str()) < 0)
        {
          throw Toolkit::Exception{"unable to bind to service endpoint: %s",
              zmq_strerror(errno)};
        }

      Toolkit::RAIIZMQSocket pXSubSocket{zmq_socket(pContext_.get(),ZMQ_XSUB)};

      if(!pXSubSocket)
        {
          throw Toolkit::Exception{"unable to create new xsub socket: %s",
              zmq_strerror(errno)};
        }

      iIPv4Only = 0;

      if(zmq_setsockopt(pXSubSocket.get(),ZMQ_IPV4ONLY,&iIPv4Only,sizeof(iIPv4Only)))
        {
          throw Toolkit::Exception{"unable to disable IPv4 only on xsub endpoint: %s",
              zmq_strerror(errno)};
        }

      bool bRun{true};

      while(bRun)
        {
          std::vector<zmq_pollitem_t> items =
            {
              {pInternalSocket.get(),0,ZMQ_POLLIN,0},
              {pDiscoverySocket.get(),0,ZMQ_POLLIN,0},
              {pXPubSocket.get(),0,ZMQ_POLLIN,0},
              {pXSubSocket.get(),0,ZMQ_POLLIN,0},
            };

          int rc = zmq_poll(&items[0],items.size(),-1);

          if(rc == -1)
            {
              continue;
            }

          for(const auto & item : items)
            {
              // process internal messages between frontend and backend
              if(item.revents & ZMQ_POLLIN)
                {
                  if(item.socket == pInternalSocket.get())
                    {
                      zmq_msg_t message;

                      zmq_msg_init(&message);

                      zmq_msg_recv(&message,pInternalSocket.get(), 0);

                      OpenTestPoint::BrokerCommand command;

                      if(!command.ParseFromArray(zmq_msg_data(&message),
                                                 zmq_msg_size(&message)))
                        {
                          zmq_msg_close(&message);

                          throw Toolkit::Exception{"unable to deserialize broker command"};
                        }

                      zmq_msg_close(&message);

                      switch(command.type())
                        {
                        case OpenTestPoint::BrokerCommand::TYPE_END:
                          Toolkit::sendSuccessResponse<OpenTestPoint::BrokerResponse>(pInternalSocket.get());
                          bRun = false;
                          break;

                        case OpenTestPoint::BrokerCommand::TYPE_READY:
                          {
                            OpenTestPoint::BrokerResponse response;
                            response.set_type(OpenTestPoint::BrokerResponse::TYPE_READY);
                            auto pReady =  response.mutable_ready();

                            pReady->set_logcontrol(pLogClient->getControlEndpoint());
                            pReady->set_logpublish(pLogClient->getPublishEndpoint());

                            std::string sSerialization{};

                            if(!response.SerializeToString(&sSerialization))
                              {
                                throw Toolkit::Exception{"unable to serialize ready message"};
                              }

                            zmq_send(pInternalSocket.get(),sSerialization.c_str(),sSerialization.length(),0);
                          }

                          break;

                        case OpenTestPoint::BrokerCommand::TYPE_ADD:
                          {
                            // Do discovery and handle missing testpoints
                            // add the probe to the backend inorder to subscribe
                            if(command.has_add())
                              {
                                const auto add = command.add();

                                std::string sRemoteDiscoveryEndpoint{add.discovery().c_str()};
                                std::string sRemotePublishEndpoint{add.publish().c_str()};

                                if(zmq_connect(pXSubSocket.get(),std::string{"tcp://"}.append(sRemotePublishEndpoint).c_str()))
                                  {
                                    pLogClient->log(OpenTestPoint::Toolkit::Log::Level::ERROR_LEVEL,
                                                    "unable to connect to %s:%s",
                                                    sRemotePublishEndpoint.c_str(),
                                                    zmq_strerror(errno));
                                  }

                                discoveryEndpoints.push_back(sRemoteDiscoveryEndpoint);

                                Toolkit::sendSuccessResponse<OpenTestPoint::BrokerResponse>(pInternalSocket.get());
                              }
                            else
                              {
                                throw Toolkit::Exception{"malformed broker command"};
                              }
                          }
                        }
                    }
                  else if(item.socket == pDiscoverySocket.get())
                    {
                      // external interface handling probe discovery
                      zmq_msg_t message;

                      zmq_msg_init(&message);

                      if(zmq_msg_recv(&message,pDiscoverySocket.get(),ZMQ_DONTWAIT) < 0)
                        {
                          pLogClient->log(OpenTestPoint::Toolkit::Log::Level::ERROR_LEVEL,
                                          "discovery socket recv error: %s",
                                          zmq_strerror(errno));

                          continue;
                        }

                      OpenTestPoint::DiscoveryRequest request;

                      if(!request.ParseFromArray(zmq_msg_data(&message),
                                                 zmq_msg_size(&message)))
                        {
                          Toolkit::sendFailureResponse<OpenTestPoint::DiscoveryResponse>(pDiscoverySocket.get(),
                                                                                         "unknown request message format");
                        }
                      else
                        {
                          switch(request.type())
                            {
                            case OpenTestPoint::DiscoveryRequest::TYPE_DISCOVERY:
                              {
                                OpenTestPoint::DiscoveryResponse response;

                                response.set_type(OpenTestPoint::DiscoveryResponse::TYPE_DISCOVERY);

                                auto pDiscovery = response.mutable_discovery();

                                std::set<std::string> uniqueTopics;

                                for(const auto & sRemoteDiscoveryEndpoint : discoveryEndpoints)
                                  {
                                    auto status = discoverRemote(sRemoteDiscoveryEndpoint,
                                                                 pContext_.get(),
                                                                 pLogClient.get());

                                    if(std::get<1>(status))
                                      {
                                        uniqueTopics.insert(std::get<0>(status).begin(),
                                                            std::get<0>(status).end());
                                      }
                                  }

                                for(const auto & sTopic : uniqueTopics)
                                  {
                                    pDiscovery->add_names(sTopic);
                                  }

                                pDiscovery->set_publish(sPublishEndpoint);

                                std::string sSerialization;

                                if(!response.SerializeToString(&sSerialization))
                                  {
                                    throw Toolkit::Exception{"unable to serialize discovery message"};
                                  }

                                zmq_send(pDiscoverySocket.get(),sSerialization.c_str(),sSerialization.length(),0);
                              }
                              break;

                            default:
                              pLogClient->log(OpenTestPoint::Toolkit::Log::Level::ERROR_LEVEL,"unknown discovery request");

                              Toolkit::sendFailureResponse<OpenTestPoint::DiscoveryResponse>(pDiscoverySocket.get(),
                                                                                             "unknown request type");

                            }
                        }

                      zmq_msg_close(&message);

                    }
                  else if(item.socket == pXPubSocket.get())
                    {
                      while(1)
                        {
                          zmq_msg_t message;

                          zmq_msg_init(&message);

                          zmq_msg_recv(&message,pXPubSocket.get(), 0);

                          int iMore{};

                          size_t sizeMore{sizeof(iMore)};

                          zmq_getsockopt(pXPubSocket.get(),ZMQ_RCVMORE,&iMore,&sizeMore);

                          zmq_msg_send(&message,pXSubSocket.get(),iMore ? ZMQ_SNDMORE : 0);

                          zmq_msg_close(&message);

                          if(!iMore)
                            {
                              break;
                            }
                        }
                    }
                  else if(item.socket ==  pXSubSocket.get())
                    {
                      while(1)
                        {
                          zmq_msg_t message;

                          zmq_msg_init(&message);

                          zmq_msg_recv(&message,item.socket, 0);

                          int iMore{};

                          size_t sizeMore{sizeof(iMore)};

                          zmq_getsockopt(item.socket,ZMQ_RCVMORE,&iMore,&sizeMore);

                          zmq_msg_send(&message,pXPubSocket.get(),iMore ? ZMQ_SNDMORE : 0);

                          zmq_msg_close(&message);

                          if(!iMore)
                            {
                              break;
                            }
                        }
                    }
                }
            }
        }
    }
  catch(std::exception & exp)
    {
      std::cerr<<exp.what()<<std::endl;
    }
}

std::tuple<std::set<std::string>,bool>
discoverRemote(const std::string & sDiscoveryEndpoint,
               void * pContext,
               OpenTestPoint::Toolkit::Log::Client * pLogClient)
{
  std::set<std::string> probeNames{};
  bool bSuccess{};

  try
    {
      OpenTestPoint::Toolkit::RAIIZMQSocket pDiscoveryClientSocket{zmq_socket(pContext,ZMQ_REQ)};

      if(!pDiscoveryClientSocket)
        {
          pLogClient->log(OpenTestPoint::Toolkit::Log::Level::DEBUG_LEVEL,
                          "unable to open discovery client socket %s",
                          zmq_strerror(errno));
        }
      else
        {
          std::string sPublishEndpoint{};

          if(!zmq_connect(pDiscoveryClientSocket.get(),
                          std::string{"tcp://"}.append(sDiscoveryEndpoint).c_str()))
            {
              if(!OpenTestPoint::Toolkit::transaction<OpenTestPoint::DiscoveryRequest,
                 OpenTestPoint::DiscoveryResponse>
                 (pDiscoveryClientSocket.get(),
                  OpenTestPoint::DiscoveryRequest::TYPE_DISCOVERY,
                  std::chrono::seconds{1},
                  [](OpenTestPoint::DiscoveryRequest &){},
                  [&sPublishEndpoint,
                   &probeNames](OpenTestPoint::DiscoveryResponse & response)
                  {
                    auto pDiscovery = response.mutable_discovery();

                    sPublishEndpoint = pDiscovery->publish();

                    for(int i = 0; i < pDiscovery->names_size(); ++i)
                      {
                        probeNames.insert(pDiscovery->names(i));
                      }


                  }))
                {
                  pLogClient->log(OpenTestPoint::Toolkit::Log::Level::ERROR_LEVEL,
                                  "communication timeout while discovering %s",
                                  sDiscoveryEndpoint.c_str());
                }
              else
                {
                  pLogClient->logfn(OpenTestPoint::Toolkit::Log::Level::DEBUG_LEVEL,
                                    [probeNames]()
                                    {
                                      std::list<std::string> probes{};

                                      for(const auto & name : probeNames)
                                        {
                                          probes.push_back(name);
                                        }

                                      return probes;
                                    },
                                    "available probes from %s: ",
                                    sPublishEndpoint.c_str());

                  bSuccess = true;
                }
            }
          else
            {
              pLogClient->log(OpenTestPoint::Toolkit::Log::Level::ERROR_LEVEL,
                              "unable to connect to discovery server %s %s",
                              sDiscoveryEndpoint.c_str(),
                              zmq_strerror(errno));
            }
        }
    }
  catch(OpenTestPoint::Toolkit::Exception & exp)
    {
      pLogClient->log(OpenTestPoint::Toolkit::Log::Level::ERROR_LEVEL,
                      "%s",
                      exp.what());
    }

  return std::make_tuple(probeNames,bSuccess);
}
