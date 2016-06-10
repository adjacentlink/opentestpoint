/*
 * Copyright (c) 2014,2016 - Adjacent Link LLC, Bridgewater, New Jersey
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

#ifndef OPENTESTPOINT_TOOLKIT_SERVICESINGLETON_HEADER_
#define OPENTESTPOINT_TOOLKIT_SERVICESINGLETON_HEADER_

#include "otestpoint/toolkit/singleton.h"
#include "otestpoint/toolkit/log/client.h"

#include <memory>

namespace OpenTestPoint
{
  namespace Toolkit
  {
    class ServiceSingleton : public Singleton<ServiceSingleton>
    {
    public:
      ~ServiceSingleton();

      Log::Client * logClient();

      void initialize(Log::Client * pLogClient);

    protected:
      ServiceSingleton();

    private:
      class Implementation;
      std::unique_ptr<Implementation> pImpl_;
    };
  }
}

#define OPENTESTPOINT_TOOLKIT_LOG_ERROR(fmt,args...)                    \
  OpenTestPoint::Toolkit::ServiceSingleton::instance()->logClient()->   \
  log(OpenTestPoint::Toolkit::Log::Level::ERROR_LEVEL,fmt,## args)

#define OPENTESTPOINT_TOOLKIT_LOG_ABORT(fmt,args...)                    \
  OpenTestPoint::Toolkit::ServiceSingleton::instance()->logClient()->   \
  log(OpenTestPoint::Toolkit::Log::Level::ABORT_LEVEL,fmt,## args)

#define OPENTESTPOINT_TOOLKIT_LOG_INFO(fmt,args...)                     \
  OpenTestPoint::Toolkit::ServiceSingleton::instance()->logClient()->   \
  log(OpenTestPoint::Toolkit::Log::Level::INFO_LEVEL,fmt,## args)

#define OPENTESTPOINT_TOOLKIT_LOG_DEBUG(fmt,args...)                    \
  OpenTestPoint::Toolkit::ServiceSingleton::instance()->logClient()->   \
  log(OpenTestPoint::Toolkit::Log::Level::DEBUG_LEVEL,fmt,## args)

#define OPENTESTPOINT_TOOLKIT_LOG_FN_ERROR(fn,fmt,args...)              \
  OpenTestPoint::Toolkit::ServiceSingleton::instance()->logClient()->   \
  logfn(OpenTestPoint::Toolkit::Log::Level::ERROR_LEVEL,(fn),fmt,## args)

#define OPENTESTPOINT_TOOLKIT_LOG_FN_ABORT(fn,fmt,args...)              \
  OpenTestPoint::Toolkit::ServiceSingleton::instance()->logClient()->   \
  logfn(OpenTestPoint::Toolkit::Log::Level::ABORT_LEVEL,(fn),fmt,## args)

#define OPENTESTPOINT_TOOLKIT_LOG_FN_INFO(fn,fmt,args...)               \
  OpenTestPoint::Toolkit::ServiceSingleton::instance()->logClient()->   \
  logfn(OpenTestPoint::Toolkit::Log::Level::INFO_LEVEL,(fn),fmt,## args)

#define OPENTESTPOINT_TOOLKIT_LOG_FN_DEBUG(fn,fmt,args...)              \
  OpenTestPoint::Toolkit::ServiceSingleton::instance()->logClient()->   \
  logfn(OpenTestPoint::Toolkit::Log::Level::DEBUG_LEVEL,fn,fmt,## args)

#endif // OPENTESTPOINT_TOOLKIT_SERVICESINGLETON_HEADER_
