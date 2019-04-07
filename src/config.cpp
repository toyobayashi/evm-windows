#include "config.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "path.hpp"
#include "util.h"
#include <map>

Config::Config(Cli* cli) {
  mirror = L"github";
  arch = Util::a2w(Util::getArch(), CP_UTF8);
  path = Path::join(Path::__dirname(), L"electron");
  cache = Path::join(Path::getPath(CSIDL_LOCAL_APPDATA), L"electron", L"Cache");
  root = Path::__dirname();

  std::wstring configfile = Path::join(Path::__dirname(), L"settings.txt");
  if (Path::exists(configfile)) {
    FILE* fp = nullptr;
    _wfopen_s(&fp, configfile.c_str(), L"r");

    if (fp) {
      char cline[300];
      while (fgets(cline, 300, fp) != nullptr) {
        std::string line(cline);
        line[line.size() - 1] = '\0';
        std::wstring wline = Util::a2w(line);

        auto split = wline.find(L"=");
        if (split != std::string::npos) {
          auto key = wline.substr(0, split);
          auto value = wline.substr(split + 1);
          if (key == L"mirror" && value != L"") {
            mirror = value;
          }
          else if (key == L"arch" && value != L"") {
            arch = value;
          }
          else if (key == L"path" && value != L"") {
            path = value;
          }
          else if (key == L"cache" && value != L"") {
            cache = value;
          }
          else if (key == L"root" && value != L"") {
            root = value;
          }
        }
      }
      fclose(fp);
    }
  }

  if (cli) {
    if (cli->has(L"mirror")) {
      mirror = cli->getOption(L"mirror");
    }

    if (cli->has(L"arch")) {
      arch = cli->getOption(L"arch");
    }

    if (cli->has(L"path")) {
      path = cli->getOption(L"path");
    }

    if (cli->has(L"cache")) {
      cache = cli->getOption(L"cache");
    }

    if (cli->has(L"root")) {
      root = cli->getOption(L"root");
    }
  }
}

void Config::set(const std::wstring& key, const std::wstring& value) {
  std::wstring configfile = Path::join(Path::__dirname(), L"settings.txt");
  FILE* fp = nullptr;
  if (!Path::exists(configfile)) {
    HANDLE hFile = CreateFileW(configfile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_FLAG_WRITE_THROUGH, NULL);
    if (hFile) CloseHandle(hFile);
  }
  _wfopen_s(&fp, configfile.c_str(), L"r");
  if (!fp) return;

  std::map<std::wstring, std::wstring> configmap;
  char cline[300];
  while (fgets(cline, 300, fp) != nullptr) {
    std::string line(cline);
    line[line.size() - 1] = '\0';
    std::wstring wline = Util::a2w(line);

    auto split = wline.find(L"=");
    if (split != std::string::npos) {
      auto _key = wline.substr(0, split);
      auto _value = wline.substr(split + 1);
      configmap[_key] = _value;
    }
  }
  fclose(fp);

  configmap[key] = value;

  _wfopen_s(&fp, configfile.c_str(), L"w");
  if (!fp) return;

  std::wstring newContent = L"";
  for (auto iter = configmap.begin(); iter != configmap.end(); iter++) {
    newContent += (iter->first) + L"=" + iter->second + L"\n";
  }

  std::string res = Util::w2a(newContent);

  fwrite(res.c_str(), 1, res.size(), fp);
  fclose(fp);
}
