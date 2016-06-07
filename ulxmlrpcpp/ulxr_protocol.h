/***************************************************************************
                    ulxr_protocol.h  -  rpc prootocol
                             -------------------
    begin                : Mon May 3 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_protocol.h 11073 2011-10-25 12:44:58Z korosteleva $

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

#ifndef ULXR_PROTOCOL_H
#define ULXR_PROTOCOL_H

#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <vector>


namespace ulxr {


    /** @brief A protocol object to transport XML-RPC calls.
      * @defgroup grp_ulxr_protocol A transportation protocol for an XML-RPC call
      */

    class MethodCall;
    class MethodResponse;
    class Connection;

    /** Base class for the protocol of the rpc transportation.
      * @ingroup grp_ulxr_protocol
      */
    class  Protocol
    {
    public:

        /** Constructs a Protocol.
          * @param  conn  pointer to connection object
          */
        Protocol(Connection *conn);

        /** Destroys the Protocol.
          */
        virtual ~Protocol();

        /** Writes data to the connection.
          * @param  buff pointer to data
          * @param  len  valid buffer length
          */
        void writeRaw(char const *buff, long len);

        /** Reads data from the connection.
          * @param  buff pointer to data buffer
          * @param  len  maimum number of bytes to read into buffer
          * @return number of actually read bytes
          */
        long readRaw(char *buff, long len);

        /** Opens the connection in rpc client mode.
          */
        virtual void open();

        /** Opens the connection in rpc server mode, thus waiting for
          * connections from clients.
          * @param timeout the timeout value [sec] (0 - no timeout)
          * @returns <code>true</code> when connection has been accepted
          */
        bool accept(int timeout = 0) ;

        /** Tests if the connection is open.
          * @return true if connection is already open.
          */
        bool isOpen() const;


        /** Closes the connection.
          */
        virtual void closeConnection();

        /** Stops serving for server-side connection
          */
        virtual void stopServing();

        /** Tests if there are bytes left in the message body.
          * @return true: there is at least one byte available
          */
        virtual bool hasBytesToRead() const = 0;

        /** Resets the state of the Protocol.
          * Before starting a transfer you should call this to ensure
          * a defined state of the internal state machine processing the
          * protocol.
          */
        virtual void resetConnection();

        /** Sends a MethodCall over the connection.
          * @param   call        pointer to the calling data
          * @param   resource    resource for rpc on remote host
          */
        virtual void sendRpcCall(const MethodCall &call, const std::string &resource);

        /** Sends a MethodResponse over the connection.
          * @param   resp        pointer to the response data
          */
        virtual void sendRpcResponse(const MethodResponse &resp);



        /** Tests if the response was successful regarding the transportation.
          * @param   phrase  return value describing the problem.
          * @return true  response is OK.
          */
        virtual bool isResponseStatus200(std::string &phrase) const = 0;

        /** General connection states while reading input stream
          */
        typedef enum
        {
            ConnStart,                 // 0 start and reset
            ConnPendingCR,             // 1 just found CR, wait for LF or fold whitespace
            ConnPendingHeaderLine,     // 2 new line started, may be folded
            ConnHeaderLine,            // 3 collection chars for current header line
            ConnSwitchToBody,          // 4 header completely read ==> analyse header fields
            ConnBody,                  // 5 in body, do nothing. Never reached if there is no body!
            ConnError,                 // 6 error occured, set status accordingly
            ConnLast                   // 7 last item for continuation
        }
        State;


        /** Process a chunk of input data from the connection.
          * Usually you call read() to get some more data and pass it to
          * this method. It processes the header (depending on your protocol).
          * Normally you only have to take care of two states: ConnError which
          * results in terminating the connection. ConnBody on the other side
          * indicates user data which might be fed to a parser. @see Dispatcher.
          * @param  buffer   pointer to input data
          * @param len       valid length of buffer
          * @return   current state of the processing. This return state may differ from the
          *           state returned by getConnectionState() due to internal processing.
          *           You should ignore everything except ConnBody and ConnError.
          */
        virtual State connectionMachine(char * &buffer, long &len);

        /** Adds another potential username, password and "range" for this connection (server mode).
         * @param  user   Username
         * @param  pass   Password
         * @param  realm  Synonym for area which shall be accessed
         */
        void addAuthentication(const std::string &user, const std::string &pass,
                               const std::string &realm);

        /** Sets username and password for the next transmission (client mode).
          * This makes the next transmission block use a simple authentication scheme
          * with a username and password.
          * @param  user   Username
          * @param  pass   Password
          */
        virtual void setMessageAuthentication(const std::string &user, const std::string &pass);

        /** Checks for a valid user in the current message.
          * There must be an appropriate AuthData set and the connection must support
          * username + password transmission (default is NOT to do so).
          * The caller is resonsible for sending a negative response if approriate.
          * @param  realm  Synonym for area which shall be accessed
          * @return true: access shall be granted.
          */
        virtual bool checkAuthentication(const std::string &realm) const;

        /** Sends a negative response to the caller
          * @param  realm   Description for the caller which username and password to pass in request
          * @return true: access shall be granted.
          */
        virtual void rejectAuthentication(const std::string &realm);

        /** Returns the connection object.
          * @return pointer to connection object
          */
        Connection * getConnection() const;

    protected:

        /** Sets the connection object.
          * The connection is then owned by the Protocol
          * @param  conn pointer to connection object
          */
        void setConnection(Connection *conn);

        /** Sets the connection state.
          * @param  state  new connection state
          */
        void setConnectionState(State state);

        /** Gets the connection state.
          * @return  current connection state
          */
        State getConnectionState() const;

        /** Sets the length of the remaining content.
          * @see getRemainingContentLength().
          * @param  len  number of bytes of content to come
          */
        void setRemainingContentLength(long len);

        /** Gets the length of the remaining content.
          * @return number of bytes of content to come, -1 if unknown
          */
        long getRemainingContentLength() const;

        /** Gets the length of the content.
          * @see getRemainingContentLength().
          * @return number of bytes of content, -1 if unknown
          */
        long getContentLength() const;

        /** Sets the length of the content.
          * @param len number of bytes of content
          */
        void setContentLength(long len);

        /** Extracts username and password from current message.
          * @param  user   reference to return username
          * @param  pass   reference to return password
          * @return true: username and password could be extracted
          */
        virtual bool getUserPass(std::string &user, std::string &pass) const;

        /** Determines the length of the message body.
          */
        virtual void determineContentLength() = 0;

        /** Returns the protocol name.
         * @return protocol name
         */
        virtual std::string getProtocolName() = 0;

    private:

        /** Initializes internal variables.
          */
        void init();

    private:

        struct AuthData;
        struct PImpl;
        PImpl *pimpl;

    protected:

        Protocol(const Protocol&);

    private:

        Protocol& operator=(const Protocol&); // empty!
    };


}  // namespace ulxr


#endif // ULXR_PROTOCOL_H

