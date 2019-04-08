#ifndef __INCLUDE_PROGRESS_H__
#define __INCLUDE_PROGRESS_H__

#include <string>

class ProgressBar {
public:
  ~ProgressBar();
  ProgressBar(const std::wstring&, int, int, int, int, const std::wstring& additional = L"");
  void setTitle(const std::wstring&);
  void setBase(int);
  void setPos(int);
  void setRange(int, int);
  void setAdditional(const std::wstring&);
  void print();

private:
  ProgressBar();
  std::string _title;
  int _min = 0;
  int _max = 100;
  int _base = 0;
  int _pos = 0;
  std::string _additional;
};

#endif
