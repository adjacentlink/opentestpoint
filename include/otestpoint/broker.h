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

#ifndef OPENTESTPOINT_BROKER_HEADER_
#define OPENTESTPOINT_BROKER_HEADER_

#include "otestpoint/toolkit/lifecycle.h"

namespace OpenTestPoint
{
  /**
   * @class Broker
   *
   * @brief Base class for specialized Brokers.
   *
   * A %TestPoint deployment can use one or more brokers in order to
   * create a tiered hierarchy of discovery and probe
   * interfaces. This hierarchy makes it possible to deploy a small
   * group of well defined brokers to minimize the amount of client
   * tool configuration and connections necessary to interact with a
   * %TestPoint deployment.
   */
  class Broker : public Toolkit::LifeCycle
  {
  public:
    /**
     * Destroys an instance
     */
    virtual ~Broker(){}

  protected:
    /**
     * Creates an instance
     */
    Broker(){}
  };
}

#endif // OPENTESTPOINT_BROKER_HEADER_
