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

#include "timeofday.h"
#include "timeofday.pb.h"

#include <chrono>

OpenTestPoint::TimeOfDay::TimeOfDay(ProbeIndex probeIndex):
  ProbePlugin{probeIndex}{}

OpenTestPoint::TimeOfDay::~TimeOfDay(){}

OpenTestPoint::ProbeNames
OpenTestPoint::TimeOfDay::initialize(const std::string & sConfigurationFile)
{
  OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pProbeService_,
                                       "/probe/timeofday initialize configuration: %s",
                                       sConfigurationFile.c_str());

  return {"Probes.TimeOfDay"};
}

void OpenTestPoint::TimeOfDay::start()
{
  OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pProbeService_,
                                       "/probe/timeofday start");

}

void OpenTestPoint::TimeOfDay::stop()
{
  OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pProbeService_,
                                       "/probe/timeofday stop");
}

void OpenTestPoint::TimeOfDay::destroy()
{
  OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pProbeService_,
                                       "/probe/timeofday destroy");
}

OpenTestPoint::ProbeData
OpenTestPoint::TimeOfDay::probe()
{
  OPENTESTPOINT_PROBESERVICE_LOG_DEBUG(pProbeService_,
                                       "/probe/timeofday probe");

  OpenTestPoint::Measurement_timeofday data;

  auto now =
    std::chrono::duration_cast<std::chrono::microseconds>
    (std::chrono::high_resolution_clock::now().time_since_epoch());

  data.set_microsecondssinceepoch(now.count());

  std::string sSerialization{};

  data.SerializeToString(&sSerialization);

  return {std::make_tuple("Probes.TimeOfDay",
                          sSerialization,
                          data.description().name(),
                          data.description().module(),
                          data.description().version())};
}

DECLARE_PROBEPLUGIN(OpenTestPoint::TimeOfDay)
