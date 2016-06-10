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

#include "probedirector.h"


#include "otestpoint/probebuilder.h"
#include "otestpoint/controller.h"
#include "otestpoint/toolkit/exception.h"
#include "otestpoint/toolkit/stringto.h"
#include "otestpoint/toolkit/addrinfo.h"

#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include <cstring>

namespace
{
  const char * pzSchema="\
<?xml version='1.0' encoding='UTF-8' standalone='yes'?>\
<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>\
  <xs:element name='otestpoint'>\
    <xs:complexType>\
      <xs:sequence>\
        <xs:element name='probe' maxOccurs='unbounded'>\
          <xs:complexType>\
            <xs:choice>\
              <xs:element name='plugin'>\
                <xs:complexType>\
                  <xs:attribute name='library' type='xs:string' use='required'/>\
                </xs:complexType>\
              </xs:element>\
              <xs:element name='python'>\
                <xs:complexType> \
                  <xs:attribute name='module' type='xs:string' use='required'/>\
                  <xs:attribute name='class' type='xs:string' use='required'/>\
                </xs:complexType>\
              </xs:element>\
            </xs:choice>\
            <xs:attribute name='configuration' type='xs:string' use='optional'/> \
            <xs:attribute name='rate' type='xs:unsignedShort' use='optional'/> \
            <xs:attribute name='commthreshold' type='xs:unsignedShort' use='optional'/>\
          </xs:complexType>\
        </xs:element>\
      </xs:sequence>\
      <xs:attribute name='id' type='xs:string' use='required'/>\
      <xs:attribute name='discovery' type='xs:string' use='required'/>\
      <xs:attribute name='publish' type='xs:string' use='required'/> \
      <xs:attribute name='rate' type='xs:unsignedShort' default='5'/>\
      <xs:attribute name='commthreshold' type='xs:unsignedShort' default='5'/>\
    </xs:complexType>\
  </xs:element>\
</xs:schema>";
}

OpenTestPoint::ProbeDirector::ProbeDirector(Toolkit::Log::Service & logService,
                                            Toolkit::Log::Client & logClient,
                                            const uuid_t & uuid,
                                            ProbeBuilder & builder):
  logService_(logService),
  logClient_(logClient),
  builder_(builder)
{
  uuid_copy(uuid_,uuid);
}

OpenTestPoint::Controller *
OpenTestPoint::ProbeDirector::construct(const std::string & sConfigurationFile)
{
  LIBXML_TEST_VERSION;

  xmlDocPtr pSchemaDoc{xmlReadMemory(pzSchema,
                                     strlen(pzSchema),
                                     "file:///otestpoint.xsd",
                                     NULL,
                                     0)};

  if(!pSchemaDoc)
    {
      throw Toolkit::Exception{"unable to open schema"};
    }

  xmlSchemaParserCtxtPtr pParserContext{xmlSchemaNewDocParserCtxt(pSchemaDoc)};

  if(!pParserContext)
    {
      throw Toolkit::Exception{"bad schema context"};
    }

  xmlSchemaPtr pSchema{xmlSchemaParse(pParserContext)};

  if(!pSchema)
    {
      throw Toolkit::Exception{"bad schema parser"};
    }

  xmlSchemaValidCtxtPtr pSchemaValidCtxtPtr{xmlSchemaNewValidCtxt(pSchema)};

  if(!pSchemaValidCtxtPtr)
    {
      throw Toolkit::Exception{"bad schema valid context"};
    }

  xmlSchemaSetValidOptions(pSchemaValidCtxtPtr,XML_SCHEMA_VAL_VC_I_CREATE);

  xmlDocPtr pDoc = xmlReadFile(sConfigurationFile.c_str(),nullptr,0);

  if(xmlSchemaValidateDoc(pSchemaValidCtxtPtr, pDoc))
    {
      throw Toolkit::Exception{"invalid document"};
    }

  xmlNodePtr pRoot = xmlDocGetRootElement(pDoc);

  xmlChar * pId = xmlGetProp(pRoot,BAD_CAST "id");

  xmlChar * pDiscoveryEndpoint = xmlGetProp(pRoot,BAD_CAST "discovery");

  xmlChar * pPublishEndpoint = xmlGetProp(pRoot,BAD_CAST "publish");

  std::string sEndpointBase{"tcp://127.0.0.1:"};

  builder_.buildController(logService_,
                           logClient_,
                           std::string{"tcp://"} +
                           Toolkit::getHostAddressAsString(reinterpret_cast<const char *>(pDiscoveryEndpoint),true),
                           std::string{"tcp://"} +
                           Toolkit::getHostAddressAsString(reinterpret_cast<const char *>(pPublishEndpoint),true));

  xmlFree(pDiscoveryEndpoint);

  xmlFree(pPublishEndpoint);

  xmlChar * pProbeRate = xmlGetProp(pRoot,BAD_CAST "rate");

  std::uint16_t u16ProbeRate{Toolkit::strToUINT16(reinterpret_cast<const char *>(pProbeRate))};

  xmlChar * pCommThreshold = xmlGetProp(pRoot,BAD_CAST "commthreshold");

  std::uint16_t u16CommThreshold{Toolkit::strToUINT16(reinterpret_cast<const char *>(pCommThreshold))};

  xmlFree(pProbeRate);

  xmlFree(pCommThreshold);

  for(xmlNodePtr pNode = pRoot->children; pNode; pNode = pNode->next)
    {
      if(pNode->type == XML_ELEMENT_NODE)
        {
          if(!xmlStrcmp(pNode->name,BAD_CAST "probe"))
            {

              for(xmlNodePtr pChildNode = pNode->children; pChildNode; pChildNode = pChildNode->next)
                {
                  if(pChildNode->type == XML_ELEMENT_NODE)
                    {
                      if(!xmlStrcmp(pChildNode->name,BAD_CAST "plugin"))
                        {
                          xmlChar * pLibrary = xmlGetProp(pChildNode,BAD_CAST "library");

                          xmlChar * pConfiguration = xmlGetProp(pNode,BAD_CAST "configuration");

                          std::string sConfiguration{};

                          if(pConfiguration)
                            {
                              sConfiguration = reinterpret_cast<const char *>(pConfiguration);
                              xmlFree(pConfiguration);
                            }

                          // local rate and threshold
                          std::uint16_t u16LocalProbeRate{u16ProbeRate};

                          std::uint16_t u16LocalCommThreshold{u16CommThreshold};

                          xmlChar * pProbeRate = xmlGetProp(pNode,BAD_CAST "rate");

                          if(pProbeRate)
                            {
                              u16LocalProbeRate = Toolkit::strToUINT16(reinterpret_cast<const char *>(pProbeRate));
                              xmlFree(pProbeRate);
                            }

                          xmlChar * pCommThreshold = xmlGetProp(pNode,BAD_CAST "commthreshold");

                          if(pCommThreshold)
                            {
                              u16LocalCommThreshold = Toolkit::strToUINT16(reinterpret_cast<const char *>(pCommThreshold));
                              xmlFree(pCommThreshold);
                            }

                          builder_.buildPluginProbe(reinterpret_cast<const char *>(pId),
                                                    reinterpret_cast<const char *>(pLibrary),
                                                    std::chrono::seconds{u16LocalProbeRate},
                                                    std::chrono::seconds{u16LocalCommThreshold},
                                                    sConfiguration);

                          xmlFree(pLibrary);
                        }
                      else if(!xmlStrcmp(pChildNode->name,BAD_CAST "python"))
                        {
                          xmlChar * pModule = xmlGetProp(pChildNode,BAD_CAST "module");

                          xmlChar * pClass = xmlGetProp(pChildNode,BAD_CAST "class");

                          xmlChar * pConfiguration = xmlGetProp(pNode,BAD_CAST "configuration");

                          std::string sConfiguration{};

                          if(pConfiguration)
                            {
                              sConfiguration =  reinterpret_cast<const char *>(pConfiguration);
                              xmlFree(pConfiguration);
                            }

                          // local rate and threshold
                          std::uint16_t u16LocalProbeRate{u16ProbeRate};

                          std::uint16_t u16LocalCommThreshold{u16CommThreshold};

                          xmlChar * pProbeRate = xmlGetProp(pNode,BAD_CAST "rate");

                          if(pProbeRate)
                            {
                              u16LocalProbeRate = Toolkit::strToUINT16(reinterpret_cast<const char *>(pProbeRate));
                              xmlFree(pProbeRate);
                            }

                          xmlChar * pCommThreshold = xmlGetProp(pNode,BAD_CAST "commthreshold");

                          if(pCommThreshold)
                            {
                              u16LocalCommThreshold = Toolkit::strToUINT16(reinterpret_cast<const char *>(pCommThreshold));
                              xmlFree(pCommThreshold);
                            }

                          builder_.buildPythonProbe(reinterpret_cast<const char *>(pId),
                                                    reinterpret_cast<const char *>(pModule),
                                                    reinterpret_cast<const char *>(pClass),
                                                    std::chrono::seconds{u16LocalProbeRate},
                                                    std::chrono::seconds{u16LocalCommThreshold},
                                                    sConfiguration);

                          xmlFree(pModule);
                          xmlFree(pClass);
                        }
                    }
                }
            }
        }
    }

  xmlFree(pId);

  xmlFreeDoc(pDoc);

  xmlCleanupParser();

  return builder_.getController();
}
