#include "progress.h"
#include "util.h"
#include <cstdio>
#include <cmath>

ProgressBar::~ProgressBar() {
  printf("\n");
}

ProgressBar::ProgressBar(const std::wstring& title, int min, int max, int base, int pos, const std::wstring& additional) {
  _title = Util::w2a(title);
  _min = min;
  _max = max;
  _base = base;
  _pos = pos;
  _additional = Util::w2a(additional);
  // printf("\n");
  print();
}

void ProgressBar::setTitle(const std::wstring& title) {
  _title = Util::w2a(title);
}

void ProgressBar::setBase(int base) {
  _base = base;
}

void ProgressBar::setPos(int pos) {
  _pos = pos;
}

void ProgressBar::setRange(int min, int max) {
  _min = min;
  _max = max;
}

void ProgressBar::setAdditional(const std::wstring& additional) {
  _additional = Util::w2a(additional);
}

void ProgressBar::print() {
  Util::clearLine(0);
  printf("%s ", _title.c_str());

  int progressLength = Util::getTerminalWidth() - _title.size() - _additional.size() - 14;
  double p_local = round((double)_base / (double)_max * progressLength);
  double p_current = round((double)_pos / (double)_max * progressLength);
  double percent = (double)(_base + _pos) / (double)_max * 100;
  //printf("\r");
  //printf("%.2lf / %.2lf MB ", (_base + _pos) / 1024 / 1024, _max / 1024 / 1024);
  printf("[");
  for (int i = 0; i < (int)p_local; i++) {
    printf("+");
  }
  for (int i = 0; i < (int)p_current/* - 1*/; i++) {
    printf("=");
  }
  printf(">");
  for (int i = 0; i < (int)(progressLength - p_local - p_current); i++) {
    printf(" ");
  }
  printf("] ");
  printf("%5.2lf%% ", percent);
  printf("%s", _additional.c_str());
  int marginRight = Util::getTerminalCursorPositionToRight();
  for (int i = 0; i < marginRight - 1; i++) {
    printf(" ");
  }
  for (int i = 0; i < marginRight - 1; i++) {
    printf("\b");
  }
}
