#include <windows.h>
#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds


extern BOOL MyCreatePipeEx(
    LPHANDLE lpReadPipe,
    LPHANDLE lpWritePipe,
    LPSECURITY_ATTRIBUTES lpPipeAttributes);

int main() {
  SECURITY_ATTRIBUTES secAttr;
  ZeroMemory(&secAttr, sizeof(secAttr));
  secAttr.nLength = sizeof(secAttr);
  secAttr.bInheritHandle = true;
  HANDLE hChildStdOutRd, hChildStdOutWr;
  HANDLE hChildStdErrRd, hChildStdErrWr;

  if (!MyCreatePipeEx(
          &hChildStdOutRd, &hChildStdOutWr, &secAttr)) {
    std::cerr << "CreatePipe failed!" << std::endl;
  }
  if (!SetHandleInformation(hChildStdOutRd, HANDLE_FLAG_INHERIT, 0)) {
    std::cerr << "SetHandleInformation failed!" << std::endl;
  }

  if (!MyCreatePipeEx(&hChildStdErrRd, &hChildStdErrWr, &secAttr)) {
    std::cerr << "CreatePipe failed!" << std::endl;
  }
  if (!SetHandleInformation(hChildStdErrRd, HANDLE_FLAG_INHERIT, 0)) {
    std::cerr << "SetHandleInformation failed!" << std::endl;
  }

  STARTUPINFOA startupInfo;
  ZeroMemory(&startupInfo, sizeof(startupInfo));
  startupInfo.cb = sizeof(startupInfo);
  startupInfo.dwFlags = STARTF_USESTDHANDLES;
  startupInfo.hStdError = hChildStdErrWr;
  //startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  startupInfo.hStdOutput = hChildStdOutWr;
  //startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

  PROCESS_INFORMATION processInformation;
  if (!CreateProcessA(
          ".\\tst.exe",  // lpApplicationName,
          nullptr,       // lpCommandLine,
          nullptr,       // lpProcessAttributes,
          nullptr,       // lpThreadAttributes,
          true,          // bInheritHandles,
          0,             // dwCreationFlags,
          nullptr,       // lpEnvironment,
          nullptr,       // lpCurrentDirectory,
          &startupInfo,   // lpStartupInfo,
          &processInformation)) {  // lpProcessInformation
    std::cerr << "CreateProcessA failed!" << std::endl;
  }

  char oCh, eCh;
  DWORD oRead, eRead;
  OVERLAPPED oOverlapped;
  ZeroMemory(&oOverlapped, sizeof(oOverlapped));
  oOverlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  OVERLAPPED eOverlapped;
  ZeroMemory(&eOverlapped, sizeof(eOverlapped));
  eOverlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  if (!ReadFile(hChildStdOutRd, &oCh, 1, &oRead, &oOverlapped)) {
    if (GetLastError() != ERROR_IO_PENDING) {
      std::cerr << "error" << std::endl;
      std::cerr <<  GetLastError() << std::endl;
      exit(1);
    }
  }
  if (!ReadFile(hChildStdErrRd, &eCh, 1, &eRead, &eOverlapped)) {
    if (GetLastError() != ERROR_IO_PENDING) {
      std::cerr << "error" << std::endl;
      std::cerr <<  GetLastError() << std::endl;
      exit(1);
    }
  }

  std::string oStr, eStr;
  bool x = true;
  while (true) {
    HANDLE handles[] = {x ? oOverlapped.hEvent : eOverlapped.hEvent,
      x ? eOverlapped.hEvent : oOverlapped.hEvent};
    x = !x;
    DWORD r = WaitForMultipleObjects(2, handles, false, INFINITE);
    switch (r) {
      case WAIT_OBJECT_0:
        if (oCh == 0x0a) {
          std::cout << oStr << std::endl;
          oStr = "";
        } else {
          oStr += oCh;
        }
        //std::cout << oCh << std::flush;
        if (!ReadFile(hChildStdOutRd, &oCh, 1, &oRead, &oOverlapped)) {
          if (GetLastError() != ERROR_IO_PENDING) {
            std::cerr << "error" << std::endl;
            std::cerr <<  GetLastError() << std::endl;
            exit(1);
          }
        }
        break;
      case WAIT_OBJECT_0+1:
        if (eCh == 0x0a) {
          std::cout << "\033[1;31m" << eStr << "\033[0m\n";
          eStr = "";
        } else {
          eStr += eCh;
        }
        //std::cout << eCh << std::flush;
        if (!ReadFile(hChildStdErrRd, &eCh, 1, &eRead, &eOverlapped)) {
          if (GetLastError() != ERROR_IO_PENDING) {
            std::cerr << "error" << std::endl;
            std::cerr <<  GetLastError() << std::endl;
            exit(1);
          }
        }
        break;
    }
  }

  std::this_thread::sleep_for (std::chrono::seconds(2));
}
