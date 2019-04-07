#include <zlib/unzip.h>
#include "util.h"
#include "path.hpp"

#define UNZ_BUFFER_SIZE 64 * 1024

bool Util::unzip(const std::wstring& zipFilePathW, const std::wstring& outDirW, Util::unzCallback callback, void* param) {
  unzFile unzfile = unzOpen(Util::w2a(zipFilePathW, CP_ACP).c_str());
  if (!unzfile) return false;

  unz_global_info pGlobalInfo;

  int res = unzGetGlobalInfo(unzfile, &pGlobalInfo);
  if (res != UNZ_OK) return false;

  unz_file_info pFileInfo;

  Util::unzCallbackInfo info;
  info.name = L"";
  info.currentUncompressed = 0;
  info.currentTotal = 0;
  info.uncompressed = 0;
  info.total = 0;
  info.entry = pGlobalInfo.number_entry;

  for (uLong i = 0; i < pGlobalInfo.number_entry; i++) {
    res = unzGetCurrentFileInfo(unzfile, &pFileInfo, NULL, 0, NULL, 0, NULL, 0);
    if (res != UNZ_OK) return false;

    info.total += pFileInfo.uncompressed_size;

    if ((i + 1) < pGlobalInfo.number_entry) {
      if (UNZ_OK != unzGoToNextFile(unzfile)) return false;
    }
  }

  Path::mkdirp(outDirW);
  if (UNZ_OK != unzGoToFirstFile(unzfile)) return false;

  unsigned char* readBuffer = new unsigned char[UNZ_BUFFER_SIZE];

  char szZipFName[MAX_PATH] = { 0 };
  std::wstring outfilePathW = L"";
  std::wstring tmp = L"";

  HANDLE hFile = NULL;
  char* pos = NULL;
  for (uLong i = 0; i < pGlobalInfo.number_entry; i++) {
    memset(szZipFName, 0, sizeof(szZipFName));
    res = unzGetCurrentFileInfo(unzfile, &pFileInfo, szZipFName, MAX_PATH, NULL, 0, NULL, 0);
    if (res != UNZ_OK) {
      delete[] readBuffer;
      return false;
    }

    pos = szZipFName;
    while (*pos != 0) {
      if (*pos == '/') {
        *pos = '\\';
      }
      pos++;
    }

    tmp = Util::a2w(szZipFName, CP_UTF8);
    outfilePathW = outDirW + L"\\" + tmp;

    info.name = tmp;

    switch (pFileInfo.external_fa) {
    case FILE_ATTRIBUTE_DIRECTORY:
      Path::mkdirp(outfilePathW);
      break;
    default:
      hFile = CreateFileW(outfilePathW.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
      if (hFile == INVALID_HANDLE_VALUE) {
        Path::mkdirp(Path::dirname(outfilePathW));
        hFile = CreateFileW(outfilePathW.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
          delete[] readBuffer;
          return false;
        }
      }

      res = unzOpenCurrentFile(unzfile);
      if (res != UNZ_OK) {
        CloseHandle(hFile);
        delete[] readBuffer;
        return false;
      }

      info.currentTotal = pFileInfo.uncompressed_size;
      info.currentUncompressed = 0;
      while (1) {
        memset(readBuffer, 0, UNZ_BUFFER_SIZE);
        int nReadFileSize = unzReadCurrentFile(unzfile, readBuffer, UNZ_BUFFER_SIZE);
        if (nReadFileSize < 0) {
          unzCloseCurrentFile(unzfile);
          CloseHandle(hFile);
          delete[] readBuffer;
          return false;
        }

        if (nReadFileSize == 0) {
          unzCloseCurrentFile(unzfile);
          CloseHandle(hFile);
          break;
        }
        else {
          DWORD dWrite = 0;
          BOOL bWriteSuccessed = WriteFile(hFile, readBuffer, nReadFileSize, &dWrite, NULL);
          info.uncompressed += dWrite;
          info.currentUncompressed += dWrite;
          if (callback) {
            callback(&info, param);
          }
          if (!bWriteSuccessed) {
            unzCloseCurrentFile(unzfile);
            CloseHandle(hFile);
            delete[] readBuffer;
            return false;
          }
        }
      }

      break;
    }
    if ((i + 1) < pGlobalInfo.number_entry) {
      if (UNZ_OK != unzGoToNextFile(unzfile)) {
        delete[] readBuffer;
        return false;
      }
    }
  }

  if (unzfile) {
    unzCloseCurrentFile(unzfile);
  }

  delete[] readBuffer;
  return true;
}
