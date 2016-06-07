/***************************************************************************
               rpc_server.cpp  --  test file for an rpc server

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: rpc_server.cpp 11054 2011-10-18 13:00:59Z korosteleva $

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


#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_tcpip_connection.h>  // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_signature.h>
#include <ulxmlrpcpp/ulxr_dispatcher.h>

#include "util.c"

class TestClass
{
public:
    static ulxr::MethodResponse testcall  (const ulxr::MethodCall &calldata);
    static ulxr::MethodResponse getInput  (const ulxr::MethodCall &calldata);
    static ulxr::MethodResponse getAllInputs  (const ulxr::MethodCall &calldata);
    static ulxr::MethodResponse setOutput (const ulxr::MethodCall &calldata);
};


ulxr::MethodResponse TestClass::testcall (const ulxr::MethodCall &/*calldata*/)
{
    ulxr::MethodResponse resp(ulxr::Integer(654321));
    resp.setResult(ulxr::Integer(123456));
    return resp;
}


ulxr::MethodResponse TestClass::getAllInputs (const ulxr::MethodCall &calldata)
{
    std::string s;
    for (unsigned i = 0; i < 6; ++i)
    {
        bool b = 1 & rand();
        if (b)
            s += '1';
        else
            s += '0';
    }
    return ulxr::MethodResponse (ulxr::RpcString(s));
}


ulxr::MethodResponse TestClass::getInput (const ulxr::MethodCall &calldata)
{
    ulxr::Integer nr = calldata.getParam(0);
    bool b = 1 & rand();
    return ulxr::MethodResponse (ulxr::Boolean(b));
}


ulxr::MethodResponse TestClass::setOutput (const ulxr::MethodCall &calldata)
{
    ulxr::Integer nr = calldata.getParam(0);
    ulxr::Boolean state = calldata.getParam(1);
    std::cout << "out " << nr.getInteger() << " " << state.getBoolean() << "\n";
    ulxr::MethodResponse resp;
    return resp;
}


ulxr::MethodResponse stringcall (const ulxr::MethodCall &calldata)
{
    ulxr::RpcString rpcs = calldata.getParam(0);
    std::string s = rpcs.getString();
    ulxr::MethodResponse resp;
    resp.setResult(ulxr::RpcString(s));
    return resp;
}


////////////////////////////////////////////////////////////////////////


class TestWorker
{
public:

    TestWorker () : running(true)
    {}


    ulxr::MethodResponse shutdown (const ulxr::MethodCall &/*calldata*/)
    {
        std::cout << "got signal to shut down\n";
        ulxr::MethodResponse resp;
        resp.setResult(ulxr::Boolean(running));
        running = false;
        return resp;
    }

    ulxr::MethodResponse testcall1 (const ulxr::MethodCall &calldata)
    {
        ulxr::MethodResponse resp;
        ulxr::Struct st;
        st.addMember("param 1", calldata.getParam(0));
        return resp;
    }

    ulxr::MethodResponse testcall2 (const ulxr::MethodCall &/*calldata*/)
    {
        ulxr::MethodResponse resp;
        resp.setResult(ulxr::Integer(654321));
        return resp;
    }

    bool running;
};

////////////////////////////////////////////////////////////////////////

static ulxr::MethodResponse testcall (const ulxr::MethodCall &calldata)
{
    ulxr::MethodResponse resp;
    ulxr::Integer i = calldata.getParam(0);
    ulxr::Integer i2 = calldata.getParam(1);

    ulxr::Array ar;
    ar << ulxr::Integer(1) << ulxr::Integer(2) << ulxr::RpcString("3");
    ulxr::Struct st;

    st  << ulxr::make_member("Hallo", ar)
        << ulxr::make_member("before", i)

// upon requst you may also use the following construct
//      << ("second" << i2)
// #define ULXR_STRUCT_MEMBER_FROM_NAME_VALUE

        << ulxr::make_member("num_param", ulxr::Integer(123))
        << ulxr::make_member("after", ulxr::Integer(i.getInteger()+1111))
        << ulxr::make_member("difference", ulxr::RpcString("1111"));

    resp.setResult(st);
    return ulxr::MethodResponse (st);
}


////////////////////////////////////////////////////////////////////////


int main(int argc, char **argv)
{
    ulxr::intializeLogger(argv[0]);

    std::string host = "localhost";
    if (argc > 1)
        host = argv[1];

    unsigned port = 32000;
    if (argc > 2)
        port = atoi(argv[2]);

    bool secure = haveOption(argc, argv, "ssl");
    bool chunked = haveOption(argc, argv, "chunked");

    std::string sec = "unsecured";
    if (secure)
        sec = "secured";

    std::cout << "Serving " << sec << " rpc requests at "
              << host << ":" << port << std::endl
              << "Chunked transfer: " << chunked << std::endl;

    std::auto_ptr<ulxr::TcpIpConnection> conn;
    if (secure)
    {
        ulxr::SSLConnection *ssl = new ulxr::SSLConnection (true, host, port);
        ssl->setCryptographyData("password", "foo-cert.pem", "foo-cert.pem");
        conn.reset(ssl);
    }
    else
        conn.reset(new ulxr::TcpIpConnection (true, host, port));

    ulxr::HttpProtocol prot(conn.get());
    prot.setChunkedTransfer(chunked);
    ulxr::Dispatcher server(&prot);

    try
    {
        server.addMethod(ulxr::make_method(TestClass::testcall),
                         ulxr::Struct::getValueName(),
                         "testcall_in_class_static",
                         "",
                         "Testcase with a static method in a class");

        server.addMethod(ulxr::make_method(testcall),
                         ulxr::Signature() << ulxr::Integer(),
                         "testcall_function",
                         ulxr::Signature() << ulxr::Integer()
                         << ulxr::Integer(),
                         "Testcase with a c-function");

        server.addMethod(ulxr::make_method(TestClass::getInput),
                         ulxr::Signature() << ulxr::Boolean(),
                         "getInput",
                         ulxr::Signature()<< ulxr::Integer(),
                         "get input state");

        server.addMethod(ulxr::make_method(TestClass::getAllInputs),
                         ulxr::Signature() << ulxr::RpcString(),
                         "getAllInputs",
                         ulxr::Signature(),
                         "get all input states");

        server.addMethod(ulxr::make_method(TestClass::setOutput),
                         ulxr::Signature(),
                         "setOutput",
                         ulxr::Signature() << ulxr::Integer()
                         << ulxr::Boolean(),
                         "set output state");

        server.addMethod(ulxr::make_method(stringcall),
                         ulxr::Signature() << ulxr::RpcString(),
                         "stringcall",
                         ulxr::Signature() << ulxr::RpcString(),
                         "Testcase return input string");

        TestWorker worker;

        server.addMethod(ulxr::make_method(worker, &TestWorker::testcall1),
                         ulxr::Signature(ulxr::Struct()),
                         "testcall_in_class_dynamic",
                         ulxr::Signature(ulxr::Integer()),
                         "Testcase with a dynamic method in a class");

        server.addMethod(ulxr::make_method(worker, &TestWorker::shutdown),
                         ulxr::Signature(ulxr::Boolean()),
                         "testcall_shutdown",
                         ulxr::Signature(),
                         "Testcase with a dynamic method in a class, shut down server, return old state");

        prot.setAcceptCookies(true);
        while (worker.running)
        {
            ulxr::MethodCall call = server.waitForCall();

            std::cout << "hostname: " << conn->getHostName() << std::endl;
            std::cout << "peername: " << conn->getPeerName() << std::endl;

            if (prot.hasCookie())
            {
                std::string s = prot.getCookie();
                std::cout << "received cookie: " << s << std::endl;
                prot.setServerCookie(s + "; received=true");
            }
            else
            {
                prot.setServerCookie("newcookie=mycookie");
                std::cout << "no cookie received\n";
            }

            ulxr::MethodResponse resp = server.dispatchCall(call);
            if (!prot.isTransmitOnly())
                server.sendResponse(resp);

            prot.close();
        }
    }

    catch(ulxr::Exception& ex)
    {
        std::cout << "Error occured: " << ex.why() << std::endl;
        if (prot.isOpen())
        {
            try
            {
                ulxr::MethodResponse resp(1, ex.why() );
                if (!prot.isTransmitOnly())
                    server.sendResponse(resp);
            }
            catch(...)
            {
                std::cout << "error within exception occured\n";
            }
            prot.close();
        }
        return 1;
    }
    std::cout << "Well done, Ready.\n";
    return 0;
}
