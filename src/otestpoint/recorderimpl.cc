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

#include "recorderimpl.h"
#include "otestpoint/toolkit/exception.h"
#include "otestpoint/toolkit/transaction.h"
#include "otestpoint/toolkit/log/clientbuilder.h"

#include "recorder.pb.h"
#include "probereport.pb.h"

#include <vector>
#include <sstream>

#include <zmq.h>
#include <uuid.h>
#include <sqlite3.h>
#include <arpa/inet.h>

const char * pzCreateTableSQL="\
DROP TABLE IF EXISTS probes;\
CREATE TABLE probes (time INT,\
                     uuid TEXT,\
                     probe TEXT,\
                     tag TEXT,\
                     pindex INT,\
                     offset INT,\
                     size INT,\
                     PRIMARY KEY (time,\
                                  uuid,\
                                  probe,\
                                  tag,\
                                  pindex));";

OpenTestPoint::RecorderImpl::RecorderImpl(Toolkit::Log::Service & logService,
                                          Toolkit::Log::Client & logClient,
                                          const std::string & sRecordFileName):
  logService_(logService),
  logClient_(logClient)
{
  pContext_.reset(zmq_ctx_new());

  if(!pContext_)
    {
      throw Toolkit::Exception{"Error creating new recorder messaging context: %s",
          zmq_strerror(errno)};
    }

  pInternalSocket_.reset(zmq_socket(pContext_.get(),ZMQ_PAIR));

  if(!pInternalSocket_)
    {
      throw Toolkit::Exception{"unable to create new messaging socket: %s ",
          zmq_strerror(errno)};
    }

  if(zmq_bind(pInternalSocket_.get(),"inproc://recorder") < 0)
    {
      throw Toolkit::Exception{"unable to connect to recorder endpoint:  %s ",
          zmq_strerror(errno)};
    }

  recorderFile_.open(sRecordFileName.c_str(),std::ios::out);

  if(!recorderFile_)
    {
      throw Toolkit::Exception{"unable to open record file: %s",
          sRecordFileName.c_str()};
    }


  std::string sRecordFileDBName{sRecordFileName + ".db"};

  sqlite3 * pDB{};

  if(sqlite3_open(sRecordFileDBName.c_str(), &pDB))
    {
      throw Toolkit::Exception{"unable to open database file %s: %s",
          sRecordFileDBName.c_str(),
          sqlite3_errmsg(pDB)};
    }

  pSQLiteDB_.reset(pDB);

  char * pzErrMsg{};

  if(sqlite3_exec(pSQLiteDB_.get(),"PRAGMA journal_mode = OFF", nullptr, nullptr, &pzErrMsg) != SQLITE_OK)
    {
      std::string sError{pzErrMsg};
      sqlite3_free(pzErrMsg);
      throw Toolkit::Exception{"database error: %s",sError.c_str()};
    }

  if(sqlite3_exec(pSQLiteDB_.get(),"PRAGMA synchronous = OFF", nullptr, nullptr, &pzErrMsg) != SQLITE_OK)
    {
      std::string sError{pzErrMsg};
      sqlite3_free(pzErrMsg);
      throw Toolkit::Exception{"database error: %s",sError.c_str()};
    }

  if(sqlite3_exec(pSQLiteDB_.get(),pzCreateTableSQL, nullptr, nullptr, &pzErrMsg) != SQLITE_OK)
    {
      std::string sError{pzErrMsg};
      sqlite3_free(pzErrMsg);
      throw Toolkit::Exception{"database error: %s",sError.c_str()};
    }

  thread_ = std::move(std::thread(&RecorderImpl::process,
                                  this));

  if(!Toolkit::transaction<OpenTestPoint::RecorderCommand,
     OpenTestPoint::RecorderResponse>
     (pInternalSocket_.get(),
      OpenTestPoint::RecorderCommand::TYPE_READY,
      std::chrono::seconds{5},
      [this](OpenTestPoint::RecorderCommand &){},
      [this](OpenTestPoint::RecorderResponse & response)
      {
        const auto & ready = response.ready();
        logService_.add(ready.logcontrol(),ready.logpublish());
      }))
    {
      thread_.join();
      throw Toolkit::Exception{"unable to verify processing thread creation"};
    }
}

OpenTestPoint::RecorderImpl::~RecorderImpl()
{
  if(Toolkit::transaction<OpenTestPoint::RecorderCommand,
     OpenTestPoint::RecorderResponse>
     (pInternalSocket_.get(),
      OpenTestPoint::RecorderCommand::TYPE_END,
      std::chrono::seconds{5}))
    {
      thread_.join();
    }
}

void OpenTestPoint::RecorderImpl::initialize(const std::string &)
{
  logClient_.log(Toolkit::Log::Level::DEBUG_LEVEL,"/recorder initialize");
}

void OpenTestPoint::RecorderImpl::start()
{
  logClient_.log(Toolkit::Log::Level::DEBUG_LEVEL,"/recorder start");
}

void OpenTestPoint::RecorderImpl::stop()
{
  logClient_.log(Toolkit::Log::Level::DEBUG_LEVEL,"/recorder stop");
}

void OpenTestPoint::RecorderImpl::destroy()
{
  logClient_.log(Toolkit::Log::Level::DEBUG_LEVEL,"/recorder destroy");
}

void OpenTestPoint::RecorderImpl::add(const std::string & sPublishEndpoint)
{
  if(!Toolkit::transaction<OpenTestPoint::RecorderCommand,
     OpenTestPoint::RecorderResponse>
     (pInternalSocket_.get(),
      OpenTestPoint::RecorderCommand::TYPE_ADD,
      std::chrono::seconds{5},
      [sPublishEndpoint](OpenTestPoint::RecorderCommand & command)
      {
        auto pAdd = command.mutable_add();

        pAdd->set_publish(sPublishEndpoint);
      }))
    {
      logClient_.log(Toolkit::Log::Level::DEBUG_LEVEL,
                     "/recorder timeout while adding: %s",
                     sPublishEndpoint.c_str());
    }
}

void OpenTestPoint::RecorderImpl::process()
{
  Toolkit::Log::ClientBuilder logClientBuilder{};

  std::unique_ptr<Toolkit::Log::Client>
    pLogClient{logClientBuilder.buildClient("testpoint-recorder/recorder/processor")};

  try
    {
      Toolkit::RAIIZMQSocket pInternalSocket{zmq_socket(pContext_.get(),ZMQ_PAIR)};

      if(!pInternalSocket)
        {
          throw Toolkit::Exception{"unable to create new messaging socket: %s",
              zmq_strerror(errno)};
        }

      if(zmq_connect(pInternalSocket.get(),"inproc://recorder") < 0)
        {
          throw Toolkit::Exception{"unable to connect to recorder endpoint:  %s",
              zmq_strerror(errno)};
        }

      Toolkit::RAIIZMQSocket pXSubSocket{zmq_socket(pContext_.get(),ZMQ_XSUB)};

      if(!pXSubSocket)
        {
          throw Toolkit::Exception{"unable to create new xsub socket: %s",
              zmq_strerror(errno)};
        }

      int iIPv4Only = 0;

      if(zmq_setsockopt(pXSubSocket.get(),ZMQ_IPV4ONLY,&iIPv4Only,sizeof(iIPv4Only)))
        {
          throw Toolkit::Exception{"unable to disable IPv4 only on xsub endpoint: %s",
              zmq_strerror(errno)};
        }

      // subscribe to all logs
      zmq_send(pXSubSocket.get(),"\x1",1,0);

      bool bRun{true};

      while(bRun)
        {
          std::vector<zmq_pollitem_t> items =
            {
              {pInternalSocket.get(),0,ZMQ_POLLIN,0},
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

                      OpenTestPoint::RecorderCommand command;

                      if(!command.ParseFromArray(zmq_msg_data(&message),
                                                 zmq_msg_size(&message)))
                        {
                          zmq_msg_close(&message);

                          throw Toolkit::Exception{"unable to deserialize recorder command"};
                        }

                      zmq_msg_close(&message);

                      switch(command.type())
                        {
                        case OpenTestPoint::RecorderCommand::TYPE_END:
                          Toolkit::sendSuccessResponse<OpenTestPoint::RecorderResponse>(pInternalSocket.get());
                          bRun = false;
                          break;

                        case OpenTestPoint::RecorderCommand::TYPE_READY:
                          {
                            OpenTestPoint::RecorderResponse response;
                            response.set_type(OpenTestPoint::RecorderResponse::TYPE_READY);
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

                        case OpenTestPoint::RecorderCommand::TYPE_ADD:
                          {
                            // Do discovery and handle missing testpoints
                            // add the probe to the backend inorder to subscribe
                            if(command.has_add())
                              {
                                const auto add = command.add();

                                std::string sRemotePublishEndpoint{add.publish().c_str()};

                                if(zmq_connect(pXSubSocket.get(),std::string{"tcp://"}.append(sRemotePublishEndpoint).c_str()))
                                  {
                                    pLogClient->log(OpenTestPoint::Toolkit::Log::Level::ERROR_LEVEL,
                                                    "unable to connect to %s:%s",
                                                    sRemotePublishEndpoint.c_str(),
                                                    zmq_strerror(errno));
                                  }
                                else
                                  {
                                    pLogClient->log(OpenTestPoint::Toolkit::Log::Level::DEBUG_LEVEL,
                                                    "connected to %s",
                                                    sRemotePublishEndpoint.c_str());
                                  }

                                Toolkit::sendSuccessResponse<OpenTestPoint::RecorderResponse>(pInternalSocket.get());
                              }
                            else
                              {
                                throw Toolkit::Exception{"malformed recorder command"};
                              }
                          }
                        }
                    }
                  else if(item.socket ==  pXSubSocket.get())
                    {
                      bool bFirstPart{true};
                      std::string sProbeName{};

                      while(1)
                        {
                          zmq_msg_t message;

                          zmq_msg_init(&message);

                          zmq_msg_recv(&message,item.socket, 0);

                          int iMore{};

                          size_t sizeMore{sizeof(iMore)};

                          zmq_getsockopt(item.socket,ZMQ_RCVMORE,&iMore,&sizeMore);

                          // first part holds the probe subscription name
                          if(bFirstPart)
                            {
                              sProbeName = std::string{reinterpret_cast<const char *>(zmq_msg_data(&message)),
                                                       zmq_msg_size(&message)};
                            }
                          else
                            {
                              OpenTestPoint::ProbeReport report{};
                              char buf[64];

                              if(report.ParseFromArray(zmq_msg_data(&message),
                                                       zmq_msg_size(&message)))
                                {
                                  std::stringstream sstream{};

                                  uuid_unparse(reinterpret_cast<const unsigned char *>(report.uuid().data()),buf);

                                  sstream<<"INSERT INTO probes VALUES ("
                                         <<report.timestamp()<<","
                                         <<"'"<<buf<<"',"
                                         <<"'"<<sProbeName<<"',"
                                         <<"'"<<report.tag()<<"',"
                                         <<"'"<<report.index()<<"',"
                                         <<recorderFile_.tellp()+4L<<","
                                         <<zmq_msg_size(&message)<<");";

                                  char * pzErrMsg{};

                                  if(sqlite3_exec(pSQLiteDB_.get(),sstream.str().c_str(), nullptr, nullptr, &pzErrMsg) == SQLITE_OK)
                                    {
                                      std::uint32_t u32MessageLength{htonl(static_cast<uint32_t>(zmq_msg_size(&message)))};

                                      recorderFile_.write(reinterpret_cast<const char *>(&u32MessageLength),
                                                          sizeof(u32MessageLength));

                                      recorderFile_.write(reinterpret_cast<const char *>(zmq_msg_data(&message)),
                                                          zmq_msg_size(&message)).flush();
                                    }
                                  else
                                    {
                                      char * pzErrMsg{};

                                      pLogClient->log(OpenTestPoint::Toolkit::Log::Level::ERROR_LEVEL,
                                                      "unable to insert probe database info %s",
                                                      pzErrMsg);

                                      sqlite3_free(pzErrMsg);
                                    }
                                }
                            }

                          zmq_msg_close(&message);

                          if(!iMore)
                            {
                              break;
                            }

                          bFirstPart = false;
                        }
                    }
                }
            }
        }
    }
  catch(...)
    {}
}
