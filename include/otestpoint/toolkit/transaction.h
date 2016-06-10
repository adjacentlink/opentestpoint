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

#ifndef OPENTESTPOINT_TOOLKIT_TRANSACTION_HEADER_
#define OPENTESTPOINT_TOOLKIT_TRANSACTION_HEADER_

#include "otestpoint/toolkit/exception.h"
#include <chrono>
#include <functional>
#include <zmq.h>

namespace OpenTestPoint
{
  namespace Toolkit
  {
    template<typename Request, typename Response>
    bool transaction(void * pClient,
                     typename Request::Type type,
                     std::chrono::milliseconds timeout = std::chrono::milliseconds::zero(),
                     std::function<void (Request & request)> buildRequestFunc = [](Request &){},
                     std::function<void (Response & response)> parseResponseFunc = [](Response &){})
    {
      using OpenTestPoint::Toolkit::Exception;

      Request request{};

      request.set_type(type);

      buildRequestFunc(request);

      std::string sSerialization;

      if(!request.SerializeToString(&sSerialization))
        {
          throw Exception{"unable to serialize transaction"};
        }

      if(zmq_send(pClient,sSerialization.c_str(),sSerialization.size(),0) < 0)
        {
          throw Exception{"unable to send transaction: %s",zmq_strerror(errno)};
        }

      bool bRun{true};

      while(bRun)
        {
          zmq_pollitem_t items[] =
            {
              {pClient,0,ZMQ_POLLIN,0},
            };

          int rc = zmq_poll(items,
                            1,
                            timeout == std::chrono::milliseconds::zero() ? -1 : timeout.count());

          if(rc == -1)
            {
              continue;
            }

          if(items[0].revents & ZMQ_POLLIN)
            {
              zmq_msg_t message;

              zmq_msg_init(&message);

              int size = zmq_msg_recv(&message,pClient, 0);

              if(size < 0)
                {
                  zmq_msg_close(&message);
                  throw Exception{"unable to receive transaction: %s",zmq_strerror(errno)};
                }

              Response response{};

              if(!response.ParseFromArray(zmq_msg_data(&message),size))
                {
                  zmq_msg_close(&message);
                  throw Exception{"unable to deserialize transaction"};
                }

              zmq_msg_close(&message);

              if(response.type() == Response::TYPE_ERROR)
                {
                  std::string sWhat{response.error().what()};
                  throw Exception{"%s",sWhat.c_str()};
                }

              parseResponseFunc(response);

              bRun = false;
            }
          else
            {
              return false;
            }
        }

      return true;
    }

    template <typename T>
    void sendSuccessResponse(void * pServer)
    {
      T response{};

      response.set_type(T::TYPE_SUCCESS);

      std::string sSerialization;

      using OpenTestPoint::Toolkit::Exception;

      if(!response.SerializeToString(&sSerialization))
        {
          throw Exception{"unable to serialize message"};
        }

      if(zmq_send(pServer,sSerialization.c_str(),sSerialization.size(),0) < 0)
        {
          throw Exception{"unable to send message: %s",zmq_strerror(errno)};
        }
    }

    template <typename T>
    void sendFailureResponse(void * pServer,const char *fmt,...)
    {
      char buf[2048];

      va_list ap;

      va_start(ap, fmt);

      vsnprintf(buf,sizeof(buf),fmt,ap);

      va_end(ap);

      buf[sizeof(buf)-1] = '\0';

      T response{};

      response.set_type(T::TYPE_ERROR);

      auto pError = response.mutable_error();

      pError->set_what(buf);

      std::string sSerialization;

      if(!response.SerializeToString(&sSerialization))
        {
          throw Exception{"unable to serialize message"};
        }

      if(zmq_send(pServer,sSerialization.c_str(),sSerialization.size(),0) < 0)
        {
          throw Exception{"unable to send message: %s",zmq_strerror(errno)};
        }
    }

    inline
    bool forward(void * pTarget,
                 void * pOriginator,
                 const void * p,
                 size_t len,
                 std::chrono::milliseconds timeout = std::chrono::milliseconds::zero())
    {
      using OpenTestPoint::Toolkit::Exception;

      if(zmq_send(pTarget,p,len,0) < 0)
        {
          throw Exception{"unable to send target request: %s",zmq_strerror(errno)};
        }

      bool bRun{true};

      while(bRun)
        {
          zmq_pollitem_t items[] =
            {
              {pTarget,0,ZMQ_POLLIN,0},
            };

          int rc = zmq_poll(items,
                            1,
                            timeout == std::chrono::milliseconds::zero() ? -1 : timeout.count());

          if(rc == -1)
            {
              continue;
            }

          if(items[0].revents & ZMQ_POLLIN)
            {
              while(1)
                {
                  zmq_msg_t message;

                  zmq_msg_init(&message);

                  zmq_msg_recv(&message,pTarget,0);

                  int iMore{};

                  size_t sizeMore{sizeof(iMore)};

                  zmq_getsockopt(pTarget,ZMQ_RCVMORE,&iMore,&sizeMore);

                  zmq_msg_send(&message,pOriginator,iMore ? ZMQ_SNDMORE : 0);

                  zmq_msg_close(&message);

                  if(!iMore)
                    {
                      break;
                    }
                }

              bRun = false;
            }
          else
            {
              return false;
            }
        }

      return true;
    }
  }
}


#endif // OPENTESTPOINT_TOOLKIT_TRANSACTION_HEADER_
