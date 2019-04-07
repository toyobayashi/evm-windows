#ifndef __DOWNLOAD_H__
#define __DOWNLOAD_H__

#include <string>
#include <curl/curl.h>

typedef struct progressInfo progressInfo;

typedef void (*downloadCallback)(progressInfo*, void*);

typedef struct progressInfo {
  CURL* curl;
  FILE* fp;
  long size;
  long sum;
  long total;
  int speed;
  double start_time;
  double last_time;
  double end_time;
  std::wstring path;
  downloadCallback callback;
  void* param;
  long code;
} progressInfo;

bool download (std::wstring url, std::wstring path, downloadCallback = nullptr, void* param = nullptr);

#endif // !__DOWNLOAD_H__
