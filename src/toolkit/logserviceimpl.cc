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

#include "logserviceimpl.h"
#include "logserviceimpl.pb.h"
#include "logservice.pb.h"
#include "logutils.h"

#include "otestpoint/toolkit/transaction.h"
#include "otestpoint/toolkit/exception.h"

#include <zmq.h>
#include <iostream>
#include <sstream>

OpenTestPoint::Toolkit::Log::ServiceImpl::ServiceImpl(Level level,
                                                      const std::string & sLogFileName):
  level_(level),
  sLogFileName_(sLogFileName)

{
  pContext_.reset(zmq_ctx_new());

  if(!pContext_)
    {
      throw Exception{"Error creating new log service messaging context: %s",
          zmq_strerror(errno)};
    }

  pInternalSocket_.reset(zmq_socket(pContext_.get(),ZMQ_PAIR));

  if(!pInternalSocket_)
    {
      throw Exception{"unable to create new log service internal socket: %s ",
          zmq_strerror(errno)};
    }

  if(zmq_bind(pInternalSocket_.get(),"inproc://logger") < 0)
    {
      throw Exception{"unable to bind to  log service internal endpoint:  %s ",
          zmq_strerror(errno)};
    }


  if(!sLogFileName_.empty())
    {
      logStream_.open(sLogFileName_.c_str(), std::ios::out);

      if(!logStream_)
        {
          throw Exception{"unable to open log file: %s",sLogFileName_.c_str()};
        }
    }


  thread_ = std::move(std::thread(&ServiceImpl::process,this));
}

OpenTestPoint::Toolkit::Log::ServiceImpl::~ServiceImpl()
{
  for(const auto & controlSocket: controlSockets_)
    {
      zmq_close(controlSocket);
    }

  std::string sSerialization{};

  OpenTestPoint_Toolkit::LogServiceImpl command;

  command.set_type(OpenTestPoint_Toolkit::LogServiceImpl::TYPE_END);

  if(command.SerializeToString(&sSerialization))
    {
      zmq_send(pInternalSocket_.get(),sSerialization.c_str(),sSerialization.length(),0);
    }

  thread_.join();
}

void OpenTestPoint::Toolkit::Log::ServiceImpl::add(const std::string & sControlEndpoint,
                                                   const std::string & sPublishEndpoint)
{
  OpenTestPoint_Toolkit::LogServiceImpl command;

  command.set_type(OpenTestPoint_Toolkit::LogServiceImpl::TYPE_ADD);

  auto pAdd = command.mutable_add();

  pAdd->set_control(sControlEndpoint);

  pAdd->set_publish(sPublishEndpoint);

  std::string sSerialization;

  if(!command.SerializeToString(&sSerialization))
    {
      throw Exception{"unable to serialize logger message"};
    }

  zmq_send(pInternalSocket_.get(),sSerialization.c_str(),sSerialization.length(),0);

  void * pControlSocket{};

  if(!(pControlSocket = zmq_socket(pContext_.get(),ZMQ_REQ)))
    {
      throw Exception{"unable to create log client control socket: %s (%s)",
          zmq_strerror(errno),
          sControlEndpoint.c_str()};
    }

  if(zmq_connect(pControlSocket,sControlEndpoint.c_str()) < 0)
    {
      zmq_close(pControlSocket);
      throw Exception{"unable to connect log client control socket: %s (%s)",
          zmq_strerror(errno),
          sControlEndpoint.c_str()};
    }

  controlSockets_.push_back(pControlSocket);

  transaction<OpenTestPoint_Toolkit::LogClient,
              OpenTestPoint_Toolkit::LogServer>
    (pControlSocket,
     OpenTestPoint_Toolkit::LogClient::TYPE_SETLOGLEVEL,
     std::chrono::milliseconds{10000},
     [this](OpenTestPoint_Toolkit::LogClient & request)
     {
       auto pSetLogLevel = request.mutable_setloglevel();

       pSetLogLevel->set_level(convertLogLevel(level_));
     });
}

void OpenTestPoint::Toolkit::Log::ServiceImpl::setLevel(Level level)
{
  level_ = level;

  for(const auto & pControlSocket : controlSockets_)
    {
      transaction<OpenTestPoint_Toolkit::LogClient,
                  OpenTestPoint_Toolkit::LogServer>
        (pControlSocket,
         OpenTestPoint_Toolkit::LogClient::TYPE_SETLOGLEVEL,
         std::chrono::milliseconds{10000},
         [level](OpenTestPoint_Toolkit::LogClient & request)
         {
           auto pSetLogLevel = request.mutable_setloglevel();

           pSetLogLevel->set_level(convertLogLevel(level));
         });
    }
}

void OpenTestPoint::Toolkit::Log::ServiceImpl::process()
{
  std::ostream & log = sLogFileName_.empty() ? std::cout : logStream_;

  try
    {
      Toolkit::RAIIZMQSocket pInternalSocket{zmq_socket(pContext_.get(),ZMQ_PAIR)};

      if(!pInternalSocket)
        {
          throw Toolkit::Exception{"unable to create new messaging socket: %s",
              zmq_strerror(errno)};
        }

      if(zmq_connect(pInternalSocket.get(),"inproc://logger") < 0)
        {
          throw Toolkit::Exception{"unable to connect to internal logger endpoint:  %s",
              zmq_strerror(errno)};
        }

      Toolkit::RAIIZMQSocket pXSubSocket{zmq_socket(pContext_.get(),ZMQ_XSUB)};

      if(!pXSubSocket)
        {
          throw Toolkit::Exception{"unable to create new xsub socket: %s",
              zmq_strerror(errno)};
        }

      // subscribe to all logs
      zmq_send(pXSubSocket.get(),"\x1log",4,0);

      bool bRun{true};

      while(bRun)
        {
          std::vector<zmq_pollitem_t> items =
            {
              {pInternalSocket.get(),0,ZMQ_POLLIN,0},
              {pXSubSocket.get(),0,ZMQ_POLLIN,0},
            };

          int rc = zmq_poll(&items[0], items.size(), -1);

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

                      OpenTestPoint_Toolkit::LogServiceImpl command;

                      if(!command.ParseFromArray(zmq_msg_data(&message),
                                                 zmq_msg_size(&message)))
                        {
                          zmq_msg_close(&message);

                          throw Exception{"unable to deserialize logger command"};
                        }

                      zmq_msg_close(&message);

                      switch(command.type())
                        {
                        case OpenTestPoint_Toolkit::LogServiceImpl::TYPE_END:
                          bRun = false;
                          break;

                        case OpenTestPoint_Toolkit::LogServiceImpl::TYPE_ADD:
                          {
                            // add the probe to the backend inorder to subscribe
                            if(command.has_add())
                              {
                                const auto add = command.add();

                                std::string sLogEndpoint{add.publish()};

                                if(zmq_connect(pXSubSocket.get(),sLogEndpoint.c_str()) < 0)
                                  {
                                    throw Exception{"unable to connect log sub endpoint:  %s ",
                                        zmq_strerror(errno)};
                                  }
                              }
                            else
                              {
                                throw Exception{"malformed logger command"};
                              }
                          }
                          break;
                        }
                    }
                  else if(item.socket == pXSubSocket.get())
                    {
                      zmq_msg_t message;

                      // first part is the topic
                      zmq_msg_init(&message);

                      zmq_msg_recv(&message,item.socket, 0);

                      zmq_msg_close(&message);

                      // second part is the log
                      zmq_msg_init(&message);

                      zmq_msg_recv(&message,item.socket, 0);

                      OpenTestPoint_Toolkit::LogPublisher logPublisher;

                      if(!logPublisher.ParseFromArray(zmq_msg_data(&message),
                                                      zmq_msg_size(&message)))
                        {
                          zmq_msg_close(&message);

                          throw Exception{"unable to deserialize log publisher command"};
                        }

                      if(logPublisher.type() == OpenTestPoint_Toolkit::LogPublisher::TYPE_RECORD)
                        {
                          const auto & record = logPublisher.record();

                          std::uint16_t iLength{};

                          std::stringstream ssBuf{};

                          std::stringstream ssLabel{};

                          ssLabel<<record.label();

                          for(int i = 0; i < record.logs_size(); ++i)
                            {
                              std::string sRecord{record.logs(i)};

                              if(sRecord[0] == '/')
                                {
                                  auto pos = sRecord.find(' ');

                                  ssLabel<<sRecord.substr(0,pos);

                                  if(pos != std::string::npos)
                                    {
                                      sRecord = sRecord.substr(pos+1);
                                    }
                                  else
                                    {
                                      sRecord.clear();
                                    }
                                }

                              if(!sRecord.empty())
                                {
                                  if(ssBuf.str().empty())
                                    {
                                      ssBuf<<sRecord;
                                    }
                                  else
                                    {
                                      ssBuf<<" "<<sRecord;
                                    }
                                }
                            }

                          using Clock = std::chrono::high_resolution_clock;

                          auto timestamp = std::chrono::microseconds{record.timestamp()};

                          std::time_t t{Clock::to_time_t(Clock::time_point{timestamp})};;

                          std::tm ltm;

                          localtime_r(&t, &ltm);

                          char buf[64];

                          iLength =
                            snprintf(buf, sizeof(buf),"%02d:%02d:%02d.%06zu",
                                     ltm.tm_hour,
                                     ltm.tm_min,
                                     ltm.tm_sec,
                                     timestamp.count()%1000000);

                          buf[iLength] ='\0';

                          log<<buf
                             <<" "
                             <<logLevelToString(record.level()).c_str()
                             <<" ["
                             <<ssLabel.str()
                             <<"] "
                             <<ssBuf.str()
                             <<std::endl;
                        }
                      zmq_msg_close(&message);

                      zmq_msg_close(&message);
                    }
                }
            }
        }
    }
  catch(Exception & exp)
    {
      std::cerr<<exp.what()<<std::endl;
    }
  catch(...)
    {}
}
