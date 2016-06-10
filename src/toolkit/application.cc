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

#include "otestpoint/toolkit/application.h"
#include "otestpoint/toolkit/log/servicebuilder.h"
#include "otestpoint/toolkit/log/clientbuilder.h"
#include "otestpoint/toolkit/stringto.h"
#include "otestpoint/toolkit/servicesingleton.h"

#include <iostream>
#include <fstream>
#include <mutex>
#include <sstream>

#include <getopt.h>
#include <unistd.h>
#include <uuid.h>
#include <signal.h>


namespace
{
  std::mutex mutex{};

  void sighandler(int)
  {
    mutex.unlock();
  }
}

class OpenTestPoint::Toolkit::Application::Implementation
{
public:
  std::string sName_;
  uuid_t uuid_;
  std::unique_ptr<Toolkit::Log::Service> pLogService_;
  Toolkit::Log::Client * pLogClient_;
};

OpenTestPoint::Toolkit::Application::Application(const std::string & sName):
  pImpl_{new Implementation}
{
  pImpl_->sName_ = sName;
  uuid_generate(pImpl_->uuid_);
}

OpenTestPoint::Toolkit::Application::~Application()
{}


const uuid_t & OpenTestPoint::Toolkit::Application::getUUID() const
{
  return pImpl_->uuid_;
}

const std::string & OpenTestPoint::Toolkit::Application::getName() const
{
  return pImpl_->sName_;
}

OpenTestPoint::Toolkit::Log::Service & OpenTestPoint::Toolkit::Application::getLogService()
{
  return *pImpl_->pLogService_.get();
}

OpenTestPoint::Toolkit::Log::Client & OpenTestPoint::Toolkit::Application::getLogClient()
{
  return *pImpl_->pLogClient_;
}

int OpenTestPoint::Toolkit::Application::main(int argc, char * argv[])
{
  try
    {
      std::vector<option> options =
        {
          {"help",     0, nullptr, 'h'},
          {"realtime", 0, nullptr, 'r'},
          {"loglevel", 1, nullptr, 'l'},
          {"logfile",  1, nullptr, 'f'},
          {"daemonize",0, nullptr, 'd'},
          {"version" , 0, nullptr, 'v'},
          {"pidfile" , 1, nullptr,  2},
          {"uuidfile", 1, nullptr,  3},
          {"priority", 1, nullptr,  'p'},
        };

      std::string sOptString{"hrvdf:l:p:b:"};

      int iOption{};
      int iOptionIndex{};
      bool bDaemonize{};
      bool bRealtime{};
      int  iLogLevel{doGetDefaultLogLevel()};
      int  iPriority{doGetDefaultPriorityLevel()};
      std::string sLogFile{};
      std::string sPIDFile{};
      std::string sUUIDFile{};

      // add any specialized options
      std::vector<option> additionalOptions = doGetOptions();

      options.insert(options.end(),
                     additionalOptions.begin(),
                     additionalOptions.end());

      // add any specialized optsting items
      sOptString += doGetOptString();

      // close out the options
      options.push_back({0, 0,nullptr,0 });

      while((iOption = getopt_long(argc,
                                   argv,
                                   sOptString.c_str(),
                                   &options[0],
                                   &iOptionIndex)) != -1)
        {
          switch(iOption)
            {
            case 'h':
              usage();
              return EXIT_SUCCESS;
              break;

            case 'r':
              bRealtime = true;
              if(optarg)
                {
                  std::cout<<optarg<<std::endl;
                }
              break;

            case 'd':
              bDaemonize = true;
              break;

            case 'v':
              std::cout<<VERSION<<std::endl;
              return EXIT_SUCCESS;

            case 'f':
              sLogFile = optarg;
              break;

            case 2:
              sPIDFile = optarg;
              break;

            case 3:
              sUUIDFile = optarg;
              break;

            case 'p':
              try
                {
                  iPriority = OpenTestPoint::Toolkit::strToINT32(optarg,0,99);
                }
              catch(...)
                {
                  std::cerr<<"invalid priority: "<<optarg<<std::endl;
                  return EXIT_FAILURE;
                }

              break;

            case 'l':
              try
                {
                  iLogLevel = OpenTestPoint::Toolkit::strToINT32(optarg,0,4);
                }
              catch(...)
                {
                  std::cerr<<"invalid log level: "<<optarg<<std::endl;
                  return EXIT_FAILURE;
                }

              break;


            case '?':
              if(optopt == 't')
                {
                  std::cerr<<"option -"<<static_cast<char>(optopt)<<" requires an argument."<<std::endl;
                }
              else if(isprint(optopt))
                {
                  std::cerr<<"unknown option -"<<static_cast<char>(optopt)<<"."<<std::endl;
                }
              else
                {
                  std::cerr<<"bad option"<<std::endl;
                }
              return EXIT_FAILURE;

            default:
              if(!doProcessOption(iOption,optarg))
                {
                  return EXIT_FAILURE;
                }
            }
        }

      if(bDaemonize)
        {
          if(sLogFile.empty() && iLogLevel != 0)
            {
              std::cerr<<"unable to daemonize log level must be 0 when logging to stdout"<<std::endl;;
              return EXIT_FAILURE;
            }

          if(daemon(1,0))
            {
              std::cerr<<"unable to daemonize"<<std::endl;
              return EXIT_FAILURE;
            }
        }


      Log::ServiceBuilder serviceBuilder{};

      pImpl_->pLogService_.reset(serviceBuilder.buildService(static_cast<Log::Level>(iLogLevel),
                                                             sLogFile));

      Log::ClientBuilder clientBuilder{};

      std::string sLabel{getName()};

      pImpl_->pLogClient_ = clientBuilder.buildClient(sLabel);

      // ownership transfer
      ServiceSingleton::instance()->initialize(pImpl_->pLogClient_);

      pImpl_->pLogService_->add(pImpl_->pLogClient_->getControlEndpoint(),
                                pImpl_->pLogClient_->getPublishEndpoint());

      std::string sConfigurationXML{};

      if(optind >= argc)
        {
          std::cerr<<"Missing CONFIG_URL"<<std::endl;
          return EXIT_FAILURE;
        }
      else
        {
          sConfigurationXML = argv[optind];
        }

      std::stringstream ss;
      for(int i = 0; i < argc; ++i)
        {
          ss<<argv[i]<<" ";
        }

      char uuidBuf[37];
      uuid_unparse(pImpl_->uuid_,uuidBuf);

      OPENTESTPOINT_TOOLKIT_LOG_INFO("/application commandline: %s",ss.str().c_str());
      OPENTESTPOINT_TOOLKIT_LOG_INFO("/application uuid: %s",uuidBuf);

      doInitialize(sConfigurationXML);

      if(bRealtime)
        {
          struct sched_param schedParam{iPriority};

          if(sched_setscheduler(0,SCHED_RR,&schedParam))
            {
              const char * pzAbort{"unable to set realtime scheduler"};

              if(!sLogFile.empty() || !iLogLevel)
                {
                  std::cerr<<pzAbort<<std::endl;
                }

              OPENTESTPOINT_TOOLKIT_LOG_ABORT("/application %s",pzAbort);

              return EXIT_FAILURE;
            }
        }

      if(!sPIDFile.empty())
        {
          std::ofstream pidFile{sPIDFile.c_str(),std::ios::out};

          if(pidFile)
            {
              pidFile<<getpid()<<std::endl;
            }
          else
            {
              return EXIT_FAILURE;
            }
        }

      if(!sUUIDFile.empty())
        {
          std::ofstream uuidFile{sUUIDFile.c_str(),std::ios::out};

          if(uuidFile)
            {
              uuidFile<<uuidBuf<<std::endl;
            }
          else
            {
              return EXIT_FAILURE;
            }
        }


      doStart();

      struct sigaction action;

      memset(&action,0,sizeof(action));

      action.sa_handler = sighandler;

      sigaction(SIGINT,&action,nullptr);
      sigaction(SIGQUIT,&action,nullptr);

      mutex.lock();

      // signal handler unlocks mutex
      mutex.lock();

      doStop();

      doDestroy();

      OPENTESTPOINT_TOOLKIT_LOG_INFO("/application shutdown");

      sleep(1);

      ServiceSingleton::instance()->destroy();
    }
  catch(const std::exception & exp)
    {
      std::cerr<<exp.what()<<std::endl;

      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

void OpenTestPoint::Toolkit::Application::usage() const
{
  std::string sPrologue{doGetPrologue()};

  if(!sPrologue.empty())
    {
      std::cout<<sPrologue<<std::endl;
      std::cout<<std::endl;
    }

  std::cout<<"usage: "<<getName()<<" [OPTIONS]... CONFIG_URL"<<std::endl;
  std::cout<<std::endl;
  std::cout<<" CONFIG_URL                      URL of XML configuration file."<<std::endl;
  std::cout<<std::endl;
  std::cout<<"options:"<<std::endl;
  std::cout<<"  -d, --daemonize                Run in the background."<<std::endl;
  std::cout<<"  -h, --help                     Print this message and exit."<<std::endl;
  std::cout<<"  -f, --logfile FILE             Log to a file instead of stdout."<<std::endl;
  std::cout<<"  -l, --loglevel [0,4]           Set initial log level."<<std::endl;
  std::cout<<"                                  default: "<<doGetDefaultLogLevel()<<std::endl;
  std::cout<<"  --pidfile FILE                 Write application pid to file."<<std::endl;
  std::cout<<"  -p, --priority [0,99]          Set realtime priority level."<<std::endl;
  std::cout<<"                                 Only used with -r, --realtime."<<std::endl;
  std::cout<<"                                  default: "<<doGetDefaultPriorityLevel()<<std::endl;
  std::cout<<"  -r, --realtime                 Set realtime scheduling."<<std::endl;
  std::cout<<"  --uuidfile FILE                Write the application instance UUID to file."<<std::endl;
  std::cout<<"  -v, --version                  Print version and exit."<<std::endl;
  std::cout<<std::endl;

  auto additionalOptions =  doGetOptionsUsage();

  if(!additionalOptions.empty())
    {
      std::cout<<"additional options:"<<std::endl;

      for(const auto & option : additionalOptions)
        {
          std::cerr<<option<<std::endl;
        }
      std::cout<<std::endl;
    }

  std::string sEpilogue{doGetEpilogue()};

  if(!sEpilogue.empty())
    {
      std::cout<<std::endl;
      std::cout<<sEpilogue<<std::endl;
      std::cout<<std::endl;
    }
}
