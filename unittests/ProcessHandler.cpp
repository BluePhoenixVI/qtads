
// Isolate the process handling code.
#include "ProcessHandler.h"
#include <fcntl.h>
#include <io.h>
BOOL CALLBACK SearchForQTADSWindow(_In_ HWND hwnd, _In_ LPARAM lParam);
bool CreatePipes(
    PHANDLE StdoutReadHandle, PHANDLE StdoutWriteHandle, PHANDLE StdinReadHandle,
    PHANDLE StdinWriteHandle);


bool ProcessHandler::StartQTADS(std::string Filename)
{
    HANDLE stdoutReadHandle = NULL;
    HANDLE stdoutWriteHandle = NULL;
    HANDLE stdinReadHandle = NULL;
    HANDLE stdinWriteHandle = NULL;
    char CommandLine[80]; // "BreakpointTest.t3"

    



    strcpy_s(CommandLine, "-d ");
    strcat_s(CommandLine, Filename.c_str());

    CreatePipes(&stdoutReadHandle, &stdoutWriteHandle, &stdinReadHandle, &stdinWriteHandle);

    STARTUPINFOA si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = stdinReadHandle;
    si.hStdOutput = stdoutWriteHandle;

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    BOOL RetValue = CreateProcessA(
        "./qtads.exe", CommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &piProcInfo);

    CloseHandle(stdoutWriteHandle);
    CloseHandle(stdinReadHandle);

    if (RetValue == FALSE) {
        CloseHandle(stdoutReadHandle);
        CloseHandle(stdinWriteHandle);
        return false;
    }

    int fdIn = _open_osfhandle(reinterpret_cast<intptr_t>(stdinWriteHandle), _O_WRONLY);
    int fdOut = _open_osfhandle(reinterpret_cast<intptr_t>(stdoutReadHandle), _O_RDONLY);

    FILE* fileIn = _fdopen(fdIn, "wb");
    FILE* fileOut = _fdopen(fdOut, "rb");
    QTadsInput = fileIn;
    QTadsOutput = fileOut;

    return true;
}

void ProcessHandler::StopQTADS()
{
    if (piProcInfo.hProcess != NULL) {
        for (int i = 1; i < 10; i++) {
            if (QTADSRunning()) {
                EnumWindows(SearchForQTADSWindow, reinterpret_cast<LPARAM>(this));
            } else {
                break;
            }
            Sleep(100);
        }
    }
}

bool ProcessHandler::QTADSRunning()
{
    if (piProcInfo.hProcess != NULL) {
        DWORD ExitCode;
        GetExitCodeProcess(piProcInfo.hProcess, &ExitCode);
        return (ExitCode == STILL_ACTIVE);
    }
    return false;
}

ProcessHandler::~ProcessHandler()
{
    if (piProcInfo.hProcess != NULL) {
        DWORD ExitCode;
        GetExitCodeProcess(piProcInfo.hProcess, &ExitCode);
        if (ExitCode == STILL_ACTIVE) {
            TerminateProcess(piProcInfo.hProcess, -1);
            // If we get here, something has gone wrong.
        }
        CloseHandle(piProcInfo.hProcess);
        piProcInfo.hProcess = NULL;
    }
    if (piProcInfo.hThread) {
        CloseHandle(piProcInfo.hThread);
        piProcInfo.hThread = NULL;
    }
}

// Creates the pipes needed to redirect the stdin and stdout of QTADS
bool CreatePipes(
    PHANDLE StdoutReadHandle, PHANDLE StdoutWriteHandle, PHANDLE StdinReadHandle,
    PHANDLE StdinWriteHandle)
{
    SECURITY_ATTRIBUTES saAttr;
    memset(&saAttr, 0, sizeof(saAttr));
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    return (
        CreatePipe(StdoutReadHandle, StdoutWriteHandle, &saAttr, 0) == TRUE
        && CreatePipe(StdinReadHandle, StdinWriteHandle, &saAttr, 0) == TRUE);
}

// Callback for the EnumWindows function. 
BOOL CALLBACK SearchForQTADSWindow(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    ProcessHandler* ph = reinterpret_cast<ProcessHandler*>(lParam);
    DWORD WindowOwner;
    if (GetWindowThreadProcessId(hwnd, &WindowOwner) != 0) {
        if (WindowOwner == ph->piProcInfo.dwProcessId) {
            SendMessageTimeoutW(hwnd, WM_CLOSE, 0, 0, SMTO_ABORTIFHUNG, 10, NULL);
        }
    }
    return TRUE;
}