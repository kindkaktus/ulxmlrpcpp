

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <list>
#include <unistd.h>

#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_signature.h>
#include <ulxmlrpcpp/ulxr_dispatcher.h>

static bool haveOption(int argc, char **argv, const char *name)
{
    for (int i = 0; i < argc; ++i)
    {
        if (strcmp(argv[i], name) == 0)
            return true;
    }
    return false;
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
        ulxr::RpcString myServiceName = calldata.getParam(0);
        //std::cout << "Service Name: " << myServiceName.getString() << "\n";
        ulxr::MethodResponse resp;

        const char certTempl[] = "Certificate:\n\
                                     Data:\n\
                                     Version: 3 (0x2)\n\
                                     Serial Number:\n\
                                     db:8d:e2:c2:9f:c9:b0:d1\n\
                                     Signature Algorithm: sha1WithRSAEncryption\n\
                                     Issuer: C=NL, ST=NB, L=Eindhoven, O=Sioux, OU=DC, CN=Jeroen/emailAddress=jeroen@jeroen.ws\n\
                                     Validity\n\
                                     Not Before: Apr 14 12:40:22 2008 GMT\n\
                                     Not After : Apr 14 12:40:22 2011 GMT\n\
                                     Subject: C=NL, ST=NB, L=Eindhoven, O=Sioux, OU=%s, CN=Jeroen/emailAddress=jeroen@jeroen.ws\n\
                                     Subject Public Key Info:\n\
                                     Public Key Algorithm: rsaEncryption\n\
                                     RSA Public Key: (1024 bit)\n\
                                     Modulus (1024 bit):\n\
                                                         00:dc:5c:63:78:6d:f2:35:6e:05:6d:b0:93:b9:7e:\n\
                                                         6a:df:2d:24:9a:78:37:12:0d:de:91:d7:e1:72:dc:\n\
                                                         ef:75:1d:2b:60:52:e5:31:09:85:84:e8:38:aa:00:\n\
                                                         fa:c1:15:bc:2c:63:58:81:d5:b9:3a:f3:55:31:5f:\n\
                                                         77:3d:9a:6a:fb:60:42:1e:4d:b0:e4:ee:ef:9d:b7:\n\
                                                         a3:a0:ae:76:ba:20:b0:1b:bd:a9:15:5a:fb:39:40:\n\
                                                         52:af:91:ac:2e:95:cc:00:9d:22:c2:5b:a0:7c:9b:\n\
                                                         8d:22:ca:95:ed:83:a5:ac:fd:82:72:57:bd:a4:61:\n\
                                                         af:ec:91:b6:c5:bd:bd:a2:6b\n\
                                                         Exponent: 65537 (0x10001)\n\
                                                         X509v3 extensions:\n\
                                                         X509v3 Basic Constraints:\n\
                                                         CA:FALSE\n\
                                                         Netscape Comment:\n\
                                                         OpenSSL Generated Certificate\n\
                                                         X509v3 Subject Key Identifier:\n\
                                                         37:18:CD:16:B5:3C:0E:89:FE:93:75:B3:E8:FC:DC:79:61:4B:CB:0C\n\
                                                         X509v3 Authority Key Identifier:\n\
                                                         keyid:E3:24:72:BE:C4:9E:DE:F4:80:D9:A2:7C:2B:AA:60:A5:E9:22:3C:04\n\
                                                         DirName:/C=NL/ST=NB/L=Eindhoven/O=Sioux/OU=DC/CN=Jeroen/emailAddress=jeroen@jeroen.ws\n\
                                                         serial:FA:C4:FC:92:38:F0:18:87\n\
                                                         \n\
                                                         Signature Algorithm: sha1WithRSAEncryption\n\
                                                         73:ea:d6:bc:3e:53:73:bf:8a:8b:b4:9e:07:78:79:91:4e:d3:\n\
                                                         92:2f:b0:3f:5e:d5:4f:a9:9f:0f:a6:90:51:61:73:10:08:88:\n\
                                                         4a:c0:09:03:d6:d3:19:52:40:18:94:61:0a:26:74:be:f0:e4:\n\
                                                         2d:34:02:21:38:e2:a7:c7:64:82:ec:97:30:c0:07:a7:df:56:\n\
                                                         b2:d4:2b:16:6c:80:e9:f1:06:94:a4:56:bc:4e:6a:5a:7f:2a:\n\
                                                         a4:01:75:63:3c:23:9d:9c:90:4c:11:08:16:43:75:a7:b6:73:\n\
                                                         e3:97:68:92:37:7c:43:d4:95:76:9e:e3:f7:06:29:a2:f3:6b:\n\
                                                         58:d0\
                                                         ";
        char cert[1024*8 + 1] = {};
        snprintf(cert, sizeof(cert)-1, certTempl, myServiceName.getString().c_str());
        resp.setResult(ulxr::Base64(cert));
        return resp;
    }

    // Return a sorted merged array from two given arrays
    ulxr::MethodResponse mergeArrays (const ulxr::MethodCall &calldata)
    {
        ulxr::Array myArr1 = calldata.getParam(0);
        ulxr::Array myArr2 = calldata.getParam(1);

        std::list<int> l1, l2;
        for (unsigned int i=0; i < myArr1.size(); ++i)
        {
            ulxr::Integer val = myArr1.getItem(i);
            l1.push_back(val.getInteger());
        }
        for (unsigned int i=0; i < myArr2.size(); ++i)
        {
            ulxr::Integer val = myArr2.getItem(i);
            l2.push_back(val.getInteger());
        }
        l1.sort();
        l2.sort();
        l1.merge(l2);

        ulxr::Array retVal;
        for (std::list<int>::const_iterator it = l1.begin(), end=l1.end(); it!=end; ++it)
            retVal.addItem(ulxr::Integer(*it));
        ulxr::MethodResponse resp;
        resp.setResult(retVal);
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
            ulxr::SSLConnection *ssl = new ulxr::SSLConnection (myIP, port);
            ssl->setCryptographyData("password", "foo-cert.pem", "foo-cert.pem");
            conn.reset(ssl);
        }
        else
        {
            conn.reset(new ulxr::TcpIpConnection (myIP, port));
        }

        conn->setTcpNoDelay(true);
        prot.reset(new ulxr::HttpProtocol(conn.get()));
        ulxr::Dispatcher server(prot.get());


        TestWorker worker;


        server.addMethod(ulxr::make_method(worker, &TestWorker::getCertificate),
                         ulxr::Signature(ulxr::Base64()),
                         "getCertificate",
                         ulxr::Signature(ulxr::RpcString()),
                         "Get Certificate for a given service");

        server.addMethod(ulxr::make_method(worker, &TestWorker::mergeArrays),
                         ulxr::Signature(ulxr::Array()),
                         "mergeArrays",
                         ulxr::Signature() << ulxr::Array() << ulxr::Array(),
                         "Return a sorted merged from two given arrays");

        server.addMethod(ulxr::make_method(worker, &TestWorker::shutdown),
                         ulxr::Signature(ulxr::Boolean()),
                         "testcall_shutdown",
                         ulxr::Signature(),
                         "Testcase with a dynamic method in a class, shut down server, return old state");

        prot->setAcceptCookies(true);

        while (worker.running)
        {
            try
            {
                /*timeval startTick, endTick;

                std::cout << "\n\nWaiting for connection.....\n";
                gettimeofday(&startTick, NULL);
                */
                ulxr::MethodCall call = server.waitForCall();
                /*gettimeofday(&endTick, NULL);

                long secs = endTick.tv_sec - startTick.tv_sec;
                long usecs = endTick.tv_usec - startTick.tv_usec;

                double elapsed = (double)secs * 1000;
                elapsed += (double)usecs/(double)1000;

                std::cout << "waitForCall took " << elapsed << " msec\n";

                std::cout << "hostname: " << conn->getHostName() << "\n";
                std::cout << "peername: " << conn->getPeerName() << "\n";

                if (prot.hasCookie())
                {
                    std::string s = prot.getCookie();
                    std::cout << "received cookie: " << s << "\n";
                    prot.setServerCookie(s + "; received=true");
                }
                else
                {
                    prot.setServerCookie("newcookie=mycookie");
                    std::cout << "no cookie received\n";
                }


                gettimeofday(&startTick, NULL);
                */
                ulxr::MethodResponse resp = server.dispatchCall(call);
                /*
                gettimeofday(&endTick, NULL);
                secs = endTick.tv_sec - startTick.tv_sec;
                usecs = endTick.tv_usec - startTick.tv_usec;

                elapsed = (double)secs * 1000;
                elapsed += (double)usecs/(double)1000;

                std::cout << "dispatchCall took " << elapsed << " msec\n";
                */
                server.sendResponse(resp);
                prot->close();

            }
            catch(ulxr::Exception& ex)
            {
                std::cout << "**** Error occured: " << ex.why() << std::endl;
                if (prot.get() && prot->isOpen())
                {
                    try
                    {
                        ulxr::MethodResponse resp(1, ex.why() );
                        server.sendResponse(resp);
                    }
                    catch(...)
                    {
                        std::cout << "error within exception occured\n";
                    }
                    prot->close();
                }
                std::cout << "Continue....\n";
            }
        }
    }

    catch(ulxr::Exception& ex)
    {
        std::cout << "Exception caucht, type: " << typeid(ex).name() << "\n";
        std::cout << "Error occured: " << ex.why() << std::endl;
        if (prot.get() && prot->isOpen())
        {
            prot->close();
        }
        return 1;
    }
    return 0;
}
