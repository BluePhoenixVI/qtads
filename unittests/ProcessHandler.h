

// Cannot find a platform-independent method of starting a process, accessing it's
// stdin and stdout handles, and stopping it.
// So all of that code is in a helper class for easier porting to other systems
#include <memory>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <string>

class ProcessHandler
{
public:
    FILE* QTadsInput = NULL;
    FILE* QTadsOutput = NULL;

    ProcessHandler()
    {
        memset(&piProcInfo, 0, sizeof(piProcInfo));
    }
    bool StartQTADS(std::string Filename);
    void StopQTADS();
    bool QTADSRunning();

    ~ProcessHandler();

    // /////////////////////////////////////////////////////////////////////////
    // Code below is platform specific
public:
    // Stores handles to the started process - QTADS
    PROCESS_INFORMATION piProcInfo;
};