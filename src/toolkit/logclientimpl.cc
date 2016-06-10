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

#include "logclientimpl.h"
#include "logservice.pb.h"
#include "logutils.h"
#include "otestpoint/toolkit/exception.h"
#include "otestpoint/toolkit/transaction.h"

#include <chrono>
#include <zmq.h>
#include <iostream>


OpenTestPoint::Toolkit::Log::ClientImpl::ClientImpl(const std::string & sLabel):
  Client{sLabel},
  sInternalEndpoint_{std::string{"inproc://loggerimpl."}
    + std::to_string(reinterpret_cast<unsigned long>(this))},
  level_{Level::NOLOG_LEVEL}
{
  pContext_.reset(zmq_ctx_new());

  if(!pContext_)
    {
      throw Exception{"Error creating new controller messaging context: %s",
          zmq_strerror(errno)};
    }

  pPublishSocket_.reset(zmq_socket(pContext_.get(),ZMQ_PUB));

  if(!pPublishSocket_)
    {
      throw Exception{"unable to create logger publish socket: %s",
          zmq_strerror(errno)};
    }

  if(zmq_bind(pPublishSocket_.get(), "tcp://127.0.0.1:*") < 0)
    {
      throw Exception{"unable to bind to logger publish endpoint:  %s",
          zmq_strerror(errno)};
    }

  char buf[1024];
  size_t len{sizeof(buf)};

  if(zmq_getsockopt(pPublishSocket_.get(),ZMQ_LAST_ENDPOINT,buf,&len))
    {
      throw Toolkit::Exception{"unable to determine logger publish endpoint : %s",
          zmq_strerror(errno)};
    }

  sPublishEndpoint_ = buf;

  pInternalSocket_.reset(zmq_socket(pContext_.get(),ZMQ_PAIR));

  if(!pInternalSocket_)
    {
      throw Exception{"unable to create internal logger frontend socket: %s ",
          zmq_strerror(errno)};
    }

  if(zmq_bind(pInternalSocket_.get(),sInternalEndpoint_.c_str()) < 0)
    {
      throw Exception{"unable to bind internal logger frontend socket:  %s ",
          zmq_strerror(errno)};
    }


  thread_ = std::move(std::thread(&ClientImpl::process,this));

  zmq_msg_t message;

  zmq_msg_init(&message);

  zmq_msg_recv(&message,pInternalSocket_.get(),0);

  sControlEndpoint_ = std::string{reinterpret_cast<const char *>(zmq_msg_data(&message)),
                                  zmq_msg_size(&message)};

  zmq_msg_close(&message);
}

OpenTestPoint::Toolkit::Log::ClientImpl::~ClientImpl()
{
  zmq_send(pInternalSocket_.get(),"end",3,0);
  thread_.join();
}

void OpenTestPoint::Toolkit::Log::ClientImpl::log(Level level, const char *fmt,...)
{
  if(allowLog_i(level))
    {
      auto now = std::chrono::high_resolution_clock::now();

      char buff[1024];

      va_list ap;

      va_start(ap, fmt);

      vsnprintf(buff,sizeof(buff),fmt,ap);

      va_end(ap);

      log_i(level,now,{buff});
    }
}

bool OpenTestPoint::Toolkit::Log::ClientImpl::allowLog_i(Level level)
{
  Level cached = level_;

  if(static_cast<int>(level) <= static_cast<int>(cached))
    {
      return true;
    }
  else
    {
      return false;
    }
}

void OpenTestPoint::Toolkit::Log::ClientImpl::log_i(Level level,
                                                    const std::chrono::high_resolution_clock::time_point & timestamp,
                                                    const std::list<std::string> & strings)
{
  OpenTestPoint_Toolkit::LogPublisher message;

  message.set_type(OpenTestPoint_Toolkit::LogPublisher::TYPE_RECORD);

  auto pRecord = message.mutable_record();

  pRecord->set_level(convertLogLevel(level));

  pRecord->set_label(sLabel_);

  pRecord->set_timestamp(std::chrono::duration_cast<std::chrono::microseconds>(timestamp.time_since_epoch()).count());

  for(const auto & log : strings)
    {
      pRecord->add_logs(log);
    }

  std::string sSerialization{};

  if(message.SerializeToString(&sSerialization))
    {
      zmq_send(pPublishSocket_.get(),"log",3,ZMQ_SNDMORE);
      zmq_send(pPublishSocket_.get(),sSerialization.c_str(),sSerialization.length(),0);
    }
}

std::string OpenTestPoint::Toolkit::Log::ClientImpl::getControlEndpoint() const
{
  return sControlEndpoint_;
}

std::string OpenTestPoint::Toolkit::Log::ClientImpl::getPublishEndpoint() const
{
  return sPublishEndpoint_;
}

void OpenTestPoint::Toolkit::Log::ClientImpl::process()
{
  try
    {
      RAIIZMQSocket pControlSocket{zmq_socket(pContext_.get(),ZMQ_REP)};

      if(!pControlSocket)
        {
          throw Exception{"unable to create log client control socket: %s",
              zmq_strerror(errno)};
        }

      if(zmq_bind(pControlSocket.get(),"tcp://127.0.0.1:*") < 0)
        {
          throw Exception{"unable to bind log client control endpoint:  %s",
              zmq_strerror(errno)};
        }

      char buf[1024];
      size_t len{sizeof(buf)};

      if(zmq_getsockopt(pControlSocket.get(),ZMQ_LAST_ENDPOINT,buf,&len))
        {
          throw Toolkit::Exception{"unable to determine log client control endpoint : %s",
              zmq_strerror(errno)};
        }

      RAIIZMQSocket pInternalSocket{zmq_socket(pContext_.get(),ZMQ_PAIR)};

      if(!pInternalSocket)
        {
          throw Exception{"unable to create internal log client backend socket: %s (%s)",
              zmq_strerror(errno),
              sInternalEndpoint_.c_str()};
        }

      if(zmq_connect(pInternalSocket.get(),sInternalEndpoint_.c_str()) < 0)
        {
          throw Exception{"unable to connect internal log client backend socket:  %s (%s)",
              zmq_strerror(errno),
              sInternalEndpoint_.c_str()};
        }

      zmq_send(pInternalSocket.get(),buf,strlen(buf),0);

      bool bRun{true};

      while(bRun)
        {
          zmq_pollitem_t items[] =
            {
              {pControlSocket.get(),0,ZMQ_POLLIN,0},
              {pInternalSocket.get(),0,ZMQ_POLLIN,0},
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

              int iSize = zmq_msg_recv(&message,pControlSocket.get(),0);

              OpenTestPoint_Toolkit::LogClient request{};

              if(iSize > 0)
                {
                  if(!request.ParseFromArray(zmq_msg_data(&message),zmq_msg_size(&message)))
                    {
                      sendFailureResponse<OpenTestPoint_Toolkit::LogServer>(pControlSocket.get(),
                                                                            "unknown message");
                      zmq_msg_close(&message);
                    }
                  else
                    {
                      switch(request.type())
                        {
                        case OpenTestPoint_Toolkit::LogClient::TYPE_SETLOGLEVEL:

                          if(request.has_setloglevel())
                            {
                              const auto setLogLevel = request.setloglevel();

                              level_ = convertLogLevel(setLogLevel.level());

                              sendSuccessResponse<OpenTestPoint_Toolkit::LogServer>(pControlSocket.get());
                            }
                          else
                            {
                              sendFailureResponse<OpenTestPoint_Toolkit::LogServer>(pControlSocket.get(),
                                                                                    "invalid message format");
                            }

                          break;
                        }
                    }
                }

              zmq_msg_close(&message);
            }

          if(items[1].revents & ZMQ_POLLIN)
            {
              zmq_msg_t message;

              zmq_msg_init(&message);

              zmq_msg_recv(&message,pInternalSocket.get(),0);

              zmq_msg_close(&message);

              bRun = false;
            }
        }
    }
  catch(Exception & exp)
    {
      std::cerr<<exp.what();
    }
}
