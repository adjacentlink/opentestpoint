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

#include "otestpoint/probebuilder.h"
#include "otestpoint/toolkit/exception.h"

#include "probecontainer.h"
#include "controllerimpl.h"

#include <memory>
#include <uuid.h>

class OpenTestPoint::ProbeBuilder::Impl
{
public:
  Impl(const uuid_t & uuid):
    probeIndex_{}
  {
    uuid_copy(uuid_,uuid);
  }

  uuid_t uuid_;
  ProbeIndex probeIndex_;
  std::unique_ptr<ControllerImpl> pControllerImpl_;
};

OpenTestPoint::ProbeBuilder::ProbeBuilder(const uuid_t & uuid):pImpl_{new Impl{uuid}}{}

void OpenTestPoint::ProbeBuilder::buildController(Toolkit::Log::Service & logService,
                                                  Toolkit::Log::Client & logClient,
                                                  const std::string & sServiceEndpoint,
                                                  const std::string & sPublishEndpoint)

{
  if(!pImpl_->pControllerImpl_)
    {
      pImpl_->pControllerImpl_.reset(new ControllerImpl{logService,
            logClient,
            sServiceEndpoint,
            sPublishEndpoint});
    }
  else
    {
      throw Toolkit::Exception{"Controller already created"};
    }
}


void
OpenTestPoint::ProbeBuilder::buildPluginProbe(const std::string & sNodeId,
                                              const std::string & sLibrary,
                                              const std::chrono::seconds & probeRate,
                                              const std::chrono::seconds & commTimeout,
                                              const std::string & sConfigurationFile)
{
  if(pImpl_->pControllerImpl_)
    {
      pImpl_->pControllerImpl_->add(new ProbeContainer{pImpl_->uuid_,
            sNodeId,
            pImpl_->probeIndex_++,
            sLibrary,
            probeRate,
            commTimeout},
        sConfigurationFile);
    }
  else
    {
      throw Toolkit::Exception{"Controller not created"};
    }
}

void
OpenTestPoint::ProbeBuilder::buildPythonProbe(const std::string & sNodeId,
                                              const std::string & sModule,
                                              const std::string & sClass,
                                              const std::chrono::seconds & probeRate,
                                              const std::chrono::seconds & commTimeout,
                                              const std::string & sConfigurationFile)
{
  if(pImpl_->pControllerImpl_)
    {
      pImpl_->pControllerImpl_->add(new ProbeContainer{pImpl_->uuid_,
            sNodeId,
            pImpl_->probeIndex_++,
            sModule,
            sClass,
            probeRate,
            commTimeout},
        sConfigurationFile);
    }
  else
    {
      throw Toolkit::Exception{"Controller not created"};
    }
}

OpenTestPoint::Controller * OpenTestPoint::ProbeBuilder::getController()
{
  if(pImpl_->pControllerImpl_)
    {
      return pImpl_->pControllerImpl_.release();
    }
  else
    {
      throw Toolkit::Exception{"Controller not created"};
    }
}
