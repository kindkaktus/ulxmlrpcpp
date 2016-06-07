/***************************************************************************
                     xmlfunc.cpp  -  test suite for xml-rpc
                             -------------------
    begin                : Son Mar 10 19:33:24 CET 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: xmlfunc.cpp 10942 2011-09-13 14:35:52Z korosteleva $

***************************************************************************/

/**************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***************************************************************************/

#define DEBUG


#include <cstring>
#include <iostream>
#include <map>
#include <ctime>
#include <cstdlib>
#include <cstdio>

#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_call.h>
#include <ulxmlrpcpp/ulxr_response.h>
#include <ulxmlrpcpp/ulxr_valueparse.h>
#include <ulxmlrpcpp/ulxr_callparse.h>
#include <ulxmlrpcpp/ulxr_responseparse.h>

const char *crashPattern    =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>                                           \
  <methodResponse>                                                                    \
    <params><param><value><array><data><value>                                        \
      <struct>                                                                        \
       <member><name>Welt</name><value><string>Hallo</string></value></member>        \
       <member><name>World</name><value><string>Hello</string></value></member>       \
      </struct>                                                                       \
      </value>                                                                        \
      <value><struct>                                                                 \
       <member><name>Gemeinde</name><value><string>Liebe</string></value></member>    \
       <member><name>Trek</name><value><string>Star</string></value></member>         \
       <member><name>Yoda</name><value><string>Yedi</string></value></member>         \
      </struct></value>                                                               \
      </data></array></value></param></params>                                        \
      </methodResponse>                                                               \
";

const char *emptyRespPattern    =  "<methodResponse>\n"
                                   "  <params>\n"
                                   "  </params>\n"
                                   "</methodResponse>\n";

const char *emptyArrayPattern   =  "<methodResponse>\n"
                                   "  <params>\n"
                                   "   <param>\n"
                                   "    <value>\n"
                                   "     <array><data></data></array>\n"
                                   "    </value>\n"
                                   "   </param>\n"
                                   "  </params>\n"
                                   "</methodResponse>\n";

const char *emptyStructPattern   =  "<methodResponse>\n"
                                    "  <params>\n"
                                    "   <param>\n"
                                    "    <value>\n"
                                    "     <struct>\n"
                                    "     </struct>\n"
                                    "    </value>\n"
                                    "   </param>\n"
                                    "  </params>\n"
                                    "</methodResponse>\n";

const char *int1Pattern   =  "<value><i4>123</i4></value>";
const char *int2Pattern   =  "<value><int>321</int></value>";

const char *boolPattern   =  "<value><boolean>false</boolean></value>";

const char *doublePattern =  "<value><double>123.456000</double></value>";

const char *stringPattern =  "<value><string>&amp;\"'string&lt;>&amp;\"'</string></value>";

const char *implPattern   =  "<value>implizit string</value>";

const char *datePattern   =  "<value><dateTime.iso8601>20020310T10:23:45</dateTime.iso8601></value>";

const char *base64Pattern =  "<value><base64>ABASrt466a90</base64></value>";

const char *struct1Pattern = "<value>\n"
                             " <struct>\n"
                             "  <member>\n"
                             "   <name>faultString</name>\n"
                             "   <value><string>faultstr1</string></value>\n"
                             "  </member>\n"
                             "  <member>\n"
                             "   <name>faultcode</name>\n"
                             "   <value><i4>123</i4></value>\n"
                             "  </member>\n"
                             " </struct>\n"
                             "</value>\n\n";

const char *struct2Pattern =
    "<value>\n"
    " <struct>\n"
    "  <member>\n"
    "   <value><string>faultstr2</string></value>\n"
    "   <name>faultString</name>\n"
    "  </member>\n"
    "  <member>\n"
    "   <value><i4>456</i4></value>\n"
    "   <name>faultcode</name>\n"
    "  </member>\n"
    " </struct>\n"
    "</value>";

const char *arrayPattern  =
    "<value>\n"
    " <array>\n"
    "  <data>\n"
    "   <value><boolean>true</boolean></value>\n"
    "   <value><i4>123</i4></value>\n"
    "  </data>\n"
    " </array>\n"
    "</value>";

const char *respPattern  =
    "<methodResponse>\n"
    " <fault>\n"
    "  <value>\n"
    "   <struct>\n"
    "    <member>\n"
    "     <name>faultCode</name>\n"
    "     <value><i4>123</i4></value>\n"
    "    </member>\n"
    "    <member>\n"
    "     <name>faultString</name>\n"
    "     <value><string>faultstr_r</string></value>\n"
    "    </member>\n"
    "   </struct>\n"
    "  </value>\n"
    " </fault>\n"
    "</methodResponse>\n";

const char *callPattern1  =
    "<?xml version = '1.0'?>\n"
    "  <methodCall>\n"
    "    <methodName>system.getCapabilities</methodName>\n"
    "   <params>\n"
    "      <param><value><string><![CDATA[]]></string></value></param>\n"
    "   </params>\n"
    "</methodCall>\n";

const char *callPattern2  =
    "<methodCall>\n"
    " <methodName>shell.call</methodName>\n"
    " <params>\n"
    "  <param>\n"
    "   <value><string>string</string></value>\n"
    "  </param>\n"
    "  <param>\n"
    "   <value><i4>123</i4></value>\n"
    "  </param>\n"
    "  <param>\n"
    "   <value><boolean>true</boolean></value>\n"
    "  </param>\n"
    " </params>\n"
    "</methodCall>\n\n";

const char *emptyCallPattern1  =
    "<methodCall>\n"
    " <methodName>shell.call</methodName>\n"
    " <params>\n"
    " </params>\n"
    "</methodCall>\n\n";

const char *emptyCallPattern2  =
    "<methodCall>\n"
    " <methodName>shell.call</methodName>\n"
    " <params/>\n"
    "</methodCall>\n\n";

void testPattern(const char *pattern)
{
    std::cout << pattern
              << std::endl
              << "-----------------------------------------------------\n";
    ulxr::ValueParser parser;
    bool done = false;
    std::cout << "Parser start\n";
    if (!parser.parse(pattern, strlen(pattern), done))
    {
        std::cerr << parser.getErrorString(parser.getErrorCode()).c_str()
                  << " at line "
                  << parser.getCurrentLineNumber()
                  << std::endl;
    }
    std::cout << "Parser finish\n";

    ulxr::Value val = parser.getValue();
    std::cout << val.getSignature(true) << std::endl;
    std::cout << val.getXml(0) << std::endl;
    if (val.isString() )
    {
        ulxr::RpcString str = val;
        std::cout << "EinRpcString: " << str.getString() << std::endl;
        return;
    }
    std::cout << "====================================================\n";
}


void testCallPattern(const char *pattern)
{
    std::cout << pattern << std::endl
              << "-----------------------------------------------------\n";
    ulxr::MethodCallParser parser;
    bool done = false;
    if (!parser.parse(pattern, strlen(pattern), done))
    {
        std::cerr << parser.getErrorString(parser.getErrorCode()).c_str()
                  << " at line "
                  << parser.getCurrentLineNumber()
                  << std::endl;
    }

    std::cout << parser.getMethodCall().getSignature(true) << std::endl;
    std::cout << parser.getMethodCall().getXml(0) << std::endl;
    std::cout << parser.getMethodName() << std::endl;
    std::cout << parser.numParams() << std::endl;
    for (unsigned i = 0; i < parser.numParams(); ++i)
        std::cout << parser.getParam(i).getSignature(true) << std::endl;
    std::cout << "====================================================\n";
}


void testcrashPattern(const char *pattern)
{
    ulxr::MethodResponseParser parser;
    bool done = false;
    if (!parser.parse(pattern, strlen(pattern), done))
    {
        std::cerr << parser.getErrorString(parser.getErrorCode()).c_str()
                  << " at line "
                  << parser.getCurrentLineNumber()
                  << std::endl;
    }

    ulxr::MethodResponse listresp = parser.getMethodResponse();
    std::cout << listresp.getXml() << std::endl;
    ulxr::Array *arrresp = const_cast<ulxr::Array*>(listresp.getResult().getArray());
    for(unsigned i = 0; i < arrresp->size(); i++)
    {
        ulxr::Value item = arrresp->getItem(i);
        const ulxr::Struct *structresp = item.getStruct();
        std::vector<std::string> key = structresp->getMemberNames();
        for(unsigned j = 0; j < key.size(); j++)
        {
            std::cout << structresp << " ";
            std::cout << i + 1 << " " << key[j] << " ";
            bool  h = structresp->hasMember(key[j]); std::cout << h << " ";
            ulxr::Value v = structresp->getMember(key[j]); ///< this segfaults
            ulxr::RpcString s = (ulxr::RpcString)v;
            std::cout << std::endl;
        }
    }
}

void testRespPattern(const char *pattern)
{
    std::cout << pattern << std::endl
              << "-----------------------------------------------------\n";
    ulxr::MethodResponseParser parser;
    bool done = false;
    if (!parser.parse(pattern, strlen(pattern), done))
    {
        std::cerr << parser.getErrorString(parser.getErrorCode()).c_str()
                  << " at line "
                  << parser.getCurrentLineNumber()
                  << std::endl;
    }

    ulxr::Value val = parser.getValue();
    std::cout << "!Value...\n";
    std::cout << val.getSignature(true) << std::endl;
    std::cout << val.getXml(0) << std::endl;
    std::cout << "Response...\n";
    std::cout << parser.getMethodResponse().getSignature(true) << std::endl;
    std::cout << parser.getMethodResponse().getXml(0) << std::endl;
    std::cout << "====================================================\n";
}

// #define STRESS_IT

int main(int argc, char * argv [])
{
    ulxr::enableXmlPrettyPrint(true);

    int success = 0;

#ifdef STRESS_IT
    for (int often = 0; often < 1000; ++often)
    {
#endif
        try
        {
            ulxr::Boolean b(true);
            ulxr::Integer i(123);
            ulxr::Double d(123.456);
            ulxr::RpcString s("<>&\"'string<>&\"'0xd9f0�)/()/");
            ulxr::DateTime dt("20020310T10:23:45");
            std::time_t t = std::time(0);
            ulxr::DateTime dt2(t);
            ulxr::Base64 b64("ABASrt466a90");

            typedef std::map<std::string, ulxr::Value> members;
            typedef std::pair<std::string, ulxr::Value> member_pair;
            members val;
            {
                ulxr::Array arr1;
                unsigned szarr = sizeof(arr1);
                std::cout << szarr << std::endl;
            }

            ulxr::ValueBase *vb = new ulxr::RpcString("asdfasdf");
            delete vb;

            ulxr::Struct st;
            ulxr::Array ar;

            ulxr::Value rv = i;
            i = rv;
            std::cout << "rv(i) = " << rv.getSignature(true) << std::endl;

            rv = d;
            d = rv;
            std::cout << d.getDouble() << std::endl;
            std::cout << "rv(d) = " << rv.getSignature(true) << std::endl;

            rv = b;
            b = rv;
            std::cout << "rv(b) = " << rv.getSignature(true) << std::endl;

            rv = s;
            s = rv;
            std::cout << "rv(s) = " << rv.getSignature(true) << std::endl;

            rv = dt;
            dt = rv;
            std::cout << "rv(dt) = " << rv.getSignature(true) << std::endl;

            rv = b64;
            b64 = rv;
            std::cout << "rv(b64) = " << rv.getSignature(true) << std::endl;

            rv = ar;
            ar = rv;
            std::cout << "rv(ar) = " << rv.getSignature(true) << std::endl;

            rv = st;
            st = rv;
            std::cout << "rv(st) = " << rv.getSignature(true) << std::endl << std::endl;

            ar.addItem(b);
            ar.addItem(i);
            ar.addItem(d);
            ar.addItem(s);
            ar.addItem(b64);
            ar.addItem(st);

            st.addMember ("intmem", i);
            st.addMember ("dmem", d);
            st.addMember ("arrmem", ar);

            std::cout << i.getSignature(true) << std::endl;
            std::cout << i.getXml(0) << std::endl << std::endl;

            std::cout << rv.getSignature(true) << std::endl;
            std::cout << rv.getXml(0) << std::endl << std::endl;

            std::cout << b.getSignature(true) << std::endl;
            std::cout << b.getXml(0) << std::endl << std::endl;

            std::cout << d.getSignature(true) << std::endl;
            std::cout << d.getXml(0) << std::endl << std::endl;

            std::cout << s.getSignature(true) << std::endl;
            std::cout << s.getXml(0) << std::endl << std::endl;

            std::cout << dt.getSignature(true) << std::endl;
            std::cout << dt.getXml(0) << std::endl << std::endl;

            std::cout << dt2.getSignature(true) << std::endl;
            std::cout << dt2.getXml(0) << std::endl << std::endl;

            std::cout << b64.getSignature(true) << std::endl;
            std::cout << b64.getXml(0) << std::endl << std::endl;

            std::cout << st.getSignature(true) << std::endl;
            std::cout << st.getXml(0) << std::endl << std::endl;

            std::cout << ar.getSignature(true) << std::endl;
            std::cout << ar.getXml(0) << std::endl << std::endl;

            st.clear();
            st.addMember ("intmem", i);
            st.addMember ("dmem", d);

            ulxr::MethodCall mc ("test.call");
            mc.addParam (st);
            mc.addParam (ar);
            mc.addParam (b);

            std::cout << mc.getXml(0) << std::endl;
            std::cout << mc.getSignature(true) << std::endl << std::endl;

            ulxr::MethodCall sc ("shell.call");
            sc.addParam(s);
            sc.addParam (i);
            sc.addParam (b);

            std::cout << sc.getXml(0) << std::endl;
            std::cout << sc.getSignature(true) << std::endl << std::endl;

            std::cout << "----------------------------------------------------\n";

            std::cout << "MethodResponse abc\n";

            ulxr::MethodResponse mr1(123, "faultstr_m");
            std::cout << mr1.getXml(0) << std::endl;
            std::cout << mr1.getSignature(true) << std::endl << std::endl;

            mr1.setResult(st);
            std::cout << mr1.getXml(0) << std::endl;
            std::cout << mr1.getSignature(true) << std::endl << std::endl;

            ulxr::MethodResponse mr2(d);
            std::cout << mr2.getXml(0) << std::endl;
            std::cout << mr2.getSignature(true) << std::endl << std::endl;

            ulxr::MethodResponse mr3;
            std::cout << mr3.getXml(0) << std::endl;
            std::cout << mr3.getSignature(true) << std::endl << std::endl;

            std::cout << "----------------------------------------------------\n";

            std::cout << "Testing patterns\n";

            std::cout << "boolPattern\n" << std::flush;
            testPattern(boolPattern);

            std::cout << "int1Pattern\n";
            testPattern(int1Pattern);

            std::cout << "int2Pattern\n";
            testPattern(int2Pattern);

            std::cout << "doublePattern\n";
            testPattern(doublePattern);

            std::cout << "stringPattern\n";
            testPattern(stringPattern);

            std::cout << "base64Pattern\n";
            testPattern(base64Pattern);

            std::cout << "datePattern\n";
            testPattern(datePattern);

            std::cout << "struct1Pattern\n";
            testPattern(struct1Pattern);

            std::cout << "struct2Pattern\n";
            testPattern(struct2Pattern);

            std::cout << "arrayPattern\n";
            testPattern(arrayPattern);

            std::cout << "callPattern1\n";
            testCallPattern(callPattern1);

            std::cout << "callPattern2\n";
            testCallPattern(callPattern2);

            std::cout << "emptyCallPattern1\n";
            testCallPattern(emptyCallPattern1);

            std::cout << "emptyCallPattern2\n";
            testCallPattern(emptyCallPattern2);

            std::cout << "respPattern\n";
            testRespPattern(respPattern);

            std::cout << "implPattern\n";
            testPattern(implPattern);

            std::cout << "emptyArrayPattern\n";
            testRespPattern(emptyArrayPattern);

            std::cout << "emptyRespPattern\n";
            testRespPattern(emptyRespPattern);

            std::cout << "emptyRespPattern\n";
            testcrashPattern(crashPattern);

            std::cout << "emptyStructPattern\n";
            testRespPattern(emptyStructPattern);

            ar.clear();
            ar << ulxr::Integer(1) << ulxr::Integer(2) << ulxr::RpcString("3") << ulxr::Double(5.0);

            ar << 11 << 22 << "33"
               << 5.5 << true;

            std::cout << ar.getXml(0) << std::endl;
            std::cout << "====================================================\n";

            st.clear();
            st
                    << ulxr::make_member("before", i)
                    << ulxr::make_member("Hallo", ar)
                    << ulxr::make_member("Hallo double", 1.0)
                    << ulxr::make_member("Hallo bool", true)

// upon requst you may also use the following construct
// #define ULXR_STRUCT_MEMBER_FROM_NAME_VALUE
                    << (std::string("second") << ulxr::Integer(2))

                    << ulxr::make_member("difference", "1111")
                    ;

            std::cout << st.getXml(0) << std::endl;
            std::cout << "====================================================\n";

        }
        catch(ulxr::Exception &ex)
        {
            std::cout << "Error occured: "
                      << ex.why() << std::endl;
            success = 1;
        }
#ifdef STRESS_IT
    }
#endif

    int major, minor, patch;
    bool debug;
    std::string info;
    ulxr::getVersion (major, minor, patch, debug, info);

    std::cout << "Version of the ulxmlrpcpp lib in use: "
              << major << "." << minor << "." << patch << std::endl
              << "Debugging was turned " << (debug ? "ON" : "OFF") << std::endl
              << "Additional info: " << info << std::endl
              << "Ready.\n";

    return success;
}




