#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_signature.h>
#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_mprpc_server.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>
#include <sys/time.h>
#include <time.h>

//@note the source should be in utf-8

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

struct StrBool
{
    StrBool() {}
    StrBool(const std::string& anS, bool anB) : s(anS), b(anB) {}
    std::string s;
    bool b;
    bool operator==(const StrBool& aRhs) const  { return (s == aRhs.s) && (b == aRhs.b);  }
    bool operator!=(const StrBool& aRhs) const  {  return !(*this==aRhs);  }
};

struct MyStruct
{
    MyStruct() {}
    MyStruct(int anI, const std::vector<StrBool>& anStrBools) : i(anI), strBools(anStrBools) {}
    int i;
    std::vector<StrBool> strBools;
    bool operator==(const MyStruct& aRhs) const  { return (i == aRhs.i) && (strBools == aRhs.strBools); }
    bool operator!=(const MyStruct& aRhs) const  { return !(*this==aRhs);  }
};

template <typename DestContainerT>
DestContainerT deserializeIntArray(const ulxr::Array& anArray)
{
    DestContainerT myRetVal;
    for (size_t i=0; i< anArray.size(); ++i)
    {
        const int myElemInt = ulxr::Integer(anArray.getItem(i)).getInteger();
        myRetVal.push_back(myElemInt);
    }
    return myRetVal;
}

MyStruct deserializeMyStruct(const ulxr::Struct& aStruct)
{
    MyStruct myRetVal;
    myRetVal.i =  ulxr::Integer(aStruct.getMember("i")).getInteger();
    ulxr::Array myArray = ulxr::Array(aStruct.getMember("strBools"));
    for (size_t i=0; i< myArray.size(); ++i)
    {
        const ulxr::Struct myElem = ulxr::Struct(myArray.getItem(i));
        StrBool myStrBool;
        myStrBool.s = ulxr::RpcString(myElem.getMember("s")).getString();
        myStrBool.b = ulxr::Boolean(myElem.getMember("b")).getBoolean();
        myRetVal.strBools.push_back(myStrBool);
    }
    return myRetVal;
}

template <typename SrcContainerT>
ulxr::Array serializeIntContainer(const SrcContainerT& aContainer)
{
    ulxr::Array myRetVal;
    for (typename SrcContainerT::const_iterator it = aContainer.begin(), end = aContainer.end(); it != end; ++it)
        myRetVal.addItem(ulxr::Integer(*it));
    return myRetVal;
}

ulxr::Struct serializeMyStruct(const MyStruct& aMyStruct)
{
    ulxr::Struct myRetVal;

    ulxr::Array myStrBools;
    for (std::vector<StrBool>::const_iterator it = aMyStruct.strBools.begin(), end = aMyStruct.strBools.end(); it != end; ++it)
    {
        ulxr::Struct myStrBool;
        myStrBool << ulxr::make_member("s", ulxr::RpcString(it->s))
                  << ulxr::make_member("b", ulxr::Boolean(it->b));
        myStrBools.addItem(myStrBool);
    }
    myRetVal << ulxr::make_member("i", ulxr::Integer(aMyStruct.i))
             << ulxr::make_member("strBools", myStrBools);
    return myRetVal;
}

#define TEST_ASSERT(condition) if (!(condition)) { std::cerr << "TEST ASSERTION FAILED at " << __FILE__  << ":" <<__LINE__ << "\n" << #condition << "\n"; throw std::runtime_error("TEST FAILED");}
#define TEST_ASSERT_EQUALS(actual, expected) if (actual != expected) { std::cerr << "TEST EQUALITY ASSERTION FAILED at " << __FILE__  << ":" <<__LINE__ << "\nActual: " << actual << "\nExpected: " << expected << "\n"; throw std::runtime_error("TEST FAILED");}
#define TEST_ASSERT_EQUALS_NOPRINT(actual, expected) if (actual != expected) { std::cerr << "TEST EQUALITY ASSERTION FAILED at " << __FILE__  << ":" <<__LINE__ << "\n" << #actual << " != " << #expected << "\n"; throw std::runtime_error("TEST FAILED");}

class TestWorker
{
public:

    TestWorker ()
    {}
    ulxr::MethodResponse echo(const ulxr::MethodCall &args)
    {
        // args: int, boolean, double, date, date, string, base64, array of int, struct {int, array of struct {string, bool} }
        int myIntArg = ulxr::Integer(args.getParam(0)).getInteger();
        bool myBoolArg = ulxr::Boolean(args.getParam(1)).getBoolean();
        double myDoubleArg = ulxr::Double(args.getParam(2)).getDouble();
        std::string myDateArg1 = ulxr::DateTime(args.getParam(3)).getDateTime();
        std::string myDateArg2 = ulxr::DateTime(args.getParam(4)).getDateTime();
        std::string myStrArg = ulxr::RpcString(args.getParam(5)).getString();
        std::string myB64Arg = ulxr::Base64(args.getParam(6)).getString();
        std::vector<int> myArrIntArg = deserializeIntArray<std::vector<int> >(ulxr::Array(args.getParam(7)));
        MyStruct myStructArg = deserializeMyStruct(ulxr::Struct(args.getParam(8)));

        ulxr::MethodResponse resp;
        ulxr::Struct myRetVal;
        myRetVal << ulxr::make_member("arg1", ulxr::Integer(myIntArg))
                 << ulxr::make_member("arg2", ulxr::Boolean(myBoolArg))
                 << ulxr::make_member("arg3", ulxr::Double(myDoubleArg))
                 << ulxr::make_member("arg4", ulxr::DateTime(myDateArg1))
                 << ulxr::make_member("arg5", ulxr::DateTime(myDateArg2))
                 << ulxr::make_member("arg6", ulxr::RpcString(myStrArg))
                 << ulxr::make_member("arg7", ulxr::Base64(myB64Arg))
                 << ulxr::make_member("arg8", serializeIntContainer<std::vector<int> >(myArrIntArg))
                 << ulxr::make_member("arg9", serializeMyStruct(myStructArg));

        resp.setResult(myRetVal);
        return resp;
    }
};

//@return call success flag
void callEcho(ulxr::Requester& aClient)
{
    // Setup call args
    const int myIntArg = -123;
    const bool myBoolArg = true;
    const double myDoubleArg = -3.1415;
    const std::string myDateArg1 = "20110912T10:23:45";
    const time_t myDateArg2 = 0;
    std::string myStrArg  = "<>&\"'string\t\r\n</xml>&amp;&lt;\"'0xd9f0Анди)/()/";
    std::string myB64Arg  =  "<>&\"'binary garbage\xf9\xFF\t\r\n";
    myB64Arg += '\0';
    myB64Arg += "\x2F</xml>&amp;&lt;\"'0xd9f0Анди)/()/";
    std::vector<int> myArrayArg;
    myArrayArg.push_back(0);
    myArrayArg.push_back(999);
    myArrayArg.push_back(-1);
    std::vector<StrBool> myStrBools;
    myStrBools.push_back(StrBool("s1", true));
    myStrBools.push_back(StrBool("s2", false));
    const MyStruct myStructArg(9876, myStrBools);

    ulxr::MethodCall echoProxyFunc ("echo");
    echoProxyFunc.addParam(ulxr::Integer(myIntArg));
    echoProxyFunc.addParam(ulxr::Boolean(myBoolArg));
    echoProxyFunc.addParam(ulxr::Double(myDoubleArg));
    echoProxyFunc.addParam(ulxr::DateTime(myDateArg1));
    echoProxyFunc.addParam(ulxr::DateTime(myDateArg2));
    echoProxyFunc.addParam(ulxr::RpcString(myStrArg));
    echoProxyFunc.addParam(ulxr::Base64(myB64Arg));
    echoProxyFunc.addParam(serializeIntContainer<std::vector<int> >(myArrayArg));
    echoProxyFunc.addParam(serializeMyStruct(myStructArg));

    ulxr::MethodResponse resp = aClient.call(echoProxyFunc, "/RPC2");

    const ulxr::Struct myResp = ulxr::Struct(resp.getResult());
    TEST_ASSERT(!myResp.hasMember("faultCode"));
    TEST_ASSERT_EQUALS(ulxr::Integer(myResp.getMember("arg1")).getInteger(), myIntArg);
    TEST_ASSERT_EQUALS(ulxr::Boolean(myResp.getMember("arg2")).getBoolean(), myBoolArg);
    TEST_ASSERT_EQUALS(ulxr::Double(myResp.getMember("arg3")).getDouble(), myDoubleArg);
    TEST_ASSERT_EQUALS(ulxr::DateTime(myResp.getMember("arg4")).getDateTime(), myDateArg1);
    //@todo fix&enable this check (use local time)
    // TEST_ASSERT_EQUALS(ulxr::DateTime(myResp.getMember("arg5")).getDateTime(), "19700101T00:00:00");
    TEST_ASSERT_EQUALS(ulxr::RpcString(myResp.getMember("arg6")).getString(), myStrArg);
    TEST_ASSERT_EQUALS(ulxr::Base64(myResp.getMember("arg7")).getString(), myB64Arg);
    TEST_ASSERT_EQUALS_NOPRINT(deserializeIntArray<std::vector<int> >(ulxr::Array(myResp.getMember("arg8"))), myArrayArg);
    TEST_ASSERT_EQUALS_NOPRINT(deserializeMyStruct(ulxr::Struct(myResp.getMember("arg9"))), myStructArg);
}

struct ExecTime
{
    ExecTime(size_t aNumCalls, size_t aNoSsl, size_t anSsl)
        : numCalls(aNumCalls), noSsl(aNoSsl), ssl(anSsl)
    {}

    size_t get(bool anIsSsl) const
    {
        return anIsSsl?ssl:noSsl;
    }

    size_t numCalls;
    size_t noSsl;
    size_t ssl;
};

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    // parse command-line args
    bool myIpv4Only = haveOption(argc, argv, "ipv4-only");
    bool myConnectToIpv4 = haveOption(argc, argv, "connect-ipv4") || myIpv4Only;
    bool myUseSsl = haveOption(argc, argv, "ssl");
    bool myIsPerformanceTest = haveOption(argc, argv, "performance");

    const std::string ipv4 = "127.0.0.1";
    const std::string ipv6 = myIpv4Only? "" : "::1";
    const unsigned port = 32000;

    std::cout << "Prepare to serve " << (myUseSsl ? "secured" : "unsecured") << " rpc requests at " << ipv4 << ":" << port;
    if (!myIpv4Only)
    {
        std::cout << ", " << ipv6 << ":" << port;
    }
    std::cout << std::endl;

    ulxr::IP myIP;
    myIP.ipv4 = ipv4;
    myIP.ipv6 = ipv6;

    std::auto_ptr<ulxr::HttpProtocol> mySvrProto;
    try
    {
        // Setup server
        std::auto_ptr<ulxr::TcpIpConnection> mySvrConn;
        if (myUseSsl)
        {
            ulxr::SSLConnection *ssl = new ulxr::SSLConnection (myIP, port, false);
            ssl->setCryptographyData("password", "foo-cert.pem", "foo-cert.pem");
            mySvrConn.reset(ssl);
        }
        else
        {
            mySvrConn.reset(new ulxr::TcpIpConnection (myIP, port));
        }

        mySvrConn->setTcpNoDelay(true);
        mySvrProto.reset(new ulxr::HttpProtocol(mySvrConn.get()));

        unsigned int myNumProc = 1;
        ulxr::MultiProcessRpcServer server(mySvrProto.get(), myNumProc);

        TestWorker worker;

        server.addMethod(ulxr::make_method(worker, &TestWorker::echo),
                         ulxr::Signature(ulxr::Struct()),
                         "echo",
                         ulxr::Signature() << ulxr::Integer() << ulxr::Boolean() << ulxr::Double() << ulxr::DateTime() << ulxr::DateTime() << ulxr::RpcString() << ulxr::Base64() << ulxr::Array() << ulxr::Struct());

        server.start();
        mysleep(500); // wait for the service to start

        timeval startTick, endTick;
        gettimeofday(&startTick, NULL);
        const ExecTime myExpectedExecTime(1000, 5000, 10000);
        const size_t myNumCalls = myIsPerformanceTest ? myExpectedExecTime.numCalls : 1;

        std::cout << "Prepare to request " << (myUseSsl ? "secured" : "unsecured") << " communication to " << (myConnectToIpv4 ? ipv4 : ipv6) << ":" << port << std::endl;
        std::auto_ptr<ulxr::TcpIpConnection> myClientConn;
        if (myUseSsl)
            myClientConn.reset(new ulxr::SSLConnection (myConnectToIpv4 ? ipv4 : ipv6, port, false));
        else
            myClientConn.reset(new ulxr::TcpIpConnection (myConnectToIpv4 ? ipv4 : ipv6, port));
        myClientConn->setTcpNoDelay(true);
        ulxr::HttpProtocol myClientProto(myClientConn.get());
        ulxr::Requester myClient(&myClientProto);

        for (size_t iCall = 0; iCall < myNumCalls; ++iCall)
        {
            callEcho(myClient);
        }
        if (myIsPerformanceTest)
        {
            gettimeofday(&endTick, NULL);
            long secs = endTick.tv_sec - startTick.tv_sec;
            long usecs = endTick.tv_usec - startTick.tv_usec;
            double elapsed = (double)secs * 1000;
            elapsed += (double)usecs/(double)1000;

            std::cout << "Finished. Elapsed Time for " << myNumCalls << " calls: " << (int)elapsed << " msec\n";
            TEST_ASSERT((int)elapsed < myExpectedExecTime.get(myUseSsl));
        }
    }
    catch(ulxr::Exception &ex)
    {
        std::cerr << "Error occurred: " << ex.why() << std::endl;
        return 1;
    }
    catch(std::exception &ex)
    {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
        return 1;
    }
    catch(...)
    {
        std::cerr << "unknown Error occured.\n";
        return 1;
    }

    return 0;
}
