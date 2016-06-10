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
#include <Python.h>
#include "otestpoint/toolkit/stringto.h"
#include "probemanager.h"
#include <google/protobuf/stubs/common.h>
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <uuid.h>

#ifndef HAVE_SECURE_GETENV
#  ifdef HAVE___SECURE_GETENV
#    define secure_getenv __secure_getenv
#  else
#    error neither secure_getenv nor __secure_getenv is available
#  endif
#endif

int main()
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  Py_InitializeEx(0);
  PyEval_InitThreads();

  try
    {
      const char * pzStatus = secure_getenv("status");
      const char * pzNodeId = secure_getenv("nodeid");
      const char * pzProbeIndex = secure_getenv("probeindex");
      const char * pzProbeRate = secure_getenv("proberate");
      const char * pzUUID = secure_getenv("uuid");

      if(!pzStatus || !pzNodeId || !pzProbeIndex ||
         !pzUUID || !pzProbeRate)
        {
          std::cerr<<"Error otestpoint-probe is for use by libotestpoint."<<std::endl;
          return EXIT_FAILURE;
        }

      sigignore(SIGINT);
      sigignore(SIGQUIT);

      uuid_t uuid;

      uuid_parse(pzUUID,uuid);

      OpenTestPoint::ProbeManager probeManager{pzStatus,
          pzNodeId,
          OpenTestPoint::Toolkit::strToUINT16(pzProbeIndex),
          uuid,
          OpenTestPoint::Toolkit::strToUINT16(pzProbeRate)};

      probeManager.run();

    }
  catch(std::exception & exp)
    {
      std::cerr<<exp.what()<<std::endl;

      return EXIT_FAILURE;
    }

  Py_Finalize();
}
