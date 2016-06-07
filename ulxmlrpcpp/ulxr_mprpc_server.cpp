/***************************************************************************
          mprocess_rpc_client.cpp.cpp  -  multi process rpc server
                             -------------------
    begin                : Sun May 29 2005
    copyright            : (C) 2005 Dmitry Nizovtsev <funt@alarit.com>
                                    Olexander Shtepa <isk@alarit.com>

    $Id: ulxr_mprpc_server.cpp 11073 2011-10-25 12:44:58Z korosteleva $

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




//#define ULXR_DEBUG_OUTPUT
//#define ULXR_SHOW_TRACE

#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <ulxmlrpcpp/ulxr_dispatcher.h>
#include <ulxmlrpcpp/ulxr_protocol.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_xmlparse.h>
#include <ulxmlrpcpp/ulxr_xmlparse_base.h>
#include <ulxmlrpcpp/ulxr_callparse.h>
#include <ulxmlrpcpp/ulxr_mprpc_server.h>

#include <iostream>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>



namespace ulxr {


    MultiProcessRpcServerError::MultiProcessRpcServerError(const std::string& what_arg): _what(what_arg)
    {}

    MultiProcessRpcServerError::~MultiProcessRpcServerError() throw()
    {}

    const char*  MultiProcessRpcServerError::what () const throw()
    {
        return this->_what.c_str();
    }


    MultiProcessRpcServer::MultiProcessRpcServer(ulxr::Protocol* poProtocol,  size_t aNumProcesses)
        :   theNumProcesses(aNumProcesses)
        , theDispatcher(NULL)
    {
        if (aNumProcesses == 0)
            throw MultiProcessRpcServerError("At least handler process expected");
        theDispatcher = new ulxr::Dispatcher(poProtocol);
    }

    MultiProcessRpcServer::~MultiProcessRpcServer()
    {
        terminateAllHandlers();
        waitForAllHandlersFinish();
        delete theDispatcher;
    }

    void MultiProcessRpcServer::preProcessCall(MethodCall & aCall, const Protocol *aConnectionProtocol)
    {}

    void MultiProcessRpcServer::preProcessResponse(MethodResponse &/*resp*/)
    {}


    void MultiProcessRpcServer::startChildLoop()
    {
        ULXR_TRACE("startChildLoop");

        Protocol* protocol = theDispatcher->getProtocol();
        Dispatcher waiter(protocol);

        while(true)
        {
            try
            {
                ULXR_TRACE("Process ");
                MethodCall call = waiter.waitForCall();

                ULXR_TRACE("Process ");
                preProcessCall(call, waiter.getProtocol());
                MethodResponse resp = theDispatcher->dispatchCall(call);
                preProcessResponse(resp);

                protocol->sendRpcResponse(resp);
                protocol->closeConnection();
            }
            catch (ConnectionException &ex)
            {
                if (protocol->isOpen())
                {
                    try
                    {
                        MethodResponse resp(ex.getStatusCode(), ex.why() );
                        protocol->sendRpcResponse(resp);
                    }
                    catch(...)
                    {}
                    protocol->closeConnection();
                }
            }
            catch(Exception& ex)
            {
                if (protocol->isOpen())
                {
                    try
                    {
                        MethodResponse resp(1, ex.why() );
                        protocol->sendRpcResponse(resp);
                    }
                    catch(...)
                    {}
                    protocol->closeConnection();
                }
            }
            catch(std::exception& ex)
            {
                if (protocol->isOpen())
                {
                    try
                    {
                        MethodResponse resp(1, ex.what() );
                        protocol->sendRpcResponse(resp);
                    }
                    catch(...)
                    {}
                    protocol->closeConnection();
                }
            }
            catch(...)
            {
                if (protocol->isOpen())
                {
                    try
                    {
                        MethodResponse resp(1, "Unknown error occured" );
                        protocol->sendRpcResponse(resp);
                    }
                    catch(...)
                    {}
                    protocol->closeConnection();
                }
            }

        } // while true
    }

    void  MultiProcessRpcServer::start()
    {
        if (!theDispatcher)
            throw MultiProcessRpcServerError("Dispatcher not initialized");
        if (!theDispatcher->getProtocol())
            throw MultiProcessRpcServerError("Protocol not initialized");

        ULXR_TRACE(("Starting Multi-Process XMLRPC Server with " + toString(theNumProcesses) + " processes.").c_str());

        for (unsigned int i = 0; i < theNumProcesses; ++i)
        {
            pid_t ppid = fork();
            if (ppid == -1)
                throw MultiProcessRpcServerError("Cannot create handler process.");

            if (ppid == 0)// child
            {

                try
                {
                    startChildLoop();
                }
                catch (...)
                {}
                _exit(1);
            }

            // parent
            theProcessPool.push_back(ppid);
        }
        if (!theProcessPool.empty())
        {
            // from now childern serve connections so the parent shall phase out
            theDispatcher->getProtocol()->stopServing();
        }
    }

    void MultiProcessRpcServer::terminateAllHandlers()
    {
        while (!theProcessPool.empty())
        {
            std::vector<pid_t>::iterator it = theProcessPool.begin();
            kill(*it, SIGTERM);
            kill(*it, SIGKILL);
            theProcessPool.erase(it);
        }
    }

    void MultiProcessRpcServer::waitForAllHandlersFinish()
    {
        while (!theProcessPool.empty())
        {
            std::vector<pid_t>::iterator it = theProcessPool.begin();
            int status;
            waitpid(*it, &status, 0 );
            theProcessPool.erase(it);
        }
    }

    std::vector<pid_t> MultiProcessRpcServer::getHandlers() const
    {
        return theProcessPool;
    }


    void
    MultiProcessRpcServer::addMethod (MethodAdder::StaticMethodCall_t adr,
                                      const std::string &ret_signature,
                                      const std::string &name,
                                      const std::string &signature,
                                      const std::string &help)
    {
        theDispatcher->addMethod(adr, ret_signature, name, signature, help);
    }


    void
    MultiProcessRpcServer::addMethod (MethodAdder::DynamicMethodCall_t wrapper,
                                      const std::string &ret_signature,
                                      const std::string &name,
                                      const std::string &signature,
                                      const std::string &help)
    {
        theDispatcher->addMethod(wrapper, ret_signature, name, signature, help);
    }


    void
    MultiProcessRpcServer::addMethod (MethodAdder::SystemMethodCall_t adr,
                                      const std::string &ret_signature,
                                      const std::string &name,
                                      const std::string &signature,
                                      const std::string &help)
    {
        theDispatcher->addMethod(adr, ret_signature, name, signature, help);
    }


    void
    MultiProcessRpcServer::addMethod (MethodAdder::StaticMethodCall_t adr,
                                      const Signature &ret_signature,
                                      const std::string &name,
                                      const Signature &signature,
                                      const std::string &help)
    {
        theDispatcher->addMethod(adr, ret_signature, name, signature, help);
    }


    void
    MultiProcessRpcServer::addMethod (MethodAdder::DynamicMethodCall_t wrapper,
                                      const Signature &ret_signature,
                                      const std::string &name,
                                      const Signature &signature,
                                      const std::string &help)
    {
        theDispatcher->addMethod(wrapper, ret_signature, name, signature, help);
    }


    void
    MultiProcessRpcServer::addMethod (MethodAdder::SystemMethodCall_t adr,
                                      const Signature &ret_signature,
                                      const std::string &name,
                                      const Signature &signature,
                                      const std::string &help)
    {
        theDispatcher->addMethod(adr, ret_signature, name, signature, help);
    }


    void
    MultiProcessRpcServer::removeMethod(const std::string &name)
    {
        theDispatcher->removeMethod(name);
    }

} // namespace ulxr
