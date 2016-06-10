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

#ifndef OPENTESTPOINT_TOOLKIT_LOGUTILS_HEADER_
#define OPENTESTPOINT_TOOLKIT_LOGUTILS_HEADER_

#include "otestpoint/toolkit/log/level.h"
#include "loglevel.pb.h"

namespace OpenTestPoint
{
  namespace Toolkit
  {
    namespace Log
    {
      inline
      std::string logLevelToString(OpenTestPoint_Toolkit::LogLevel level)
      {
        switch(level)
          {
          case OpenTestPoint_Toolkit::LogLevel::TYPE_NOLOG:
            return "NOLOG";

          case OpenTestPoint_Toolkit::LogLevel::TYPE_ABORT:
            return "ABORT";

          case OpenTestPoint_Toolkit::LogLevel::TYPE_ERROR:
            return "ERROR";

          case OpenTestPoint_Toolkit::LogLevel::TYPE_INFO:
            return "INFO";

          case OpenTestPoint_Toolkit::LogLevel::TYPE_DEBUG:
            return "DEBUG";
          }

        return "UNKNOWN";
      }

      inline
      std::string logLevelToString(Level level)
      {
        switch(level)
          {
          case Level::NOLOG_LEVEL:
            return "NOLOG";

          case Level::ABORT_LEVEL:
            return "ABORT";

          case Level::ERROR_LEVEL:
            return "ERROR";

          case Level::INFO_LEVEL:
            return "INFO";

          case Level::DEBUG_LEVEL:
            return "DEBUG";
          }

        return "UNKNOWN";
      }

      inline
      OpenTestPoint_Toolkit::LogLevel convertLogLevel(Level level)
      {
        switch(level)
          {
          case Level::NOLOG_LEVEL:
            return OpenTestPoint_Toolkit::LogLevel::TYPE_NOLOG;

          case Level::ABORT_LEVEL:
            return OpenTestPoint_Toolkit::LogLevel::TYPE_ABORT;

          case Level::ERROR_LEVEL:
            return OpenTestPoint_Toolkit::LogLevel::TYPE_ERROR;

          case Level::INFO_LEVEL:
            return OpenTestPoint_Toolkit::LogLevel::TYPE_INFO;

          case Level::DEBUG_LEVEL:
            return OpenTestPoint_Toolkit::LogLevel::TYPE_DEBUG;
          }

        return  OpenTestPoint_Toolkit::LogLevel::TYPE_NOLOG;
      }

      inline
      Level convertLogLevel(OpenTestPoint_Toolkit::LogLevel level)
      {
        switch(level)
          {
          case OpenTestPoint_Toolkit::LogLevel::TYPE_NOLOG:
            return Level::NOLOG_LEVEL;

          case OpenTestPoint_Toolkit::LogLevel::TYPE_ABORT:
            return Level::ABORT_LEVEL;

          case OpenTestPoint_Toolkit::LogLevel::TYPE_ERROR:
            return Level::ERROR_LEVEL;

          case OpenTestPoint_Toolkit::LogLevel::TYPE_INFO:
            return Level::INFO_LEVEL;

          case OpenTestPoint_Toolkit::LogLevel::TYPE_DEBUG:
            return Level::DEBUG_LEVEL;
          }

        return Level::NOLOG_LEVEL;
      }

    }
  }
}


#endif // OPENTESTPOINT_TOOLKIT_LOGUTILS_HEADER_
