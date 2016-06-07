#include <iostream>

#define	ULXR_DEBUG_OUTPUT	1

#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <memory>
#include <fstream>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h>  // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_signature.h>
#include <ulxmlrpcpp/ulxr_dispatcher.h>

#include <ulxmlrpcpp/contrib/ssl_connection.h>

int iTestNum;

void doTestHandle(funtik::SSLConnection *conn,ulxr::Dispatcher& server,std::string strServerState)
{
    for(int i=0; i<8; i++)
    {
        try {
            std::cout <<"Test Number: "<<++iTestNum<<"  "<<strServerState.c_str()<< std::endl;
            ulxr::MethodCall call = server.waitForCall();
            ulxr::MethodResponse resp = server.dispatchCall(call);
            server.sendResponse(resp);

            server.getProtocol()->close();
            std::cout <<"Access allow"<<std::endl;


        }
        catch(ulxr::Exception& ex)
        {
            std::cout <<"Access denied"<<std::endl;
            std::cout << "Error occured: " << ex.why()
                      << std::endl;
            if (server.getProtocol()->isOpen())
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

                server.getProtocol()->close();
            }

        }
    }

}


ulxr::MethodResponse checkaccess (const ulxr::MethodCall &calldata)
{
    ulxr::RpcString rpcs = calldata.getParam(0);
    std::string s = "Access allow";
    ulxr::MethodResponse resp;
    resp.setResult(ulxr::RpcString(s));
    return resp;
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

    std::auto_ptr<funtik::SSLConnection> conn = std::auto_ptr<funtik::SSLConnection>(new funtik::SSLConnection (true, host, port));



    ulxr::HttpProtocol prot(conn.get());
    ulxr::Dispatcher server(&prot);


    try
    {
        server.addMethod(ulxr::make_method(checkaccess),
                         ulxr::Signature() << ulxr::RpcString(),
                         "checkaccess",
                         ulxr::Signature() << ulxr::RpcString(),
                         "Testcase return  string");

//start handle
//Check authentification options.


//without autentification.
//#1
        conn->setCryptographyData("", "sessioncert.pem", "sessioncert.pem");
        doTestHandle(conn.get(),server,"SSL server without SSL authentification. Used only session certificate.");

//#2
        conn->enableAuth(funtik::SSLConnection::CA_AUTH);
        conn->setCAFile("./ca-cert.pem");
        doTestHandle(conn.get(),server,"Set server SSL authentification via CA certificates.");

//#3
        conn->enableAuth(funtik::SSLConnection::FINGERPRINT_AUTH);

        std::ifstream fsFingerprintFile("./fingerprint_storage");
        if(!fsFingerprintFile)
        {
            std::cout<<"Cannt open file"<<std::endl;
        }
        while(fsFingerprintFile.good())
        {
            char tmp_buf[256];
            fsFingerprintFile.getline(tmp_buf,256);
            conn->addFingerprintData(tmp_buf);
        }
        fsFingerprintFile.close();

        doTestHandle(conn.get(),server,"Set server SSL authentification via Fingerprint of client`s certificate.");

//#4
        conn->enableAuth(funtik::SSLConnection::CHECK_REVOCATIONCERT);

        std::ifstream fsRevocationFile("./revocation_storage");
        if(!fsRevocationFile)
        {
            std::cout<<"Cannt open revocation file"<<std::endl;
        }
        while(fsRevocationFile.good())
        {
            char tmp_buf[256];
            fsRevocationFile.getline(tmp_buf,256);
            conn->addRevocationData(tmp_buf);
        }
        fsRevocationFile.close();

        doTestHandle(conn.get(),server,"Set server SSL authentification via Fingerprint of client`s certificate,check in revocation certificates.");

    }
    catch(ulxr::Exception& ex)
    {
        std::cout << "Error occured: " << ex.why() << std::endl;

    }
    std::cout << "Well done, Ready.\n";
    return 0;
}

#else

int main(int argc, char **argv)
{
    std::cout << "SSL stuff is disabled\n";
    return 0;
}
