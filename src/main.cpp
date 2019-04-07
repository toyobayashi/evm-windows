#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include "util.h"
#include "path.hpp"
#include "cli.h"
#include "download.h"
#include "config.h"

#define EVM_VERSION "1.0.0"

static void onUnzip(Util::unzCallbackInfo* info, void* param) {
  /*int* line = NULL;
  std::string displayName = "";

  line = (int*)param;
  Util::clearLine(*line);
  displayName = Util::w2a(info->name, CP_ACP);
  printf("Name: %s\n", displayName.c_str());
  printf("Current: %d / %d, %.2lf%%\n", info->currentUncompressed, info->currentTotal, 100 * (double)info->currentUncompressed / (double)info->currentTotal);
  printf("Total: %d / %d, %.2lf%%\n", info->uncompressed, info->total, 100 * (double)info->uncompressed / (double)info->total);
  if (*line == 0) {
    *line = 3;
  }*/

  Util::clearLine(0);
  printf("Extracting: %.2lf%%", 100 * (double)info->uncompressed / (double)info->total);
}

static void onDownload(progressInfo* info, void* param) {
  Util::clearLine(0);
  printf("Downloading %s: %.2lf%%...", Util::w2a(*((std::wstring*)param)).c_str(), 100 * (double)(info->size + info->sum) / info->total);
}

static bool install(const std::wstring& version, const Config& config) {
  if (Path::exists(Path::join(config.root, version))) {
    printf("Version %s is already installed.\n", Util::w2a(version).c_str());
    return true;
  }

  std::wstring electronName = std::wstring(L"electron-v") + version + L"-win32-" + config.arch + L".zip";
  std::wstring shatxtName = std::wstring(L"SHASUMS256.txt-") + version;
  std::wstring zipPath = Path::join(config.cache, electronName);
  std::wstring shatxt = Path::join(config.cache, shatxtName);

  bool res = true;

  if (!Path::exists(zipPath)) {
    if (config.mirror == L"github") {
      res = download(
        std::wstring(L"https://github.com/electron/electron/releases/download/v") + version + L"/electron-v" + version + L"-win32-" + config.arch + L".zip",
        zipPath,
        onDownload,
        &electronName
      );
    }
    else if (config.mirror == L"taobao") {
      res = download(
        std::wstring(L"https://npm.taobao.org/mirrors/electron/") + version + L"/electron-v" + version + L"-win32-" + config.arch + L".zip",
        zipPath,
        onDownload,
        &electronName
      );
    }
    else {
      res = download(
        config.mirror + version + L"/electron-v" + version + L"-win32-" + config.arch + L".zip",
        zipPath,
        onDownload,
        &electronName
      );
    }
    printf("\n");
  }

  if (!Path::exists(shatxt)) {
    if (config.mirror == L"github") {
      res = download(
        std::wstring(L"https://github.com/electron/electron/releases/download/v") + version + L"/SHASUMS256.txt",
        shatxt,
        onDownload,
        &shatxtName
      );
    }
    else if (config.mirror == L"taobao") {
      res = download(
        std::wstring(L"https://npm.taobao.org/mirrors/electron/") + version + L"/SHASUMS256.txt",
        shatxt,
        onDownload,
        &shatxtName
      );
    }
    else {
      res = download(
        config.mirror + version + L"/SHASUMS256.txt",
        shatxt,
        onDownload,
        &shatxtName
      );
    }
    printf("\n");
  }

  if (!res) return false;

  res = Util::unzip(zipPath, Path::join(config.root, version), onUnzip);
  printf("\n");
  return res;
}

static void list(const Config& config) {
  auto li = Path::readdir(config.root);

  printf("\n");
  for (unsigned int i = 0; i < li.size(); i++) {
    if (li[i].find_first_of(L".") != li[i].find_last_of(L".") && Path::isDirectory(Path::join(config.root, li[i]))) {
      std::string ver = Path::readFile(Path::join(config.path, L"version"));
      if (ver != "") {
        ver.erase(0, ver.find_first_not_of(" "));
        ver.erase(ver.find_last_not_of(" ") + 1);
      }
      std::wstring wver = Util::a2w(ver);
      if (wver == li[i] || wver == std::wstring(L"v") + li[i]) {
        printf("  * %s (Currently using %s executable)\n", Util::w2a(li[i]).c_str(), Util::isX64(Path::join(config.root, li[i], L"electron.exe")) ? Util::w2a(L"x64").c_str() : Util::w2a(L"ia32").c_str());
      }
      else {
        printf("    %s\n", Util::w2a(li[i]).c_str());
      }
    }
  }
}

static bool uninstall(const std::wstring& version, const Config& config) {
  std::wstring p = Path::join(config.root, version);
  return Path::remove(p);
}

static bool use(const std::wstring& version, const Config& config) {
  std::wstring p = Path::join(config.root, version);
  if (!Path::exists(p)) {
    if (!install(version, config)) {
      return false;
    }
  }
  else {
    std::wstring currentArch = Util::isX64(Path::join(p, L"electron.exe")) ? L"x64" : L"ia32";
    if (currentArch != config.arch) {
      if (!uninstall(version, config)) return false;
      if (!install(version, config)) {
        return false;
      }
    }
  }
  if (Path::exists(config.path)) {
    if (!Path::rmdir(config.path)) {
      return false;
    }
  }
  Path::mklinkp(config.path, p);
  printf("Now using Electron v%s (%s)\n", Util::w2a(version).c_str(), Util::w2a(config.arch).c_str());
  return true;
}

static void printHelp() {
  printf("\n");
  printf("Electron Version Manager %s\n\n", EVM_VERSION);

  printf("Usage:\n\n");
  printf("  evm version: \t\t\t\tDisplays the current running version of evm for Windows. Aliased as v.\n");
  printf("  evm arch [ia32 | x64]: \t\tShow or set electron arch.\n");
  printf("  evm mirror [github | taobao | <url>]: Show or set mirror.\n");
  printf("  evm path [<dir>]: \t\t\tShow or set electron directory.\n");
  printf("  evm cache [<dir>]: \t\t\tShow or set electron download cache directory.\n");
  printf("  evm root [<dir>]: \t\t\tShow or set the directory where evm should store different versions of electron.\n");
  printf("  evm install <version> [options]\n");
  printf("  evm list: \t\t\t\tList the electron installations.\n");
  printf("  evm use <version> [options]: \t\tSwitch to use the specified version.\n");
  printf("  evm uninstall <version>: \t\tThe version must be a specific version.\n");

  printf("\n");

  printf("Options:\n\n");
  printf("  --arch=<ia32 | x64>\n");
  printf("  --mirror=<github | taobao | <url>>\n");
  printf("  --path=<dir>\n");
  printf("  --cache=<dir>\n");
  printf("  --root=<dir>\n");
}

int wmain(int argc, wchar_t** argv) {
  Cli cli(argc, argv);
  Config config(&cli);

  std::wstring command = cli.getCommand();
  if (command == L"version" || command == L"v") {
    printf("%s\n", EVM_VERSION);
    return 0;
  }

  if (command == L"arch") {
    if (cli.getArgument().size() == 0) {
      printf("%s\n", Util::w2a(config.arch).c_str());
      return 0;
    }
    
    config.set(L"arch", cli.getArgument()[0]);
    return 0;
  }

  if (command == L"mirror") {
    if (cli.getArgument().size() == 0) {
      printf("%s\n", Util::w2a(config.mirror).c_str());
      return 0;
    }

    config.set(L"mirror", cli.getArgument()[0]);
    return 0;
  }

  if (command == L"path") {
    if (cli.getArgument().size() == 0) {
      printf("%s\n", Util::w2a(config.path).c_str());
      return 0;
    }

    config.set(L"path", cli.getArgument()[0]);
    return 0;
  }

  if (command == L"cache") {
    if (cli.getArgument().size() == 0) {
      printf("%s\n", Util::w2a(config.cache).c_str());
      return 0;
    }

    config.set(L"cache", cli.getArgument()[0]);
    return 0;
  }

  if (command == L"root") {
    if (cli.getArgument().size() == 0) {
      printf("%s\n", Util::w2a(config.root).c_str());
      return 0;
    }

    config.set(L"root", cli.getArgument()[0]);
    return 0;
  }

  if (command == L"install") {
    if (cli.getArgument().size() == 0) {
      printf("Example: \n\n  evm.exe install 4.1.4\n");
      return 0;
    }

    install(cli.getArgument()[0], config);
    return 0;
  }

  if (command == L"list") {
    list(config);
    return 0;
  }

  if (command == L"use") {
    if (cli.getArgument().size() == 0) {
      printf("Example: \n\n  evm.exe use 4.1.4\n");
      return 0;
    }

    if (!use(cli.getArgument()[0], config)) {
      printf("Use failed.\n");
    }
    return 0;
  }

  if (command == L"uninstall") {
    if (cli.getArgument().size() == 0) {
      printf("Example: \n\n  evm.exe uninstall 4.1.4\n");
      return 0;
    }

    if (!uninstall(cli.getArgument()[0], config)) {
      printf("Uninstall failed.\n");
    }
    return 0;
  }

  printHelp();
  return 0;
}
