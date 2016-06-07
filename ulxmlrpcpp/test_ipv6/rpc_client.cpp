

#include <cstdlib>
#include <iostream>
#include <ctime>
#include <memory>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_ssl_connection.h>  // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_except.h>


static bool haveOption(int argc, char **argv, const char *name)
{
    for (int i = 0; i < argc; ++i)
    {
        if (strcmp(argv[i], name) == 0)
            return true;
    }
    return false;
}

void mysleep(unsigned int aMsec)
{
    timespec myTime;
    myTime.tv_sec = aMsec / 1000;
    myTime.tv_nsec = ((long)(aMsec % 1000)) * 1000000;
    nanosleep (&myTime, NULL);
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

        bool secure = haveOption(argc, argv, "ssl");

        std::string sec = "unsecured";
        if (secure)
            sec = "secured";

        std::cout << "Requesting " << sec << " rpc calls at "
                  << host << ":" << port << std::endl;

        const unsigned int myNumParallelRequests = 4;
        std::vector<pid_t> myPids;
        const unsigned int myNumRequestsPerClient = 100;

        std::cout << "call getCert: \n";
        timeval startTick, endTick;
        gettimeofday(&startTick, NULL);

        for (unsigned int clnt = 0; clnt < myNumParallelRequests; ++clnt)
        {
            pid_t ppid = fork();
            if (ppid == 0)
            {   // child
                for (unsigned i=0; i < myNumRequestsPerClient; ++i)
                {
                    std::auto_ptr<ulxr::TcpIpConnection> conn;
                    if (secure)
                        conn.reset(new ulxr::SSLConnection (host, port, false));
                    else
                        conn.reset(new ulxr::TcpIpConnection (host, port));

                    conn->setTcpNoDelay(true);
                    ulxr::HttpProtocol prot(conn.get());
                    ulxr::Requester client(&prot);


                    // Call getCert
                    //
                    std::string myCustNameArg  = "CUST_<&>\r\nPASSWORD";
                    ulxr::MethodCall getCertProxyFunc ("getCertificate");
                    getCertProxyFunc.addParam(ulxr::RpcString(myCustNameArg.c_str()));

                    ulxr::MethodResponse resp = client.call(getCertProxyFunc, "/RPC2");

                    std::string myCert = ulxr::RpcString(resp.getResult()).getString();
                    std::cout << "Request#" << i << ": "<< myCert << "\n";
                }
                _exit(0);
            } // child

            // parent
            myPids.push_back(ppid);
        }

        for (int i=0; i < myPids.size(); ++i)
        {
            int status;
            waitpid(myPids[i], &status, 0);
        }
        gettimeofday(&endTick, NULL);

        long secs = endTick.tv_sec - startTick.tv_sec;
        long usecs = endTick.tv_usec - startTick.tv_usec;
        double elapsed = (double)secs * 1000;
        elapsed += (double)usecs/(double)1000;
        std::cout << "Finished. Elapsed Time : " << elapsed << " msec\n";
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
