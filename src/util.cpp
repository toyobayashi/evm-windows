#include <Windows.h>
#include <ShlObj.h>
#include "util.h"
#include "path.hpp"
#include <PicoSHA2/pichosha2.h>

HANDLE Util::_consoleHandle = nullptr;

void Util::clearLine(unsigned short lineNumber) {
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  COORD targetFirstCellPosition;
  short tmp = 0;
  DWORD size = 0;
  DWORD cCharsWritten = 0;

  if (!_consoleHandle) _consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  if (!GetConsoleScreenBufferInfo(_consoleHandle, &csbi)) return;
  tmp = csbi.dwCursorPosition.Y - lineNumber;
  targetFirstCellPosition.X = 0;
  targetFirstCellPosition.Y = tmp < 0 ? 0 : tmp;
  size = csbi.dwSize.X * lineNumber;

  if (!FillConsoleOutputCharacter(_consoleHandle, (TCHAR)' ', size, targetFirstCellPosition, &cCharsWritten)) return;
  if (!GetConsoleScreenBufferInfo(_consoleHandle, &csbi)) return;
  if (!FillConsoleOutputAttribute(_consoleHandle, csbi.wAttributes, size, targetFirstCellPosition, &cCharsWritten)) return;
  SetConsoleCursorPosition(_consoleHandle, targetFirstCellPosition);
}

int Util::getTerminalWidth() {
  CONSOLE_SCREEN_BUFFER_INFO bInfo;
  GetConsoleScreenBufferInfo(_consoleHandle, &bInfo);
  return bInfo.dwSize.X;
}

int Util::getTerminalCursorPositionToRight() {
  CONSOLE_SCREEN_BUFFER_INFO bInfo;
  GetConsoleScreenBufferInfo(_consoleHandle, &bInfo);
  return bInfo.dwSize.X - bInfo.dwCursorPosition.X;
}

bool Util::isX64(const std::wstring& p) {
  DWORD access_mode = (GENERIC_READ | GENERIC_WRITE);

  DWORD share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  HANDLE hFile =
    CreateFileW(p.c_str(),
      access_mode,
      share_mode,
      NULL,
      OPEN_ALWAYS,
      FILE_FLAG_SEQUENTIAL_SCAN,
      NULL);

  if (hFile == INVALID_HANDLE_VALUE) {
    return false;
  }
  DWORD high_size;
  DWORD file_size = GetFileSize(hFile, &high_size);

  DWORD mmf_size = 512 * 1024;
  DWORD size_high = 0;

  HANDLE  hFm = CreateFileMappingW(hFile,
    NULL,
    PAGE_READWRITE,
    size_high,
    mmf_size,
    NULL);

  if (hFm == NULL) {
    CloseHandle(hFile);
    return false;
  }

  size_t view_size = 1024 * 256;
  DWORD view_access = FILE_MAP_ALL_ACCESS;

  char* base_address = (char*)MapViewOfFile(hFm, view_access, 0, 0, view_size);
  if (base_address != NULL) {
    bool flag;
    IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)base_address;
    IMAGE_NT_HEADERS* pNt = (IMAGE_NT_HEADERS*)(pDos->e_lfanew + (char*)pDos);

    if (pNt->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64 || pNt->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
      flag = true;
    else
      flag = false;
    UnmapViewOfFile(base_address);
    CloseHandle(hFm);
    CloseHandle(hFile);
    return flag;
  }
  else {
    return false;
  }
}

bool Util::createShortcut(const std::wstring& lpszFileName, const std::wstring& lpszLnkFileDir, const std::wstring& lpszLnkFileName, const std::wstring& lpszWorkDir, unsigned short wHotkey, const std::wstring& lpszDescription, int iShowCmd) {
  HRESULT hr = 0;
  IShellLink* pLink = NULL;
  IPersistFile* ppf = NULL;

  if (lpszLnkFileDir == L"") return false;

  hr = CoInitialize(NULL);
  if (FAILED(hr)) return false;

  hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)& pLink);

  if (FAILED(hr)) {
    if (pLink) pLink->Release();
    return false;
  }

  if (lpszFileName == L"") {
    pLink->SetPath(Path::__filename().c_str());
  }
  else {
    pLink->SetPath(lpszFileName.c_str());
  }

  if (lpszWorkDir != L"")
    pLink->SetWorkingDirectory(lpszWorkDir.c_str());

  if (wHotkey != 0)
    pLink->SetHotkey(wHotkey);

  if (lpszDescription != L"")
    pLink->SetDescription(lpszDescription.c_str());

  pLink->SetShowCmd(iShowCmd);

  std::wstring wsz = L"";

  if (lpszLnkFileName == L"") {
    wsz = Path::join(lpszLnkFileDir, Path::basename(Path::__filename(), L".lnk"));
  } else {
    wsz = Path::join(lpszLnkFileDir, lpszLnkFileName);
  }

  hr = pLink->QueryInterface(IID_IPersistFile, (void**)& ppf);

  Path::mkdirp(Path::dirname(wsz));
  hr = ppf->Save(wsz.c_str(), TRUE);
  ppf->Release();
  pLink->Release();
  CoUninitialize();
  return true;
}

std::string Util::getArch() {
#ifdef _WIN64
  return "x64";
#else
  int res = 0;
  IsWow64Process(GetCurrentProcess(), &res);
  return res == 0 ? "ia32" : "x64";
#endif
}

std::string Util::sha256(const std::wstring& path) {
  if (!Path::exists(path)) return "";
  std::ifstream f(path, std::ios::binary);
  std::vector<unsigned char> s(picosha2::k_digest_size);
  picosha2::hash256(f, s.begin(), s.end());
  std::string hexHash = "";
  picosha2::bytes_to_hex_string(s.begin(), s.end(), hexHash);
  return hexHash;
}
