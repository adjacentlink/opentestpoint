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

#include "otestpoint/brokerbuilder.h"
#include "otestpoint/toolkit/exception.h"

#include "brokerimpl.h"

#include <memory>
#include <uuid.h>

class OpenTestPoint::BrokerBuilder::Impl
{
public:
  Impl(const uuid_t & uuid)
  {
    uuid_copy(uuid_,uuid);
  }

  uuid_t uuid_;
  std::unique_ptr<BrokerImpl> pBrokerImpl_;
};

OpenTestPoint::BrokerBuilder::BrokerBuilder(const uuid_t & uuid):
  pImpl_{new Impl{uuid}}{}


void OpenTestPoint::BrokerBuilder::buildBroker(Toolkit::Log::Service & logService,
                                               Toolkit::Log::Client & logClient,
                                               const std::string & sServiceEndpoint,
                                               const std::string & sPublishEndpoint)
{
  if(!pImpl_->pBrokerImpl_)
    {
      pImpl_->pBrokerImpl_.reset(new BrokerImpl{logService,
            logClient,
            sServiceEndpoint,
            sPublishEndpoint});
    }
  else
    {
      throw Toolkit::Exception{"Broker already created"};
    }
}

void OpenTestPoint::BrokerBuilder::addTestPoint(const std::string & sDiscoveryEndpoint,
                                                const std::string & sPublishEndpoint)
{
  if(pImpl_->pBrokerImpl_)
    {
      pImpl_->pBrokerImpl_->add(sDiscoveryEndpoint,sPublishEndpoint);
    }
  else
    {
      throw Toolkit::Exception{"Broker not created"};
    }
}

OpenTestPoint::Broker * OpenTestPoint::BrokerBuilder::getBroker()
{
  if(pImpl_->pBrokerImpl_)
    {
      return pImpl_->pBrokerImpl_.release();
    }
  else
    {
      throw Toolkit::Exception{"Broker not created"};
    }
}
