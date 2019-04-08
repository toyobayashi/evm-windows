#include <sstream>
#include <math.h>
#include <time.h>
#include "download.h"
#include <curl/curl.h>
#include <iostream>
#include "path.hpp"

//static size_t onDataString(void* buffer, size_t size, size_t nmemb, progressInfo * userp) {
//  const char* d = (const char*)buffer;
//  // userp->headerString.append(d, size * nmemb);
//  std::string tmp(d);
//  unsigned int contentlengthIndex = tmp.find(std::string("Content-Length: "));
//  if (contentlengthIndex != std::string::npos) {
//    userp->total = atoi(tmp.substr(16, tmp.find_first_of('\r')).c_str()) + userp->size;
//  }
//  return size * nmemb;
//}

static size_t onDataWrite(void* buffer, size_t size, size_t nmemb, progressInfo * userp) {
  if (userp->code == -1) {
    curl_easy_getinfo(userp->curl, CURLINFO_RESPONSE_CODE, &(userp->code));
  }
  if (userp->code >= 400) {
    return size * nmemb;
  }

  if (userp->total == -1) {
    curl_off_t cl;
    curl_easy_getinfo(userp->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &cl);
    if (cl != -1) {
      userp->total = (long)cl + userp->size;
    } else {
      return size * nmemb;
    }
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
  if (now - userp->last_time > 200) {
    userp->last_time = now;
    userp->speed = 0;
    if (userp->callback) {
      userp->callback(userp, userp->param);
    }
  } else if (userp->sum == userp->total - userp->size) {
    userp->end_time = clock();
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

  /*headers = curl_slist_append(headers, "Connection: Keep-Alive");
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

  // curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &onDataString);
  // curl_easy_setopt(curl, CURLOPT_HEADERDATA, &info);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &onDataWrite);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &info);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

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
