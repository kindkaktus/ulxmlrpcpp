/***************************************************************************
             mprocess_server.cpp  -  multi process rpc server test
                             -------------------
    begin                : Sun May 29 2005
    copyright            : (C) 2005 funtik <funt@alarit.com>

    $Id: mprocess_server.cpp 11056 2011-10-18 13:20:50Z korosteleva $

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
#include <ulxmlrpcpp/ulxr_tcpip_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_signature.h>
#include <ulxmlrpcpp/ulxr_dispatcher.h>
#include <ulxmlrpcpp/contrib/mprocess_rpc_server.h>

#include <cstring>
#include <memory>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

#include <sys/types.h>
#include <unistd.h>


funtik::MultiProcessRpcServer server;
//flag for shutdown
int sst=0;

ulxr::MethodResponse check (const ulxr::MethodCall &calldata)
{

    pid_t pid=getpid();
    std::cout<<"It is a check query. Process handle pid: "<<pid<<std::endl;

    ulxr::RpcString rpcs = calldata.getParam(0);
    std::string s = "It is RPC server."
                    "Your query string is: "+rpcs.getString();
    ulxr::MethodResponse resp;
    resp.setResult(ulxr::RpcString(s));
    return resp;
}

ulxr::MethodResponse finish_server (const ulxr::MethodCall &/*calldata*/)
{
    std::string s = "Shutdown server.";
    pid_t pid=getppid();
    if(kill(pid,SIGTERM))
    {
        std::cout<<"Cannt finish server work"<<std::endl;
    }
    ulxr::MethodResponse resp;
    resp.setResult(ulxr::RpcString(s));
    return resp;
}

void sigterm_handler(int /*signal*/)
{
    sst=1;
}

int main(int argc, char **argv)
{
    std::string host = "localhost";
    if (argc > 1)
        host = argv[1];

    unsigned port = 32000;
    if (argc > 2)
        port = atoi(argv[2]);


    std::cout << "Serving " << " securing " << " rpc requests at "
              << host << ":" << port << std::endl;

    try
    {
        std::auto_ptr<ulxr::TcpIpConnection> conn = std::auto_ptr<ulxr::TcpIpConnection>(new ulxr::TcpIpConnection (true, 0, port));

        ulxr::HttpProtocol prot(conn.get());

        server.getDispatcher()->setProtocol(&prot);
        ulxr::Dispatcher *dsp=server.getDispatcher();

        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_handler=sigterm_handler;
        sa.sa_flags=SA_NOCLDSTOP | SA_RESTART;
        sigaction(SIGTERM,&sa,0);


        ulxr::Struct ulxrStruct;
        ulxrStruct.addMember("first",ulxr::Integer());
        dsp->addMethod(ulxr::make_method(check),
                       ulxr::Signature() << (ulxrStruct<<ulxr::make_member("uptime",ulxr::Integer()) ),
                       "check_sig",
                       ulxr::Signature()<<  ulxr::RpcString(),
                       "Testcase return  string");

        dsp->addMethod(ulxr::make_method(check),
                       ulxr::Signature() << ulxr::RpcString(),
                       "check",
                       ulxr::Signature() << ulxr::RpcString(),
                       "Testcase return  string");

        dsp->addMethod(ulxr::make_method(finish_server),
                       ulxr::Signature() << ulxr::RpcString(),
                       "finish_server",
                       ulxr::Signature(),
                       "Testcase shutdown server");

        server.setState(funtik::MultiProcessRpcServer::RUN);
        while((server.getState()==funtik::MultiProcessRpcServer::RUN) && sst==0)
        {
            server.printProcess();

            if(!server.waitConnection())
            {
                std::cout<<"signale receive"<<std::endl;
                continue;
            }
            try {
                server.handleRequest();
            }
            catch(ulxr::Exception& ex)
            {
                std::cout << "Error occured: " << ex.why() << std::endl;
            }

        }//while
        server.terminateAllProcess(true);
    }
    catch(ulxr::Exception& ex)
    {
        std::cout << "Error occured: " << ex.why() << std::endl;
    }
    std::cout << "Well done, Ready.\n";
    return 0;
}
