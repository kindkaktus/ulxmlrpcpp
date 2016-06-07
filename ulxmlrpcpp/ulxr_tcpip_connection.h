/***************************************************************************
               ulxr_tcpip_connection.h  -  tcpip connection

    begin                : Sun Apr 29 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_tcpip_connection.h 11073 2011-10-25 12:44:58Z korosteleva $

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

#ifndef ULXR_TCPIP_CONNECTION_H
#define ULXR_TCPIP_CONNECTION_H

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <list>

#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_connection.h>

namespace ulxr {

    struct IP
    {
        std::string ipv4;
        std::string ipv6;

        IP() {}
        IP(const std::string& anIpv4, const std::string& anIpv6) : ipv4(anIpv4), ipv6(anIpv6) {}
        inline bool operator==(const IP& rhs) { return (rhs.ipv4 == ipv4 && rhs.ipv6 == ipv6); }
    };

    /** Run a generic tcp/ip connection between a client and a server.
      * @ingroup grp_ulxr_connection
      */
    class  TcpIpConnection : public Connection
    {
    public:
        static const size_t DefConnectionTimeout = 2; //sec

        /** Constructs a generic connection for a client.
          * The connection is not yet open after construction.
          * @param  host  hostname or IP address or the remote peer
          * If aRemoteHost represents a symbolic hostname which resolves both to IPv4 and IPv6,
            first IPv6 conneciton is tried to be established, and if it fails, IPv4 is subsequently tried
          * @param  port    port
          * param aTcpConnectionTimeout TCP connection timeout in seconds
          */
        TcpIpConnection(const std::string& aRemoteHost, unsigned port, size_t aTcpConnectionTimeout = DefConnectionTimeout);

        /** Constructs a generic connection for a server
        * The connection is not yet open after construction.
        * @param  IP IP address(es) to listen on.
        * @param  port    port on the the server.
        */
        TcpIpConnection(const IP &aListenIp, unsigned port);

        /** Destroys the connection.
          */
        virtual ~TcpIpConnection();

        /** Opens the connection in client mode.
         */
        virtual void open();

        /** Opens the connection in server mode, thus waiting for
          * connections from clients.
          * @param timeout the timeout value [sec] for incoming data (0 - no timeout)
          * @returns <code>true</code> when connection has been accepted
          */
        virtual bool accept(int timeout = 0);

        virtual void close();

        virtual void stopServing();

        /**
          * Returns port.
          */
        unsigned getPort();


        /** Gets the name of the remote machine.
          * attention Does not seem to work on all plattforms.
          * @return  the remote name.
          */
        std::string getPeerName() const;

        /** Sets abort on close flag for a socket. Timeout value comes from \c getTimeout(),
          * @param bOn    sets SO_LINGER state of the socket on/off.
          * @return status of setsockopt
          * See -s 3SOCKET setsockopt and sys/socket.h for details.
          */
        int abortOnClose(int bOn);

        /** Sets the socket buffer mechanism.
          * By disabling the buffer mechanism all data is sent immediately.
          * This may result in higher performance but will also increase network traffic with more but
          * smaller packets. Use with care.
          * @param bOn sets the TCP_NODELAY option on the socket
          */
        void setTcpNoDelay(bool bOn);

        /** Portable function to return the current socket error number.
          * @return error number (errno under Unices)
          */
        virtual int getLastError();

        /** Checks if the connection is run as server.
          */
        bool isServerMode() const;

        virtual int getServerIpv4Handle();
        virtual int getServerIpv6Handle();

    protected:

        /** Creates a \c hostent struct from a host name.
          * @return pointer to \c hostent structure
          */
        struct hostent *getHostAdress(const std::string &hostname);

        class ServerSocketData;


        /** Gets the pointer to the server-data.
          * @return   pointer to server
          */
        ServerSocketData *getServerData () const;

        bool isValidIpv4(const std::string& anAddr);
        bool isValidIpv6(const std::string& anAddr);
        std::string getIpv4ByName(const std::string& aHostName);
        std::string getIpv6ByName(const std::string& aHostName);
        IP getIpByName(const std::string& aHostName);
        bool isValidPort(unsigned int aPort);
        /**
         * @pre all sockets are in listening state
         * @param aListenSockets [in, out] ths input is a list of listening sockets to wait for connection on,
         *        the output contains only those sockets for which the subsequent call to accept() is guaranteed to complete without blocking
         * @param aTimeout time to wait, 0 means no timeout (wait until connected)
         * @throw ConnectionException
         */
        void waitForConnection(std::list<int>& aListenSockets, int aTimeout = 0);

        // set socket behavior to blocking/non-blocking
        void setNonblock(bool aSet, bool ignoreErrors = false);
        void setNonblockIgnoreErrors(bool aSet); // shortcut for setNonblock(aSet, true);

    private:

        /** Initializes internal variables.
          * @param   port   the port used for the connection
          */
        void init (unsigned port);

        /** Sets the socket buffer mechanism.
          * @return status of setsockopt
          */
        int doTcpNoDelay();

        //@nothrow
        //@return if non-blocking connection succeeds the function return true and anErrorMsg stays intact
        //                                  otherwise the function return false and anErrorMsg is appended with the error message
        bool connectNonBlocking(const struct sockaddr *name, socklen_t namelen, std::string& anErrorMsg, bool anIsIpv6);

    private:

        // forbid them all due to internal pointers
        TcpIpConnection(const TcpIpConnection&);
        TcpIpConnection& operator=(const TcpIpConnection&); // empty!

    private:

        struct PImpl;
        PImpl *pimpl;
        int    noDelayOpt;
        bool isIpv4;
        bool isIpv6;
        const size_t theTcpConnectionTimeoutSec;
    };


    /** Helper class to handle the server socket.
      */
    class  TcpIpConnection::ServerSocketData
    {
    public:

        /** Construct the server socket.
          * @param  s_no  the socket handle
          */
        ServerSocketData(int ipv4_sock_no, int ipv6_sock_no);

        /** Destroy the server socket.
          */
        virtual ~ServerSocketData();

        /** Gets the socket handle.
          * @return  the socket handle
          */
        int getIpv4Socket() const;
        int getIpv6Socket() const;

        /** Closes the socket.
          */
        void close();

        /** Queries if the socket has been closed.
          */
        bool isIpv4Open();
        bool isIpv6Open();

    private:
        int    ipv4_socket_no;
        int    ipv6_socket_no;
    };


}  // namespace ulxr


#endif // ULXR_TCPIP_CONNECTION_H
