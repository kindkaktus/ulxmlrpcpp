/***************************************************************************
                ulxr_ssl_connection.cpp  -  ssl connection
                             -------------------
    begin                : Mon May 3 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_ssl_connection.cpp 11073 2011-10-25 12:44:58Z korosteleva $

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


#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <openssl/err.h>
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <openssl/ssl.h>
#include <string.h>
#include <sstream>

#ifndef __OpenBSD__
#ifndef strlcpy
static size_t strlcpy(char* dst, const char* src, size_t siz)
{
    char* d = dst;
    const char* s = src;
    size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0')
                break;
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (siz != 0)
            *d = '\0';   /* NUL-terminate dst */
        while (*s++)
            ;
    }

    return(s - src - 1); /* count does not include NUL */
}
#endif
#endif

namespace ulxr {

    bool SSLConnection::SSL_initialized = false;


    static int password_cb(char *buf,int num, int /*rwflag*/, void *userdata)
    {
        ULXR_TRACE("password_cb");
        SSLConnection *conn = (SSLConnection *)userdata;
        std::string pass = conn->getPassword();

        if((unsigned int)num < pass.length()+1)
            return 0;

        strlcpy(buf, pass.c_str(), pass.length()+1);
        return(strlen(buf));
    }


    SSLConnection::SSLConnection(const std::string& aRemoteHost, unsigned port, bool anAllowEcCiphers, size_t aTcpConnectionTimeout)
        : TcpIpConnection(aRemoteHost, port, aTcpConnectionTimeout)
        , theSSL(NULL)
        , theSSL_ctx(NULL)
        , theAllowEcCiphers(anAllowEcCiphers)
    {
        ULXR_TRACE("SSLConnection (client mode)");
        init();
    }

    SSLConnection::SSLConnection(const IP &aListenIp, unsigned port, bool anAllowEcCiphers)
        : TcpIpConnection(aListenIp, port)
        , theSSL(NULL)
        , theSSL_ctx(NULL)
        , theAllowEcCiphers(anAllowEcCiphers)

    {
        ULXR_TRACE("SSLConnection (server mode)");
        init();
    }


    void
    SSLConnection::setCryptographyData (const std::string &in_password,
                                        const std::string &in_certfile,
                                        const std::string &in_keyfile)
    {
        password = in_password;

        if (!in_certfile.empty())
        {
            if (SSL_CTX_use_certificate_file(theSSL_ctx, in_certfile.c_str(), SSL_FILETYPE_PEM) <= 0)
                throw ConnectionException(SystemError, "SSLConnection::setCryptographyData: problem setting up certificate from file: "+in_certfile, 500);
            certfile = in_certfile;
        }
        if (!in_keyfile.empty())
        {
            if (SSL_CTX_use_PrivateKey_file(theSSL_ctx, in_keyfile.c_str(), SSL_FILETYPE_PEM) <= 0)
                throw ConnectionException(SystemError, "SSLConnection::setCryptographyData: problem setting up key from file: "+in_keyfile, 500);
            keyfile = in_keyfile;
        }
    }


    void SSLConnection::initializeCTX()
    {
        ULXR_TRACE("initializeCTX");
        theSSL_ctx = SSL_CTX_new (SSLv23_method());
        if (!theSSL_ctx)
            throw ConnectionException(SystemError,  "problem creating SSL conext object", 500);

        //@note explicit adding ECC to the list of ciphers seems no more needed for OpenSSL 0.9.9+ (is already there).
        const char* cipher = theAllowEcCiphers?"ALL:ECCdraft":"ALL:!ECCdraft";
        if (!SSL_CTX_set_cipher_list(theSSL_ctx,cipher))
        {
            SSL_CTX_free(theSSL_ctx);
            theSSL_ctx = NULL;
            throw ConnectionException(SystemError,  "SSL_CTX_set_cipher_list failed", 500);
        }

        SSL_CTX_set_default_passwd_cb(theSSL_ctx, password_cb);
        SSL_CTX_set_default_passwd_cb_userdata(theSSL_ctx, this);

        theSSL = NULL;
    }


    void SSLConnection::init()
    {
        if (!SSL_initialized)
        {
            SSL_library_init();
            SSLeay_add_ssl_algorithms();
            SSL_load_error_strings();
            SSL_initialized = true;
        }

        initializeCTX();
    }


    SSLConnection::~SSLConnection()
    {
        ULXR_TRACE("~SSLConnection");
        if (theSSL_ctx)
        {
            SSL_CTX_free(theSSL_ctx);
            theSSL_ctx = NULL;
        }
        if (theSSL)
        {
            SSL_free(theSSL);
            theSSL = NULL;
        }
    }


    void SSLConnection::close()
    {
        TcpIpConnection::close();
        if (theSSL)
        {
            SSL_free(theSSL);
            theSSL = NULL;
        }
    }


    size_t SSLConnection::low_level_write(char const *buff, long len)
    {
        ULXR_TRACE("SSLConnection::low_level_write");

        size_t ret;
        while (true)
        {
            ULXR_TRACE("SSLConnection::low_level_write 2");
            ret = SSL_write(theSSL, buff, len);
            ULXR_TRACE("SSLConnection::low_level_write 3 " << ret);

            if (ret >= 0)
                break;

            ULXR_TRACE("SSLConnection::low_level_write 4");
            int mySslErr = SSL_get_error(theSSL, ret);
            switch (mySslErr)
            {
            case SSL_ERROR_NONE:
                ULXR_TRACE("SSL_ERROR_NONE");
                break;

            case SSL_ERROR_WANT_WRITE:
                ULXR_TRACE("SSL_ERROR_WANT_WRITE");
                continue;

            default:
                ULXR_TRACE("default");
                throw ConnectionException(SystemError, "Could not perform SSL_write() call. SSL_write() returned " + toString(mySslErr), 500);
            }
        }
        ULXR_TRACE("/SSLConnection::low_level_write " << ret);
        return ret;
    }


    bool SSLConnection::hasPendingInput() const
    {
        const int avail = SSL_pending(theSSL);
        ULXR_TRACE("hasPendingInput " << avail);
        return avail != 0;
    }


    size_t SSLConnection::low_level_read(char *buff, long len)
    {
        ULXR_TRACE("SSLConnection::low_level_read");
        size_t ret;

        while (true)
        {
            ULXR_TRACE("SSLConnection::low_level_read 2");
            ret = SSL_read(theSSL, buff, len);
            ULXR_TRACE("SSLConnection::low_level_read 3 " << ret);

            if (ret >= 0)
                break;

            ULXR_TRACE("SSLConnection::low_level_read 4");
            const int mySslErr = SSL_get_error(theSSL, ret);
            switch (mySslErr)
            {
            case SSL_ERROR_NONE:
                ULXR_TRACE("SSL_ERROR_NONE");
                break;

            case SSL_ERROR_WANT_READ:
                ULXR_TRACE("SSL_ERROR_WANT_READ");
                continue;

            default:
                throw ConnectionException(SystemError, "Could not perform SSL_read() call. SSL_read() returned " + toString(mySslErr), 500);
            }
        }
        ULXR_TRACE("/SSLConnection::low_level_read " << ret);
        return ret;
    }

    void SSLConnection::createSSL()
    {
        ULXR_TRACE("createSSL");
        if (!theSSL_ctx)
            throw RuntimeException(ApplicationError, "Attempt to initialize SSL connection using non-initialized SSL context");
        if (theSSL)
            throw RuntimeException(ApplicationError, "Attempt to initialize SSL connection on top of the already initialized SSL connection");
        theSSL = SSL_new (theSSL_ctx);
        if (!theSSL)
            throw ConnectionException(SystemError, "problem creating SSL connection object from SSL conext", 500);

        if (! SSL_set_fd (theSSL, getHandle()))
            throw ConnectionException(SystemError, "Problem set file descriptor for SSL", 500);

    }


    void SSLConnection::open()
    {
        ULXR_TRACE("SSLConnection::open");
        if (theSSL)
        {
            throw RuntimeException(ApplicationError, "Attempt to open an already open SSL connection");
        }

        TcpIpConnection::open(); // create TCP connection
        createSSL(); // create SSL context
        handshakeNonBlocking(); // SSL connect on top of the created TCP connection

        //@todo for performance reasons it might make sense to use SSL_SESSION (SSL_set_session in open() and SSL_get1_session() in close)

        ULXR_TRACE("/SSLConnection::open");
    }

    void SSLConnection::handshakeNonBlocking()
    {
        ULXR_TRACE("SSLConnection::handshakeNonBlocking");

        //@note implementation ideas borrowed from ACE library (http://www.dre.vanderbilt.edu/~schmidt/DOC_ROOT/ACE/ace/SSL/SSL_SOCK_Connector.cpp)

        if (SSL_is_init_finished (theSSL))
        {
            ULXR_TRACE("/SSLConnection::handshakeNonBlocking (already connected)");
        }

        if (!SSL_in_connect_init (theSSL))
        {
            SSL_set_connect_state (theSSL);
        }

        setNonblock(true);

        do
        {
            const int myConnectStatus = SSL_connect (theSSL);
            const int mySslErr = SSL_get_error(theSSL, myConnectStatus);

            fd_set read_fd, write_fd;
            FD_ZERO(&read_fd);
            FD_ZERO(&write_fd);

            int sock = getHandle();

            switch (mySslErr)
            {
            case SSL_ERROR_NONE:
                ULXR_TRACE("SSLConnection::handshakeNonBlocking: connected");
                setNonblock(false);
                return;

            case SSL_ERROR_WANT_WRITE:
                // ULXR_TRACE("SSLConnection::handshakeNonBlocking SSL_ERROR_WANT_WRITE");
                FD_SET((unsigned)sock, &write_fd);
                break;

            case SSL_ERROR_WANT_READ:
                // ULXR_TRACE("SSLConnection::handshakeNonBlocking SSL_ERROR_WANT_READ");
                FD_SET((unsigned)sock, &read_fd);
                break;

            default:
                setNonblockIgnoreErrors(false);
                throw ConnectionException(SystemError, "SSL_connect() failed with code " + toString(mySslErr), 500);
            }// switch

            timeval myTimeout;
            const unsigned int myTimeoutSec = getTimeout();
            myTimeout.tv_sec = myTimeoutSec;
            myTimeout.tv_usec = 0;

            const int mySelectStatus = select (sock + 1, &read_fd, &write_fd, NULL, &myTimeout);
            if (mySelectStatus < 0)
            {
                setNonblockIgnoreErrors(false);
                throw ConnectionException(SystemError, "Failed to SSL handshake. " + getErrorString(getLastError()), 500);
            }
            else if (mySelectStatus  == 0)
            {
                setNonblockIgnoreErrors(false);
                throw ConnectionException(SystemError, "Failed to SSL handshake. Connection timed out after " + toString(myTimeoutSec) + " seconds.", 500);
            }

            // we are (most likely) done
        }
        while (!SSL_is_init_finished (theSSL));

        setNonblock(false);
        ULXR_TRACE("/SSLConnection::handshakeNonBlocking");
    }


    bool SSLConnection::accept(int in_timeout)
    {
        ULXR_TRACE("SSLConnection::accept");

        if (theSSL)
            throw RuntimeException(ApplicationError, "Attempt to accept an already open SSL connection");

        if (SSL_CTX_use_certificate_file(theSSL_ctx, certfile.c_str(), SSL_FILETYPE_PEM) <= 0)
            throw ConnectionException(SystemError, "problem setting up certificate", 500);

        if (SSL_CTX_use_PrivateKey_file(theSSL_ctx, keyfile.c_str(), SSL_FILETYPE_PEM) <= 0)
            throw ConnectionException(SystemError, "problem setting up private key", 500);


        if (!TcpIpConnection::accept(in_timeout))
            return false;

        createSSL();
        int myRet = SSL_accept(theSSL);
        if (myRet != 1)
            throw ConnectionException(SystemError, "Problem starting SSL connection (server mode). SSL_accept() returned " +  toString(SSL_get_error(theSSL, myRet)), 500);

        ULXR_TRACE("SSL connection using " << SSL_get_cipher (theSSL));
        return true;
    }

    std::string SSLConnection::getPassword() const
    {
        return password;
    }


}  // namespace ulxr
