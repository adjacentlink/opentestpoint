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

#include "pluginprobeadapter.h"
#include "otestpoint/toolkit/exception.h"

#include <dlfcn.h>

OpenTestPoint::PluginProbeAdapter::PluginProbeAdapter(ProbeIndex probeIndex,
                                                      const std::string & sLibrary,
                                                      ProbeService * pProbeService):
  ProbePlugin{probeIndex}
{
  using CreateFunction = ProbePlugin * (*)(ProbeIndex);

  CreateFunction pCreateFunction{};

  if((pLib_ = dlopen(sLibrary.c_str(),RTLD_NOW)) == 0)
    {
      throw Toolkit::Exception{"Unable to create %s: %s",
          sLibrary.c_str(),
          dlerror()};
    }

  if((pCreateFunction = reinterpret_cast<CreateFunction>((void (*)(int))(dlsym(pLib_,"create")))) == 0)
    {
      dlclose(pLib_);
      throw Toolkit::Exception{"%s  missing create symbol",sLibrary.c_str()};
    }

  pPlugin_ = pCreateFunction(probeIndex);

  pPlugin_->setProbeService(pProbeService);
}

OpenTestPoint::PluginProbeAdapter::~PluginProbeAdapter()
{
  delete pPlugin_;
  dlclose(pLib_);
}

OpenTestPoint::ProbeNames
OpenTestPoint::PluginProbeAdapter::initialize(const std::string & sConfigurationFile)
{
  return pPlugin_->initialize(sConfigurationFile);
}

void OpenTestPoint::PluginProbeAdapter::start()
{
  pPlugin_->start();
}

void OpenTestPoint:: PluginProbeAdapter::stop()
{
  pPlugin_->stop();
}

void OpenTestPoint::PluginProbeAdapter::destroy()
{
  pPlugin_->destroy();
}

OpenTestPoint::ProbeData
OpenTestPoint::PluginProbeAdapter::probe()
{
  return pPlugin_->probe();
}
