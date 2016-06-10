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

#ifndef OPENTESTPOINT_TOOLKIT_APPLICATION_HEADER_
#define OPENTESTPOINT_TOOLKIT_APPLICATION_HEADER_

#include "otestpoint/toolkit/log/service.h"
#include "otestpoint/toolkit/log/client.h"

#include <string>
#include <vector>
#include <memory>
#include <uuid.h>
#include <getopt.h>

namespace OpenTestPoint
{
  namespace Toolkit
  {
    class Application
    {
    public:
      virtual ~Application();

      int main(int argc, char * argv[]);

    protected:
      Application(const std::string & sName);

      const std::string & getName() const;

      const uuid_t & getUUID() const;

      Log::Client & getLogClient();

      Log::Service & getLogService();

      virtual void doInitialize(const std::string &){};

      virtual void doStart(){};

      virtual void doStop(){};

      virtual void doDestroy(){}

      virtual std::vector<option> doGetOptions() const
      {
        return {};
      }

      virtual std::vector<std::string> doGetOptionsUsage() const
      {
        return {};
      }

      virtual std::string doGetOptString() const
      {
        return {};
      }

      virtual bool doProcessOption(int iOptOpt, const char * pzOptArg)
      {
        (void) iOptOpt;
        (void) pzOptArg;
        return false;
      }

      virtual std::uint16_t doGetDefaultLogLevel() const
      {
        return DEFAULT_LOG_LEVEL;
      }

      virtual std::uint16_t doGetDefaultPriorityLevel() const
      {
        return DEFAULT_PRIORITY_LEVEL;
      }

      virtual std::string doGetPrologue() const
      {
        return "";
      }

      virtual std::string doGetEpilogue() const
      {
        return "";
      }

    private:
      class Implementation;
      std::unique_ptr<Implementation> pImpl_;

      enum
        {
          DEFAULT_LOG_LEVEL = 0,
          DEFAULT_PRIORITY_LEVEL = 20,
        };

      void usage() const;
    };
  }
}

#define DECLARE_OPENTESTPOINT_TOOLKIT_APPLCIATION(X,name)               \
  int main(int argc, char * argv[])                                     \
  {                                                                     \
    std::unique_ptr<OpenTestPoint::Toolkit::Application> p{new X{(name)}}; \
    return p->main(argc,argv);                                          \
  }

#endif // OPENTESTPOINT_TOOLKIT_APPLICATION_HEADER_
