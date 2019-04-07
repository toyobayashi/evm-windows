#ifndef __INCLUDE_PATH_HPP__
#define __INCLUDE_PATH_HPP__

#include <string>
#include <vector>
#include <stdio.h>
#include <io.h>
#include <shlwapi.h>
#include <Windows.h>
#include <ShlObj.h>
#include "util.h"

class Path {
public:
  static std::wstring dirname(const std::wstring& p) {
    const wchar_t* buf = p.c_str();
    wchar_t tmp[MAX_PATH] = { 0 };
    StrCpyW(tmp, buf);
    PathRemoveBackslashW(tmp);
    PathRemoveFileSpecW(tmp);
    std::wstring res = tmp;
    return res;
  }

  static std::wstring join(std::wstring arg1, std::wstring arg2 = L"") {
    int pos = -1;
    while (((pos = arg1.find(L"\\\\")) != std::wstring::npos)) {
      arg1.replace(pos, 2, L"\\");
    }

    while (((pos = arg2.find(L"\\\\")) != std::wstring::npos)) {
      arg2.replace(pos, 2, L"\\");
    }

    const wchar_t* buf1 = arg1.c_str();
    const wchar_t* buf2 = arg2.c_str();

    wchar_t tmp1[MAX_PATH] = { 0 };
    wchar_t tmp2[MAX_PATH] = { 0 };

    StrCpyW(tmp1, buf1);
    StrCpyW(tmp2, buf2);

    if (arg2 != L"") {
      PathRemoveBackslashW(tmp1);
    }

    wchar_t res[MAX_PATH];
    PathCombineW(res, tmp1, tmp2);

    return res;
  }

  template <typename... Args>
  static std::wstring join(std::wstring arg1, std::wstring arg2, Args... args) {
    return Path::join(Path::join(arg1, arg2), args...);
  }

  static bool unlink(const std::wstring& link) {
    if (!exists(link)) {
      return true;
    }
    return _wunlink(link.c_str()) == 0;
  }

  static bool rmdir(const std::wstring& dir) {
    if (!exists(dir)) {
      return true;
    }
    return _wrmdir(dir.c_str()) == 0;
  }

  static bool remove(const std::wstring& path) {
    if (!exists(path)) {
      return true;
    }

    struct _stat info;
    int res = _wstat(path.c_str(), &info);

    if (info.st_mode & S_IFDIR) {
      std::vector<std::wstring> items = readdir(path);
      if (items.size() != 0) {
        bool res = true;
        for (unsigned int i = 0; i < items.size(); i++) {
          const std::wstring& item = items[i];
          if (!remove(Path::join(path, item))) {
            res = false;
          }
        }
        bool lastResult = rmdir(path);
        return res ? lastResult : res;
      }
      else {
        return rmdir(path);
      }
    }

    return unlink(path);
  }

  static void mklinkp(const std::wstring& link, const std::wstring& target) {
    if (!PathFileExistsW(target.c_str())) return;
    if (PathIsDirectoryW(target.c_str())) {
      mkdirp(dirname(link));
      std::wstring cmd = L"mklink /D ";
      cmd += link;
      cmd += L" ";
      cmd += target;
      cmd += L">nul";
      system(Util::w2a(cmd).c_str());
      return;
    }
    mkdirp(dirname(link));
    std::wstring cmd = L"mklink ";
    cmd += link;
    cmd += L" ";
    cmd += target;
    cmd += L">nul";
    system(Util::w2a(cmd).c_str());
  }

  static bool mkdirp(const std::wstring& p) {
    if (PathIsRelativeW(p.c_str())) {
      return false;
    }

    if (PathFileExistsW(p.c_str())) {
      if (PathIsDirectoryW(p.c_str())) {
        return true;
      }
      return false;
    }
    if (PathIsRootW(p.c_str())) {
      return false;
    }
    if (!mkdirp(dirname(p))) {
      return false;
    }
    return CreateDirectoryW(p.c_str(), nullptr);
  }

  static bool rename(const std::wstring& source, const std::wstring& dest) {
    if (!exists(source)) {
      return false;
    }

    if (exists(dest)) {
      struct _stat info;
      _wstat(dest.c_str(), &info);
      if (info.st_mode & S_IFDIR) {
        return false;
      }
      if (!rename(dest, dest + L".tmp")) {
        return false;
      }
    }

    int res = _wrename(source.c_str(), dest.c_str());
    if (res == 0) {
      remove(dest + L".tmp");
      return true;
    }
    else {
      rename(dest + L".tmp", dest);
      return false;
    }
  }

  static std::string readFile(const std::wstring& path) {
    if (!exists(path)) {
      return "";
    }

    struct _stat info;
    _wstat(path.c_str(), &info);

    if (info.st_mode & S_IFDIR) {
      return "";
    }

    long size = info.st_size;

    FILE* fp = nullptr;
    _wfopen_s(&fp, path.c_str(), L"rb");
    if (fp == nullptr) {
      return "";
    }

    unsigned char* buf = new unsigned char[size + 1];
    memset(buf, 0, size + 1);
    fread(buf, sizeof(unsigned char), size, fp);
    fclose(fp);
    std::string res((const char*)buf);
    delete[] buf;
    return res;
  }

  static std::vector<std::wstring> readdir(const std::wstring& path) {
    struct _wfinddata_t file;
    intptr_t hFile;

    hFile = _wfindfirst((path + L"\\*.*").c_str(), &file);
    if (hFile == -1) {
      return {};
    }

    std::vector<std::wstring> res;

    std::wstring item = file.name;
    if (item != L"." && item != L"..") {
      res.push_back(item);
    }

    while (_wfindnext(hFile, &file) == 0) {
      std::wstring item = file.name;
      if (item != L"." && item != L"..") {
        res.push_back(item);
      }
    }
    _findclose(hFile);

    return res;
  }

  static bool exists(const std::wstring& p) {
    return PathFileExistsW(p.c_str());
  }

  static bool isDirectory(const std::wstring& p) {
    return PathIsDirectoryW(p.c_str());
  }

  static std::wstring __filename() {
    wchar_t __filename[260] = { 0 };
    GetModuleFileNameW(nullptr, __filename, MAX_PATH);
    return __filename;
  }

  static std::wstring __dirname() {
    return dirname(__filename());
  }

  static std::wstring getPath(int csidl) {
    wchar_t tmp[MAX_PATH] = { 0 };
    SHGetSpecialFolderPathW(NULL, tmp, csidl, false);
    return tmp;
  }

  static std::wstring basename(const std::wstring& p) {
    wchar_t tmp[MAX_PATH] = { 0 };
    StrCpyW(tmp, p.c_str());
    PathStripPathW(tmp);
    return tmp;
  }

  static std::wstring basename(const std::wstring& p, const std::wstring& ext) {
    wchar_t tmp[MAX_PATH] = { 0 };
    StrCpyW(tmp, p.c_str());
    PathStripPathW(tmp);
    PathRenameExtensionW(tmp, ext.c_str());
    return tmp;
  }

  static std::wstring extname(const std::wstring& p) {
    return PathFindExtensionW(p.c_str());
  }

private:
  Path();
};

#endif // !__INCLUDE_PATH_HPP__
