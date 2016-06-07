/***************************************************************************
           ulxr-method_adder.h  -  interface for adding methods
                             -------------------
    begin                : Thu Jul 12 2007
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_method_adder.h 10942 2011-09-13 14:35:52Z korosteleva $

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

#ifndef ULXR_METHOD_ADDER_H
#define ULXR_METHOD_ADDER_H

#include <ulxmlrpcpp/ulxmlrpcpp.h>

namespace ulxr {


    class MethodResponse;
    class MethodCall;
    class Dispatcher;
    class Signature;


    namespace hidden {


        /** Internal helper class, not intended for public use.
         */
        class  MethodWrapperBase
        {
        public:

            virtual ~MethodWrapperBase();

            virtual MethodResponse call(const MethodCall &calldata) const = 0;
        };


        /** Internal helper class template, not intended for public use.
         */
        template <class T>
        class MethodWrapper : public MethodWrapperBase
        {
        public:

            typedef MethodResponse (T::*PMF)(const MethodCall &calldata);

            virtual ~MethodWrapper()
            {
            }

            virtual MethodResponse call (const MethodCall &calldata) const
            {
                return (obj->*adr) (calldata);
            }

            MethodWrapper(T *o, PMF a)
                : obj(o), adr(a)
            {}

        private:
            T    *obj;
            PMF   adr ;

        private:
            // forbid them all due to internal pointers
            const MethodWrapper& operator= (const MethodWrapper&);
            MethodWrapper (const MethodWrapper&);
        };


    }  // namespace hidden


    /** Define interface for adding rpc method to a dispatcher
      * @ingroup grp_ulxr_rpc
      */
    class  MethodAdder
    {
    public:

        typedef MethodResponse (*StaticMethodCall_t)(const MethodCall &);

        typedef MethodResponse (*SystemMethodCall_t)(const MethodCall &,
                const Dispatcher* disp);

        typedef hidden::MethodWrapperBase*  DynamicMethodCall_t;    // call Wrappers call();

        enum   CallType { CallNone,
                          CallSystem,
                          CallStatic,
                          CallDynamic
                        };

    protected:

        friend StaticMethodCall_t make_method(const StaticMethodCall_t);

        typedef union
        {
            StaticMethodCall_t    static_function;
            SystemMethodCall_t    system_function;
            DynamicMethodCall_t   dynamic_function;
        } MethodCall_t;

    public:

        virtual ~MethodAdder()
        {}

        /** Adds a user defined (static) method to the dispatcher.
          * You access a remote method by sending the "official" name. Sometimes
          * a method accepts different parameter sets (overloading in C++).
          * In this case you add the according signature. Finally you can
          * add a description to show the usage of this method.
          * @param  adr            the pointer to the implementation of the method
          * @param  ret_signature  the signature of the return value
          * @param  name           the name of the method
          * @param  signature      the signature of the parameters
          * @param  help           short usage description
          */
        virtual void addMethod (StaticMethodCall_t adr,
                                const std::string &ret_signature,
                                const std::string &name,
                                const std::string &signature,
                                const std::string &help = "") = 0;

        /** Adds a user defined (dynamic) method to the dispatcher.
          * You access a remote method by sending the "official" name. Sometimes
          * a method accepts different parameter sets (overloading in C++).
          * In this case you add the according signature. Finally you can
          * add a description to show the usage of this method.
          * @param  wrapper        the pointer to the wrapper to the method.
          *                        Important: Dispatcher owns now and deletes this wrapper object!
          * @param  ret_signature  the signature of the return value
          * @param  name           the name of the method
          * @param  signature      the signature of the parameters
          * @param  help           short usage description
          */
        virtual void addMethod (DynamicMethodCall_t wrapper,
                                const std::string &ret_signature,
                                const std::string &name,
                                const std::string &signature,
                                const std::string &help = "") = 0;

        /** Adds a system internal method to the dispatcher.
          * You access a remote method by sending the "official" name. Sometimes
          * a method accepts different parameter sets (overloading in C++).
          * In this case you add the according signature. Finally you can
          * add a description to show the usage of this method.
          * @param  adr            the pointer to the implementation of the method
          * @param  ret_signature  the signature of the return value
          * @param  name           the name of the method
          * @param  signature      the signature of the parameters
          * @param  help           short usage description
          */
        virtual void addMethod (SystemMethodCall_t adr,
                                const std::string &ret_signature,
                                const std::string &name,
                                const std::string &signature,
                                const std::string &help = "") = 0;

        /** Adds a user defined (static) method to the dispatcher.
          * You access a remote method by sending the "official" name. Sometimes
          * a method accepts different parameter sets (overloading in C++).
          * In this case you add the according signature. Finally you can
          * add a description to show the usage of this method.
          * @param  adr            the pointer to the implementation of the method
          * @param  ret_signature  the signature of the return value
          * @param  name           the name of the method
          * @param  signature      the signature of the parameters
          * @param  help           short usage description
          */
        virtual void addMethod (StaticMethodCall_t adr,
                                const Signature &ret_signature,
                                const std::string &name,
                                const Signature &signature,
                                const std::string &help = "") = 0;

        /** Adds a user defined (dynamic) method to the dispatcher.
          * You access a remote method by sending the "official" name. Sometimes
          * a method accepts different parameter sets (overloading in C++).
          * In this case you add the according signature. Finally you can
          * add a description to show the usage of this method.
          * @param  wrapper        the pointer to the wrapper to the method.
          *                        Important: Dispatcher owns now and deletes this wrapper object!
          * @param  ret_signature  the signature of the return value
          * @param  name           the name of the method
          * @param  signature      the signature of the parameters
          * @param  help           short usage description
          */
        virtual void addMethod (DynamicMethodCall_t wrapper,
                                const Signature &ret_signature,
                                const std::string &name,
                                const Signature &signature,
                                const std::string &help = "") = 0;

        /** Adds a system internal method to the dispatcher.
          * You access a remote method by sending the "official" name. Sometimes
          * a method accepts different parameter sets (overloading in C++).
          * In this case you add the according signature. Finally you can
          * add a description to show the usage of this method.
          * @param  adr            the pointer to the implementation of the method
          * @param  ret_signature  the signature of the return value
          * @param  name           the name of the method
          * @param  signature      the signature of the parameters
          * @param  help           short usage description
          */
        virtual void addMethod (SystemMethodCall_t adr,
                                const Signature &ret_signature,
                                const std::string &name,
                                const Signature &signature,
                                const std::string &help = "") = 0;

        /** Removes a method if available
          * @param name   method name
          */
        virtual void removeMethod(const std::string &name) = 0;
    };


    /** Creates a wrapper object to a method of a worker class.
      * new ulxr::hidden::MethodWrapper<classname> (&obj, &classname::method),
      * @param w     reference to worker class
      * @param pmf   adress of member function in worker class
      * @return pointer to wrapper object
      */

    template <class T>
    inline hidden::MethodWrapperBase*
    make_method(T &w, typename hidden::MethodWrapper<T>::PMF pmf)
    {
        return new hidden::MethodWrapper<T> (&w, pmf);
    }


    typedef MethodAdder::StaticMethodCall_t MethodAdder_StaticMethodCall_t;

    /** Specialisation of template to achive uniform look in responder creation.
      * @param ptr   pointer to worker function
      * @return pointer to the function
      */
    inline MethodAdder_StaticMethodCall_t
    make_method(MethodAdder_StaticMethodCall_t ptr)
    {
        return ptr;
    }


} // namespace

#endif // ULXR_METHOD_ADDER_H
