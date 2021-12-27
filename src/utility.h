// Utility functions to make it easier to interoperate with C++.

#pragma once

#include "string_view.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <string>

extern "C" {
#include <ngx_core.h>
}

namespace datadog {
namespace nginx {

inline std::string to_string(const ngx_str_t &ngx_str) {
  return {reinterpret_cast<char *>(ngx_str.data), ngx_str.len};
}

inline string_view to_string_view(const ngx_str_t& s) {
  return {reinterpret_cast<char *>(s.data), s.len};
}

inline string_view str(const ngx_str_t& s) {
  return to_string_view(s);
}

ngx_str_t to_ngx_str(ngx_pool_t *pool, const std::string &s);

ngx_str_t to_ngx_str(ngx_pool_t *pool, string_view s);

inline ngx_str_t to_ngx_str(string_view s) {
  ngx_str_t result;
  result.len = s.size();
  result.data = reinterpret_cast<unsigned char *>(const_cast<char *>(s.data()));
  return result;
}

// Convert the epoch denoted by epoch_seconds, epoch_milliseconds to an
// std::chrono::system_clock::time_point duration from the epoch.
std::chrono::system_clock::time_point to_system_timestamp(
    time_t epoch_seconds, ngx_msec_t epoch_milliseconds);

// Apply `f` to each element of an ngx_list_t.
template <class T, class F>
void for_each(const ngx_list_t &list, F f) {
  auto part = &list.part;
  auto elements = static_cast<T *>(part->elts);
  for (ngx_uint_t i = 0;; i++) {
    if (i >= part->nelts) {
      if (!part->next) return;
      part = part->next;
      elements = static_cast<T *>(part->elts);
      i = 0;
    }
    f(elements[i]);
  }
}

// Apply `f` to each element of an ngx_array_t.
template <class T, class F>
void for_each(const ngx_array_t &array, F f) {
  auto elements = static_cast<T *>(array.elts);
  auto n = array.nelts;
  for (size_t i = 0; i < n; ++i) f(elements[i]);
}

// Perform the transformations on header characters described by
// http://nginx.org/en/docs/http/ngx_http_core_module.html#var_http_
inline char header_transform_char(char c) {
  if (c == '-') return '_';
  return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

}  // namespace nginx
}  // namespace datadog
