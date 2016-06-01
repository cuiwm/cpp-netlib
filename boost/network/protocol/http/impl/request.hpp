
// Copyright Dean Michael Berris 2007,2009,2010.
// Copyright Michael Dickey 2008.
// Copyright Google, Inc. 2015
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_NETWORK_PROTOCOL_HTTP_REQUEST_IMPL_20070908_1_HPP__
#define BOOST_NETWORK_PROTOCOL_HTTP_REQUEST_IMPL_20070908_1_HPP__

/** request.hpp
  *
  * This file implements the basic request object required
  * by the HTTP client implementation. The basic_request
  * object encapsulates a URI which is parsed at runtime.
  */

#include <cstdint>
#include <vector>
#include <network/uri/uri.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/fusion/container/map.hpp>
#include <boost/fusion/sequence/intrinsic/at_key.hpp>
#include <boost/fusion/sequence/intrinsic/value_at_key.hpp>
#include <boost/network/constants.hpp>
#include <boost/network/protocol/http/message.hpp>
#include <boost/network/protocol/http/message/async_message.hpp>
#include <boost/network/protocol/http/message/header.hpp>
#include <boost/network/protocol/http/message/wrappers/body.hpp>
#include <boost/network/protocol/http/message/wrappers/headers.hpp>
#include <boost/network/support/is_async.hpp>

namespace boost {
namespace network {
namespace http {

/**
 * basic_request
 *
 * This is the basic implementation of an HTTP request object.
 *
 */
template <class Tag>
struct basic_request : public basic_message<Tag> {
  ::network::uri uri_;
  std::uint16_t source_port_;
  typedef basic_message<Tag> base_type;

 public:
  typedef Tag tag;
  typedef std::string string_type;
  typedef std::uint16_t port_type;

  explicit basic_request(string_type const& uri_)
      : uri_(uri_), source_port_(0) {}

  explicit basic_request(::network::uri const& uri_)
      : uri_(uri_), source_port_(0) {}

  basic_request() : base_type(), source_port_(0) {}

  basic_request(basic_request const& other)
      : base_type(other), uri_(other.uri_), source_port_(other.source_port_) {}

  basic_request& operator=(basic_request rhs) {
    rhs.swap(*this);
    return *this;
  }

  void swap(basic_request& other) {
    base_type& base_ref(other);
    basic_request<Tag>& this_ref(*this);
    base_ref.swap(this_ref);
    boost::swap(other.uri_, this->uri_);
    boost::swap(other.source_port_, this->source_port_);
  }

  string_type const host() const { return uri_.host().to_string(); }

  port_type port() const {
    if (uri_.has_port()) {
      return uri_.port<std::uint16_t>();
    }
    else {
      typedef constants<Tag> consts;
      return boost::iequals(uri_.scheme(), string_type(consts::https())) ? 443 : 80;
    }
  }

  string_type const path() const { return uri_.path().to_string(); }

  string_type const query() const { return uri_.query().to_string(); }

  string_type const anchor() const { return uri_.fragment().to_string(); }

  string_type const protocol() const { return uri_.scheme().to_string(); }

  void uri(string_type const& new_uri) const { uri_ = new_uri; }

  ::network::uri const& uri() const { return uri_; }

  void source_port(const std::uint16_t port) { source_port_ = port; }

  std::uint16_t source_port() const { return source_port_; }
};

/** This is the implementation of a POD request type
 *  that is specificially used by the HTTP server
 *  implementation. This fully specializes the
 *  basic_request template above to be
 *  primarily and be solely a POD for performance
 *  reasons.
 *
 *  Reality check: This is not a POD because it contains a non-POD
 *  member, the headers vector. :(
 */
template <class Tag>
struct not_quite_pod_request_base {
  typedef Tag tag;
  typedef std::string string_type;
  typedef request_header header_type;
  typedef std::vector<header_type> vector_type;
  typedef vector_type headers_container_type;
  typedef std::uint16_t port_type;
  mutable string_type source;
  mutable port_type source_port;
  mutable string_type method;
  mutable string_type destination;
  mutable std::uint8_t http_version_major;
  mutable std::uint8_t http_version_minor;
  mutable vector_type headers;
  mutable string_type body;

  void swap(not_quite_pod_request_base& r) const {
    using std::swap;
    swap(method, r.method);
    swap(source, r.source);
    swap(source_port, r.source_port);
    swap(destination, r.destination);
    swap(http_version_major, r.http_version_major);
    swap(http_version_minor, r.http_version_minor);
    swap(headers, r.headers);
    swap(body, r.body);
  }
};

template <>
struct basic_request<tags::http_server>
    : not_quite_pod_request_base<tags::http_server> {};

template <class Tag>
inline void swap(basic_request<Tag>& lhs, basic_request<Tag>& rhs) {
  lhs.swap(rhs);
}

}  // namespace http

namespace http {
namespace impl {

template <>
struct request_headers_wrapper<tags::http_server> {
  basic_request<tags::http_server> const& request_;
  explicit request_headers_wrapper(
      basic_request<tags::http_server> const& request_)
      : request_(request_) {}
  typedef std::vector<http::request_header> headers_container_type;
  operator headers_container_type() { return request_.headers; }
};

template <>
struct body_wrapper<basic_request<tags::http_server> > {
  typedef std::string string_type;
  basic_request<tags::http_server> const& request_;
  explicit body_wrapper(basic_request<tags::http_server> const& request_)
      : request_(request_) {}
  operator string_type() { return request_.body; }
};

}  // namespace impl
}  // namespace http
}  // namespace network
}  // namespace boost

#endif  // BOOST_NETWORK_PROTOCOL_HTTP_REQUEST_IMPL_20070908_1_HPP__
