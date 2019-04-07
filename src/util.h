#ifndef __INCLUDE_UTIL_HPP__
#define __INCLUDE_UTIL_HPP__

#include <string>
#include <Windows.h>

class Util {
private:
  Util();
  static HANDLE _consoleHandle;

public:
  typedef struct unzCallbackInfo {
    std::wstring name;
    unsigned long currentUncompressed;
    unsigned long currentTotal;
    unsigned long uncompressed;
    unsigned long total;
    unsigned long entry;
  } unzCallbackInfo;

  typedef void (*unzCallback)(unzCallbackInfo*, void*);

  static std::string w2a(const std::wstring&, unsigned int codePage = CP_ACP);
  static std::wstring a2w(const std::string&, unsigned int codePage = CP_ACP);

  static std::string getArch();
  static bool unzip(const std::wstring& zipFilePathW, const std::wstring& outDirW, Util::unzCallback, void* param = nullptr);
  static void clearLine(unsigned short);
  static bool isX64(const std::wstring&);

  static bool createShortcut(
    const std::wstring& lpszFileName,
    const std::wstring& lpszLnkFileDir,
    const std::wstring& lpszLnkFileName,
    const std::wstring& lpszWorkDir = L"",
    unsigned short wHotkey = 0,
    const std::wstring& lpszDescription = L"",
    int iShowCmd = SW_SHOWNORMAL
  );
};

#endif
