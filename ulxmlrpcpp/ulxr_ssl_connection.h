/***************************************************************************
                   ulxr_ssl_connection.h  -  ssl connection
                             -------------------
    begin                : Mon May 3 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_ssl_connection.h 11073 2011-10-25 12:44:58Z korosteleva $

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

#ifndef ULXR_SSL_CONNECTION_H
#define ULXR_SSL_CONNECTION_H


#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_tcpip_connection.h>

// Forward declarations from OpenSSL
struct ssl_st;
struct ssl_ctx_st;
typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;


namespace ulxr {


    /** Class for ssl connections between XML RPC client and server.
      * @ingroup grp_ulxr_connection
      */
    class  SSLConnection : public TcpIpConnection
    {
    public:

        /** Constructs a generic connection.
          * The connection is not yet open after construction.
          */
        SSLConnection(const std::string& aRemoteHost, unsigned port, bool anAllowEcCiphers, size_t aTcpConnectionTimeout = TcpIpConnection::DefConnectionTimeout);
        SSLConnection(const IP &aListenIp, unsigned port, bool anAllowEcCiphers);


        /** Constructs a connection.
          * The connection is not yet open after construction.
          */
        virtual ~SSLConnection();

        /** Closes the connection.
          */
        virtual void close();

        /** Opens the connection in rpc client mode.
          */
        virtual void open();

        /** Opens the connection in rpc server mode, thus waiting for
          * connections from clients.
          * @param timeout the timeout value [sec] (0 - no timeout)
          * @returns <code>true</code> when connection has been accepted
          */
        virtual bool accept(int timeout = 0);


        /** Returns the password.
          * @return password
          */
        std::string getPassword() const;

        /** Sets the cryptography data.
          * @param  password   password for the crypto files
          * @param  certfile   name of the servers certificate file (PEM format)
          * @param  keyfile    name of the servers private key file (PEM format)
          */
        void setCryptographyData (const std::string &password,
                                  const std::string &certfile,
                                  const std::string &keyfile);

    protected:

        /** Checks if there is input data which can immediately be read.
          * @return true: data available
          */
        virtual bool hasPendingInput() const;

    private:

        SSL          *theSSL;
        SSL_CTX      *theSSL_ctx;
        const bool theAllowEcCiphers;

        std::string   password;
        std::string   keyfile;
        std::string   certfile;

        static bool SSL_initialized;

        /** Create SSL object.
          */
        void createSSL();

        /** Initialise SSL context data.
          */
        void initializeCTX();

        /** Actually writes data to the connection.
          * @param  buff pointer to data
          * @param  len  valid buffer length
          * @return  result from api write function
          */
        size_t virtual low_level_write(char const *buff, long len);

        /** Reads data from the connection.
          * @param  buff pointer to data buffer
          * @param  len  maimum number of bytes to read into buffer
          * @return  result from api read function
          */
        size_t virtual low_level_read(char *buff, long len);

        /** Initializes internal variables.
          */
        void init();

        // Perform SSL handshake without blocking on top of the already existing TCP connection
        // This way SSL connection is established
        void handshakeNonBlocking();
    };


}  // namespace ulxr



#endif // ULXR_SSL_CONNECTION_H

