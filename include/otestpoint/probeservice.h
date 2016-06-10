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

#ifndef OPENTESTPOINT_PROBESERVICE_HEADER_
#define OPENTESTPOINT_PROBESERVICE_HEADER_

#include "otestpoint/toolkit/log/client.h"

#include <memory>

namespace OpenTestPoint
{
  /**
   * @class ProbeService
   *
   * @brief Provides access to probe services.
   *
   * The ProbeService provides access to probe services such as
   * logging.
   */
  class ProbeService
  {
  public:
    /**
     * Destroys an instance
     */
    virtual ~ProbeService(){};

    /**
     * Gets the log client instance
     *
     * @return A borrowed pointer to a log client instance.
     */
    virtual Toolkit::Log::Client * logClient() = 0;

  protected:
    /**
     * Creates an instance
     */
    ProbeService(){};
  };
}


/**
 * Logs a printf style log message at ERROR level.
 *
 * @param pService PlatformService reference
 * @param fmt format string (see printf)
 * @param args Variable data (see printf)
 */
#define OPENTESTPOINT_PROBESERVICE_LOG_ERROR(pService,fmt,args...)      \
  (pService)->logClient()->log(OpenTestPoint::Toolkit::Log::Level::ERROR_LEVEL, \
                               fmt,## args)

/**
 * Logs a printf style log message at ABORT level.
 *
 * @param pService PlatformService reference
 * @param fmt format string (see printf)
 * @param args Variable data (see printf)
 */
#define OPENTESTPOINT_PROBESERVICE_LOG_ABORT(pService,fmt,args...)      \
  (pService)->logClient()->log(OpenTestPoint::Toolkit::Log::Level::ABORT_LEVEL, \
                               fmt,## args)

/**
 * Logs a printf style log message at INFO level.
 *
 * @param pService PlatformService reference
 * @param fmt format string (see printf)
 * @param args Variable data (see printf)
 */
#define OPENTESTPOINT_PROBESERVICE_LOG_INFO(pService,fmt,args...)       \
  (pService)->logClient()->log(OpenTestPoint::Toolkit::Log::Level::INFO_LEVEL, \
                               fmt,## args)

/**
 * Logs a printf style log message at DEBUG level.
 *
 * @param pService PlatformService reference
 * @param fmt format string (see printf)
 * @param args Variable data (see printf)
 */
#define OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pService,fmt,args...)      \
  (pService)->logClient()->log(OpenTestPoint::Toolkit::Log::Level::DEBUG_LEVEL, \
                               fmt,## args)

/**
 * Logs a printf style log message at ERROR level with appended
 * message output from a callable.
 *
 * @param  pService PlatformService reference
 * @param fn Callable object returning std::list<string>
 * @param fmt format string (see printf)
 * @param args Variable data (see printf)
 *
 * @note Callable object is executed based on log level.
 */
#define OPENTESTPOINT_PROBESERVICE_LOG_FN_ERROR(pService,fn,fmt,args...) \
  (pService)->logClient()->logfn(OpenTestPoint::Toolkit::Log::Level::ERROR_LEVEL,fn,fmt,## args)

/**
 * Logs a printf style log message at ABORT level with appended
 * message output from a callable.
 *
 * @param  pService PlatformService reference
 * @param fn Callable object returning std::list<string>
 * @param fmt format string (see printf)
 * @param args Variable data (see printf)
 *
 * @note Callable object is executed based on log level.
 */
#define OPENTESTPOINT_PROBESERVICE_LOG_FN_ABORT(pService,fn,fmt,args...) \
  (pService)->logClient()->logfn(OpenTestPoint::Toolkit::Log::Level::ABORT_LEVEL,fn,fmt,## args)

/**
 * Logs a printf style log message at INFO level with appended
 * message output from a callable.
 *
 * @param  pService PlatformService reference
 * @param fn Callable object returning std::list<string>
 * @param fmt format string (see printf)
 * @param args Variable data (see printf)
 *
 * @note Callable object is executed based on log level.
 */
#define OPENTESTPOINT_PROBESERVICE_LOG_FN_INFO(pService,fn,fmt,args...) \
  (pService)->logClient()->logfn(OpenTestPoint::Toolkit::Log::Level::INFO_LEVEL,fn,fmt,## args)

/**
 * Logs a printf style log message at DEBUG level with appended
 * message output from a callable.
 *
 * @param  pService PlatformService reference
 * @param fn Callable object returning std::list<string>
 * @param fmt format string (see printf)
 * @param args Variable data (see printf)
 *
 * @note Callable object is executed based on log level.
 */
#define OPENTESTPOINT_PROBESERVICE_LOG_FN_DEBUG(pService,fn,fmt,args...) \
  (pService)->logClient()->logfn(OpenTestPoint::Toolkit::Log::Level::DEBUG_LEVEL,fn,fmt,## args)


#endif // OPENTESTPOINT_PROBESERVICE_HEADER_
