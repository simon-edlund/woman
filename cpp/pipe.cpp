/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright 1995 - 1997 Microsoft Corporation.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
\******************************************************************************/

/*++
Copyright (c) 1997  Microsoft Corporation
Module Name:
    pipeex.c
Abstract:
    CreatePipe-like function that lets one or both handles be overlapped
Author:
    Dave Hart  Summer 1997
Revision History:
--*/
#include <windows.h>
#include <stdio.h>

static volatile long pipeSerialNumber;

BOOL MyCreatePipeEx(
    LPHANDLE lpReadPipe,
    LPHANDLE lpWritePipe,
    LPSECURITY_ATTRIBUTES lpPipeAttributes)
/*++
Routine Description:
    The CreatePipeEx API is used to create an anonymous pipe I/O device.
    Unlike CreatePipe FILE_FLAG_OVERLAPPED may be specified for one or
    both handles.
    Two handles to the device are created.  One handle is opened for
    reading and the other is opened for writing.  These handles may be
    used in subsequent calls to ReadFile and WriteFile to transmit data
    through the pipe.
Arguments:
    lpReadPipe - Returns a handle to the read side of the pipe.  Data
        may be read from the pipe by specifying this handle value in a
        subsequent call to ReadFile.
    lpWritePipe - Returns a handle to the write side of the pipe.  Data
        may be written to the pipe by specifying this handle value in a
        subsequent call to WriteFile.
    lpPipeAttributes - An optional parameter that may be used to specify
        the attributes of the new pipe.  If the parameter is not
        specified, then the pipe is created without a security
        descriptor, and the resulting handles are not inherited on
        process creation.  Otherwise, the optional security attributes
        are used on the pipe, and the inherit handles flag effects both
        pipe handles.
Return Value:
    TRUE - The operation was successful.
    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.
--*/

{
  HANDLE ReadPipeHandle, WritePipeHandle;
  DWORD dwError;
  char pipeNameBuffer[ MAX_PATH ];

  //
  //  Set the default timeout to 120 seconds
  //

  DWORD nSize = 4096;

  sprintf(pipeNameBuffer,
          "\\\\.\\Pipe\\RemoteExeAnon.%08x.%08x",
          GetCurrentProcessId(),
          InterlockedIncrement(&pipeSerialNumber)
          );

  ReadPipeHandle = CreateNamedPipeA(
                       pipeNameBuffer,
                       PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
                       PIPE_TYPE_BYTE | PIPE_WAIT,
                       1,             // Number of pipes
                       nSize,         // Out buffer size
                       nSize,         // In buffer size
                       120 * 1000,    // Timeout in ms
                       lpPipeAttributes
                       );

  if (! ReadPipeHandle) {
    return FALSE;
  }

  WritePipeHandle = CreateFileA(
                      pipeNameBuffer,
                      GENERIC_WRITE,
                      0,                         // No sharing
                      lpPipeAttributes,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                      NULL                       // Template file
                    );

  if (INVALID_HANDLE_VALUE == WritePipeHandle) {
    dwError = GetLastError();
    CloseHandle( ReadPipeHandle );
    SetLastError(dwError);
    return FALSE;
  }

  *lpReadPipe = ReadPipeHandle;
  *lpWritePipe = WritePipeHandle;
  return( TRUE );
}
