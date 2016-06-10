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

#ifndef OPENTESTPOINT_PROBEBUILDER_HEADER_
#define OPENTESTPOINT_PROBEBUILDER_HEADER_

#include "otestpoint/controller.h"
#include "otestpoint/toolkit/log/service.h"
#include "otestpoint/toolkit/log/client.h"

#include <string>
#include <uuid.h>

namespace OpenTestPoint
{
  /**
   * @class ProbeBuilder
   *
   * @brief Builder used to build a Controller instance managing
   * C++ and Python Probe instances.
   *
   * All probes running on a single node are managed by a
   * Controller. The ProbeBuilder is part of a Builder/Director
   * design pattern for abstracting construction from configuration.
   */
  class ProbeBuilder
  {
  public:
    /**
     * Creates a ProbeBuilder instance
     *
     * @param uuid UUID of the controller
     */
    ProbeBuilder(const uuid_t & uuid);

    /**
     * Builds a Controller instance
     *
     * @param logService Log service instance used to create
     * additional log clients. Shared reference with the main
     * application.
     * @param logClient Log client instance used to log
     * output. Shared reference with the main application.
     * @param sServiceEndpoint Discovery service 0MQ REQ socket
     * endpoint. IPv4 or IPv6.
     * @param sPublishEndpoint %Probe report 0MQ PUB socket
     * endpoint. IPv4 or IPv6.
     *
     * @throws Toolkit::Exception on build error.
     */
    void buildController(Toolkit::Log::Service & logService,
                         Toolkit::Log::Client & logClient,
                         const std::string & sServiceEndpoint,
                         const std::string & sPublishEndpoint);

    /**
     * Builds a probe instance from a C++ plugin
     *
     * @param sNodeId Controller id. Same for all controller probes.
     * @param sLibrary Name of the plugin library.
     * @param probeRate %Probe collection rate
     * @param commTimeout %Probe communication timeout threshold
     * @param sConfigurationFile Name of the plugin configuration
     * file. May be an empty string.
     *
     * @throws Toolkit::Exception on build error.
     */
    void buildPluginProbe(const std::string & sNodeId,
                          const std::string & sLibrary,
                          const std::chrono::seconds & probeRate,
                          const std::chrono::seconds & commTimeout,
                          const std::string & sConfigurationFile);

    /**
     * Builds a probe instance from a python module
     *
     * @param sNodeId Controller id. Same for all controller probes.
     * @param sModule Python module name.
     * @param sClass Python class name specializing
     * adjacentlink.testpoint.Probe.
     * @param probeRate %Probe collection rate
     * @param commTimeout %Probe communication timeout threshold
     * @param sConfigurationFile Name of the plugin configuration
     * file. May be an empty string.
     *
     * @throws Toolkit::Exception on build error.
     */
    void buildPythonProbe(const std::string & sNodeId,
                          const std::string & sModule,
                          const std::string & sClass,
                          const std::chrono::seconds & probeRate,
                          const std::chrono::seconds & commTimeout,
                          const std::string & sConfigurationFile);

    /**
     * Gets the Controller instance.
     *
     * @return Pointer to the created Controller which has been
     * populated with probes.
     *
     * @throws Toolkit::Exception on build error.
     *
     * @note Ownership is transferred to the caller.
     */
    Controller * getController();

  private:
    class Impl;
    Impl * pImpl_;
  };
}

#endif // OPENTESTPOINT_PROBEBUILDER_HEADER_
