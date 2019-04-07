#ifndef __INCLUDE_CLI_HPP__
#define __INCLUDE_CLI_HPP__

#include <map>
#include <string>
#include <vector>

class Cli {
private:
  Cli();
  std::wstring _command = L"";
  std::map<std::wstring, std::wstring> _option;
  std::vector<std::wstring> _;
public:
  Cli(int argc, wchar_t** argv);
  bool has(const std::wstring&) const;
  const std::wstring& getCommand() const;
  std::wstring getOption(const std::wstring&);
  const std::vector<std::wstring>& getArgument() const;
  void showDetail() const;
};

#endif
