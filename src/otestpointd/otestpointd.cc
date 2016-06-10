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

#include "otestpoint/toolkit/lifecycleapplication.h"
#include "otestpoint/probebuilder.h"
#include "probedirector.h"


class Daemon : public OpenTestPoint::Toolkit::LifeCycleApplication<OpenTestPoint::ProbeBuilder,
                                                                   OpenTestPoint::ProbeDirector>
{
public:
  Daemon(const std::string & sName):
    LifeCycleApplication{sName}{};

  std::string doGetPrologue() const override
  {
    return \
      "OpenTestPoint probe controller daemon.\n\n"
      "The otestpointd application contains a single Controller\n"
      "that instantiates and manages one or more probes based on input XML\n"
      "configuration. Each probe is created within its own isolated process,\n"
      "otestpoint-probe, which acts as a container for the plugin and\n"
      "provides process isolation between probes.";
  }


  std::string doGetEpilogue() const override
  {
    return \
      "Sample XML configuration:\n\n"
      "<otestpoint id='node-1' discovery='node-1:8881' publish='node-1:8882'>\n"
      "  <probe>\n"
      "    <plugin library='libprobeplugintimeofday.so'/>\n"
      "  </probe>\n"
      "  <probe configuration='probe-emane-physicallayer.xml'>\n"
      "    <python module='adjacentlink.testpoint.emane'\n"
      "            class='PhysicalLayer'/>\n"
      "  </probe>\n"
      "  <probe configuration='probe-resources-processcpumeminfo.xml'>\n"
      "    <python module='adjacentlink.testpoint.resources'\n"
      "            class='ProcessCPUMemInfo'/>\n"
      "  </probe>\n"
      "  <probe>\n"
      "    <python module='adjacentlink.testpoint.resources'\n"
      "            class='SystemCPUMemInfo'/>\n"
      "  </probe>\n"
      "</otestpoint>\n";
  }


  ~Daemon(){}
};


DECLARE_OPENTESTPOINT_TOOLKIT_APPLCIATION(Daemon,
                                          "otestpointd");
