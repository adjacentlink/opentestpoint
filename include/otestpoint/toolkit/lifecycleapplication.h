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

#ifndef OPENTESTPOINT_TOOLKIT_LIFECYCLEAPPLICATION_HEADER_
#define OPENTESTPOINT_TOOLKIT_LIFECYCLEAPPLICATION_HEADER_

#include "otestpoint/toolkit/application.h"
#include "otestpoint/toolkit/lifecycle.h"

namespace OpenTestPoint
{
  namespace Toolkit
  {
    template<typename Builder, typename Director>
    class LifeCycleApplication : public Application
    {
    public:
      LifeCycleApplication(const std::string & sName_):
        Application{sName_},
        builder_{getUUID()}{}

    private:
      void doInitialize(const std::string & sConfigurationFile) override
      {
        getLogClient().log(Log::Level::DEBUG_LEVEL,
                           "/application/lifecycle initialize configuration: %s",
                           sConfigurationFile.empty() ? "(none)" : sConfigurationFile.c_str());

        pDirector_.reset(new Director{getLogService(),
              getLogClient(),
              getUUID(),
              builder_});

        pController_.reset(pDirector_->construct(sConfigurationFile));

        pController_->initialize();
      }

      void doStart() override
      {
        getLogClient().log(Log::Level::DEBUG_LEVEL,"/application/lifecycle start");

        pController_->start();

        getLogClient().log(Log::Level::DEBUG_LEVEL,"/application/lifecycle post start");

        pController_->postStart();
      }

      void doStop() override
      {
        getLogClient().log(Log::Level::DEBUG_LEVEL,"/application/lifecycle stop");

        pController_->stop();
      }

      void doDestroy() override
      {
        getLogClient().log(Log::Level::DEBUG_LEVEL,"/application/lifecycle destroy");

        pController_->destroy();
      }

    protected:
      Builder builder_;
      std::unique_ptr<Director> pDirector_;
      std::unique_ptr<LifeCycle> pController_;
    };
  }
}

#endif // OPENTESTPOINT_TOOLKIT_LIFECYCLEAPPLICATION_HEADER_
