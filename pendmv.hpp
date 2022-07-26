#pragma once

#ifndef UNICODE
  #define UNICODE
#endif

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <cstdio>

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

auto fmtmsg(const DWORD err, HLOCAL& h) -> DWORD {
  return ::FormatMessage(
    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
    nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    reinterpret_cast<LPWSTR>(&h), 0, nullptr
  );
}

auto ftime2stime(FILETIME& ft, SYSTEMTIME& st) -> DWORD {
  if (!::FileTimeToLocalFileTime(&ft, &ft)) return ::GetLastError();
  if (!::FileTimeToSystemTime(&ft, &st)) return ::GetLastError();
  return ERROR_SUCCESS;
}
