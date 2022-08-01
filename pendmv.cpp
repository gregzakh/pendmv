#include <cwchar>
#include "pendmv.hpp"

int main(void) {
  HKEY    rk{};
  auto  ls{0l};
  auto sz{0ul};

  std::unique_ptr<HKEY, AutoDeleter<HKEY, AuRegCloseKey>> key(
    ((ls = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
      L"SYSTEM\\CurrentControlSet\\Control\\Session Manager",
      0, KEY_QUERY_VALUE, &rk
    )), rk)
  );
  if (ERROR_SUCCESS != ls) {
    getlasterror(ls);
    return 1;
  }

  ls = queryvalue(rk, L"PendingFileRenameOperations", nullptr, &sz);
  if (ERROR_SUCCESS != ls) {
    getlasterror(ls);
    return 1;
  }
  std::vector<BYTE> buf(sz);
  ls = queryvalue(rk, L"PendingFileRenameOperations", &buf[0], &sz);
  if (ERROR_SUCCESS != ls) {
    getlasterror(ls);
    return 1;
  }

  auto res = reinterpret_cast<PWSTR>(&buf[0]);
  while (1) {
    if (!wcslen(res)) break;
    printf("Source: %ws\n", &res[4]);
    res += wcslen(res) + 1;
    printf("Target: %ws\n\n", wcslen(res) ? &res[4] : L"DELETE");
    res += wcslen(res) + 1;
  }

  buf.resize(sizeof(SYSTEMTIME));
  buf.shrink_to_fit();
  buf.clear();

  ls = queryinfo(rk, buf);
  if (ERROR_SUCCESS != ls) {
    getlasterror(ls);
    return 1;
  }

  ls = ftime2stime(buf);
  if (ERROR_SUCCESS != ls) {
    getlasterror(ls);
    return 1;
  }
  auto tm = *reinterpret_cast<WORD(*)[8]>(&buf[0]);
  printf(
    "Time of last update to pending move key: %.2hu/%.2hu/%.4hu %.2hu:%.2hu:%.2hu\n",
                                             tm[1], tm[3], tm[0], tm[4], tm[5], tm[6]
  );

  std::vector<BYTE> ().swap(buf);

  return 0;
}
