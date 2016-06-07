#include <iostream>

#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <ctime>
#include <memory>
#include <fstream>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h>  // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_except.h>

#include <ulxmlrpcpp/contrib/ssl_connection.h>

int iTestNum;

void doTestHandle(funtik::SSLConnection *conn,ulxr::Requester& client,ulxr::HttpProtocol& prot,std::string strClientState)
{
    ulxr::MethodCall checkaccess ("checkaccess");
    checkaccess.addParam(ulxr::RpcString("string"));


    try {
        std::cout<<"  "<<strClientState.c_str();
        if(!prot.isOpen())
            prot.open();

        ulxr::MethodResponse resp = client.call(checkaccess, "/RPC2");
//    	    std::cout << "call result: \n";
//	    	std::cout << resp.getXml(0) << std::endl;
        std::cout <<"Access allow"<<std::endl;

    }
    catch(funtik::SSLConnectionException& ex)
    {
        std::cout <<"Access denied"<<std::endl;
        std::cout << "Error occured: " << ex.why()
                  << std::endl;
    }
    if(prot.isOpen())
        prot.close();

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


        std::cout << "Serving " << " securing " << " rpc requests at "
                  << host << ":" << port << std::endl;


        std::auto_ptr<funtik::SSLConnection> conn = std::auto_ptr<funtik::SSLConnection>(new funtik::SSLConnection (false, host, port));



        ulxr::HttpProtocol prot(conn.get());
        ulxr::Requester client(&prot);

        time_t starttime = time(0);


        for(int i=0; i<4; i++)
        {
//#1 test
//disable all auth options.
            std::cout <<"Test Number: "<<++iTestNum;
            conn->disableAuth();
            doTestHandle(conn.get(),client,prot,"Without client Certificate.");

//#2 test
            std::cout <<"Test Number: "<<++iTestNum;
            conn->setCryptographyData("password", "nosignedcert.pem", "nosignedcert.pem");
            doTestHandle(conn.get(),client,prot,"Load unsigned certificate. ");

//#3 test
            std::cout <<"Test Number: "<<++iTestNum;
            conn->enableAuth(funtik::SSLConnection::MANDATORY_CLIENTCERT);
            doTestHandle(conn.get(),client,prot,"Set use unsigned certificate.");

//#4 test
            std::cout <<"Test Number: "<<++iTestNum;
            conn->setCryptographyData("", "signedcert.pem", "signedcert.pem");
            doTestHandle(conn.get(),client,prot,"Load signed revocation certificate.");


//#5 test
            std::cout <<"Test Number: "<<++iTestNum;
            conn->setCryptographyData("", "signedcert2.pem", "signedcert2.pem");
            doTestHandle(conn.get(),client,prot,"Load signed good certificate.");

//#6 test
            std::cout <<"Test Number: "<<++iTestNum;
            conn->enableAuth(funtik::SSLConnection::CA_AUTH);
            conn->setCAFile("./ca-cert.pem");
            doTestHandle(conn.get(),client,prot,"Set CA certificate checking server.");


//#7 test
            std::cout <<"Test Number: "<<++iTestNum;
            conn->enableAuth(funtik::SSLConnection::FINGERPRINT_AUTH);

            std::ifstream fsFingerprintFile("fingerprint_storage");
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
            doTestHandle(conn.get(),client,prot,"Set checking server certificate(Fingerprint only).");

//#8 test
            std::cout <<"Test Number: "<<++iTestNum;
            conn->enableAuth(funtik::SSLConnection::CHECK_REVOCATIONCERT);

            std::ifstream fsRevocationFile("revocation_storage");
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
            doTestHandle(conn.get(),client,prot,"Set Fingerprint certificate checking server(Fingerprint and revocation).");


            time_t endtime = time(0);
            unsigned mins = (endtime - starttime) / 60;
            unsigned secs = (endtime - starttime) % 60;
            std::cout << "\nOverall time needed: " << mins << ":"
                      << secs << std::endl;
        }
    }//for

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

    std::cout << "Well done, Ready.\n";
    return 0;
}

#else

int main(int argc, char **argv)
{
    std::cout << "SSL stuff is disabled\n";
    return 0;
}

