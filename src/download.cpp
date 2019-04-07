#include <sstream>
#include <math.h>
#include <time.h>
#include "download.h"
#include <curl/curl.h>
#include <iostream>
#include "path.hpp"

//using namespace std;
//
//// #define PROGRESS_LENGTH 38
//#define PROGRESS 50
//
//#ifdef _WIN32
//static HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
//#endif
//
//static int getTerminalWidth() {
//#ifdef _WIN32
//  CONSOLE_SCREEN_BUFFER_INFO bInfo;
//  GetConsoleScreenBufferInfo(hOutput, &bInfo);
//  return bInfo.dwSize.X;
//#else
//  return PROGRESS + 38;
//#endif
//}
//
//static int getTerminalCursorPositionToRight() {
//#ifdef _WIN32
//  CONSOLE_SCREEN_BUFFER_INFO bInfo;
//  GetConsoleScreenBufferInfo(hOutput, &bInfo);
//  // CloseHandle(hOutput);
//  return bInfo.dwSize.X - bInfo.dwCursorPosition.X;
//#else
//  return 5;
//#endif
//}
//
//void progress (double local, double current, double max, double speed) {
//  int progressLength = getTerminalWidth() - PROGRESS;
//  double p_local = round(local / max * progressLength);
//  double p_current = round(current / max * progressLength);
//  double percent = (local + current) / max * 100;
//  printf("\r");
//  printf("%.2lf / %.2lf MB ", (local + current) / 1024 / 1024, max / 1024 / 1024);
//  printf("[");
//  for (int i = 0; i < (int)p_local; i++) {
//    printf("+");
//  }
//  for (int i = 0; i < (int)p_current/* - 1*/; i++) {
//    printf("=");
//  }
//  printf(">");
//  for (int i = 0; i < (int)(progressLength - p_local - p_current); i++) {
//    printf(" ");
//  }
//  printf("] ");
//  printf("%5.2f%% ", percent);
//  printf("%7.2lf KB/s", speed / 1024);
//  int marginRight = getTerminalCursorPositionToRight();
//  printf((String(" ").repeat(marginRight) + String("\b").repeat(marginRight)).toCString());
//}
//
//void progress (double current, double max) {
//  int progressLength = getTerminalWidth() - PROGRESS;
//  double p_current = floor(current / max * progressLength);
//  double percent = current / max * 100;
//  printf("Completed: %.0lf / %.0lf ", current, max);
//  printf("[");
//  for (int i = 0; i < (int)p_current - 1; i++) {
//    printf("=");
//  }
//  printf(">");
//  for (int i = 0; i < (int)(progressLength - p_current); i++) {
//    printf(" ");
//  }
//  printf("] ");
//  printf("%5.2f%% ", percent);
//  int marginRight = getTerminalCursorPositionToRight();
//  printf((String(" ").repeat(marginRight) + String("\b").repeat(marginRight)).toCString());
//}
//
//const int BuffSize = 1024;

static size_t onDataString(void* buffer, size_t size, size_t nmemb, progressInfo * userp) {
  const char* d = (const char*)buffer;
  // userp->headerString.append(d, size * nmemb);
  std::string tmp(d);
  unsigned int contentlengthIndex = tmp.find(std::string("Content-Length: "));
  if (contentlengthIndex != std::string::npos) {
    userp->total = atoi(tmp.substr(16, tmp.find_first_of('\r')).c_str()) + userp->size;
  }
  return size * nmemb;
}

static size_t onDataWrite(void* buffer, size_t size, size_t nmemb, progressInfo * userp) {
  if (userp->code == -1) {
    curl_easy_getinfo(userp->curl, CURLINFO_RESPONSE_CODE, &(userp->code));
  }
  if (userp->code >= 400) {
    return size * nmemb;
  }

  if (userp->fp == nullptr) {
    Path::mkdirp(Path::dirname(userp->path));
    _wfopen_s(&(userp->fp), (userp->path + L".tmp").c_str(), L"ab+");
    if (!(userp->fp)) {
      return size * nmemb;
    }
  }

  size_t iRec = fwrite(buffer, size, nmemb, userp->fp);
  if (iRec < 0) {
    return iRec;
  }
  userp->sum += iRec;
  userp->speed += iRec;
  int now = clock();
  if (now - userp->last_time > 100) {
    // progress(userp->size, userp->sum, userp->total, userp->speed * 2);
    userp->last_time = now;
    userp->speed = 0;
    if (userp->callback) {
      userp->callback(userp, userp->param);
    }
  }

  if (userp->sum == userp->total - userp->size) {
    userp->end_time = clock();
    // progress(userp->size, userp->sum, userp->total, (double)userp->sum / ((userp->end_time - userp->start_time) / 1000));
    if (userp->callback) {
      userp->callback(userp, userp->param);
    }
  }
  
  return iRec;
}

bool download (std::wstring url, std::wstring path, downloadCallback callback, void* param) {
  if (Path::exists(path)) {
    if (Path::isDirectory(path)) {
      return false;
    }
    return true;
  }

  CURL* curl = curl_easy_init();
  struct curl_slist* headers = nullptr;

  /*headers = curl_slist_append(headers, "X-Unity-Version: 2017.4.2f2");
  headers = curl_slist_append(headers, "Connection: Keep-Alive");
  headers = curl_slist_append(headers, "Accept-Encoding: gzip");*/
  headers = curl_slist_append(headers, "Accept: */*");
  headers = curl_slist_append(headers, "User-Agent: Electron Version Manager");

  struct _stat stat;
  int res = _wstat((path + L".tmp").c_str(), &stat);
  long size = stat.st_size;

  if (size != 0) {
    headers = curl_slist_append(headers, (std::string("Range: bytes=") + std::to_string(size) + "-").c_str());
  }

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  curl_easy_setopt(curl, CURLOPT_URL, Util::w2a(url, CP_UTF8).c_str());

  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

  progressInfo info;
  info.path = path;
  info.curl = curl;
  info.fp = nullptr;
  info.size = size;
  info.sum = 0;
  info.speed = 0;
  info.start_time = clock();
  info.end_time = 0;
  info.last_time = 0;
  info.total = -1;
  info.param = param;
  info.code = -1;
  info.callback = callback;

  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &onDataString);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &info);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &onDataWrite);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &info);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  /*console::log(String("GET ") + url + " HTTP/1.1\n");
  struct curl_slist* ph = headers;
  while (ph != NULL) {
    console::log(ph->data);
    ph = ph->next;
  }
  console::log("");*/

  CURLcode code = curl_easy_perform(curl);

  if (code != CURLE_OK) {
    printf("%s\n", curl_easy_strerror(code));
    if (info.fp != nullptr) {
      fclose(info.fp);
      info.fp = nullptr;
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return false;
  }

  if (info.fp != nullptr) {
    fclose(info.fp);
    info.fp = nullptr;
    if (Path::exists(path + L".tmp")) {
      Path::rename(path + L".tmp", path);
    }
  }
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return true;
}
