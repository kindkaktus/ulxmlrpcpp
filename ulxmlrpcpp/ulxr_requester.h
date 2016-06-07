/***************************************************************************
             ulxr_requester.h  -  send rpc request ("rpc-client")
                             -------------------
    begin                : Sat Mar 23 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_requester.h 11073 2011-10-25 12:44:58Z korosteleva $

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

#ifndef ULXR_REQUESTER_H
#define ULXR_REQUESTER_H

#include <exception>

#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_call.h>
#include <ulxmlrpcpp/ulxr_response.h>


namespace ulxr {


    class Protocol;
    class Connection;

    /** XML RPC Requester (rpc client).
      * The requester takes the MethodCall, converts it to xml and sends
      * it over the connection. It then waits for the response which must
      * be further processes by the caller.
      * @ingroup grp_ulxr_rpc
      */
    class  Requester
    {
    public:

        /** Constructs a requester.
          * @param  prot        pointer to an existing Connection
          */
        Requester(Protocol* prot);

        virtual ~Requester();

        /** Performs a virtual call to the remote method
          * "behind" the connection.
          * @param   call      the data for the call
          * @param   resource  resource for rpc on remote host
          * @return the methods response
          */
        MethodResponse call (const MethodCall& call,
                             const std::string &resource);

        /** Performs a virtual call to the remote method
          * "behind" the connection.
          * @param   call      the data for the call
          * @param   resource  resource for rpc on remote host
          * @param   user      user name
          * @param   pass      password
          * @return the methods response
          */
        MethodResponse call (const MethodCall& call,
                             const std::string &resource,
                             const std::string &user,
                             const std::string &pass);


        /** Waits for the response from the remote server.
          * @param  conn   connection to wait for data
          * @return methode response
          */
        static MethodResponse waitForResponse(Protocol *conn);


    protected:
        /** Sends the call data to the remote method.
          * @param   call      the data for the call
          * @param   resource  resource for rpc on remote host
          */
        void send_call(const MethodCall& call,
                       const std::string &resource);

        /** Waits for the response from the remote server.
          * @return methode response
          */
        MethodResponse waitForResponse();


    private:
        Protocol          *protocol;
    };


}  // namespace ulxr


#endif // ULXR_REQUESTER_H
