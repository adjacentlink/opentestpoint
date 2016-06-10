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
 * Derived work.
 *
 * Copyright (c) 2013-2014 - Adjacent Link LLC, Bridgewater, New Jersey
 * Copyright (c) 2009,2012 - DRS CenGen, LLC, Columbia, Maryland
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of DRS CenGen, LLC nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
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
 */

#include "otestpoint/toolkit/exception.h"

#include <cstdlib>
#include <cstring>

namespace
{
  std::string scaleNumericalStringRepresentation(const std::string & sValue)
  {
    std::string sTmpParameter(sValue);

    std::uint8_t u8PowerOf10 = 0;

    switch(*(sValue.end() - 1))
      {
      case 'G':
        sTmpParameter.assign(sValue,0,sValue.size() - 1);
        u8PowerOf10 = 9;
        break;

      case 'M':
        sTmpParameter.assign(sValue,0,sValue.size() - 1);
        u8PowerOf10 = 6;
        break;

      case 'K':
        sTmpParameter.assign(sValue,0,sValue.size() - 1);
        u8PowerOf10 = 3;
        break;
      }

    if(u8PowerOf10 != 0)
      {
        // location of decimal point, if exists
        std::string::size_type indexPoint =  sTmpParameter.find(".",0);

        if(indexPoint != std::string::npos)
          {
            std::string::size_type numberOfDigitsAfterPoint =
              sTmpParameter.size() - indexPoint - 1;

            if(numberOfDigitsAfterPoint > u8PowerOf10)
              {
                // need to move the decimal point, enough digits are present
                sTmpParameter.insert(sTmpParameter.size() - (numberOfDigitsAfterPoint - u8PowerOf10),
                                     ".");
              }
            else
              {
                // need to append 0s
                sTmpParameter.append(u8PowerOf10 - numberOfDigitsAfterPoint,'0');
              }

            // remove original decimal point
            sTmpParameter.erase(indexPoint,1);
          }
        else
          {
            // need to append 0s
            sTmpParameter.append(u8PowerOf10,'0');
          }
      }

    return sTmpParameter;
  }
}

inline
std::int64_t OpenTestPoint::Toolkit::strToINT64(const std::string & sValue,
                                                std::int64_t i64Min,
                                                std::int64_t i64Max)
{
  long long llValue = 0;

  if(sValue.empty())
    {
      throw Exception{"Empty string in numeric conversion"};
    }
  else
    {
      std::string sTmpParameter(scaleNumericalStringRepresentation(sValue));

      char * pEnd = 0;

      //Clear errno before making call because Ubuntu does not
      //clear it when a call is made
      errno = 0;

      llValue =  std::strtoll(sTmpParameter.c_str(),&pEnd,0);

      if(errno == ERANGE ||
         llValue < i64Min ||
         llValue > i64Max)
        {
          throw Exception{"Error %s out of range [%jd,%jd]",sValue.c_str(),i64Min,i64Min};
        }
      else if(pEnd != 0 && *pEnd !='\0')
        {
          throw Exception{"Error invalid character in numeric: '%c'",*pEnd};
        }
    }

  return llValue;
}

inline
std::uint64_t OpenTestPoint::Toolkit::strToUINT64(const std::string & sValue,
                                                  std::uint64_t u64Min,
                                                  std::uint64_t u64Max)
{
  unsigned long long ullValue = 0;

  if(sValue.empty())
    {
      throw Exception{"Empty string in numeric conversion"};
    }
  else
    {
      std::string sTmpParameter(scaleNumericalStringRepresentation(sValue));

      char * pEnd = 0;

      //Clear errno before making call because Ubuntu does not
      //clear it when a call is made
      errno = 0;

      ullValue =  std::strtoull(sTmpParameter.c_str(),&pEnd,0);

      if(errno == ERANGE ||
         ullValue < u64Min ||
         ullValue > u64Max)
        {
          throw Exception{"Error %s out of range [%ju,%ju]",sValue.c_str(), u64Min,u64Max};
        }
      else if(pEnd != 0 && *pEnd !='\0')
        {
          throw Exception{"Error invalid character in numeric: '%c'",*pEnd};
        }
    }

  return ullValue;
}

inline
std::int32_t OpenTestPoint::Toolkit::strToINT32(const std::string & sValue,
                                                std::int32_t i32Min,
                                                std::int32_t i32Max)
{
  return static_cast<std::int32_t>(strToINT64(sValue,i32Min,i32Max));
}

inline
std::uint32_t OpenTestPoint::Toolkit::strToUINT32(const std::string & sValue,
                                                  std::uint32_t u32Min,
                                                  std::uint32_t u32Max)
{
  return static_cast<std::uint32_t>(strToUINT64(sValue,u32Min,u32Max));
}

inline
std::int16_t OpenTestPoint::Toolkit::strToINT16(const std::string & sValue,
                                                std::int16_t i16Min,
                                                std::int16_t i16Max)
{
  return static_cast<std::int16_t>(strToINT64(sValue,i16Min,i16Max));
}

inline
std::uint16_t OpenTestPoint::Toolkit::strToUINT16(const std::string & sValue,
                                                  std::uint16_t u16Min,
                                                  std::uint16_t u16Max)
{
  return static_cast<std::uint16_t>(strToUINT64(sValue,u16Min,u16Max));
}

inline
std::int8_t OpenTestPoint::Toolkit::strToINT8(const std::string & sValue,
                                              std::int8_t i8Min,
                                              std::int8_t i8Max)
{
  return static_cast<std::int8_t>(strToINT64(sValue,i8Min,i8Max));
}

inline
std::uint8_t OpenTestPoint::Toolkit::strToUINT8(const std::string & sValue,
                                                std::uint8_t u8Min,
                                                std::uint8_t u8Max)
{
  return static_cast<std::uint8_t>(strToUINT64(sValue,u8Min,u8Max));
}

inline
bool OpenTestPoint::Toolkit::strToBool(const std::string & sValue)
{
  if(!strcasecmp(sValue.c_str(),"on")   ||
     !strcasecmp(sValue.c_str(),"yes")  ||
     !strcasecmp(sValue.c_str(),"true") ||
     !strcasecmp(sValue.c_str(),"1"))
    {
      return true;
    }
  else if(!strcasecmp(sValue.c_str(),"off")   ||
          !strcasecmp(sValue.c_str(),"no")  ||
          !strcasecmp(sValue.c_str(),"false") ||
          !strcasecmp(sValue.c_str(),"0"))
    {
      return false;
    }
  else
    {
      throw Exception{"Error invalid boolean conversion: '%s'",
          sValue.c_str()};
    }
}

inline
double OpenTestPoint::Toolkit::strToDouble(const std::string & sValue,
                                           double dMin,
                                           double dMax)
{
  double dValue = 0;

  if(sValue.empty())
    {
      throw Exception{"Error empty string in numeric conversion"};
    }
  else
    {
      std::string sTmpParameter(scaleNumericalStringRepresentation(sValue));

      char * pEnd = 0;

      //Clear errno before making call because Ubuntu does not
      //clear it when a call is made
      errno = 0;

      dValue =  std::strtod(sTmpParameter.c_str(),&pEnd);

      if(errno == ERANGE ||
         dValue < dMin ||
         dValue > dMax)
        {
          throw Exception{"Error out of range [%lf,%lf]",dMin,dMax};
        }
      else if(pEnd != 0 && *pEnd !='\0')
        {
          throw Exception{"Error invalid character in numeric: '%c'",*pEnd};
        }
    }

  return dValue;
}

inline
float OpenTestPoint::Toolkit::strToFloat(const std::string & sValue,
                                         float fMin,
                                         float fMax)
{
  return static_cast<float>(strToDouble(sValue,fMin,fMax));
}
