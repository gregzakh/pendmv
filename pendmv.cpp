#include <memory>
#include <vector>
#include <string>
#include <locale>
#include "pendmv.hpp"

auto getlasterror(const DWORD err) {
  std::locale::global(std::locale(""));
  HLOCAL loc{};
  DWORD size{};

  std::unique_ptr<HLOCAL, AutoDeleter<HLOCAL, AuLocalFree>> x(
    ((size = fmtmsg(err, loc)), loc)
  );
  printf("[!] %.*ws\n", size - 1, size ?
    reinterpret_cast<LPWSTR>(x.get()) : L"Unknown error has been occured."
  );
}

int main(void) {
  FILETIME   ft{};
  SYSTEMTIME st{};
  HKEY       rk{};
  DWORD     len{};
  DWORD     err{};
  LSTATUS ls = ::RegOpenKeyEx(
    HKEY_LOCAL_MACHINE,
    L"SYSTEM\\CurrentControlSet\\Control\\Session Manager",
    0, KEY_QUERY_VALUE, &rk
  );
  auto readvalue = [rk, &len, &ls](const PWSTR value) -> LSTATUS {
    ls = ::RegQueryValueEx(
      rk, value, nullptr, nullptr, nullptr, &len
    );
    if (ERROR_SUCCESS != ls) return ls;

    std::vector<BYTE> buf(len);
    ls = ::RegQueryValueEx(
      rk, value, nullptr, nullptr, &buf[0], &len
    );
    if (ERROR_SUCCESS != ls) return ls;

    PWSTR res = reinterpret_cast<PWSTR>(&buf[0]);
    while (1) {
      if (!wcslen(res)) break;
      printf("Source: %ws\n", &res[4]);
      res += wcslen(res) + 1;
      if (wcslen(res)) {
        printf("Target: %ws\n\n", &res[4]);
        res += wcslen(res) + 1;
      }
      else {
        printf("Target: DELETE\n\n");
        res += 1;
      }
    }

    return ERROR_SUCCESS;
  };
  std::unique_ptr<HKEY, AutoDeleter<HKEY, AuRegCloseKey>> key(rk);

  ls = readvalue(L"PendingFileRenameOperations"); // ??? PendingFileRenameOperations2
  if (ERROR_SUCCESS != ls) {
    getlasterror(ls);
    return ls;
  }

  ls = ::RegQueryInfoKey(
    key.get(), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, &ft
  );
  if (ERROR_SUCCESS != ls) {
    getlasterror(ls);
    return ls;
  }
  err = ftime2stime(ft, st);
  if (ERROR_SUCCESS != err) {
    getlasterror(err);
    return 1;
  }
  printf("Time of last update to pending moves key: %.2hu/%.2hu/%.4hu %.2hu:%.2hu:%.2hu\n",
    st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond
  );

  return 0;
}
