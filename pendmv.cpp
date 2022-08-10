#include <cwchar>
#include "pendmv.hpp"

int main(void) {
  HKEY    rk{};
  auto sz{0ul};
  std::vector<BYTE> buf(0);

  try {
    std::unique_ptr<HKEY, _RegCloseKey> key(
      ((err(::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\Session Manager",
        0, KEY_QUERY_VALUE, &rk
      ))), rk)
    );

    err(queryvalue(rk, L"PendingFileRenameOperations", nullptr, &sz));
    buf.resize(sz);
    err(queryvalue(rk, L"PendingFileRenameOperations", &buf[0], &sz));

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

    err(queryinfo(rk, buf));
    err(ftm2stm(buf));
    auto tm = *reinterpret_cast<WORD(*)[8]>(&buf[0]);
    printf("Time of last update to pending move key: ");
    printf("%.2hu/%.2hu/%.4hu %.2hu:%.2hu:%2hu\n",
              tm[1], tm[3], tm[0], tm[4], tm[5], tm[6]);
  }
  catch (const std::wstring e) {
    printf("%ws\n", e.c_str());
  }

  std::vector<BYTE> ().swap(buf);

  return 0;
}
