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

#ifndef OPENTESTPOINT_BROKERBUILDER_HEADER_
#define OPENTESTPOINT_BROKERBUILDER_HEADER_

#include "otestpoint/broker.h"
#include "otestpoint/toolkit/log/service.h"
#include "otestpoint/toolkit/log/client.h"

#include <string>
#include <uuid.h>

namespace OpenTestPoint
{
  /**
   * @class BrokerBuilder
   *
   * @brief Builder used to build a Broker instance.
   *
   * The BrokerBuilder is part of a Builder/Director design pattern
   * for abstracting construction from configuration.
   */
  class BrokerBuilder
  {
  public:
    /**
     * Creates an instance
     *
     * @param uuid UUID of the broker
     */
    BrokerBuilder(const uuid_t & uuid);

    /**
     * Builds a Broker instance from a python module
     *
     * @param logService Log service instance used to create
     * additional log clients. Shared reference with the main
     * application.
     * @param logClient Log client instance used to log
     * output. Shared reference with the main application.
     * @param sDiscoveryEndpoint %Broker's discovery service 0MQ REQ
     * socket endpoint. IPv4 or IPv6.
     * @param sPublishEndpoint %Broker's probe report 0MQ PUB
     * socket endpoint. IPv4 or IPv6.
     *
     * @throws Toolkit::Exception on build error.
     */
    void buildBroker(Toolkit::Log::Service & logService,
                     Toolkit::Log::Client & logClient,
                     const std::string & sDiscoveryEndpoint,
                     const std::string & sPublishEndpoint);

    /**
     * Adds a %TestPoint instance to proxy
     *
     * @param sDiscoveryEndpoint %Controller discovery 0MQ REQ socket
     * endpoint. IPv4 or IPv6.
     * @param sPublishEndpoint %Controller report 0MQ PUB socket
     * endpoint. IPv4 or IPv6.
     *
     * @throws Toolkit::Exception on build error.
     */
    void addTestPoint(const std::string & sDiscoveryEndpoint,
                      const std::string & sPublishEndpoint);

    /**
     * Gets the Broker instance.
     *
     * @return Pointer to the created Broker which has been
     * populated with TestPoints.
     *
     * @throws Toolkit::Exception on build error.
     *
     * @note Ownership is transferred to the caller.
     */
    Broker * getBroker();

  private:
    class Impl;
    Impl * pImpl_;
  };
}

#endif // OPENTESTPOINT_BROKERBUILDER_HEADER_
