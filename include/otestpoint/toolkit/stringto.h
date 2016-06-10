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
 * Copyright (c) 2009 - DRS CenGen, LLC, Columbia, Maryland
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

#ifndef OPENTESTPOINT_TOOLKIT_FROMSTRING_HEADER_
#define OPENTESTPOINT_TOOLKIT_FROMSTRING_HEADER_

#include <string>
#include <cstdint>
#include <limits>

namespace OpenTestPoint
{
  namespace Toolkit
  {
    /**
     * Convert string to an std::int64_t
     *
     * @param sValue String to convert
     * @param i64Min Minimum value in range
     * @param i64Max Maximum value in range
     *
     * @return std::int64_t value
     *
     * @exception FatalException Thrown when an error is encountered during
     * conversion either to input format or out of range value.
     */
    std::int64_t strToINT64(const std::string & sValue,
                            std::int64_t i64Min = std::numeric_limits<std::int64_t>::min(),
                            std::int64_t i64Max = std::numeric_limits<std::int64_t>::max());

    /**
     * Convert string to an std::uint64_t
     *
     * @param sValue String to convert
     * @param u64Min Minimum value in range
     * @param u64Max Maximum value in range
     *
     * @return std::uint64_t value
     *
     * @exception FatalException Thrown when an error is encountered during
     * conversion either to input format or out of range value.
     */
    std::uint64_t strToUINT64(const std::string & sValue,
                              std::uint64_t u64Min = std::numeric_limits<std::uint64_t>::min(),
                              std::uint64_t u64Max = std::numeric_limits<std::uint64_t>::max());


    /**
     * Convert string to an std::int32_t
     *
     * @param sValue String to convert
     * @param i32Min Minimum value in range
     * @param i32Max Maximum value in range
     *
     * @return std::int32_t value
     *
     * @exception FatalException Thrown when an error is encountered during
     * conversion either to input format or out of range value.
     */
    std::int32_t strToINT32(const std::string & sValue,
                            std::int32_t i32Min = std::numeric_limits<std::int32_t>::min(),
                            std::int32_t i32Max = std::numeric_limits<std::int32_t>::max());

    /**
     * Convert string to an std::uint32_t
     *
     * @param sValue String to convert
     * @param u32Min Minimum value in range
     * @param u32Max Maximum value in range
     *
     * @return std::uint32_t value
     *
     * @exception FatalException Thrown when an error is encountered during
     * conversion either to input format or out of range value.
     */
    std::uint32_t strToUINT32(const std::string & sValue,
                              std::uint32_t u32Min = std::numeric_limits<std::uint32_t>::min(),
                              std::uint32_t u32Max = std::numeric_limits<std::uint32_t>::max());

    /**
     * Convert string to an std::int16_t
     *
     * @param sValue String to convert
     * @param i16Min Minimum value in range
     * @param i16Max Maximum value in range
     *
     * @return std::int16_t value
     *
     * @exception FatalException Thrown when an error is encountered during
     * conversion either to input format or out of range value.
     */
    std::int16_t strToINT16(const std::string & sValue,
                            std::int16_t i16Min = std::numeric_limits<std::int16_t>::min(),
                            std::int16_t i16Max = std::numeric_limits<std::int16_t>::max());

    /**
     * Convert string to an std::uint16_t
     *
     * @param sValue String to convert
     * @param u16Min Minimum value in range
     * @param u16Max Maximum value in range
     *
     * @return std::uint16_t value
     *
     * @exception FatalException Thrown when an error is encountered during
     * conversion either to input format or out of range value.
     */
    std::uint16_t strToUINT16(const std::string & sValue,
                              std::uint16_t u16Min = std::numeric_limits<std::uint16_t>::min(),
                              std::uint16_t u16Max = std::numeric_limits<std::uint16_t>::max());

    /**
     * Convert string to an std::int8_t
     *
     * @param sValue String to convert
     * @param i8Min Minimum value in range
     * @param i8Max Maximum value in range
     *
     * @return std::int8_t value
     *
     * @exception FatalException Thrown when an error is encountered during
     * conversion either to input format or out of range value.
     */
    std::int8_t strToINT8(const std::string & sValue,
                          std::int8_t i8Min = std::numeric_limits<std::int8_t>::min(),
                          std::int8_t i8Max = std::numeric_limits<std::int8_t>::max());

    /**
     * Convert string to an  std::uint8_t
     *
     * @param sValue String to convert
     * @param u8Min Minimum value in range
     * @param u8Max Maximum value in range
     *
     * @return  std::uint8_t value
     *
     * @exception FatalException Thrown when an error is encountered during
     * conversion either to input format or out of range value.
     */
    std::uint8_t strToUINT8(const std::string & sValue,
                            std::uint8_t u8Min = std::numeric_limits<std::uint8_t>::min(),
                            std::uint8_t u8Max = std::numeric_limits<std::uint8_t>::max());

    /**
     * Convert string to a float
     *
     * @param sValue String to convert
     * @param fMin Minimum value in range
     * @param fMax Maximum value in range
     *
     * @return float value
     *
     * @exception FatalException Thrown when an error is encountered during
     * conversion either to input format or out of range value.
     */
    float strToFloat(const std::string & sValue,
                     float fMin = std::numeric_limits<float>::lowest(),
                     float fMax = std::numeric_limits<float>::max());

    /**
     * Convert string to a double
     *
     * @param sValue String to convert
     * @param dMin Minimum value in range
     * @param dMax Maximum value in range
     *
     * @return double value
     *
     * @exception FatalException Thrown when an error is encountered during
     * conversion either to input format or out of range value.
     */
    double strToDouble(const std::string & sValue,
                       double dMin = std::numeric_limits<double>::lowest(),
                       double dMax = std::numeric_limits<double>::max());


    /**
     * Convert string to an bool
     *
     * @param sValue String to convert
     * @return bool value
     *
     * @exception FatalException Thrown when an error is encountered during
     * conversion either to input format or out of range value.
     */
    bool strToBool(const std::string & sValue);
  }
}

#include "otestpoint/toolkit/stringto.inl"

#endif // OPENTESTPOINT_TOOLKIT_FROMSTRING_HEADER_
