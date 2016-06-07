

#include <cstdlib>
#include <iostream>
#include <ctime>
#include <memory>
#include <cstring>
#include <unistd.h>

#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_except.h>

#include "boost/foreach.hpp"
#define foreach BOOST_FOREACH

static bool haveOption(int argc, char **argv, const char *name)
{
    for (int i = 0; i < argc; ++i)
    {
        if (strcmp(argv[i], name) == 0)
            return true;
    }
    return false;
}

int main(int argc, char **argv)
{
    try
    {
        std::string host = "localhost";
        if (argc > 1)
            host = argv[1];

        unsigned port = 32000;
        if (argc > 2)
            port = atoi(argv[2]);

        bool big = haveOption(argc, argv, "big");
        bool secure = haveOption(argc, argv, "ssl");

        std::string sec = "unsecured";
        if (secure)
            sec = "secured";

        std::cout << "Requesting " << sec << " rpc calls at "
                  << host << ":" << port << std::endl;

        std::auto_ptr<ulxr::TcpIpConnection> conn;
        if (secure)
            conn.reset(new ulxr::SSLConnection (host, port));
        else
            conn.reset(new ulxr::TcpIpConnection (host, port));

        conn->setTcpNoDelay(true);
        ulxr::HttpProtocol prot(conn.get());
        ulxr::Requester client(&prot);


        /////////////////////////////

        ulxr::MethodCall list_methods ("system.listMethods");

        ulxr::MethodCall method_help ("system.methodHelp");
        method_help.addParam(ulxr::RpcString("system.methodHelp"));

        ulxr::MethodCall method_sig ("system.methodSignature");
        method_sig.addParam(ulxr::RpcString("system.methodSignature"));

        ulxr::MethodResponse resp;

        /////////////////////////////
        //
        prot.setAcceptCookies(true);
        size_t myPayloadBytesTransferred = 0;

        // TODO: write adapters

        // Call mergeArrays
        //
        time_t starttime = time(0);
        int numCalls = 10;
        for (int i=0; i < numCalls; ++i)
        {
            std::vector<int> v1;
            v1.push_back(3); v1.push_back(1); v1.push_back(2);
            std::vector<int> v2;
            v1.push_back(4); v1.push_back(1); v1.push_back(6);

            ulxr::Array arr1, arr2;
            foreach (int val, v1)
                arr1.addItem(ulxr::Integer(val));
            foreach (int val, v2)
                arr2.addItem(ulxr::Integer(val));

            ulxr::MethodCall mergeArraysProxyFunc ("mergeArrays");
            mergeArraysProxyFunc.addParam(arr1).addParam(arr2);
            resp = client.call(mergeArraysProxyFunc, "/RPC2");
            ulxr::Array retVal = resp.getResult();
            std::vector<int> v;
            for (unsigned int i=0; i < retVal.size(); ++i)
            {
                ulxr::Integer val = retVal.getItem(i);
                v.push_back(val.getInteger());
            }

            std::vector<int> expRes;
            expRes.push_back(1); expRes.push_back(1); expRes.push_back(2);
            expRes.push_back(3); expRes.push_back(4); expRes.push_back(6);
            if (expRes != v)
                std::cerr << "Merge call produced invalid result\n";
            else
                std::cout << "Call ok" << std::endl;

            myPayloadBytesTransferred += (v1.size() + v2.size() + v.size());

        }
        time_t endtime = time(0);
        time_t totalsecs = endtime - starttime;
        std::cout << "\n" << numCalls << " remote calls to 'mergeArrays' performed\n";
        std::cout << "Time: " << totalsecs << " sec, "<< (totalsecs?numCalls/totalsecs:0) << " calls/sec\n";
        std::cout << "Transferred payload: " << myPayloadBytesTransferred/1024 << " KB, " << (totalsecs?myPayloadBytesTransferred/totalsecs:0)/1024<< " KB/sec\n";


        // Call getCert
        //
        starttime = time(0);
        numCalls = 100;
        for (int i=0; i < numCalls; ++i)
        {
            std::string myCustNameArg  = "CUST_PASSWORD";
            ulxr::MethodCall getCertProxyFunc ("getCertificate");
            getCertProxyFunc.addParam(ulxr::RpcString(myCustNameArg.c_str()));
            //std::cout << "call getCert: \n";
            //timeval startTick, endTick;
            //gettimeofday(&startTick, NULL);
            resp = client.call(getCertProxyFunc, "/RPC2");
            //gettimeofday(&endTick, NULL);

            //long secs = endTick.tv_sec - startTick.tv_sec;
            //long usecs = endTick.tv_usec - startTick.tv_usec;

            //double elapsed = (double)secs * 1000;
            //elapsed += (double)usecs/(double)1000;

            //std::cout << "call took " << elapsed << " msec\n";
            ulxr::Base64 b64 = resp.getResult();
            std::string myCert = b64.getString();
            myPayloadBytesTransferred += (myCustNameArg.size() + myCert.size());
            //std::cout <<  "getCert result: " << b64.getString() << "\n";
        }
        endtime = time(0);
        totalsecs = endtime - starttime;
        std::cout << "\n" << numCalls << "  remote calls to 'getCertificate' performed\n";
        std::cout << "Time: " << totalsecs << " sec, "<< (totalsecs?numCalls/totalsecs:0) << " calls/sec\n";
        std::cout << "Transferred payload: " << myPayloadBytesTransferred/1024 << " KB, " << (totalsecs?myPayloadBytesTransferred/totalsecs:0)/1024<< " KB/sec\n";


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
    return 0;
}
