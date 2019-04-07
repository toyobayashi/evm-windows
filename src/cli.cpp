#include "cli.h"
#include <stdio.h>
#include "util.h"

Cli::Cli(int argc, wchar_t** argv) {
  if (argc < 2) {
    return;
  }

  _command = argv[1];

  if (argc < 3) {
    return;
  }

  std::wstring arg = L"";

  for (int i = 2; i < argc; i++) {
    arg = argv[i];
    if (arg.find(L"--") == 0) {
      arg = arg.substr(2);
      auto index = arg.find(L"=");
      if (index == std::wstring::npos) {
        _option[arg] = L"true";
      } else {
        auto key = arg.substr(0, index);
        auto value = arg.substr(index + 1);
        _option[key] = value;
      }
    } else {
      _.push_back(arg);
    }
  }
}

bool Cli::has(const std::wstring& optionName) const {
  auto it = _option.find(optionName);
  if (it == _option.end()) {
    return false;
  }
  return true;
}

const std::wstring& Cli::getCommand() const {
  return _command;
}

std::wstring Cli::getOption(const std::wstring& optionName) {
  if (has(optionName)) {
    return _option[optionName];
  }
  return L"";
}

const std::vector<std::wstring>& Cli::getArgument() const {
  return _;
}

void Cli::showDetail() const {
  printf("Command: %s\n", Util::w2a(_command).c_str());

  for (auto iter = _option.begin(); iter != _option.end(); iter++) {
    printf("Option: %s = %s\n", Util::w2a(iter->first).c_str(), Util::w2a(iter->second).c_str());
  }
  
  for (unsigned int i = 0; i < _.size(); i++) {
    printf("Arguments: %s\n", Util::w2a(_[i]).c_str());
  }
}
