#include <Windows.h>
#include "util.h"

std::string Util::w2a(const std::wstring& unicode, unsigned int codePage) {
  const wchar_t* unicodeptr = unicode.c_str();
  int len = WideCharToMultiByte(codePage, 0, unicodeptr, -1, NULL, 0, NULL, NULL);
  char* tmp = new char[len] { 0 };
  WideCharToMultiByte(codePage, 0, unicodeptr, -1, tmp, len, NULL, NULL);
  std::string res = tmp;
  delete[] tmp;
  return res;
}

std::wstring Util::a2w(const std::string& multiByte, unsigned int codePage) {
  const char* multiBytePtr = multiByte.c_str();
  int len = MultiByteToWideChar(codePage, 0, multiBytePtr, -1, NULL, 0);
  wchar_t* tmp = new wchar_t[len] { 0 };
  MultiByteToWideChar(codePage, 0, multiBytePtr, -1, tmp, len);
  std::wstring res = tmp;
  delete[] tmp;
  return res;
}
