
#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <list>

#include <ulxmlrpcpp/ulxr_ssl_connection.h>  // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_signature.h>
#include <ulxmlrpcpp/ulxr_mprpc_server.h>

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


    ulxr::MethodResponse getCertificate (const ulxr::MethodCall &calldata)
    {
        std::string myServiceName = ulxr::RpcString(calldata.getParam(0)).getString();
        std::cout << "Request certificate for service: " << myServiceName << "\n";
        ulxr::MethodResponse resp;
        mysleep(10);
        resp.setResult(ulxr::RpcString("here is the certificate"));
        return resp;
    }
    bool running;
};

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    std::string ipv4 = "127.0.0.1";
    std::string ipv6 = "::1";
    if (argc > 1)
        ipv4 = argv[1];

    if (argc > 2)
        ipv6 = argv[2];

    unsigned port = 32000;
    if (argc > 3)
        port = atoi(argv[3]);

    bool secure = haveOption(argc, argv, "ssl");

    std::string sec = "unsecured";
    if (secure)
        sec = "secured";

    std::cout << "Serving " << sec << " rpc requests at "
              << ipv4 << ":" << port << std::endl
              << ipv6 << ":" << port << std::endl;

    ulxr::IP myIP;
    myIP.ipv4 = ipv4;
    myIP.ipv6 = ipv6;

    std::auto_ptr<ulxr::HttpProtocol> prot;
    try
    {
        std::auto_ptr<ulxr::TcpIpConnection> conn;
        if (secure)
        {
            ulxr::SSLConnection *ssl = new ulxr::SSLConnection (myIP, port, false);
            ssl->setCryptographyData("password", "foo-cert.pem", "foo-cert.pem");
            conn.reset(ssl);
        }
        else
        {
            conn.reset(new ulxr::TcpIpConnection (myIP, port));
        }

        conn->setTcpNoDelay(true);
        prot.reset(new ulxr::HttpProtocol(conn.get()));

        unsigned int myNumProc = 4;
        ulxr::MultiProcessRpcServer server(prot.get(), myNumProc);

        TestWorker worker;

        server.addMethod(ulxr::make_method(worker, &TestWorker::getCertificate),
                         ulxr::Signature(ulxr::RpcString()),
                         "getCertificate",
                         ulxr::Signature(ulxr::RpcString()),
                         "Get Certificate for a given service");

        server.addMethod(ulxr::make_method(worker, &TestWorker::shutdown),
                         ulxr::Signature(ulxr::Boolean()),
                         "testcall_shutdown",
                         ulxr::Signature(),
                         "Testcase with a dynamic method in a class, shut down server, return old state");


        server.start();
        server.waitForAllHandlersFinish();
    }
    catch(ulxr::Exception &ex)
    {
        std::cout << "Error occured: " << ex.why() << std::endl;
        return 1;
    }
    catch(std::exception &ex)
    {
        std::cout << "Error occured: " << ex.what() << std::endl;
        return 1;
    }
    catch(...)
    {
        std::cout << "unknown Error occured.\n";
        return 1;
    }

    return 0;
}
