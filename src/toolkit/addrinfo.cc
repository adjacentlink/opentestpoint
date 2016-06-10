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

#include "otestpoint/toolkit/addrinfo.h"
#include "otestpoint/toolkit/stringto.h"
#include "otestpoint/toolkit/exception.h"

#include <netdb.h>
#include <arpa/inet.h>
#include <limits.h>
#include <cstring>
#include <sstream>
#include <algorithm>

std::string
OpenTestPoint::Toolkit::getHostAddressAsString(const std::string & sHostName,
                                               bool bEndPoint)
{
  std::string sAddress{};
  std::string sHost{sHostName};
  std::uint16_t u16Port{};
  struct addrinfo hints;
  struct addrinfo * pInfo{};
  struct addrinfo * pEntry{};
  char buf[1024] = {};

  if(bEndPoint)
    {
      size_t pos = sHostName.rfind("/");

      if(pos == std::string::npos)
        {
          // only used ':' as a port seperator for
          // hostname or IPv4 addresses
          if(std::count(sHostName.begin(),
                        sHostName.end(),
                        ':') == 1)
            {
              pos = sHostName.rfind(":");
            }
        }

      if(pos != std::string::npos)
        {
          sHost = sHostName.substr(0,pos);
          u16Port = strToUINT16(sHostName.substr(pos+1));
        }
      else
        {
          throw Exception{"Error invalid endpoint format: %s",sHostName.c_str()};
        }
    }

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if(!getaddrinfo(sHost.c_str(),nullptr,&hints,&pInfo))
    {
      for(pEntry = pInfo; pEntry != nullptr; pEntry = pEntry->ai_next)
        {
          if(pEntry->ai_family == AF_INET)
            {
              if(inet_ntop(pEntry->ai_family,
                           &reinterpret_cast<struct sockaddr_in*>(pEntry->ai_addr)->sin_addr,
                           buf,
                           sizeof(buf)))
                {
                  sAddress = buf;
                  break;
                }
            }
          else if(pEntry->ai_family == AF_INET6)
            {
              if(inet_ntop(pEntry->ai_family,
                           &reinterpret_cast<struct sockaddr_in6*>(pEntry->ai_addr)->sin6_addr,
                           buf,
                           sizeof(buf)))
                {
                  sAddress = buf;
                  break;
                }
            }
          else
            {
              throw Exception{"Error unkown address faimly for: %s",sHost.c_str()};
            }

        }

      freeaddrinfo(pInfo);
    }
  else
    {
      throw Exception{"Error unable to resolve address for: %s",sHost.c_str()};
    }

  if(bEndPoint)
    {
      std::stringstream ssAddress{};

      if(std::count(sAddress.begin(),sAddress.end(),':'))
        {
          // IPv6
          ssAddress<<'['<<sAddress<<"]:"<<u16Port;

        }
      else
        {
          // IPv4
          ssAddress<<sAddress<<':'<<u16Port;
        }

      sAddress = ssAddress.str();
    }

  return sAddress;
}
