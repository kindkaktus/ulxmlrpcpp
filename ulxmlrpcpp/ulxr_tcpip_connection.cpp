/***************************************************************************
               ulxr_tcpip_connection.cpp  -  tcpip connection
                             -------------------
    begin                : Mon Apr 29 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_tcpip_connection.cpp 11073 2011-10-25 12:44:58Z korosteleva $

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

// #define ULXR_SHOW_TRACE
// #define ULXR_DEBUG_OUTPUT
// #define ULXR_SHOW_READ
// #define ULXR_SHOW_WRITE



#include <cstring>
#include <cerrno>
#include <algorithm>
#include <cstdio>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <cassert>
#include <unistd.h>

#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_tcpip_connection.h>
#include <ulxmlrpcpp/ulxr_except.h>


namespace ulxr {

    namespace
    {
        //@note this function supersedes the originally used inet_pton() because the latter does not understand scopeid percent notation like fe80::1%vic1 or fe80::1%2
        void getIpv6AddrInfo(const std::string& anAddrStr, int aPort, sockaddr_in6& anAddr, bool aServerUse = false)
        {
            memset(&anAddr, 0, sizeof(anAddr));
            anAddr.sin6_family = AF_INET6;

            struct addrinfo hints, *res;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family   = AF_INET6;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            hints.ai_flags = AI_NUMERICHOST;
            if (aServerUse)
                hints.ai_flags |= AI_PASSIVE;

            int myRet = getaddrinfo(anAddrStr.c_str(), NULL, &hints, &res);
            if (myRet != 0)
                throw ConnectionException(SystemError, "Failed to parse IPv6 from " + anAddrStr + " : " + std::string(gai_strerror(myRet)), 500);
            memcpy(&anAddr, res->ai_addr, res->ai_addrlen);
            freeaddrinfo(res);
            anAddr.sin6_port = htons(aPort);
        }
    }


//
// ServerSocketData contains server listening sockets. Exists (i.e. not NULL) only for server-side TcpConnection objects
//

    TcpIpConnection::ServerSocketData::ServerSocketData(int ipv4_sock_no, int ipv6_sock_no)
        : ipv4_socket_no(ipv4_sock_no)
        , ipv6_socket_no(ipv6_sock_no)
    {}


    TcpIpConnection::ServerSocketData::~ServerSocketData()
    {
        try { close(); }
        catch(...) {}
    }


    int TcpIpConnection::ServerSocketData::getIpv4Socket() const
    {
        return ipv4_socket_no;
    }

    int TcpIpConnection::ServerSocketData::getIpv6Socket() const
    {
        return ipv6_socket_no;
    }

    bool TcpIpConnection::ServerSocketData::isIpv4Open()
    {
        return ipv4_socket_no != -1;
    }

    bool TcpIpConnection::ServerSocketData::isIpv6Open()
    {
        return ipv6_socket_no != -1;
    }

    void TcpIpConnection::ServerSocketData::close()
    {
        ULXR_TRACE("close");
        if (ipv4_socket_no  >= 0)
        {
            int ret;
            do
                ret = ::close(ipv4_socket_no);
            while(ret < 0 && (errno == EINTR || errno == EAGAIN));

            ipv4_socket_no = -1;
        }
        if (ipv6_socket_no  >= 0)
        {
            int ret;
            do
                ret = ::close(ipv6_socket_no);
            while(ret < 0 && (errno == EINTR || errno == EAGAIN));

            ipv6_socket_no = -1;
        }
    }


//////////////////////////////////////////////////////////////////////////


    struct TcpIpConnection::PImpl
    {
        PImpl()
            : server_data(NULL)
        {}

        unsigned            port;

        ServerSocketData   *server_data;

        struct sockaddr_in  ipv4_hostdata;
        struct sockaddr_in6 ipv6_hostdata;

        // These fields filled when a connection has been established
        std::string           peer_name;
        struct sockaddr_in  ipv4_peerdata;
        socklen_t           ipv4_peerdata_len;
        struct sockaddr_in6 ipv6_peerdata;
        socklen_t           ipv6_peerdata_len;
    };



    TcpIpConnection::TcpIpConnection(const std::string &aRemoteHost, unsigned port, size_t aTcpConnectionTimeout)
        : Connection()
        , pimpl(new PImpl)
        , theTcpConnectionTimeoutSec(aTcpConnectionTimeout)
    {
        IP myRemoteHostIp = getIpByName(aRemoteHost);
        isIpv4 = isValidIpv4(myRemoteHostIp.ipv4);
        isIpv6 = isValidIpv6(myRemoteHostIp.ipv6);

        assert(isIpv4 || isIpv6);

        init(port);

        if (isIpv4)
        {
            int myRet = inet_pton(AF_INET, myRemoteHostIp.ipv4.c_str(), &pimpl->ipv4_hostdata.sin_addr);
            if (myRet == 0)
                throw ConnectionException(SystemError, myRemoteHostIp.ipv4 + " is invalid IPv4 address: " + getErrorString(getLastError()), 500);
            if (myRet < 0)
                throw ConnectionException(SystemError, "Failed to initialize IPv4 address " + myRemoteHostIp.ipv4 + " : " + getErrorString(getLastError()), 500);
        }
        if (isIpv6)
        {
            getIpv6AddrInfo(myRemoteHostIp.ipv6, port, pimpl->ipv6_hostdata);
        }
    }


    TcpIpConnection::TcpIpConnection(const IP& aListenIp, unsigned port)
        : Connection()
        , pimpl(new PImpl)
        , theTcpConnectionTimeoutSec(0) /* makes no sense for server*/
    {
        isIpv4 = isValidIpv4(aListenIp.ipv4);
        isIpv6 = isValidIpv6(aListenIp.ipv6);
        if (!isIpv4 && !isIpv6)
            throw ConnectionException(SystemError, "Neither '" + aListenIp.ipv4 + "' is valid IPv4 address nor '" + aListenIp.ipv6 + "' is valid IPv6 address", 500);
        init(port);

        int ipv4Sock = -1, ipv6Sock  = -1;
        if (isIpv4)
        {
            int myRet = -1;
            if ((myRet = inet_pton(AF_INET, aListenIp.ipv4.c_str(), &pimpl->ipv4_hostdata.sin_addr)) == 0)
                throw ConnectionException(SystemError, aListenIp.ipv4 + " is invalid IPv4 address: " + getErrorString(getLastError()), 500);
            if (myRet < 0)
                throw ConnectionException(SystemError, "Failed to initialize IPv4 address " + aListenIp.ipv4 + " : " + getErrorString(getLastError()), 500);
            ipv4Sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (ipv4Sock < 0)
                throw ConnectionException(SystemError, "Failed to create IPv4 TCP socket : " + getErrorString(getLastError()), 500);
            int sockOpt = 1;
            if (::setsockopt(ipv4Sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&sockOpt, sizeof(sockOpt)) < 0)
                throw ConnectionException(SystemError,  "Could not set reuse flag for socket: " + getErrorString(getLastError()), 500);
            int iOptVal = getTimeout() * 1000;
            int iOptLen = sizeof(int);
            ::setsockopt(ipv4Sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&iOptVal, iOptLen);
            ::setsockopt(ipv4Sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&iOptVal, iOptLen);

            if ((::bind(ipv4Sock, (sockaddr*) &pimpl->ipv4_hostdata, sizeof(pimpl->ipv4_hostdata))) < 0)
                throw ConnectionException(SystemError, "Could not bind to IPv4 adress " + aListenIp.ipv4 + ":" + toString(port) + " : " + getErrorString(getLastError()), 500);

            listen(ipv4Sock, SOMAXCONN);
        }
        if (isIpv6)
        {
            getIpv6AddrInfo(aListenIp.ipv6, port, pimpl->ipv6_hostdata, true);
            ipv6Sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
            if (ipv6Sock < 0)
                throw ConnectionException(SystemError, "Failed to create IPv6 TCP socket : " + getErrorString(getLastError()), 500);

            // set IPV6_V6ONLY flag on socket to allow binding both IPv4 and IPv6 to the same port (such as 0.0.0.0 and ::)
            int on = 1;
            if (::setsockopt(ipv6Sock, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on))< 0)
                throw ConnectionException(SystemError,  "Could not set IPPROTO_IPV6 flag for socket: " + getErrorString(getLastError()), 500);

            int sockOpt = 1;
            if (::setsockopt(ipv6Sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&sockOpt, sizeof(sockOpt)) < 0)
                throw ConnectionException(SystemError,  "Could not set reuse flag for socket: " + getErrorString(getLastError()), 500);
            int iOptVal = getTimeout() * 1000;
            int iOptLen = sizeof(int);
            ::setsockopt(ipv6Sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&iOptVal, iOptLen);
            ::setsockopt(ipv6Sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&iOptVal, iOptLen);

            if ((::bind(ipv6Sock, (sockaddr*) &pimpl->ipv6_hostdata, sizeof(pimpl->ipv6_hostdata))) < 0)
                throw ConnectionException(SystemError, "Could not bind to IPv6 address " + aListenIp.ipv6 + " port " + toString(port) + " : " + getErrorString(getLastError()), 500);

            listen(ipv6Sock, SOMAXCONN);
        }

        assert(ipv4Sock >= 0 || ipv6Sock >= 0);
        pimpl->server_data = new ServerSocketData(ipv4Sock, ipv6Sock);
    }

    void TcpIpConnection::init(unsigned port)
    {
        ULXR_TRACE("TcpIpConnection::init");

        if (!isValidPort(port))
            throw ConnectionException(SystemError, "Invalid port", 500);
        setTcpNoDelay(false);
        setTimeout(10);
        pimpl->port = port;
        if (isIpv4)
        {
            pimpl->ipv4_peerdata_len = sizeof(pimpl->ipv4_peerdata);
            memset(&pimpl->ipv4_hostdata, 0, sizeof(pimpl->ipv4_hostdata));
            memset(&pimpl->ipv4_peerdata, 0, sizeof(pimpl->ipv4_peerdata));

            pimpl->ipv4_hostdata.sin_port = htons(pimpl->port);
            pimpl->ipv4_hostdata.sin_family = AF_INET;
        }
        if (isIpv6)
        {
            pimpl->ipv6_peerdata_len = sizeof(pimpl->ipv6_peerdata);
            memset(&pimpl->ipv6_hostdata, 0, sizeof(pimpl->ipv6_hostdata));
            memset(&pimpl->ipv6_peerdata, 0, sizeof(pimpl->ipv6_peerdata));

            pimpl->ipv6_hostdata.sin6_port = htons(pimpl->port);
            pimpl->ipv6_hostdata.sin6_family = AF_INET6;
        }
    }


    TcpIpConnection::~TcpIpConnection()
    {
        ULXR_TRACE("~TcpIpConnection");

        if (pimpl->server_data)
        {
            delete pimpl->server_data;
            pimpl->server_data = NULL;
        }

        delete pimpl;
        pimpl = NULL;

        try	{ TcpIpConnection::close(); }
        catch (...)
        {}
    }


    bool TcpIpConnection::isServerMode() const
    {
        return !!pimpl->server_data;
    }

    void TcpIpConnection::setNonblock(bool aSet, bool ignoreErrors)
    {
        ULXR_TRACE("TcpIpConnection::setNonblock: " << aSet);

        int fd = getHandle();
        int oldflags = fcntl(fd, F_GETFL, 0);
        if (oldflags < 0)
        {
            if (ignoreErrors)
            {
                return;
            }
            throw ConnectionException(SystemError, "Failed to get mode for fd", 500);
        }
        if (aSet)
        {
            oldflags |= O_NONBLOCK;
            if  (fcntl(fd, F_SETFL, oldflags) < 0)
            {
                if (ignoreErrors)
                {
                    return;
                }
                throw ConnectionException(SystemError, "Failed to set non-blocking mode for fd", 500);
            }
        }
        else
        {
            oldflags &= ~O_NONBLOCK;
            if  (fcntl(fd, F_SETFL, oldflags) < 0)
            {
                if (ignoreErrors)
                {
                    return;
                }
                throw ConnectionException(SystemError, "Failed to set blocking mode for fd", 500);
            }
        }
    }

    void TcpIpConnection::setNonblockIgnoreErrors(bool aSet)
    {
        setNonblock(aSet, true);
    }

    bool TcpIpConnection::connectNonBlocking(const struct sockaddr *name, socklen_t namelen, std::string& anErrorMsg, bool anIsIpv6)
    {
        setNonblock(true);
        bool myIsConnected = false;

        int sock = getHandle();
        if (connect(sock, name, namelen) < 0)
        {
            if (errno == EINPROGRESS)
            {
                fd_set sel;
                FD_ZERO(&sel);
                FD_SET((unsigned)sock, &sel);
                timeval myConnectTimeout;
                myConnectTimeout.tv_sec = theTcpConnectionTimeoutSec;
                myConnectTimeout.tv_usec = 0;
                int mySelectRes = select(sock+1, NULL, &sel, NULL, &myConnectTimeout);
                if (mySelectRes < 0)
                {
                    if (!anErrorMsg.empty())
                    {
                        anErrorMsg += ". ";
                    }
                    anErrorMsg += std::string("Failed to connect ") + (anIsIpv6?"IPv6":"IPv4") + " (using select). " + getErrorString(getLastError());
                }
                else if (mySelectRes == 0)
                {
                    if (!anErrorMsg.empty())
                    {
                        anErrorMsg += ". ";
                    }
                    anErrorMsg += std::string("Failed to connect ") + (anIsIpv6?"IPv6":"IPv4") + " (using select). Connection timed out after " + toString(theTcpConnectionTimeoutSec) + " seconds.";
                }
                else
                {
                    myIsConnected = true;
                }
            }
            else
            {
                if (!anErrorMsg.empty())
                {
                    anErrorMsg += ". ";
                }
                anErrorMsg += std::string("Failed to connect ") + (anIsIpv6?"IPv6. ":"IPv4. ") + getErrorString(getLastError());
            }
        }
        else
        {
            myIsConnected = true;
        }
        setNonblock(false);
        return myIsConnected;
    }


    void TcpIpConnection::open()
    {
        ULXR_TRACE("TcpIpConnection::open");
        if (isOpen() )
            throw RuntimeException(ApplicationError, "Attempt to open an already open connection");

        if (isServerMode())
            throw ConnectionException(SystemError, "Connection is NOT prepared for server mode", 500);
//  resetConnection();

        assert(isIpv4 || isIpv6);
        std::string myLastErrorStr;

        if (isIpv6)
        {
            int sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
            if (sock < 0)
                throw ConnectionException(SystemError,  "Could not create IPv6 socket: " + getErrorString(getLastError()), 500);
            setHandle(sock);
            int iOptVal = getTimeout() * 1000;
            int iOptLen = sizeof(int);
            ::setsockopt(getHandle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&iOptVal, iOptLen);
            ::setsockopt(getHandle(), SOL_SOCKET, SO_SNDTIMEO, (char*)&iOptVal, iOptLen);
            doTcpNoDelay();
            if (connectNonBlocking((struct sockaddr *)&pimpl->ipv6_hostdata, sizeof(pimpl->ipv6_hostdata), myLastErrorStr, true))
            {
                pimpl->ipv6_peerdata_len = sizeof(pimpl->ipv6_peerdata);
                if (getpeername(getHandle(),  (struct sockaddr *)&pimpl->ipv6_peerdata, &pimpl->ipv6_peerdata_len)<0)
                    throw ConnectionException(SystemError, "Could not get peer data: " + getErrorString(getLastError()), 500);

                char myIpSzBuf[INET6_ADDRSTRLEN] = {};
                if (!inet_ntop(AF_INET6, &(pimpl->ipv6_peerdata.sin6_addr), myIpSzBuf, sizeof(myIpSzBuf)))
                    throw ConnectionException(SystemError,  "inet_ntop failed: " + getErrorString(getLastError()), 500);
                pimpl->peer_name = myIpSzBuf + std::string(":") + toString(ntohs(pimpl->ipv6_peerdata.sin6_port));

                abortOnClose(true);
                ULXR_TRACE("/TcpIpConnection::open (for IPv6)");
                return;
            }
        }

        if (isIpv4)
        {
            int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sock < 0)
                throw ConnectionException(SystemError,  "Could not create IPv4 socket: " + getErrorString(getLastError()), 500);
            setHandle(sock);
            int iOptVal = getTimeout() * 1000;
            int iOptLen = sizeof(int);
            ::setsockopt(getHandle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&iOptVal, iOptLen);
            ::setsockopt(getHandle(), SOL_SOCKET, SO_SNDTIMEO, (char*)&iOptVal, iOptLen);
            doTcpNoDelay();
            if (connectNonBlocking((struct sockaddr *)&pimpl->ipv4_hostdata, sizeof(pimpl->ipv4_hostdata), myLastErrorStr, false))
            {
                pimpl->ipv4_peerdata_len = sizeof(pimpl->ipv4_peerdata);
                if (getpeername(getHandle(),  (struct sockaddr *)&pimpl->ipv4_peerdata, &pimpl->ipv4_peerdata_len)<0)
                    throw ConnectionException(SystemError, "Could not get peer data: " + getErrorString(getLastError()), 500);
                pimpl->peer_name = inet_ntoa(pimpl->ipv4_peerdata.sin_addr) + std::string(":") + toString(ntohs(pimpl->ipv4_peerdata.sin_port));

                abortOnClose(true);
                ULXR_TRACE("/TcpIpConnection::open (for IPv4)");
                return;
            }
        }
        throw ConnectionException(SystemError, myLastErrorStr, 500);
    }


    bool TcpIpConnection::accept(int in_timeout)
    {
        if (isOpen() )
            throw RuntimeException(ApplicationError, "Attempt to accept an already open connection");

//  resetConnection();

        if (!getServerData())
            throw ConnectionException(SystemError, "Connection is NOT prepared for client mode", 500);

        assert(isIpv4 || isIpv6);

        if (isIpv4)
            pimpl->ipv4_peerdata_len = sizeof(pimpl->ipv4_peerdata);
        if (isIpv6)
            pimpl->ipv6_peerdata_len = sizeof(pimpl->ipv6_peerdata);

        std::list<int> myListenSockets;
        if (isIpv4)
            myListenSockets.push_back(getServerData()->getIpv4Socket());
        if (isIpv6)
            myListenSockets.push_back(getServerData()->getIpv6Socket());
        waitForConnection(myListenSockets, in_timeout);
        if (myListenSockets.empty())
        {
            if (in_timeout != 0)
                return false; //timeout reached
            assert(!"Infinite timeout reached while waiting for connection?!");
        }

        // Prefer IPv6 if we have both
        bool myIsIpv6Connection = std::find(myListenSockets.begin(),myListenSockets.end(), getServerData()->getIpv6Socket()) != myListenSockets.end();

        ULXR_TRACE("waiting for connection");
        do
        {
            if (myIsIpv6Connection)
                setHandle(::accept(getServerData()->getIpv6Socket(), (sockaddr*) &pimpl->ipv6_peerdata, &pimpl->ipv6_peerdata_len ));
            else
                setHandle(::accept(getServerData()->getIpv4Socket(), (sockaddr*) &pimpl->ipv4_peerdata, &pimpl->ipv4_peerdata_len ));
        }
        while(getHandle() < 0 && (errno == EINTR || errno == EAGAIN));

        if (getHandle() < 0)
            throw ConnectionException(SystemError, "Could not accept a connection: " + getErrorString(getLastError()), 500);

        doTcpNoDelay();

        if (myIsIpv6Connection)
        {
            char myIpSzBuf[INET6_ADDRSTRLEN] = {};
            if (!inet_ntop(AF_INET6, &(pimpl->ipv6_peerdata.sin6_addr), myIpSzBuf, sizeof(myIpSzBuf)))
                throw ConnectionException(SystemError,  "inet_ntop failed: " + getErrorString(getLastError()), 500);
            pimpl->peer_name = myIpSzBuf + std::string(":") + toString(ntohs(pimpl->ipv6_peerdata.sin6_port));
        }
        else
        {
            pimpl->peer_name = inet_ntoa(pimpl->ipv4_peerdata.sin_addr) + std::string(":") + toString(ntohs(pimpl->ipv4_peerdata.sin_port));
        }

        ULXR_TRACE("/accept");

        abortOnClose(true);
        return true;
    }

    int TcpIpConnection::getServerIpv4Handle ()
    {
        if (pimpl->server_data)
            return pimpl->server_data->getIpv4Socket();

        return -1;
    }

    int TcpIpConnection::getServerIpv6Handle ()
    {
        if (pimpl->server_data)
            return pimpl->server_data->getIpv6Socket();

        return -1;
    }


    unsigned TcpIpConnection::getPort()
    {
        return pimpl->port;
    }


    std::string TcpIpConnection::getPeerName() const
    {
        return pimpl->peer_name;
    }

    TcpIpConnection::ServerSocketData* TcpIpConnection::getServerData () const
    {
        return pimpl->server_data;
    }


    void TcpIpConnection::close()
    {
        ULXR_TRACE("TcpIpConnection::close");
        Connection::close();
    }

    void TcpIpConnection::stopServing()
    {
        ULXR_TRACE("stopServing");
        if (pimpl->server_data)
        {
            delete pimpl->server_data;
            pimpl->server_data = NULL;
        }
    }


    int TcpIpConnection::abortOnClose(int bOn)
    {
        linger sock_linger_struct = {1, 0};
        sock_linger_struct.l_onoff = bOn;
        sock_linger_struct.l_linger = getTimeout();

        int handle = getHandle();
        if (getServerData())
        {
            if (getServerData()->isIpv4Open())
            {
                handle = getServerData()->getIpv4Socket();
                return setsockopt(handle, SOL_SOCKET, SO_LINGER,  &sock_linger_struct, sizeof(linger));
            }
            if (getServerData()->isIpv6Open())
            {
                handle = getServerData()->getIpv6Socket();
                return setsockopt(handle, SOL_SOCKET, SO_LINGER, &sock_linger_struct, sizeof(linger));
            }
        }
        else
        {
            return setsockopt(handle, SOL_SOCKET, SO_LINGER, &sock_linger_struct, sizeof(linger));
        }
    }


    void TcpIpConnection::setTcpNoDelay(bool bOn)
    {
        noDelayOpt = 0;
        if (bOn)
            noDelayOpt = 1;
        doTcpNoDelay();
    }


    int TcpIpConnection::doTcpNoDelay()
    {
        int sock = getHandle();
        if (getServerData())
        {
            int ret = -1;
            if (getServerData()->isIpv4Open())
            {
                sock = getServerData()->getIpv4Socket();
                ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &noDelayOpt, sizeof(noDelayOpt));
            }
            if (getServerData()->isIpv6Open())
            {
                sock = getServerData()->getIpv6Socket();
                ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &noDelayOpt, sizeof(noDelayOpt));
            }
            return ret;
        }
        return setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &noDelayOpt, sizeof(noDelayOpt));
    }

    int TcpIpConnection::getLastError()
    {
        return(errno);
    }


    bool TcpIpConnection::isValidIpv4(const std::string& anAddr)
    {
        sockaddr_in myAddr = {0};
        myAddr.sin_family = AF_INET;
        int myRet = inet_pton(AF_INET, anAddr.c_str(), &myAddr.sin_addr);
        if (myRet == 1)
            return true;
        if (myRet == 0)
            return false;
        throw ConnectionException(SystemError, "isValidIpv4: inet_pton() failed for  " + anAddr + ". Error : " + getErrorString(getLastError()), 500);
    }

    bool TcpIpConnection::isValidIpv6(const std::string& anAddr)
    {
        try
        {
            sockaddr_in6 myDummyAddr;
            getIpv6AddrInfo(anAddr, 0, myDummyAddr);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    std::string TcpIpConnection::getIpv4ByName(const std::string& aHostName)
    {
        if (isValidIpv4(aHostName))
            return aHostName;
        hostent* myHost = ::gethostbyname(aHostName.c_str());
        if (!myHost)
            throw ConnectionException(SystemError, "gethostbyname() failed for host " + aHostName + " : " + std::string(hstrerror(h_errno)), 500);

        char myIpBuf[1024] = {};
        snprintf(myIpBuf, sizeof(myIpBuf), "%u.%u.%u.%u",  (unsigned int)(unsigned char)myHost->h_addr_list[0][0],
                 (unsigned int)(unsigned char)myHost->h_addr_list[0][1],
                 (unsigned int)(unsigned char)myHost->h_addr_list[0][2],
                 (unsigned int)(unsigned char)myHost->h_addr_list[0][3]);
        return myIpBuf;
    }

    std::string TcpIpConnection::getIpv6ByName(const std::string& aHostName)
    {
        if (isValidIpv6(aHostName))
            return aHostName;
        hostent* myHost = ::gethostbyname2(aHostName.c_str(), AF_INET6);
        if (!myHost)
            throw ConnectionException(SystemError, "gethostbyname2() failed for host " + aHostName + " : " + std::string(hstrerror(h_errno)), 500);
        char myIpSzBuf[INET6_ADDRSTRLEN] = {};
        if (!inet_ntop(myHost->h_addrtype, myHost->h_addr_list[0], myIpSzBuf, sizeof(myIpSzBuf)))
            throw ConnectionException(SystemError, "inet_ntop() failed for host " + aHostName + " : " + getErrorString(getLastError()), 500);
        std::string myRetVal = myIpSzBuf;
        return myRetVal;
    }

    IP TcpIpConnection::getIpByName(const std::string& aHostName)
    {
        // First, check if we have IP as argument
        IP myRetVal;
        if (isValidIpv4(aHostName))
        {
            myRetVal.ipv4 = aHostName;
            return myRetVal;
        }
        if (isValidIpv6(aHostName))
        {
            myRetVal.ipv6 = aHostName;
            return myRetVal;
        }

        // We have hosthame, try resolving it
        bool myIsIpv4Resolved = false;
        try
        {
            myRetVal.ipv4 = getIpv4ByName(aHostName);
            myIsIpv4Resolved = true;
        }
        catch(std::exception& e)
        {}
        try
        {
            myRetVal.ipv6 = getIpv6ByName(aHostName);
        }
        catch(std::exception& e)
        {
            if (!myIsIpv4Resolved)
                throw ConnectionException(SystemError, "Cannot resolve neither IPv4 nor IPv6 of " + aHostName, 500);
        }
        return myRetVal;
    }

    bool TcpIpConnection::isValidPort(unsigned int aPort)
    {
        return (aPort > 0 && aPort < (1<<16));
    }

    void TcpIpConnection::waitForConnection(std::list<int>& aListenSockets, int aTimeout)
    {
        assert(!aListenSockets.empty());
        fd_set myReadSockets;
        FD_ZERO(&myReadSockets);
        for (std::list<int>::const_iterator it = aListenSockets.begin(), end = aListenSockets.end(); it != end; ++it)
        {
            FD_SET(*it, &myReadSockets);
        }
        int myNumSocks = *(std::max_element(aListenSockets.begin(), aListenSockets.end())) + 1;
        int myRet = -1;
        if (aTimeout == 0)
        {
            myRet =  ::select(myNumSocks, &myReadSockets, NULL, NULL, NULL);
        }
        else
        {
            timeval myTv;
            myTv.tv_sec = aTimeout / 1000;
            myTv.tv_usec = (aTimeout % 1000) * 1000;
            myRet =  ::select(myNumSocks, &myReadSockets, NULL, NULL, &myTv);
        }
        if (myRet < 0)
            throw ConnectionException(SystemError, "select failed : " + getErrorString(getLastError()), 500);
        if (myRet == 0)
        {
            aListenSockets.clear();
            return;
        }
        std::list<int>::const_iterator end = aListenSockets.end();
        for (std::list<int>::iterator it = aListenSockets.begin(); it!= end; )
            if (!FD_ISSET(*it, &myReadSockets))
                it = aListenSockets.erase(it);
            else
                ++it;
    }

}  // namespace ulxr


