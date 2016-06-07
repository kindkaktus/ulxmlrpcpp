/***************************************************************************
            ulxr_requester.cpp  -  send rpc request ("rpc-client")
                             -------------------
    begin                : Sat Mar 23 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_requester.cpp 11073 2011-10-25 12:44:58Z korosteleva $

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

//#define ULXR_SHOW_TRACE
//#define ULXR_DEBUG_OUTPUT
//#define ULXR_SHOW_READ
//#define ULXR_SHOW_WRITE
//#define ULXR_SHOW_XML


#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_protocol.h>
#include <ulxmlrpcpp/ulxr_connection.h>
#include <ulxmlrpcpp/ulxr_responseparse.h>

#include <pthread.h>

#include <cerrno>
#include <memory>

namespace ulxr {


    Requester::Requester(Protocol* prot)
        : protocol(prot)
    {}


    Requester::~Requester()
    {}


    void Requester::send_call (const MethodCall &calldata,
                               const std::string &rpc_root)
    {
        ULXR_TRACE("send_call " << calldata.getMethodName());
        if (!protocol->isOpen() )
            protocol->open();
        else
            protocol->resetConnection();
        protocol->sendRpcCall(calldata, rpc_root);
    }


    MethodResponse Requester::waitForResponse()
    {
        ULXR_TRACE("waitForResponse");
        return waitForResponse(protocol);
    }


    MethodResponse
    Requester::waitForResponse(Protocol *protocol)
    {
        ULXR_TRACE("waitForResponse");
        char buffer[ULXR_RECV_BUFFER_SIZE];
        char *buff_ptr;

        std::auto_ptr<XmlParserBase> parser;
        MethodResponseParserBase *rpb = 0;
        ULXR_TRACE("waitForResponse in XML");
        MethodResponseParser *rp = new MethodResponseParser();
        rpb = rp;
        parser.reset(rp);

        bool done = false;
        long myRead;
        while (!done && protocol->hasBytesToRead()
                && ((myRead = protocol->readRaw(buffer, sizeof(buffer))) > 0) )
        {
            buff_ptr = buffer;
            while (myRead > 0)
            {
                Protocol::State state = protocol->connectionMachine(buff_ptr, myRead);
                if (state == Protocol::ConnError)
                {
                    done = true;
                    throw ConnectionException(TransportError, "network problem occured", 400);
                }

                else if (state == Protocol::ConnSwitchToBody)
                {
#ifdef ULXR_SHOW_READ
                    std::string super_data (buff_ptr, myRead);
                    while ((myRead = protocol->readRaw(buffer, sizeof(buffer))) > 0)
                        super_data.append(buffer, myRead);
                    ULXR_DOUT_READ("superdata 3 start:\n");

                    ULXR_DOUT_READ(super_data);
                    ULXR_DOUT_READ("superdata 3 end:\n" );
#endif
                    if (!protocol->hasBytesToRead())
                    {
                        throw ConnectionException(NotConformingError,
                                                  "Content-Length of message not available", 411);
                    }

                    std::string s;
                    if (!protocol->isResponseStatus200(s))
                        throw ConnectionException(TransportError, s, 500);

                }

                else if (state == Protocol::ConnBody)
                {
                    ULXR_DOUT_XML(std::string(buff_ptr, myRead));
                    if (!parser->parse(buff_ptr, myRead, false))
                    {
                        throw XmlException(parser->mapToFaultCode(parser->getErrorCode()),
                                           "Problem while parsing xml response",
                                           parser->getCurrentLineNumber(),
                                           parser->getErrorString(parser->getErrorCode()));
                    }
                    myRead = 0;
                }
            }

            if (!protocol->hasBytesToRead())
//        || parser->isComplete())
                done = true;
        }

        if (protocol->isOpen())
            protocol->closeConnection();

        return rpb->getMethodResponse();
    }


    MethodResponse
    Requester::call (const MethodCall& calldata, const std::string &rpc_root,
                     const std::string &user, const std::string &pass)
    {
        ULXR_TRACE("call(..,user, pass)");
        protocol->setMessageAuthentication(user, pass);
        send_call (calldata, rpc_root);
        return waitForResponse();
    }


    MethodResponse
    Requester::call (const MethodCall& calldata, const std::string &rpc_root)
    {
        ULXR_TRACE("call");
        send_call (calldata, rpc_root);
        return waitForResponse();
    }

}  // namespace ulxr
