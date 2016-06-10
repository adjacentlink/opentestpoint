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

#ifndef OPENTESTPOINT_PROBEPLUGIN_HEADER_
#define OPENTESTPOINT_PROBEPLUGIN_HEADER_

#include "otestpoint/types.h"
#include "otestpoint/probeserviceuser.h"

#include <string>


namespace OpenTestPoint
{
  /**
   * @class ProbePlugin
   *
   * @brief Plugin base class interfaces specialized by all probes.
   *
   * This interface provides 4 virtual methods used to transition a
   * plugin through its lifecycle:
   *   - initialize
   *   - start
   *   - stop
   *   - destory
   *
   * and a virtual method to query probe data:
   *   - probe
   *
   * @dot
   * digraph G {
   *  labelloc="t";
   *  label="Probe Plugin State Diagram";
   *  node [style="rounded,filled", fillcolor="yellow",shape="box"]
   *  PluginUninitialized -> PluginInitialized [label=" initialize "]
   *  PluginInitialized -> PluginRunning [label=" start "]
   *  PluginRunning -> PluginStopped [label=" stop "]
   *  PluginStopped -> PluginRunning [label=" start "]
   *  PluginStopped -> PluginDestroyed [label=" destroy "]
   * }
   * @enddot
   *
   */
  class ProbePlugin : public ProbeServiceUser
  {
  public:
    /**
     * Destroys an instance
     */
    virtual ~ProbePlugin(){};

    /**
     * Initializes a probe
     *
     * An optional configuration file name may be specified. The
     * presence of a configuration file name is optional from the
     * framework's perspective, it may be required by the plugin
     * implementation.
     *
     * @param sConfigurationFile Configuration file name.
     *
     * @return A list of probes names this probe publishes.
     *
     * @throws Toolkit::Exception on error
     */
    virtual ProbeNames initialize(const std::string & sConfigurationFile = "") = 0;

    /**
     * Starts the probe
     *
     * @throws Toolkit::Exception on error
     */
    virtual void start() = 0;

    /**
     * Stops the probe
     *
     * @throws Toolkit::Exception on error
     */
    virtual void stop() = 0;

    /**
     * Destroys the probe
     */
    virtual void destroy() = 0;

    /**
     * Retrieves the current probe data
     *
     * @return List of tuples. Each tuple contains the probe name,
     * the probe message serialization, the probe message tag and
     * the probe message version.
     */
    virtual ProbeData probe() = 0;

    /**
     * Gets the probe index
     *
     * @return index
     */
    ProbeIndex getIndex() const
    {
      return probeIndex_;
    }

  protected:
    /**
     * Creates an instance
     *
     * @param probeIndex %Probe index assigned by the %Controller.
     */
    ProbePlugin(ProbeIndex probeIndex):
      probeIndex_{probeIndex}{};

  private:
    ProbeIndex probeIndex_;
  };
}

#define DECLARE_PROBEPLUGIN(X)                  \
  extern "C" OpenTestPoint::ProbePlugin *       \
  create(OpenTestPoint::ProbeIndex index)       \
  {return new X{index};}

#endif // OPENTESTPOINT_PROBEPLUGIN_HEADER_
