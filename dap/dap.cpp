// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//


#include "vmdbg.h"
#include <stdio.h>
#ifdef WIN32
  #include <io.h>
  #include <fcntl.h>
#endif // OS_WINDOWS
#include <dap/io.h>
#include <dap/network.h>
#include <dap/protocol.h>
#include <dap/session.h>

std::unique_ptr<dap::Session> m_Session;

/*
#ifdef WIN32
    // Change stdin & stdout from text mode to binary mode.
    // This ensures sequences of \r\n are not changed to \n.
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif // OS_WINDOWS
*/

void CVmDebugUI::init(VMG_ const char* image_filename)
{
#ifdef WIN32
  // Change stdin & stdout from text mode to binary mode.
  // This ensures sequences of \r\n are not changed to \n.
  _setmode(_fileno(stdin), _O_BINARY);
  _setmode(_fileno(stdout), _O_BINARY);
#endif // OS_WINDOWS
//G_debugger
  m_Session = dap::Session::create();

#define LOG_TO_FILE "QTADS-DAP.log"
  std::shared_ptr<dap::Writer> log;
#ifdef LOG_TO_FILE
  log = dap::file(LOG_TO_FILE);
#endif

  m_Session->onError([&](const char* msg)
  {
    if (log)
    {
      dap::writef(log, "dap::Session error: %s\n", msg);
      log->close();
    }
    exit(EXIT_FAILURE);
  });


  // The Initialize request is the first message sent from the client and
  // the response reports debugger capabilities.
  // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Initialize
  m_Session->registerHandler([](const dap::InitializeRequest&)
  {
    dap::InitializeResponse response;
    response.supportsConfigurationDoneRequest = true;
    return response;
  });




  std::shared_ptr<dap::Reader> in = dap::file(stdin, false); // true?
  std::shared_ptr<dap::Writer> out = dap::file(stdout, false);
  if (log)
  {
    m_Session->bind(spy(in, log), spy(out, log));
  }
  else
  {
    m_Session->bind(in, out);
  }








}

void CVmDebugUI::init_after_load(VMG0_)
{

}

void CVmDebugUI::terminate(VMG0_)
{
    //m_Session.reset();
  m_Session.release(); // Yes, this is a memory leak, but QTADS is closing anyways, so it will not matter.
}

void CVmDebugUI::cmd_loop(VMG_ int bp_number, int error_code, const uchar** pc)
{

}
