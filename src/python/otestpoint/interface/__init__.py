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

from __future__ import absolute_import, division, print_function
from six import add_metaclass
from .probe import Probe
from .probeexception import ProbeException

class _MeasurementOperator(object):
    def create(self,data=None):
        probe = self._probe_class()
        probe.ParseFromString(data)
        return probe

    def pprint(self,probe):
        if hasattr(self,'format'):
            return self.format(probe)
        else:
            return str(probe)

class _MetaMeasurementOperator(type):
    def __call__(self,*args, **kwds):
        return type.__call__(self,*args, **kwds)

class _Meta(type):
    _modules = {}

    @staticmethod
    def __call__(moduleName,className):
        import inspect
        from types import MethodType

        metaClass = None

        if moduleName not in _Meta._modules:
            try:
                m = __import__(moduleName,fromlist=['*'])

                _Meta._modules[moduleName] = (m,{})
            except:
                _Meta._modules[moduleName] = None


        if _Meta._modules[moduleName] != None:

            if className not in _Meta._modules[moduleName][1]:
                try:
                    measurmentClass = getattr(_Meta._modules[moduleName][0],className)

                    metaClass  = _MetaMeasurementOperator(str(className) + '_META',
                                                          (_MeasurementOperator,),
                                                          {'_probe_class' : measurmentClass})

                    _Meta._modules[moduleName][1][className] = metaClass

                    # check for defaults
                    prefix = "default_method_"

                    for name, method in inspect.getmembers(_Meta._modules[moduleName][0],
                                                           inspect.isfunction):
                        if name.startswith(prefix):
                            setattr(metaClass,
                                    name[len(prefix):],
                                    MethodType(method,
                                               metaClass))

                    # check for specific
                    prefix = "%s_method_" % className

                    for name, method in inspect.getmembers(_Meta._modules[moduleName][0],
                                                           inspect.isfunction):
                        if name.startswith(prefix):
                            setattr(metaClass,
                                    name[len(prefix):],
                                    MethodType(method,
                                               metaClass))



                except Exception as exp:
                    print(exp)
                    _Meta._modules[moduleName][1][className] = None

            else:
                metaClass = _Meta._modules[moduleName][1][className]

        return metaClass


@add_metaclass(_Meta)
class make_measurement_operator(object):
    pass
