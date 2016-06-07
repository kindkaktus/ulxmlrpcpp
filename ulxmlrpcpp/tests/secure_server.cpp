/***************************************************************************
               secure_server.cpp  --  test file for a secure rpc server

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: secure_server.cpp 11054 2011-10-18 13:00:59Z korosteleva $

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

//#define DEBUG

#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <cstdlib>
#include <iostream>
#include <cstring>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h> // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_dispatcher.h>
#include <ulxmlrpcpp/ulxr_signature.h>
#include <ulxmlrpcpp/ulxr_callparse.h>

#include "crypt.cpp"

class SecureWorker
{
public:

    SecureWorker ()
        : running(true)
        , secureDisp (0)
    {
        secureDisp.addMethod(ulxr::make_method(*this, &SecureWorker::shutdown),
                             ulxr::Signature() << ulxr::Struct(),
                             "secure_shutdown",
                             ulxr::Signature(),
                             "Shut down secure Worker");
    }


    bool isRunning()
    {
        return running;
    }


    ulxr::MethodResponse secureDispatcher (const ulxr::MethodCall &calldata)
    {

        // decrypt base64-coded method call
        std::string mc_data = ulxr::Base64(calldata.getParam(0)).getString();

        ULXR_TRACE(mc_data);

        std::string xml_call = decrypt(mc_data, "");
        ULXR_TRACE(xml_call);

        // parse xml to get method call
        ulxr::MethodCallParser parser;
        bool done = false;
        if (!parser.parse(xml_call.data(), xml_call.length(), done))
        {
            return ulxr::MethodResponse (1, "Error parsing encrypted call");
        }

        // dispatch resulting call
        std::string respxml = secureDisp.dispatchCall(parser.getMethodCall()).getXml(0);

        ULXR_TRACE(respxml);

        // return encrypted response base64-encoded
        return ulxr::MethodResponse (ulxr::Base64(encrypt(respxml, "")));
    }

private:

    ulxr::MethodResponse shutdown (const ulxr::MethodCall &/*calldata*/)
    {
        std::cout << "got secure signal to shut down\n";
        ulxr::MethodResponse resp;
        resp.setResult(ulxr::Boolean(running));
        running = false;
        return resp;
    }

    bool running;

    ulxr::Dispatcher secureDisp;
};


////////////////////////////////////////////////////////////////////////


int main(int argc, char **argv)
{
    ulxr::intializeLogger(argv[0]);

    std::string host = "localhost";
    if (argc > 1)
        host = argv[1];

    unsigned port = 32003;
    if (argc > 2)
        port = atoi(argv[2]);

    std::cout << "Serving rpc requests at " << host << ":" << port << std::endl;

    std::string realm = "SecureRPCRealm";

    ulxr::TcpIpConnection conn (true, host, port);
    ulxr::HttpProtocol prot(&conn);

    prot.addAuthentication("ali-baba", "open-sesame", realm);
    prot.addAuthentication("bob",      "let-me-in",   realm);
    prot.addAuthentication("sue",      "let-me-out",  realm);

    ulxr::Dispatcher server(&prot);

    try
    {
        SecureWorker worker;
        server.addMethod(ulxr::make_method(worker, &SecureWorker::secureDispatcher),
                         ulxr::Signature() << ulxr::Base64(),
                         "secureDispatcher",
                         ulxr::Signature() << ulxr::Base64(),
                         "Provide access to secured methods");

        while (worker.isRunning() )
        {
            ulxr::MethodCall call = server.waitForCall();
            if (prot.checkAuthentication(realm))
            {
                ulxr::MethodResponse resp = server.dispatchCall(call);
                if (!prot.isTransmitOnly())
                    server.sendResponse(resp);
            }
            else
                prot.rejectAuthentication(realm);

            prot.close();
        }
    }
    catch(ulxr::Exception& ex)
    {
        std::cout << "Error occured: " << ex.why() << std::endl;
        if (prot.isOpen())
        {
            try {
                ulxr::MethodResponse resp(1, ex.why() );
                if (prot.isTransmitOnly())
                    server.sendResponse(resp);
            }
            catch(...)
            {
                std::cout << "error within exception occured\n";
            }
            prot.close();
        }
    }
    return 0;
}
