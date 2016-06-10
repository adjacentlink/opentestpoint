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
#include "otestpoint/brokerbuilder.h"

#include "brokerdirector.h"

class Daemon : public OpenTestPoint::Toolkit::LifeCycleApplication<OpenTestPoint::BrokerBuilder,
                                                                   OpenTestPoint::BrokerDirector>
{
public:
  Daemon(const std::string & sName):
    LifeCycleApplication{sName}{};

  ~Daemon(){}

  std::string doGetPrologue() const override
  {
    return \
      "OpenTestPoint probe broker daemon.\n\n"
      "An OpenTestPoint deployment can use one or more brokers in order to create\n"
      "a tiered hierarchy of discovery and probe interfaces. This hierarchy\n"
      "makes it possible to deploy a small group of well defined brokers to\n"
      "minimize the amount of client tool configuration and connections\n"
      "necessary to interact with an OpenTestPoint deployment.";
  }


  std::string doGetEpilogue() const override
  {
    return \
      "Sample XML Configuration\n\n"
      "<otestpoint-broker discovery='localhost:9001' publish='localhost:9002'>\n"
      "  <testpoint discovery='node-1:8881' publish='node-1:8882'/>\n"
      "  <testpoint discovery='node-2:8881' publish='node-2:8882'/>\n"
      "  <testpoint discovery='node-3:8881' publish='node-3:8882'/>\n"
      "  <testpoint discovery='node-4:8881' publish='node-4:8882'/>\n"
      "  <testpoint discovery='node-5:8881' publish='node-5:8882'/>\n"
      "</otestpoint-broker>";
  }
};

DECLARE_OPENTESTPOINT_TOOLKIT_APPLCIATION(Daemon,
                                          "otestpoint-broker");
