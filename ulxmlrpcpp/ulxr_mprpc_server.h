/***************************************************************************
             mprocess_rpc_server.h  -  multi process rpc server
                             -------------------
    begin                : Sun May 29 2005
    copyright            : (C) 2005 Dmitry Nizovtsev <funt@alarit.com>
                                    Olexander Shtepa <isk@alarit.com>

    $Id: ulxr_mprpc_server.h 11073 2011-10-25 12:44:58Z korosteleva $

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


#ifndef ULXR_MPRPC_SERVER_H
#define ULXR_MPRPC_SERVER_H


#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_dispatcher.h>
#include <ulxmlrpcpp/ulxr_tcpip_connection.h>

#include <vector>
#include <memory>



namespace ulxr {


    class  MultiProcessRpcServerError : public std::exception
    {
        std::string _what;
    public:
        MultiProcessRpcServerError(const std::string& what_arg);
        ~MultiProcessRpcServerError() throw();
        const char* what () const throw();
    };

    /**
     *  @brief multi process handler for RPC requests.
     */

    class  MultiProcessRpcServer
    {
    public:

        /**
          * @brief Constructs rpc server with process-based request handling
         	*
          * @param poProtocol 	Protocol object
          * @param aNumProcesses   number of processes
        *
          */
        MultiProcessRpcServer(ulxr::Protocol* poProtocol, size_t aNumProcesses);


        /** @brief Destructs the rpc server.
         *
         *  Destructs the rpc server.
         *  All handlers will be forced to stop
         *
         */
        virtual ~MultiProcessRpcServer();

        /**
          Start serving.
          After the function returns the protocol's connection is not usable for serving any more.
        */
        virtual void start();

        virtual void terminateAllHandlers();
        // @normally should never return unless the signal is sent
        virtual void waitForAllHandlersFinish();

        std::vector<pid_t> getHandlers() const;

        /** Processes a call after it has been recieved and before it is dispatched.
          * @param  aCall   last received call
          * @param  aConnectionProtocol   current connection
          */
        virtual void preProcessCall(MethodCall & aCall, const Protocol *aConnectionProtocol = NULL);

        /** Processes a method response before it is sent back.
          * @param  resp   response to send back
          */
        virtual void preProcessResponse(MethodResponse &resp);



        /** Adds a user defined (static) method to the dispatcher.
          * You access a remote method by sending the "official" name. Sometimes
          * a method accepts different parameter sets (overloading in C++).
          * In this case you add the according signature. Finally you can
          * add a description to show the usage of this method.
          * @param  adr            the pointer to the implementation of the method
          * @param  ret_signature  the signature of the return value
          * @param  name           the name of the method
          * @param  signature      the signature of the parameters
          * @param  help           short usage description
          */
        void addMethod (MethodAdder::StaticMethodCall_t adr,
                        const std::string &ret_signature,
                        const std::string &name,
                        const std::string &signature,
                        const std::string &help = "");

        /** Adds a user defined (dynamic) method to the dispatcher.
          * You access a remote method by sending the "official" name. Sometimes
          * a method accepts different parameter sets (overloading in C++).
          * In this case you add the according signature. Finally you can
          * add a description to show the usage of this method.
          * @param  wrapper        the pointer to the wrapper to the method.
          *                        Important: Dispatcher owns now and deletes this wrapper object!
          * @param  ret_signature  the signature of the return value
          * @param  name           the name of the method
          * @param  signature      the signature of the parameters
          * @param  help           short usage description
          */
        void addMethod (MethodAdder::DynamicMethodCall_t wrapper,
                        const std::string &ret_signature,
                        const std::string &name,
                        const std::string &signature,
                        const std::string &help = "");

        /** Adds a system internal method to the dispatcher.
          * You access a remote method by sending the "official" name. Sometimes
          * a method accepts different parameter sets (overloading in C++).
          * In this case you add the according signature. Finally you can
          * add a description to show the usage of this method.
          * @param  adr            the pointer to the implementation of the method
          * @param  ret_signature  the signature of the return value
          * @param  name           the name of the method
          * @param  signature      the signature of the parameters
          * @param  help           short usage description
          */
        void addMethod (MethodAdder::SystemMethodCall_t adr,
                        const std::string &ret_signature,
                        const std::string &name,
                        const std::string &signature,
                        const std::string &help = "");

        /** Adds a user defined (static) method to the dispatcher.
          * You access a remote method by sending the "official" name. Sometimes
          * a method accepts different parameter sets (overloading in C++).
          * In this case you add the according signature. Finally you can
          * add a description to show the usage of this method.
          * @param  adr            the pointer to the implementation of the method
          * @param  ret_signature  the signature of the return value
          * @param  name           the name of the method
          * @param  signature      the signature of the parameters
          * @param  help           short usage description
          */
        void addMethod (MethodAdder::StaticMethodCall_t adr,
                        const Signature &ret_signature,
                        const std::string &name,
                        const Signature &signature,
                        const std::string &help = "");

        /** Adds a user defined (dynamic) method to the dispatcher.
          * You access a remote method by sending the "official" name. Sometimes
          * a method accepts different parameter sets (overloading in C++).
          * In this case you add the according signature. Finally you can
          * add a description to show the usage of this method.
          * @param  wrapper        the pointer to the wrapper to the method.
          *                        Important: Dispatcher owns now and deletes this wrapper object!
          * @param  ret_signature  the signature of the return value
          * @param  name           the name of the method
          * @param  signature      the signature of the parameters
          * @param  help           short usage description
          */
        void addMethod (MethodAdder::DynamicMethodCall_t wrapper,
                        const Signature &ret_signature,
                        const std::string &name,
                        const Signature &signature,
                        const std::string &help = "");

        /** Adds a system internal method to the dispatcher.
          * You access a remote method by sending the "official" name. Sometimes
          * a method accepts different parameter sets (overloading in C++).
          * In this case you add the according signature. Finally you can
          * add a description to show the usage of this method.
          * @param  adr            the pointer to the implementation of the method
          * @param  ret_signature  the signature of the return value
          * @param  name           the name of the method
          * @param  signature      the signature of the parameters
          * @param  help           short usage description
          */
        void addMethod (MethodAdder::SystemMethodCall_t adr,
                        const Signature &ret_signature,
                        const std::string &name,
                        const Signature &signature,
                        const std::string &help = "");

        /** Removes a method if available
          * @param name   method name
          */
        void removeMethod(const std::string &name);
    private:
        MultiProcessRpcServer(const MultiProcessRpcServer&);
        MultiProcessRpcServer& operator=(const MultiProcessRpcServer&);

        void startChildLoop();

    private:
        ulxr::Dispatcher*	    theDispatcher;
        const size_t            theNumProcesses;
        std::vector<pid_t>	    theProcessPool;
    };


}


#endif // ULXR_MPRPC_SERVER_H
