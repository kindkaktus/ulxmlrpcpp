/***************************************************************************
                     ulxr_protocol.cpp  -  rpc prootocol
                             -------------------
    begin                : Mon May 3 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_protocol.cpp 11073 2011-10-25 12:44:58Z korosteleva $

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

//#define ULXR_SHOW_TRACE
//#define ULXR_DEBUG_OUTPUT
// #define ULXR_SHOW_READ
// #define ULXR_SHOW_WRITE


#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_connection.h>
#include <ulxmlrpcpp/ulxr_protocol.h>
#include <ulxmlrpcpp/ulxr_response.h>
#include <ulxmlrpcpp/ulxr_call.h>


namespace ulxr {


    struct Protocol::AuthData
    {
        AuthData(const std::string &user_, const std::string &pass_, const std::string &realm_)
            : user(user_)
            , pass(pass_)
            , realm(realm_)
        {}

        std::string  user;
        std::string  pass;
        std::string  realm;
    };


    struct Protocol::PImpl
    {
        Connection     *connection;
        bool            delete_connection;
        State           connstate;
        long            content_length;
        long            remain_content_length;

        std::vector<AuthData>  authdata;
    };


    Protocol::Protocol(const Protocol &prot)
        : pimpl (new PImpl)
    {
        *pimpl = *prot.pimpl;
    }


    Protocol::Protocol(Connection *conn)
        : pimpl (new PImpl)
    {
        pimpl->connection = conn;
        pimpl->delete_connection = false;
        ULXR_TRACE("Protocol");
        init();
    }


    Protocol::~Protocol()
    {
        ULXR_TRACE("~Protocol");
        if (pimpl->delete_connection)
            delete pimpl->connection;
        pimpl->connection = NULL;
        delete pimpl;
        pimpl = NULL;
    }


    void Protocol::init()
    {
        ULXR_TRACE("init");
        resetConnection();
    }


    void Protocol::writeRaw(char const *buff, long len)
    {
        ULXR_TRACE("writeRaw");
        getConnection()->write(buff, len);
    }


    long Protocol::readRaw(char *buff, long len)
    {
        ULXR_TRACE("readRaw, want: " << len);
        if (pimpl->remain_content_length >= 0)
        {
            ULXR_TRACE("read 0 " << len << " " << getRemainingContentLength());
            if (pimpl->remain_content_length < len)
                len = pimpl->remain_content_length;
        }

        long myRead = getConnection()->read(buff, len);

        if (pimpl->remain_content_length >= 0)
            pimpl->remain_content_length -= myRead;

        return myRead;
    }


    void Protocol::open()
    {
        ULXR_TRACE("open");
        getConnection()->open();
        resetConnection();
    }


    bool Protocol::isOpen() const
    {
        const Connection *conn = getConnection();
        bool op = conn != 0 && conn->isOpen();
        ULXR_TRACE("isOpen " << op);
        return op;
    }


    bool Protocol::accept(int _timeout)
    {
        ULXR_TRACE("accept");
        bool res = getConnection()->accept(_timeout);
        resetConnection();
        return res;
    }


    void Protocol::closeConnection()
    {
        ULXR_TRACE("closeConnection");
        if (getConnection())
            getConnection()->close();
//  resetConnection();
    }

    void Protocol::stopServing()
    {
        ULXR_TRACE("stopServing");
        if (getConnection())
            getConnection()->stopServing();
    }



    Connection* Protocol::getConnection() const
    {
        ULXR_TRACE("getConnection");
        return pimpl->connection;
    }


    void Protocol::setConnection(Connection *conn)
    {
        ULXR_TRACE("getConnection");
        pimpl->connection = conn;
        pimpl->delete_connection = true;
        ULXR_TRACE("/getConnection");
    }


    Protocol::State Protocol::getConnectionState() const
    {
        return pimpl->connstate;
    }


    void Protocol::setConnectionState(State state)
    {
        pimpl->connstate = state;
    }


    void Protocol::setRemainingContentLength(long len)
    {
        pimpl->remain_content_length = len;
    }


    long Protocol::getRemainingContentLength() const
    {
        return pimpl->remain_content_length;
    }


    long Protocol::getContentLength() const
    {
        return pimpl->content_length;
    }


    void Protocol::setContentLength(long len)
    {
        pimpl->content_length = len;
    }


    Protocol::State Protocol::connectionMachine(char * &/*buffer*/, long &/*len*/)
    {
        ULXR_TRACE("connectionMachine");
        pimpl->connstate = ConnBody;
        return ConnBody;
    }


    void Protocol::sendRpcResponse(const MethodResponse &resp)
    {
        ULXR_TRACE("sendRpcResponse");
        std::string xml = resp.getXml(0)+"\n";
        getConnection()->write(xml.c_str(), xml.length());
    }


    void Protocol::sendRpcCall(const MethodCall &call,
                               const std::string &/*resource*/)
    {
        ULXR_TRACE("sendRpcCall");
        std::string xml = call.getXml(0)+"\n";
        getConnection()->write(xml.c_str(), xml.length());
    }


    void
    Protocol::addAuthentication(const std::string &user,
                                const std::string &pass,
                                const std::string &realm)
    {
        ULXR_TRACE("addAuthentication");
        pimpl->authdata.push_back(AuthData(stripWS(user), stripWS(pass), stripWS(realm)));
    }


    bool Protocol::checkAuthentication(const std::string &realm) const
    {
        ULXR_TRACE("checkAuthentication " << realm);
        if (pimpl->authdata.size() == 0)
            return true;   // accept all

        ULXR_TRACE("checkAuthentication 1");
        std::string user, pass;
        if (!getUserPass(user, pass))
            return false;

        ULXR_TRACE("checkAuthentication 2 "
                   << "user: " << user
                   << " pass: " << pass);
        for (unsigned i = 0; i < pimpl->authdata.size(); ++i)
            if (   pimpl->authdata[i].user == user
                    && pimpl->authdata[i].pass == pass
                    && pimpl->authdata[i].realm == realm)
                return true;

        ULXR_TRACE("checkAuthentication 3");
        return false;
    }


    bool Protocol::getUserPass(std::string & /* user */,
                               std::string & /* pass */) const
    {
        ULXR_TRACE("getUserPass");
        return false;
    }


    void Protocol::rejectAuthentication(const std::string & /* realm */)
    {
        ULXR_TRACE("rejectAuthentication");
    }


    void Protocol::setMessageAuthentication(const std::string & /* user */,
                                            const std::string & /* pass */)
    {
        ULXR_TRACE("setMessageAuthentication");
    }


    void Protocol::resetConnection()
    {
        ULXR_TRACE("resetConnection");
        pimpl->connstate = ConnStart;
        pimpl->remain_content_length = -1;
        pimpl->content_length = -1;
    }


}  // namespace ulxr
