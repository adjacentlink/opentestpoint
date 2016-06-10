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

#ifndef OPENTESTPOINT_TOOLKIT_LOG_CLIENT_HEADER_
#define OPENTESTPOINT_TOOLKIT_LOG_CLIENT_HEADER_

#include <cstdarg>
#include <list>
#include <string>
#include <chrono>

#include "otestpoint/toolkit/log/level.h"

namespace OpenTestPoint
{
  namespace Toolkit
  {
    namespace Log
    {
      class Client
      {
      public:
        virtual ~Client(){};

        virtual void log(Level level, const char *fmt,...)
          __attribute__ ((format (printf, 3, 4))) = 0;


        template <typename Function>
        void logfn(Level level, Function fn,const char *fmt,...)
          __attribute__ ((format (printf, 4, 5)));

        virtual std::string getControlEndpoint() const = 0;

        virtual std::string getPublishEndpoint() const = 0;

      protected:
        Client(const std::string & sLabel);

        std::string sLabel_;

      private:
        virtual bool allowLog_i(Level level) = 0;

        virtual void log_i(Level level,
                           const std::chrono::high_resolution_clock::time_point & timestamp,
                           const std::list<std::string> & strings) = 0;
      };
    }
  }
}

#include "otestpoint/toolkit/log/client.inl"

#endif // OPENTESTPOINT_TOOLKIT_LOG_CLIENT_HEADER_
