/*
 * Copyright (C) 2003-2005 Tommi Maekitalo
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * As a special exception, you may use this file as part of a free
 * software library without restriction. Specifically, if other files
 * instantiate templates or use macros or inline functions from this
 * file, or you compile this file and link it with other files to
 * produce an executable, this file does not by itself cause the
 * resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other
 * reasons why the executable file might be covered by the GNU Library
 * General Public License.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <tnt/httperror.h>
#include <tnt/http.h>
#include <tnt/htmlescostream.h>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace tnt
{
  ////////////////////////////////////////////////////////////////////////
  // HttpError
  //

  namespace
  {
    std::string httpErrorFormat(unsigned errcode, const std::string& msg)
    {
      char d[3];
      d[2] = '0' + errcode % 10;
      errcode /= 10;
      d[1] = '0' + errcode % 10;
      errcode /= 10;
      d[0] = '0' + errcode % 10;
      std::string ret(d, 3);
      ret += ' ';
      ret += msg;
      return ret;
    }

    struct HttpMsg
    {
      unsigned statusCode;
      const char* statusText;
    };

    const static HttpMsg httpMsgs[] = {
        { 0, "UNKNOWN" },
        { HTTP_CONTINUE, "Continue" },
        { HTTP_SWITCHING_PROTOCOLS, "Switching Protocols" },
        { HTTP_OK, "OK" },
        { HTTP_PROCESSING, "Processing" },
        { HTTP_CREATED, "Created" },
        { HTTP_ACCEPTED, "Accepted" },
        { HTTP_NON_AUTHORITATIVE, "Non-Authoritative Information" },
        { HTTP_NO_CONTENT, "No Content" },
        { HTTP_RESET_CONTENT, "Reset Content" },
        { HTTP_PARTIAL_CONTENT, "Partial Content" },
        { HTTP_MULTI_STATUS, "Multi Status" },
        { HTTP_MULTIPLE_CHOICES, "Multiple Choices" },
        { HTTP_MOVED_PERMANENTLY, "Moved Permanently" },
        { HTTP_MOVED_TEMPORARILY, "Moved Temporarily" },
        { HTTP_SEE_OTHER, "See Other" },
        { HTTP_NOT_MODIFIED, "Not Modified" },
        { HTTP_USE_PROXY, "Use Proxy" },
        { HTTP_TEMPORARY_REDIRECT, "Temporary Redirect" },
        { HTTP_BAD_REQUEST, "Bad Request" },
        { HTTP_UNAUTHORIZED, "Unauthorized" },
        { HTTP_PAYMENT_REQUIRED, "Payment Required" },
        { HTTP_FORBIDDEN, "Forbidden" },
        { HTTP_NOT_FOUND, "Not Found" },
        { HTTP_METHOD_NOT_ALLOWED, "Method Not Allowed" },
        { HTTP_NOT_ACCEPTABLE, "Not Acceptable" },
        { HTTP_PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required" },
        { HTTP_REQUEST_TIME_OUT, "Request Time-out" },
        { HTTP_CONFLICT, "Conflict" },
        { HTTP_GONE, "Gone" },
        { HTTP_LENGTH_REQUIRED, "Length Required" },
        { HTTP_PRECONDITION_FAILED, "Precondition Failed" },
        { HTTP_REQUEST_ENTITY_TOO_LARGE, "Request Entity Too Large" },
        { HTTP_REQUEST_URI_TOO_LARGE, "Request URI Too Large" },
        { HTTP_UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type" },
        { HTTP_RANGE_NOT_SATISFIABLE, "Range not satisfiable" },
        { HTTP_EXPECTATION_FAILED, "Expectation Failed" },
        { HTTP_UNPROCESSABLE_ENTITY, "Unprocessable Entity" },
        { HTTP_LOCKED, "Locked" },
        { HTTP_FAILED_DEPENDENCY, "Failed Dependency" },
        { HTTP_UPGRADE_REQUIRED, "Upgrade Required" },
        { HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error" },
        { HTTP_NOT_IMPLEMENTED, "Not Implemented" },
        { HTTP_BAD_GATEWAY, "Bad Gateway" },
        { HTTP_SERVICE_UNAVAILABLE, "Service Unavailable" },
        { HTTP_GATEWAY_TIME_OUT, "Gateway Time Out" },
        { HTTP_VERSION_NOT_SUPPORTED, "Version Not Supported" },
        { HTTP_VARIANT_ALSO_VARIES, "Variant Also Varies" },
        { HTTP_INSUFFICIENT_STORAGE, "Insufficient Storage" },
        { HTTP_NOT_EXTENDED, "Not Extended" }
    };

    const static HttpMsg* httpMsgsBegin = httpMsgs;
    const static HttpMsg* httpMsgsEnd = httpMsgs + sizeof(httpMsgs)/sizeof(HttpMsg);

    inline bool operator< (const HttpMsg& m1, const HttpMsg& m2)
    { return m1.statusCode < m2.statusCode; }

    inline std::ostream& operator<< (std::ostream& out, const HttpMsg& msg)
    { return out << msg.statusCode << ' ' << msg.statusText; }

  }

  HttpReturn::HttpReturn(unsigned returncode_)
    : returncode(returncode_),
      msg(httpMessage(returncode_))
  {
  }

  HttpReturn::HttpReturn(unsigned returncode_, const char* msg_)
    : returncode(returncode_),
      msg(msg_)
  {
  }

  const char* HttpReturn::httpMessage(unsigned httpstatus)
  {
    HttpMsg msg;
    msg.statusCode = httpstatus;
    const HttpMsg* m = std::lower_bound(httpMsgsBegin, httpMsgsEnd, msg);
    return m == httpMsgsEnd || m->statusCode != httpstatus ? "-" : m->statusText;
  }

  HttpError::HttpError(unsigned errcode)
    : msg(HttpReturn::httpMessage(errcode))
  {
    std::ostringstream b;
    HtmlEscOstream sb(b);

    b << "<html><body><h1>Error</h1><p>";
    sb << msg;
    b << "</p></body></html>";
    body = b.str();

    msg = httpErrorFormat(errcode, msg);
  }

  HttpError::HttpError(unsigned errcode, const std::string& m)
    : msg(httpErrorFormat(errcode, m))
  {
    std::ostringstream b;
    HtmlEscOstream sb(b);

    b << "<html><body><h1>Error</h1><p>";
    sb << m;
    b << "</p></body></html>";
    body = b.str();
  }

  HttpError::HttpError(unsigned errcode, const std::string& m, const std::string& b)
    : msg(httpErrorFormat(errcode, m)),
      body(b)
  {
  }

  std::string HttpError::getErrmsg() const
  {
    std::string::size_type p = msg.find('\n', 4);
    return p == std::string::npos ? msg.substr(4) : msg.substr(4, p - 4);
  }

  NotFoundException::NotFoundException(const std::string& url_)
    : HttpError(HTTP_NOT_FOUND, "Not Found"),
      url(url_)
  {
  }

  NotAuthorized::NotAuthorized(const std::string& realm)
    : HttpError(HTTP_UNAUTHORIZED, "Unauthorized", "<html><body><h1>not authorized</h1></body></html>")
  {
    setHeader(httpheader::wwwAuthenticate, "Basic realm=\"" + realm + '"');
  }

  MovedTemporarily::MovedTemporarily(const std::string& url)
    : HttpError(HTTP_MOVED_TEMPORARILY,
                "Moved Temporarily",
                "<html><body>moved to <a href=\"" + url + "\">" + url + "</a></body></html>")
  {
    setHeader(httpheader::location, url);
  }
}
