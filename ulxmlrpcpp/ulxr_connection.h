/***************************************************************************
           ulxr_connection.h  -  provide a connection for rpc-data
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_connection.h 11073 2011-10-25 12:44:58Z korosteleva $

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

#ifndef ULXR_CONNECTION_H
#define ULXR_CONNECTION_H

#include <ulxmlrpcpp/ulxmlrpcpp.h>

namespace ulxr {

    /** @brief A connection object to transport XML-RPC calls.
      * @defgroup grp_ulxr_connection A connection for an XML-RPC call
      */


    /** Base class for connection between XML RPC client and server.
      * @ingroup grp_ulxr_connection
      */
    class  Connection
    {
    public:

        /** Constructs a connection.
          * The connection is not yet open after construction.
          */
        Connection();

        /** Destroys the connection.
          * The connection is closed now at the latest.
          */
        virtual ~Connection();

        /** Closes the connection.
          */
        virtual void close();

        /** Stops serving for server-side connection
        */
        virtual void stopServing() = 0;

        /** Writes data to the connection.
          * @param  buff pointer to data
          * @param  len  valid buffer length
          */
        virtual void write(char const *buff, long len);

        /** Reads data from the connection.
          * @param  buff pointer to data buffer
          * @param  len  maimum number of bytes to read into buffer
          * @return number of actually read bytes
          */
        virtual size_t read(char *buff, long len);

        /** Opens the connection in rpc client mode.
          */
        virtual void open() = 0;

        /** Opens the connection in rpc server mode, thus waiting for
          * connections from clients.
          * @param timeout the timeout value [sec] (0 - no timeout)
          * @returns <code>true</code> when connection has been accepted
          */
        virtual bool accept(int timeout = 0) = 0;

        /** Tests if the connection is open.
          * @return true if connection is already open.
          */
        virtual bool isOpen() const;

        /** Sets timeout for read/write operations
          * @param  to_sec  time in seconds
          */
        void setTimeout(unsigned to_sec);

        /** Gets timeout for read/write operations
          * @return time in seconds
          */
        unsigned getTimeout() const;

        /** Portable function to return the current error number.
         * @return error number (errno under Unices)
         */
        virtual int getLastError();

        /** Portable function to return the error description for a given error number
         * @param  err_number    system dependent error code
         * @return error string
         */
        virtual std::string getErrorString(int err_number);

        /** Returns the file handle of the connection.
          * @return  the file handle
          */
        int getHandle() const;

        virtual int getServerIpv4Handle() = 0;
        virtual int getServerIpv6Handle() = 0;

    protected:

        /** Sets the file handle of the connection.
          * @param handle  the file handle
          */
        void setHandle(int handle);

        /** Actually writes data to the connection.
          * @param  buff pointer to data
          * @param  len  valid buffer length
          * @return  result from api write function
          */
        virtual size_t low_level_write(char const *buff, long len);

        /** Reads data from the connection.
          * @param  buff pointer to data buffer
          * @param  len  maimum number of bytes to read into buffer
          * @return  result from api read function
          */
        virtual size_t low_level_read(char *buff, long len);

        /** Checks if there is input data which can intermediately be read.
          * @return true: data available
          */
        virtual bool hasPendingInput() const;

    private:

        /** Initializes internal variables.
          */
        void init();

    private:
        int                    fd_handle;
        unsigned               theRwTimeoutSec;
    };


}  // namespace ulxr


#endif // ULXR_CONNECTION_H
