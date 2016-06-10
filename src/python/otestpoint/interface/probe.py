#
# Copyright (c) 2014,2016 - Adjacent Link LLC, Bridgewater, New Jersey
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
OpenTestPoint Probe API
"""

class Probe:
    """
    OpenTestPoint daemon probe interface.
    
    A probe may generate one or more distinct types of probe data
    messages.

    Each probe has access to an instance of
    otestpoint.toolkit.logger.Logger via self._logger.

    """
    def initialize(self,configurationFile=None):
        """
        Initializes a probe with an optional configuration file. Not all
        probes will require a configuration file. A probe should
        perform any resource acquisition in this method.

        This method transitions a probe to the initialize state.

        Parameters:
        configurationFile -- Optional configuration file name

        Returns:
        A list of the probe names this probe will generate.

        Exceptions:
        Throws ProbeException on error.
        """
        raise NotImplementedError

    def start(self):
        """
        Starts a probe.

        This method transitions a probe to the running state.

        Exceptions:
        Throws ProbeException on error.
        """
        raise NotImplementedError

    def stop(self):
        """
        Stops a probe.

        This method transitions a probe to the stopped state.

        Exceptions:
        Throws ProbeException on error.
        """
        raise NotImplementedError

    def destroy(self):
        """
        Destroys a probe. This method is the opposite of initialize. A
        probe should perform any resource cleanup in this method.

        This method transitions a probe to the destroyed state.

        Exceptions:
        Throws ProbeException on error.
        """
        raise NotImplementedError

    def probe(self):
        """Retrieve probe data. This method is invoked to retrieve data for
        all advertised probes.

        The method is only invoked on a probe in the running state.

        Returns: 
        A list of probe data entries. Where each probe data entry is a
        list containing four items: probe name which must match one of
        the advertised names returned in initialize(), serialized
        probe data, probe tag string which must match the protocol
        buffer description tag, probe version which must match the
        protocol buffer description version.

        Exceptions:
        Throws ProbeException on error.

        """
        raise NotImplementedError
