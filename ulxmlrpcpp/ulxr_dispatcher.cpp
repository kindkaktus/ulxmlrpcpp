/***************************************************************************
        ulxr_dispatcher.cpp  -  answer rpc requests ("rpc-server")
                             -------------------
    begin                : Sat Mar 23 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_dispatcher.cpp 10939 2011-09-12 13:22:25Z korosteleva $

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
//#define ULXR_SHOW_READ
//#define ULXR_SHOW_WRITE
//#define ULXR_SHOW_XML

#include <algorithm>
#include <memory>

#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_dispatcher.h>
#include <ulxmlrpcpp/ulxr_protocol.h>
#include <ulxmlrpcpp/ulxr_callparse.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_signature.h>

namespace ulxr {



    Dispatcher::MethodCallDescriptor::MethodCallDescriptor(const MethodCall &call)
    {
        method_name = call.getMethodName();
        documentation = "";
        return_signature = "";

        signature = call.getSignature(false);

        calltype = CallNone;
        invoked = 0;
        enabled = true;
    }


    Dispatcher::MethodCallDescriptor::MethodCallDescriptor(
        CallType type,
        const std::string &ret_sig,
        const std::string &name,
        const std::string &sig,
        const std::string &help)
    {
        method_name = name;
        documentation = help;
        return_signature = ret_sig;
        signature = sig;
        calltype = type;
        invoked = 0;
        enabled = true;
    }


    unsigned long Dispatcher::MethodCallDescriptor::getInvoked() const
    {
        return invoked;
    }


    void Dispatcher::MethodCallDescriptor::incInvoked() const
    {
        ++invoked;
    }


    bool Dispatcher::MethodCallDescriptor::isEnabled() const
    {
        return enabled;
    }


    void Dispatcher::MethodCallDescriptor::setEnabled(bool ena) const
    {
        enabled = ena;
    }


    std::string
    Dispatcher::MethodCallDescriptor::getSignature(bool with_name,
            bool with_return) const
    {
        ULXR_TRACE("getSignature");
        std::string s;
        std::string rs = return_signature;
        if (rs.length() == 0)
            rs = "void";  // emergency brake

        std::string sig = signature;
        if (sig.length() == 0)
            sig = "void";  // emergency brake

        if (with_return && with_name)
            s = rs + " " + method_name + "(" + sig + ")";

        else if (!with_return && with_name)
            s = method_name + "(" + sig + ")";

        else if (with_return && !with_name)
        {
            s = rs;
            if (sig.length() != 0)
                s += "," + sig;
        }

        else if (!with_return && !with_name)
            s = sig;

        return s;
    }


    std::string Dispatcher::MethodCallDescriptor::getMethodName() const
    {
        return method_name;
    }


    std::string Dispatcher::MethodCallDescriptor::getParameterSignature() const
    {
        return signature;
    }


    std::string Dispatcher::MethodCallDescriptor::getReturnValueSignature() const
    {
        return return_signature;
    }


    std::string Dispatcher::MethodCallDescriptor::getDocumentation() const
    {
        return documentation;
    }


    Dispatcher::CallType Dispatcher::MethodCallDescriptor::getCallType() const
    {
        return calltype;
    }



///////////////////////////////////////////////////////////////////////////////////////



    Dispatcher::Dispatcher (Protocol* prot)
    {
        protocol = prot;
        setupSystemMethods();
    }


    void free_dynamic_method (const Dispatcher::MethodCallMap::value_type &method)
    {
        if (method.first.getCallType() == Dispatcher::CallDynamic)
        {
            ULXR_TRACE("Now deleting dynamic function: " + method.first.getSignature(true, true));
            delete method.second.dynamic_function;
            const_cast<Dispatcher::MethodCallMap::value_type&>(method).second.dynamic_function = 0;
        }
    }


    Dispatcher::~Dispatcher ()
    {
        ULXR_TRACE("~Dispatcher ()");
        std::for_each(methodcalls.begin(), methodcalls.end(), free_dynamic_method);
        methodcalls.clear();
    }


    void Dispatcher::removeMethod(const std::string &name)
    {
        ULXR_TRACE("removeMethod " << name);
        MethodCallMap::iterator it;
        for(it = methodcalls.begin(); it != methodcalls.end(); ++it)
        {
            if (name == (*it).first.getMethodName())
            {
                free_dynamic_method(*it);
                methodcalls.erase(it);
            }
        }
    }


    void Dispatcher::addMethod (StaticMethodCall_t func,
                                const std::string &ret_signature,
                                const std::string &name,
                                const std::string &signature,
                                const std::string &help)
    {
        ULXR_TRACE("addMethod(static)");
        MethodCallDescriptor desc (CallStatic, ret_signature, name, signature, help);
        MethodCall_t mct;
        mct.static_function = func;
        addMethodDescriptor (desc, mct);
    }


    void Dispatcher::addMethod (DynamicMethodCall_t func,
                                const std::string &ret_signature,
                                const std::string &name,
                                const std::string &signature,
                                const std::string &help)
    {
        ULXR_TRACE("addMethod(dynamic)");
        MethodCallDescriptor desc (CallDynamic, ret_signature, name, signature, help);
        MethodCall_t mct;
        mct.dynamic_function = func;  // takes ownership
        addMethodDescriptor (desc, mct);
    }


    void Dispatcher::addMethod (SystemMethodCall_t func,
                                const std::string &ret_signature,
                                const std::string &name,
                                const std::string &signature,
                                const std::string &help)
    {
        ULXR_TRACE("addMethod(system)");
        MethodCallDescriptor desc (CallSystem, ret_signature, name, signature, help);
        MethodCall_t mct;
        mct.system_function = func;
        addMethodDescriptor (desc, mct);
    }


    void Dispatcher::addMethod (StaticMethodCall_t func,
                                const Signature &ret_signature,
                                const std::string &name,
                                const Signature &signature,
                                const std::string &help)
    {
        addMethod(func, ret_signature.getString(), name, signature.getString(), help);
    }


    void Dispatcher::addMethod (DynamicMethodCall_t func,
                                const Signature &ret_signature,
                                const std::string &name,
                                const Signature &signature,
                                const std::string &help)
    {
        addMethod(func, ret_signature.getString(), name, signature.getString(), help);
    }


    void Dispatcher::addMethod (SystemMethodCall_t func,
                                const Signature &ret_signature,
                                const std::string &name,
                                const Signature &signature,
                                const std::string &help)
    {
        addMethod(func, ret_signature.getString(), name, signature.getString(), help);
    }


    void
    Dispatcher::addMethodDescriptor (const MethodCallDescriptor &desc,
                                     MethodCall_t mct)
    {
        ULXR_TRACE("addMethodDescriptor " << desc.getSignature(true, false));
        if (methodcalls.find(desc) != methodcalls.end() )
            throw RuntimeException(ApplicationError, "Method exists already: " + desc.getSignature(true, false));

        methodcalls.insert(std::make_pair(desc, mct));
    }


    MethodCall Dispatcher::waitForCall(int _timeout)
    {
        ULXR_TRACE("waitForCall");
        if (!protocol->isOpen())
        {
            if (!protocol->accept(_timeout))
                return MethodCall();  // // @todo throw exception?
        }
        else
            protocol->resetConnection();

        char buffer[ULXR_RECV_BUFFER_SIZE];
        char *buff_ptr;


        std::auto_ptr<XmlParserBase> parser;
        MethodCallParserBase *cpb = 0;
        ULXR_TRACE("waitForCall in XML");
        MethodCallParser *cp = new MethodCallParser();
        cpb = cp;
        parser.reset(cp);

        bool done = false;
        long myRead;
        while (!done && ((myRead = protocol->readRaw(buffer, sizeof(buffer))) > 0) )
        {
            buff_ptr = buffer;
            while (myRead > 0)
            {
                Protocol::State state = protocol->connectionMachine(buff_ptr, myRead);
                if (state == Protocol::ConnError)
                    throw ConnectionException(TransportError, "network problem occured", 500);

                else if (state == Protocol::ConnSwitchToBody)
                {
                    if (!protocol->hasBytesToRead())
                    {
#ifdef ULXR_SHOW_READ
                        std::string super_data(buff_ptr, myRead);
                        while ((myRead = protocol->readRaw(buffer, sizeof(buffer))) > 0)
                            super_data.append(buffer, myRead);
                        ULXR_DOUT_READ("superdata 1 start:\n"
                                       << super_data
                                       << "superdata 1 end:\n");
#endif
                        throw ConnectionException(NotConformingError,  "Content-Length of message not available", 411);
                    }
                }

                else if (state == Protocol::ConnBody)
                {
                    ULXR_DOUT_XML(std::string(buff_ptr, myRead));
                    if (!parser->parse(buff_ptr, myRead, done))
                    {
                        ULXR_DOUT("errline: " << parser->getCurrentLineNumber());
                        ULXR_DWRITE(buff_ptr, myRead);
                        ULXR_DOUT("") ;

                        throw XmlException(parser->mapToFaultCode(parser->getErrorCode()),
                                           "Problem while parsing xml request",
                                           parser->getCurrentLineNumber(),
                                           parser->getErrorString(parser->getErrorCode()));
                    }
                    myRead = 0;
                }
            }

            if (!protocol->hasBytesToRead())
//        || parser->isComplete())
                done = true;
        }

        ULXR_TRACE("waitForCall got " << cpb->getMethodCall().getXml());
        return cpb->getMethodCall();
    }


    const Dispatcher::MethodCallDescriptor* const Dispatcher::getMethod(unsigned index)
    {
        ULXR_TRACE("getMethod");
        unsigned i = 0;
        MethodCallMap::iterator it;
        for (it = methodcalls.begin(); it != methodcalls.end(); ++it, ++i)
            if (i == index)
                return &(*it).first;

        throw RuntimeException(ApplicationError, "Index too big for Dispatcher::getMethod()");
    }


    unsigned Dispatcher::numMethods() const
    {
        ULXR_TRACE("numMethods");
        unsigned i = 0;
        MethodCallMap::const_iterator it;
        for (it = methodcalls.begin(); it != methodcalls.end(); ++it)
            ++i;
        return i;
    }


    bool Dispatcher::hasMethod(const MethodCall &call) const
    {
        MethodCallDescriptor desc(call);
        return methodcalls.find(desc) != methodcalls.end();
    }


    MethodResponse Dispatcher::dispatchCall(const MethodCall &call) const
    {
        ULXR_TRACE("dispatchCall");
        try
        {
            return dispatchCallLoc(call);
        }

        catch (Exception &ex)
        {
            return MethodResponse (ex.getFaultCode(), ex.why());
        }

        catch (std::exception &ex)
        {
            return MethodResponse (ApplicationError, ex.what());
        }

        catch (...)
        {
            return MethodResponse (SystemError, "Unknown error occured");
        }
    }


    MethodResponse Dispatcher::dispatchCallLoc(const MethodCall &call) const
    {
        ULXR_TRACE("dispatchCallLoc: " << call.getMethodName());

        MethodCallDescriptor desc(call);
        MethodCallMap::const_iterator it;
        if ((it = methodcalls.find(desc)) != methodcalls.end() )
        {
            MethodCall_t mc = (*it).second;
            if (!(*it).first.isEnabled())
            {
                std::string s = "method \"";
                s += desc.getSignature(true, false);
                s += "\": currently unavailable.";
                return MethodResponse (MethodNotFoundError, s);
            }

            else
            {
                if ((*it).first.calltype == CallSystem)
                {
                    ULXR_TRACE("Now calling system function: " + (*it).first.getSignature(true, true));
                    (*it).first.incInvoked();
                    return mc.system_function(call, this);
                }

                else if ((*it).first.calltype == CallStatic)
                {
                    ULXR_TRACE("Now calling static function: " + (*it).first.getSignature(true, true));
                    (*it).first.incInvoked();
                    return mc.static_function(call);
                }

                else if ((*it).first.calltype == CallDynamic)
                {
                    ULXR_TRACE("Now calling dynamic function: " + (*it).first.getSignature(true, true));
                    (*it).first.incInvoked();
                    return mc.dynamic_function->call(call);
                }

                else
                {
                    std::string s = "method \"";
                    s += desc.getSignature(true, false);
                    s += "\": internal problem to find method.";
                    return MethodResponse (MethodNotFoundError, s);
                }
            }
        }

        std::string s = "method \"";
        s += desc.getSignature(true, false);
        s += "\" unknown method and/or signature.";
        return MethodResponse (MethodNotFoundError, s);
    }


    void Dispatcher::sendResponse(const MethodResponse &resp)
    {
        ULXR_TRACE("sendResponse");
        protocol->sendRpcResponse(resp);
    }


    void Dispatcher::setupSystemMethods()
    {
        ULXR_TRACE("setupSystemMethods");

        addMethod(&Dispatcher::xml_pretty_print,
                  "", "ulxmlrpcpp.pretty_print", "bool",
                  "Enable pretty-printed xml responses.");

        //--

        addMethod(&Dispatcher::system_listMethods,
                  "array", "system.listMethods", "",
                  "Lists all methods implemented by this server.");

        addMethod( &Dispatcher::system_listMethods,
                   "array","system.listMethods", "string",
                   "Lists all methods implemented by this server (overloaded).");

        addMethod( &Dispatcher::system_methodSignature,
                   "array", "system.methodSignature", "string",
                   "Returns an array of possible signatures for this method.");

        addMethod(&Dispatcher::system_methodHelp,
                  "string", "system.methodHelp", "string",
                  "Returns a documentation string describing the use of this method.");

        addMethod(&Dispatcher::system_getCapabilities,
                  "struct", "system.getCapabilities", "",
                  "Returns Structs describing available capabilities.");
    }


    MethodResponse
    Dispatcher::xml_pretty_print(const MethodCall &calldata,
                                 const Dispatcher *disp)
    {
        ULXR_TRACE("xml_pretty_print");
        if (calldata.numParams() > 1)
            throw ParameterException(InvalidMethodParameterError,
                                     "At most 1 parameter allowed for \"system.listMethods\"");

        if (   calldata.numParams() == 1
                && calldata.getParam(0).getType() != RpcBoolean)
            throw ParameterException(InvalidMethodParameterError,
                                     "Parameter 1 not of type \"Boolean\" \"ulxmlrpcpp.xml_pretty_print\"");

        bool enable = Boolean(calldata.getParam(0)).getBoolean();
        enableXmlPrettyPrint(enable);
        return MethodResponse (Void());
    }


    MethodResponse
    Dispatcher::system_listMethods(const MethodCall &calldata,
                                   const Dispatcher *disp)
    {
        ULXR_TRACE("system_listMethods");
        if (calldata.numParams() > 1)
            throw ParameterException(InvalidMethodParameterError,
                                     "At most 1 parameter allowed for \"system.listMethods\"");

        if (   calldata.numParams() == 1
                && calldata.getParam(0).getType() != RpcStrType)
            throw ParameterException(InvalidMethodParameterError,
                                     "Parameter 1 not of type \"String\" \"system.listMethods\"");

// FIXME: what to do with param 1 if present ??

        Array arr;
        std::string m_prev;

        MethodCallMap::const_iterator it;
        for (it = disp->methodcalls.begin(); it != disp->methodcalls.end(); ++it)
            if (   m_prev != (*it).first.method_name
                    && (*it).first.method_name.length() != 0)
            {
                arr.addItem(RpcString((*it).first.method_name));
                m_prev = (*it).first.method_name;
            }
        return MethodResponse (arr);
    }


    MethodResponse
    Dispatcher::system_methodSignature(const MethodCall &calldata,
                                       const Dispatcher *disp)
    {
        ULXR_TRACE("system_methodSignature");
        if (calldata.numParams() != 1)
            throw ParameterException(InvalidMethodParameterError,
                                     "Exactly 1 parameter allowed for \"system.methodSignature\"");

        if (calldata.getParam(0).getType() != RpcStrType)
            throw ParameterException(InvalidMethodParameterError,
                                     "Parameter 1 not of type \"String\" \"system.listMethods\"");

        RpcString vs = calldata.getParam(0);
        std::string name = vs.getString();
        MethodCallMap::const_iterator it;
        Array ret_arr;
        for (it = disp->methodcalls.begin(); it != disp->methodcalls.end(); ++it)
        {
            Array sigarr;
            std::string sig = (*it).first.getSignature(true, true);
            if (name == (*it).first.method_name && sig.length() != 0)
            {
                std::size_t pos;
                while ((pos = sig.find(',')) != std::string::npos)
                {
                    sigarr.addItem(RpcString(sig.substr(0, pos)));
                    sig.erase(0, pos+1);
                }
                sigarr.addItem(RpcString(sig));
                ret_arr.addItem(sigarr);
            }
        }

        if (ret_arr.size() == 0)
            return MethodResponse (Integer(1));  // non-Array ==< no signatures
        else
            return MethodResponse (ret_arr);
    }


    MethodResponse
    Dispatcher::system_methodHelp(const MethodCall &calldata,
                                  const Dispatcher *disp)
    {
        ULXR_TRACE("system_methodHelp");
        if (calldata.numParams() != 1)
            throw ParameterException(InvalidMethodParameterError,
                                     "Exactly 1 parameter allowed for \"system.methodHelp\"");

        if (calldata.getParam(0).getType() != RpcStrType)
            throw ParameterException(InvalidMethodParameterError,
                                     "Parameter 1 not of type \"String\" \"system.listMethods\"");

        RpcString vs = calldata.getParam(0);
        std::string name = vs.getString();
        std::string s;

        MethodCallMap::const_iterator it;
        std::string s_prev;
        for (it = disp->methodcalls.begin(); it != disp->methodcalls.end(); ++it)
            if (name == (*it).first.method_name && (*it).first.documentation.length() != 0)
            {
                if (   s_prev != (*it).first.documentation
                        && (*it).first.documentation.length() != 0)
                {
                    if (s.length() != 0)
                        s = "* " +s + "\n* ";
                    s += (*it).first.documentation;
                }
                s_prev = (*it).first.documentation;
            }

        return MethodResponse (RpcString(s));
    }


    void Dispatcher::getCapabilities (Struct &str) const
    {
        // parent::getCapabilities (str);  just in case..
        str.addMember("specUrl",
                      RpcString("http://xmlrpc-epi.sourceforge.net/specs/rfc.fault_codes.php"));
        str.addMember("specVersion", Integer(20010516));
    }


    MethodResponse
    Dispatcher::system_getCapabilities(const MethodCall &calldata,
                                       const Dispatcher *disp)
    {
        if (calldata.numParams() > 1)
            throw ParameterException(InvalidMethodParameterError,
                                     "No parameters allowed for \"system.listMethods\"");

        Struct sysCap;
        disp->getCapabilities(sysCap);

        Struct opStr;
        opStr.addMember("faults_interop", sysCap);
        return MethodResponse (opStr);
    }


    Protocol* Dispatcher::getProtocol() const
    {
        return protocol;
    }


    void Dispatcher::setProtocol(Protocol *prot)
    {
        protocol = prot;
    }

    namespace hidden {

        MethodWrapperBase::~MethodWrapperBase()
        {
        }

    }

}  // namespace ulxr

