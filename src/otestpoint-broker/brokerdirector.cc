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

#include "brokerdirector.h"

#include "otestpoint/brokerbuilder.h"
#include "otestpoint/toolkit/exception.h"
#include "otestpoint/toolkit/stringto.h"
#include "otestpoint/toolkit/addrinfo.h"

#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include <libxml/xpath.h>

#include <cstring>
#include <iostream>

namespace
{
  const char * pzSchema="\
<?xml version='1.0' encoding='UTF-8' standalone='yes'?>\
<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>\
  <xs:element name='otestpoint-broker'>\
    <xs:complexType>\
      <xs:sequence>\
        <xs:element name='testpoint' maxOccurs='unbounded'>\
          <xs:complexType>\
            <xs:attribute name='discovery' type='xs:string' use='required'/> \
            <xs:attribute name='publish' type='xs:string' use='required'/>   \
          </xs:complexType>\
        </xs:element>\
      </xs:sequence>\
      <xs:attribute name='discovery' type='xs:string' use='required'/> \
      <xs:attribute name='publish' type='xs:string' use='required'/> \
    </xs:complexType>\
  </xs:element>\
</xs:schema>";
}

OpenTestPoint::BrokerDirector::BrokerDirector(Toolkit::Log::Service & logService,
                                              Toolkit::Log::Client & logClient,
                                              const uuid_t & uuid,
                                              BrokerBuilder & builder):
  logService_(logService),
  logClient_(logClient),
  builder_(builder)
{
  uuid_copy(uuid_,uuid);
}

OpenTestPoint::Broker *
OpenTestPoint::BrokerDirector::construct(const std::string & sConfigurationFile)
{
  LIBXML_TEST_VERSION;

  xmlDocPtr pSchemaDoc{xmlReadMemory(pzSchema,
                                     strlen(pzSchema),
                                     "file:///otestpoint-broker.xsd",
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

  xmlChar * pDiscoveryEndpoint = xmlGetProp(pRoot,BAD_CAST "discovery");

  xmlChar * pPublishEndpoint = xmlGetProp(pRoot,BAD_CAST "publish");

  std::string sEndpointBase{"tcp://127.0.0.1:"};

  builder_.buildBroker(logService_,
                       logClient_,
                       std::string{"tcp://"} +
                       Toolkit::getHostAddressAsString(reinterpret_cast<const char *>(pDiscoveryEndpoint),
                                                       true),
                       std::string{"tcp://"} +
                       Toolkit::getHostAddressAsString(reinterpret_cast<const char *>(pPublishEndpoint),
                                                       true));

  xmlFree(pDiscoveryEndpoint);

  xmlFree(pPublishEndpoint);



  xmlXPathContextPtr pXPathCtxt{xmlXPathNewContext(pDoc)};

  xmlXPathObjectPtr pXPathObj{xmlXPathEvalExpression(BAD_CAST "/otestpoint-broker/testpoint",
                                                     pXPathCtxt)};

  if(!pXPathObj)
    {
      xmlXPathFreeContext(pXPathCtxt);
      xmlFreeDoc(pDoc);
      throw Toolkit::Exception{"unable to evaluate xpath"};
    }

  int iSize = pXPathObj->nodesetval->nodeNr;

  if(!iSize)
    {
      throw Toolkit::Exception{"no testpoints defined"};
    }

  for(int i = 0; i < iSize; ++i)
    {
      xmlChar * pDiscovery  = xmlGetProp(pXPathObj->nodesetval->nodeTab[i],BAD_CAST "discovery");
      xmlChar * pPublish  = xmlGetProp(pXPathObj->nodesetval->nodeTab[i],BAD_CAST "publish");
      builder_.addTestPoint(Toolkit::getHostAddressAsString(reinterpret_cast<char *>(pDiscovery),true),
                            Toolkit::getHostAddressAsString(reinterpret_cast<char *>(pPublish),true));
      xmlFree(pDiscovery);
      xmlFree(pPublish);
    }

  xmlXPathFreeObject(pXPathObj);

  xmlXPathFreeContext(pXPathCtxt);

  xmlFreeDoc(pDoc);

  xmlCleanupParser();

  return builder_.getBroker();
}
