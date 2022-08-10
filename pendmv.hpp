#pragma once

#ifndef UNICODE
  #define UNICODE
#endif

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <cstdio>
#include <string>
#include <memory>
#include <vector>
#include <locale>

#pragma comment (lib, "advapi32.lib")

template<typename T, bool F(T)>
struct AutoDeleter {
  using pointer = T;

  void operator() (T t) const {
    printf("[%c] status: %lu\n", F(t) ? '*' : '!', ::GetLastError());
  }
};

bool fnLocalFree(const HLOCAL h) { return nullptr == ::LocalFree(h); }
bool fnRegCloseKey(const HKEY h) { return ERROR_SUCCESS == ::RegCloseKey(h); }

using _LocalFree = AutoDeleter<HLOCAL, fnLocalFree>;
using _RegCloseKey = AutoDeleter<HKEY, fnRegCloseKey>;

auto fmtmsg(const DWORD err, HLOCAL& h) {
  return ::FormatMessage(
    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
    nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    reinterpret_cast<LPWSTR>(&h), 0, nullptr
  );
}

auto ftm2stm(std::vector<BYTE>& v) {
  if (!::FileTimeToLocalFileTime(
    reinterpret_cast<PFILETIME>(&v[0]),
    reinterpret_cast<LPFILETIME>(&v[0])
  )) return ::GetLastError();
  if (!::FileTimeToSystemTime(
    reinterpret_cast<PFILETIME>(&v[0]),
    reinterpret_cast<LPSYSTEMTIME>(&v[0])
  )) return ::GetLastError();

  return 0ul;
}

template<typename... ArgTypes>
auto queryvalue(const HKEY& key, const PWSTR val, ArgTypes... args) {
  return ::RegQueryValueEx(key, val, nullptr, nullptr, args...);
}

auto queryinfo(const HKEY& key, std::vector<BYTE>& v) {
  return ::RegQueryInfoKey(
    key, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, reinterpret_cast<PFILETIME>(&v[0])
  );
}

auto getlasterror(const DWORD err) {
  HLOCAL loc{};
  DWORD size{};

  std::locale::global(std::locale(""));
  std::unique_ptr<HLOCAL, _LocalFree> x(((size = fmtmsg(err, loc)), loc));
  std::wstring msg(reinterpret_cast<LPWSTR>(loc));
  return msg.substr(0, size - sizeof(WCHAR));
}

auto err(LSTATUS ls) {
  if (ERROR_SUCCESS != ls) throw (getlasterror(ls));
}
