/*
 * Copyright (C) 2009 by Dr. Marc Boris Duerner
 * Copyright (C) 2009 by Tommi Meakitalo
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * cxxtools as published by the Free Software Foundation; either
 * version 2.1 of the cxxtools, or (at your option) any later version.
 * 
 * As a special exception, you may use this file as part of a free
 * software library without restriction. Specifically, if other files
 * instantiate templates or use macros or inline functions from this
 * file, or you compile this file and link it with other files to
 * produce an executable, this file does not by itself cause the
 * resulting executable to be covered by the GNU General Public
 * cxxtools. This exception does not however invalidate any other
 * reasons why the executable file might be covered by the GNU Library
 * General Public cxxtools.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public cxxtools for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * cxxtools along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef cxxtools_xmlrpc_Responder_h
#define cxxtools_xmlrpc_Responder_h

#include <cxxtools/xmlrpc/api.h>
#include <cxxtools/xmlrpc/fault.h>
#include <cxxtools/xmlrpc/scanner.h>
#include <cxxtools/xmlrpc/formatter.h>
#include <cxxtools/xml/xmlreader.h>
#include <cxxtools/xml/xmlwriter.h>
#include <cxxtools/httpserver.h>
#include <cxxtools/serializer.h>
#include <cxxtools/textstream.h>

namespace cxxtools {

namespace xmlrpc {

class Service;
class ServiceProcedure;

class CXXTOOLS_XMLRPC_API HttpXmlRpcResponder : public net::HttpResponder
{
    enum State
    {
        OnBegin,
        OnMethodCallBegin,
        OnMethodNameBegin,
        OnMethodName,
        OnMethodNameEnd,
        OnParams,
        OnParam,
        OnParamsEnd,
        OnMethodCallEnd
    };

    public:
        HttpXmlRpcResponder(Service& service);

        ~HttpXmlRpcResponder();

        void beginRequest(std::istream& in, net::HttpRequest& request);

        std::size_t readBody(std::istream& is);

        void replyError(std::ostream& os, net::HttpRequest& request,
                        net::HttpReply& reply, const std::exception& ex);

        void reply(std::ostream& os, net::HttpRequest& request, net::HttpReply& reply);

    protected:
        void advance(const cxxtools::xml::Node& node);

    private:
       State _state;
       TextIStream _ts;
       xml::XmlReader _reader;
       xml::XmlWriter _writer;
       Scanner _scanner;
       Formatter _formatter;
       Service* _service;
       DeserializationContext _context;
       ServiceProcedure* _proc;
       IDeserializer** _args;
       ISerializer* _result;
       Fault _fault;
};

}

}

#endif
