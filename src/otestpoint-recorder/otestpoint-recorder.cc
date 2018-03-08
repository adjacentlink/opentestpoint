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

#include "otestpoint/toolkit/lifecycleapplication.h"
#include "otestpoint/recorderbuilder.h"
#include "recorderdirector.h"

class Daemon : public OpenTestPoint::Toolkit::LifeCycleApplication<OpenTestPoint::RecorderBuilder,
                                                                   OpenTestPoint::RecorderDirector>
{
public:
  Daemon(const std::string & sName):
    LifeCycleApplication{sName}{};

  ~Daemon(){}

  std::string doGetPrologue() const override
  {
    return \
      "OpenTestPoint probe recorder daemon.\n\n"
      "OpenTestPoint recorders can be used as part of an OpenTestPoint\n"
      "deployment to record all probes available from one or more publishers.\n"
      "The otestpoint-recorder application can be configured with one or more\n"
      "publisher endpoints. For each listed endpoint, otestpoint-recorder will\n"
      "subscribe to all messages and log all received probes in Probe Message\n"
      "Stream Format.";
  }


  std::string doGetEpilogue() const override
  {
    return \
      "Sample XML Configuration\n\n"
      "<otestpoint-recorder file='persist/1/var/log/testpoint-recorder.data'>\n"
      "  <testpoint publish='node-1:8882'/>\n"
      "</otestpoint-recorder>\n\n"
      "Probe Message Stream Format\n\n"
      "Probe Message Stream Format uses length prefix framing, where the\n"
      "length of the serialized ProbeReport message is output as an unsigned\n"
      "32-bit integer value (4 bytes) in network byte order preceding the\n"
      "output of the serialized message\n\n"
      "Recording SQLite Database\n\n"
      "In addition to a flat log file, otestpoint-recorder creates an SQLite\n"
      "database that contains probe meta information and probe log file offsets\n"
      "to make it easier to find specific probes of interest. The database\n"
      "contains a single probes table with the following items:\n\n"
      " time  -  Probe timestamp in seconds since the epoch.\n"
      " uuid  -  The UUID of the Controller owning the reporting probe\n"
      "          instance.\n"
      " probe -  The name of the probe.\n"
      " tag   -  The tag assigned to the Controller owning the reporting probe\n"
      "          instance.\n"
      " pindex - The index of the probe. Each probe instantiated by the same\n"
      "          Controller is assigned a unique sequential index.\n"
      " offset - The offset of the serialized probe message. (Note: This\n"
      "          offset points to the start of the serialized probe message\n"
      "          not the length field of the entry in Probe Message Stream\n"
      "          Format.)\n"
      " size -   The size of the serialized probe message.\n\n"
      "The SQLite database file will have the same name as the log file with\n"
      "an additional .db extension appended.";
  }
};

DECLARE_OPENTESTPOINT_TOOLKIT_APPLCIATION(Daemon,
                                          "otestpoint-recorder");
