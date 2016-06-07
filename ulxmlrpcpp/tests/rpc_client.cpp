/***************************************************************************
             rpc_client.cpp  --  test file for an rpc client

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: rpc_client.cpp 11054 2011-10-18 13:00:59Z korosteleva $

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

#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <cstdlib>
#include <iostream>
#include <ctime>
#include <memory>
#include <cstring>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h>  // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_except.h>

#include "util.c"

int main(int argc, char **argv)
{
    try
    {
        ulxr::intializeLogger(argv[0]);

        std::string host = "localhost";
        if (argc > 1)
            host = argv[1];

        unsigned port = 32000;
        if (argc > 2)
            port = atoi(argv[2]);

        bool big = haveOption(argc, argv, "big");
        bool secure = haveOption(argc, argv, "ssl");
        bool chunked = haveOption(argc, argv, "chunked");

        std::string sec = "unsecured";
        if (secure)
            sec = "secured";

        std::cout << "Requesting " << sec << " rpc calls at "
                  << host << ":" << port << std::endl
                  << "Chunked transfer: " << chunked << std::endl;

        std::auto_ptr<ulxr::TcpIpConnection> conn;
        if (secure)
            conn.reset(new ulxr::SSLConnection (false, host, port));
        else
            conn.reset(new ulxr::TcpIpConnection (false, host, port));

        ulxr::HttpProtocol prot(conn.get());
        prot.setChunkedTransfer(chunked);
        ulxr::Requester client(&prot);

        time_t starttime = time(0);

        ulxr::MethodCall testcall_shutdown ("testcall_shutdown");

        ulxr::MethodCall testcall_function ("testcall_function");
        testcall_function.addParam(ulxr::Integer(456))
        .addParam(ulxr::Integer(123));

        ulxr::MethodCall stringcall ("stringcall");
//    std::string big_str (5 * 1000 * 1000, 'x');
        std::string big_str (10 * 1000 * 100, '<');
        stringcall.addParam(ulxr::RpcString(big_str));

        ulxr::MethodCall testcall_static ("testcall_in_class_static");

        ulxr::MethodCall testcall_dynamic ("testcall_in_class_dynamic");
        testcall_dynamic.addParam(ulxr::Integer(456));

        /////////////////////////////

        ulxr::MethodCall list_methods ("system.listMethods");

        ulxr::MethodCall method_help ("system.methodHelp");
        method_help.addParam(ulxr::RpcString("system.methodHelp"));

        ulxr::MethodCall method_sig ("system.methodSignature");
        method_sig.addParam(ulxr::RpcString("system.methodSignature"));

        ulxr::MethodCall method_sig2 ("system.methodSignature");
        method_sig2.addParam(ulxr::RpcString("testcall_in_class_dynamic"));

        ulxr::MethodResponse resp;

        /////////////////////////////

        std::cout << "call list_methods: \n";
        prot.setAcceptCookies(true);
        prot.setClientCookie(" cookie-test=123 ");
        resp = client.call(list_methods, "/RPC2");

#ifdef ULXR_DEBUG_OUTPUT
        std::cout << "call result: \n";
        std::cout << resp.getXml(0) << std::endl;
#endif

        if (!prot.hasCookie() || prot.getCookie() != "cookie-test=123; received=true")
        {
            std::cout << "bad received cookie: " << prot.getCookie() << std::endl;
            return 1;
        }

        std::cout << "call method_help: \n";
        prot.setClientCookie("  cookie-test-1=123;   cookie-test-2=456;   cookie-test-3=789  ");
        resp = client.call(method_help, "/RPC2");

#ifdef ULXR_DEBUG_OUTPUT
        std::cout << "call result: \n";
        std::cout << resp.getXml(0) << std::endl;
#endif

        if (!prot.hasCookie() || prot.getCookie() != "cookie-test-1=123; cookie-test-2=456; cookie-test-3=789; received=true")
        {
            std::cout << "bad received cookie: " << prot.getCookie() << std::endl;
            return 1;
        }

        std::cout << "call method_sig: \n";
        prot.setClientCookie("");
        resp = client.call(method_sig, "/RPC2");

        std::cout << "hostname: " << conn->getHostName() << std::endl;
        std::cout << "peername: " << conn->getPeerName() << std::endl;

#ifdef ULXR_DEBUG_OUTPUT
        std::cout << "call result: \n";
        std::cout << resp.getXml(0) << std::endl;
#endif

        if (!prot.hasCookie() || prot.getCookie() != "newcookie=mycookie")
        {
            std::cout << "bad received cookie: " << prot.getCookie() << std::endl;
            return 1;
        }

        std::cout << "call method_sig2: \n";
        resp = client.call(method_sig2, "/RPC2");

#ifdef ULXR_DEBUG_OUTPUT
        std::cout << "call result: \n";
        std::cout << resp.getXml(0) << std::endl;
#endif

        std::cout << "call testcall_function: \n";
        resp = client.call(testcall_function, "/RPC2");

#ifdef ULXR_DEBUG_OUTPUT
        std::cout << "call result: \n";
        std::cout << resp.getXml(0) << std::endl;
#endif

        std::cout << "call testcall_static: \n";
        resp = client.call(testcall_static, "/RPC2");

#ifdef ULXR_DEBUG_OUTPUT
        std::cout << "call result: \n";
        std::cout << resp.getXml(0) << std::endl;
#endif

        std::cout << "call testcall_dynamic: \n";
        resp = client.call(testcall_dynamic, "/RPC2");

#ifdef ULXR_DEBUG_OUTPUT
        std::cout << "call result: \n";
        std::cout << resp.getXml(0) << std::endl;
#endif

        std::string ret_str;
        if (big)
        {
            std::cout << "call stringcall: \n";
            resp = client.call(stringcall, "/RPC2");

            ret_str = resp.getXml(0);

#ifdef ULXR_DEBUG_OUTPUT
            std::cout << "call result: \n";

            std::cout << "call result: [No need to display "
                      << ret_str.length()
                      << " bytes.]\n";
            //    std::cout << resp.getXml(0) << std::endl;
#endif
        }

        std::cout << "call testcall_shutdown: \n";
        resp = client.call(testcall_shutdown, "/RPC2");

#ifdef ULXR_DEBUG_OUTPUT
        std::cout << "call result: \n";
        std::cout << resp.getXml(0) << std::endl;
#endif

        time_t endtime = time(0);
        time_t totalsecs = endtime - starttime;
        time_t mins = totalsecs / 60;
        time_t secs = totalsecs % 60;
        std::cout << "\nOverall time needed: "
                  << mins << ":" << secs << std::endl;
        if (big)
            std::cout << "Two-way transmission of "
                      << ret_str.length() << " bytes"
                      << "\nKBytes/s: " << (ret_str.length() / totalsecs / 1024 * 2) << std::endl;
    }

    catch(ulxr::Exception &ex)
    {
        std::cout << "Error occured: " << ex.why() << std::endl;
        return 1;
    }

    catch(...)
    {
        std::cout << "unknown Error occured.\n";
        return 1;
    }

    std::cout << "Well done, Ready.\n";
    return 0;
}
