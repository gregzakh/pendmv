#pragma once

#ifndef UNICODE
  #define UNICODE
#endif

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <cstdio>
#include <memory>
#include <vector>
#include <locale>

#pragma comment (lib, "advapi32.lib")

template<typename T, bool (*Cleanup)(T)>
struct AutoDeleter {
  using pointer = T;

  void operator ()(T t) const {
    if (!Cleanup(t))
      printf("[!] Cleanup (%lu) fatal error.\n", ::GetLastError());
#ifdef DEBUG
    else
      printf("[*] success.\n");
#endif
  }
};

bool AuLocalFree(const HLOCAL h) { return nullptr == ::LocalFree(h); }
bool AuRegCloseKey(const HKEY h) { return ERROR_SUCCESS == ::RegCloseKey(h); }

auto fmtmsg(const DWORD err, HLOCAL& h) {
  return ::FormatMessage(
    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
    nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    reinterpret_cast<LPWSTR>(&h), 0, nullptr
  );
}

auto ftime2stime(std::vector<BYTE>& buf) -> DWORD {
  if (!::FileTimeToLocalFileTime(
    reinterpret_cast<PFILETIME>(&buf[0]), reinterpret_cast<LPFILETIME>(&buf[0])
  )) return ::GetLastError();
  if (!::FileTimeToSystemTime(
    reinterpret_cast<PFILETIME>(&buf[0]), reinterpret_cast<LPSYSTEMTIME>(&buf[0])
  )) return ::GetLastError();
  return ERROR_SUCCESS;
}

template<typename HKEY, typename PWSTR, typename... ArgTypes>
auto queryvalue(const HKEY& key, const PWSTR val, ArgTypes... args) {
  return ::RegQueryValueEx(key, val, nullptr, nullptr, args...);
}

auto queryinfo(const HKEY& key, std::vector<BYTE>& buf) {
  return ::RegQueryInfoKey(
    key, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, reinterpret_cast<PFILETIME>(&buf[0])
  );
}

auto getlasterror(const DWORD err) {
  std::locale::global(std::locale(""));
  HLOCAL loc{};
  DWORD size{};

  std::unique_ptr<HLOCAL, AutoDeleter<HLOCAL, AuLocalFree>> x(
    ((size = fmtmsg(err, loc)), loc)
  );
  printf("[%c] %.*ws\n", ERROR_SUCCESS == err ? '*' : '!', size - 1, size ?
         reinterpret_cast<LPWSTR>(loc) : L"Unknowsn error has been occured.");
}

struct winerr {
  void set(LSTATUS ls) {
    status = ls;
  }

  bool get(void) {
    if (ERROR_SUCCESS != status) {
      getlasterror(status);
      return true;
    }
    return false;
  }

  private:
    LSTATUS status;
};
