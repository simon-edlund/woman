#include <windows.h>
#undef max
#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <string>
#include <algorithm>
#include "tclap/CmdLine.h"


extern BOOL MyCreatePipeEx(
    LPHANDLE lpReadPipe,
    LPHANDLE lpWritePipe,
    LPSECURITY_ATTRIBUTES lpPipeAttributes);

void ReadChar(HANDLE hFile, char* ch, OVERLAPPED* overlapped) {
  DWORD read;
  if (!ReadFile(hFile, ch, 1, &read, overlapped)) {
    if (GetLastError() != ERROR_IO_PENDING) {
      std::cerr << "error: " << GetLastError() << std::endl;
      exit(1);
    }
  }
}


int main(int argc, char *argv[]) {
  try {
    TCLAP::CmdLine cmd("Run a process catching stdout/stderr", ' ', "0.1");


    TCLAP::SwitchArg colorSwitch("c", "color",
                                 "Colorized output",
                                 false);
    cmd.add(colorSwitch);

    TCLAP::ValueArg<std::string> directory(
        "d", "directory", "Specify working directory",
        false, "",
        "DIRECTORY");
    cmd.add(directory);

    TCLAP::UnlabeledValueArg<std::string> executable(
        "exefile", "executable to run",
        true, "-",
        "EXECUTABLE");
    cmd.add(executable);

    // Parse the args.
    cmd.parse(argc, argv);

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
    startupInfo.hStdOutput = hChildStdOutWr;

    PROCESS_INFORMATION processInformation;
    if (!CreateProcessA(
            executable.getValue().c_str(),  // lpApplicationName,
            nullptr,       // lpCommandLine,
            nullptr,       // lpProcessAttributes,
            nullptr,       // lpThreadAttributes,
            true,          // bInheritHandles,
            0,             // dwCreationFlags,
            nullptr,       // lpEnvironment,
            // lpCurrentDirectory,
            directory.isSet() ? directory.getValue().c_str() : nullptr,
            &startupInfo,   // lpStartupInfo,
            &processInformation)) {  // lpProcessInformation
      std::cerr << "CreateProcessA failed!" << std::endl;
      std::exit(1);
    }

    char oCh, eCh;
    DWORD oRead, eRead;
    OVERLAPPED oOverlapped;
    ZeroMemory(&oOverlapped, sizeof(oOverlapped));
    oOverlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    OVERLAPPED eOverlapped;
    ZeroMemory(&eOverlapped, sizeof(eOverlapped));
    eOverlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    ReadChar(hChildStdOutRd, &oCh, &oOverlapped);
    ReadChar(hChildStdErrRd, &eCh, &eOverlapped);

    std::string oStr, eStr;
    bool doQuit{false};
    bool eFirst{true};
    HANDLE handles[] = {
      eFirst ? eOverlapped.hEvent : oOverlapped.hEvent,
      eFirst ? oOverlapped.hEvent : eOverlapped.hEvent,
      processInformation.hProcess
    };
    DWORD r = WaitForMultipleObjects(3, handles, false, INFINITE);
    if (!eFirst) {
      switch (r) {
        case WAIT_OBJECT_0:   r = WAIT_OBJECT_0 + 1; break;
        case WAIT_OBJECT_0+1: r = WAIT_OBJECT_0;     break;
      }
    }
    switch (r) {
      case WAIT_OBJECT_0:
        if (eCh == '\n') {
          std::cout << "\033[1;31m" << eStr << "\033[0m\n";
          eStr.clear();
          eFirst = false;
        } else if if (eCh != '\r'){
          eStr += eCh;
        }
        ReadChar(hChildStdErrRd, &eCh, &eOverlapped);
        break;
      case WAIT_OBJECT_0+1:
        if (oCh == '\r') {}
        else if (oCh == '\n') {
          std::cout << oStr << std::endl;
          oStr.clear();
          eFirst = true;
        } else if if (oCh != '\r'){
          oStr += oCh;
        }
        ReadChar(hChildStdOutRd, &oCh, &oOverlapped);
        break;
      case WAIT_OBJECT_0+2:
        doQuit = true;
        break;
    }


    DWORD exitCode;
    if (GetExitCodeProcess(processInformation.hProcess, &exitCode)) {
      std::cout << "exit code: " << exitCode << std::endl;
      std::exit(exitCode);
    }
  }
  catch (TCLAP::ArgException &e)  // catch any exceptions
  {
    std::cerr << "error: " << e.error() << "for arg " << e.argId() << std::endl;
    std::exit(1);
  }
  catch(...) {
    std::cerr << "WTF!" << std::endl;
    std::exit(1);
  }
  std::exit(0);
}
