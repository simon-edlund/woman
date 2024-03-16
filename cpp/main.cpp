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

  if (!MyCreatePipeEx(
          &hChildStdOutRd, &hChildStdOutWr, &secAttr)) {
       std::cerr << "CreatePipe failed!" << std::endl;
   }
  if (!SetHandleInformation(hChildStdOutRd, HANDLE_FLAG_INHERIT, 0)) {
     std::cerr << "SetHandleInformation failed!" << std::endl;
   }

   if (!MyCreatePipeEx(&hChildStdOutRd, &hChildStdOutWr, &secAttr)) {
     std::cerr << "CreatePipe failed!" << std::endl;
   }
   if (!SetHandleInformation(hChildStdOutRd, HANDLE_FLAG_INHERIT, 0)) {
     std::cerr << "SetHandleInformation failed!" << std::endl;
   }

  HANDLE hChildStdErrRd, hChildStdErrWr;
  if (!CreatePipe(&hChildStdErrRd, &hChildStdErrWr, &secAttr, 0)) {
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

  while (true) {
    char ch;
    DWORD read;
    OVERLAPPED rdOverlapped;
    ZeroMemory(&rdOverlapped, sizeof(rdOverlapped));
    OVERLAPPED wrOverlapped;
    ZeroMemory(&wrOverlapped, sizeof(wrOverlapped));
    
    if (ReadFile(hChildStdOutRd, &ch, 1, &read, &rdOverlapped)) {
      std::cout << ch << std::flush;
    } else {
      std::cerr << "error" << std::endl;
    }
    if (ReadFile(hChildStdErrRd, &ch, 1, &read, &wrOverlapped)) {
      std::cout << ch << std::flush;
    } else {
      std::cerr << "error" << std::endl;
    }

  }
  
  std::this_thread::sleep_for (std::chrono::seconds(2));
}
