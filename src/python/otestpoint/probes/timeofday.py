#
# Copyright (c) 2014,2016,2019 - Adjacent Link LLC, Bridgewater,
# New Jersey
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#  * Neither the name of Adjacent Link LLC nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# See toplevel COPYING for more information.
#

"""
Simple Time of Day probe example
"""

from __future__ import absolute_import, division, print_function
from otestpoint.interface import Probe
import otestpoint.toolkit.logger as Logger
from .timeofday_pb2 import Measurement_timeofday
import time

class TimeOfDay(Probe):
    def initialize(self,configurationFile=None):
        """
        Initialize the probe.

        Returns:
        The probe name list.
        """
        self._logger.log(Logger.DEBUG_LEVEL,"/TimeOfDay initialize configuration: %s" % configurationFile)
        return ("Probes.TimeOfDay",)

    def start(self):
        """
        Starts the probe.

        This method does nothing.
        """
        self._logger.log(Logger.DEBUG_LEVEL,'/TimeOfDay start')

    def stop(self):
        """
        Stops the probe.

        This method does nothing.
        """
        self._logger.log(Logger.DEBUG_LEVEL,'/TimeOfDay stop')

    def destroy(self):
        """
        Destroys the probe.

        This method does nothing.
        """
        self._logger.log(Logger.DEBUG_LEVEL,'/TimeOfDay destroy')

    def probe(self):
        """
        Gets the current time of day probe data
        """
        self._logger.log(Logger.DEBUG_LEVEL,'/TimeOfDay probe')

        probeData = Measurement_timeofday()

        probeData.microsecondsSinceEpoch = int(time.time() * 1000000)

        return (("Probes.TimeOfDay",
                 probeData.SerializeToString(),
                 probeData.description.name,
                 probeData.description.module,
                 probeData.description.version),)


def Measurement_timeofday_method_format(self,probe):
    """
    Generates formatted time of day data.
    """
    return "time = %s" % probe.microsecondsSinceEpoch

def Measurement_timeofday_method_diff(self,probe):
    """
    Computes diff between time values and caches last time.
    """
    delta = 0

    if hasattr(self,'_last'):
        delta = probe.microsecondsSinceEpoch - self._last;

    self._last = probe.microsecondsSinceEpoch

    return delta
