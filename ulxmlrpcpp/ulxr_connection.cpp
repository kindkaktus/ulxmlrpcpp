/***************************************************************************
          ulxr_connection.cpp  -  provide a connection for rpc-data
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_connection.cpp 11073 2011-10-25 12:44:58Z korosteleva $

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
#include <memory>
#include <ctype.h>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <csignal>
#include <cstdio>

#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_connection.h>
#include <ulxmlrpcpp/ulxr_except.h>


namespace ulxr
{
    void Connection::init()
    {
        ULXR_TRACE("Connection::init");
        fd_handle = -1;
        theRwTimeoutSec = 10;
        signal (SIGPIPE, SIG_IGN);  // prevent SIGKILL while write()-ing in closing pipe
    }


    Connection::Connection()
    {
        ULXR_TRACE("Connection");
        init();
    }


    Connection::~Connection()
    {
        ULXR_TRACE("~Connection");
        try
        {
            close();
        }
        catch(...)
        {
            // forget exception?
        }
    }


    bool Connection::isOpen() const
    {
        return fd_handle >= 0;
    }


    size_t Connection::low_level_write(char const *buff, long len)
    {
        ULXR_TRACE("Connection::low_level_write " << len);
        return ::write(fd_handle, buff, len);
    }


    void Connection::write(char const *buff, long len)
    {
        ULXR_TRACE("Connection::write " << len);
        ULXR_DWRITE_WRITE(buff, len);
        ULXR_DOUT_WRITE("");

        long written;

        if (!buff || !isOpen())
            throw RuntimeException(ApplicationError, "Precondition failed for write() call");

        if (len == 0)
            return;

        fd_set wfd;
        timeval wait;

        while (buff != 0 && len > 0)
        {
            FD_ZERO(&wfd);
            FD_SET((unsigned) fd_handle, &wfd);
            int ready;

            const unsigned myTimeoutSec = getTimeout();
            wait.tv_sec = myTimeoutSec;
            wait.tv_usec = 0;

            while((ready = select(fd_handle+1, 0, &wfd, 0, &wait)) < 0)
            {
                if(errno == EINTR || errno == EAGAIN)
                {
                    // signal received, continue select
                    wait.tv_sec = myTimeoutSec;
                    wait.tv_usec = 0;
                    continue;
                }
                else
                    throw ConnectionException(SystemError, "Could not perform select() call: " + getErrorString(getLastError()), 500);
            }
            if(ready == 0)
                throw ConnectionException(SystemError, "Timeout while attempting to write (after " + toString(myTimeoutSec) + "seconds.", 500);

            if(FD_ISSET(fd_handle, &wfd))
            {
                if ( (written = low_level_write(buff, len)) < 0)
                {
                    switch(getLastError())
                    {
                    case EAGAIN:
                    case EINTR:
                        errno = 0;
                        continue;

                    case EPIPE:
                        close();
                        throw ConnectionException(TransportError,
                                                  "Attempt to write to a connection already closed by the peer", 500);
                    /*break; */

                    default:
                        throw ConnectionException(SystemError,
                                                  "Could not perform low_level_write() call: " + getErrorString(getLastError()), 500);

                    }
                }
                else
                {
                    buff += written;
                    len -= written;
                }
            }
        }

    }


    bool Connection::hasPendingInput() const
    {
        return false;
    }


    size_t Connection::low_level_read(char *buff, long len)
    {
        ULXR_TRACE("Connection::low_level_read");
        return ::read(fd_handle, buff, len);
        ULXR_TRACE("/Connection::low_level_read");
    }


    size_t Connection::read(char *buff, long len)
    {
        long myRead = 0;

        ULXR_TRACE("Connection::read");

        if (!buff || !isOpen())
            throw RuntimeException(ApplicationError, "Precondition failed for read() call");

        if (len <= 0)
            return 0;

        fd_set rfd;

        FD_ZERO(&rfd);
        FD_SET((unsigned) fd_handle, &rfd);

        if (hasPendingInput())
        {
            ULXR_TRACE("Connection: has pending input");
            if( (myRead = low_level_read(buff, len)) < 0)
            {
                throw ConnectionException(SystemError, "Could not perform read() call on pending input: " + getErrorString(getLastError()), 500);
            }
            ULXR_TRACE("read pending read " + toString(myRead) + " and wanted " + toString(len));
        }
        else
        {
            ULXR_TRACE("Connection: no pending input");

            const unsigned myTimeoutSec = getTimeout();
            timeval wait;
            wait.tv_sec = myTimeoutSec;
            wait.tv_usec = 0;
            int ready;
            while((ready = ::select(fd_handle+1, &rfd, 0, 0, &wait)) < 0)
            {
                ULXR_TRACE("read ~select");
                if (errno == EINTR || errno == EAGAIN)
                {
                    // signal received, continue select
                    wait.tv_sec = myTimeoutSec;
                    wait.tv_usec = 0;
                    continue;
                }

                else
                {
                    throw ConnectionException(SystemError, "Could not perform select() call: " + getErrorString(getLastError()), 500);
                }
            }

            if (ready == 0)
            {
                throw ConnectionException(SystemError,
                                          "Timeout while attempting to read (after " + toString(myTimeoutSec) + " seconds).", 500);
            }

            if (FD_ISSET(fd_handle, &rfd))
            {
                while ( (myRead = low_level_read(buff, len)) < 0)
                {
                    ULXR_TRACE("Connection::read:: got " << getErrorString(getLastError()));
                    switch(getLastError())
                    {
                    case EAGAIN:
                    case EINTR:
                        errno = 0;
                        continue;

                    default:
                        throw ConnectionException(SystemError,
                                                  "Could not perform read() call: "
                                                  + getErrorString(getLastError()), 500);
                    }
                }
            }
        }


        ULXR_TRACE("Connection: reading " << myRead << " bytes");
        ULXR_DWRITE_READ(buff, myRead);

        // have Content-length field and got unexpected EOF?
        // otherwise caller gets until EOF
        if (myRead == 0 /*&& getBytesToRead() >= 0*/)
        {
            close();
            throw ConnectionException(TransportError,
                                      "Attempt to read from a connection already closed by the peer", 500);
        }

        return myRead;
    }

    void Connection::close()
    {
        ULXR_TRACE("Connection::close");
        if (isOpen())
        {
            int ret;
            ULXR_TRACE("Connection::close (2)");
            do
            {
                ret=::close(fd_handle);
            }
            while(ret < 0 && (errno == EINTR || errno == EAGAIN));

            if (ret < 0)
                throw ConnectionException(TransportError,
                                          "Close failed: "+getErrorString(getLastError()), 500);
        }
        fd_handle = -1;
        ULXR_TRACE("/Connection::close");
    }


    int Connection::getLastError()
    {
        return(errno);
    }


    std::string Connection::getErrorString(int err_number)
    {
        return(getLastErrorString(err_number));
    }


    int Connection::getHandle() const
    {
        return fd_handle;
    }


    void Connection::setHandle(int handle)
    {
        close();
        fd_handle = handle;
    }

    void Connection::setTimeout(unsigned to_sec)
    {
        ULXR_TRACE("Set read/write timeout to " << to_sec << " sec");
        theRwTimeoutSec = to_sec;
    }


    unsigned Connection::getTimeout() const
    {
        return theRwTimeoutSec;
    }

}  // namespace ulxr
