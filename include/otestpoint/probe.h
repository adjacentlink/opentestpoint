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

#ifndef OPENTESTPOINT_PROBE_HEADER_
#define OPENTESTPOINT_PROBE_HEADER_

#include "otestpoint/types.h"
#include "otestpoint/toolkit/log/client.h"

#include <string>

namespace OpenTestPoint
{
  /**
   * @class Probe
   *
   * @brief C++ and Python probe wrapper class.
   *
   * This class wraps all C++ and Python probes to provide a common
   * interface to the respective probe containers.
   */
  class Probe
  {
  public:
    /**
     * Destroys an instance
     */
    virtual ~Probe();

    /**
     * Initializes a probe
     *
     * @param sConfigurationFile Configuration file name. May be an
     * empty string.
     *
     * @throws Toolkit::Excpetion on error.
     */
    virtual ProbeNames initialize(const std::string & sConfigurationFile = "") = 0;

    /**
     * Starts a probe
     *
     * @throws Toolkit::Excpetion on error.
     */
    virtual void start() = 0;

    /**
     * Stops a probe
     *
     * @throws Toolkit::Excpetion on error.
     */
    virtual void stop() = 0;

    /**
     * Destroys a probe
     *
     * @throws Toolkit::Excpetion on error.
     */
    virtual void destroy() = 0;

    /**
     * Gets the internal probe control endpoint
     *
     * @return control endpoint
     */
    std::string getProbeControlEndpoint() const;

    /**
     * Gets the internal probe report endpoint
     *
     * @return report endpoint
     */
    std::string getProbePublishEndpoint() const;

    /**
     * Gets the internal log control endpoint
     *
     * @return control endpoint
     */
    std::string getLogControlEndpoint() const;

    /**
     * Gets the internal log publish endpoint
     *
     * @return log endpoint
     */
    std::string getLogPublishEndpoint() const;


    /**
     * Gets the node id.
     *
     * @return id
     */
    std::string getNodeId() const;

    /**
     * Gets the probe index
     *
     * @return index
     */
    ProbeIndex getProbeIndex() const;

  protected:
    /**
     * Creates an instance
     *
     * @param sNodeId Controller node id
     * @param probeIndex %Probe index
     *
     * @throws Toolkit::Exception on build error.
     */

    Probe(const std::string & sNodeId,
          ProbeIndex probeIndex);

    const std::string sNodeId_;
    const ProbeIndex probeIndex_;
    std::string sProbeControlEndpoint_;
    std::string sProbePublishEndpoint_;
    std::string sLogControlEndpoint_;
    std::string sLogPublishEndpoint_;
  };
}

#include "otestpoint/probe.inl"

#endif // OPENTESTPOINT_PROBE_HEADER_
