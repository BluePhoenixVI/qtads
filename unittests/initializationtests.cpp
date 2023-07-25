
#include "ProcessHandler.h"
#include "dap/io.h"
#include "dap/network.h"
#include "dap/protocol.h"
#include <dap/session.h>
#include <gtest/gtest.h>

namespace LifecycleTests {

// Still, we need helper functions for starting the dap session we use
// for unit testing QTads-dap
void SetupSession(dap::Session* session, ProcessHandler* PH)
{
    std::shared_ptr<dap::Reader> in = dap::file(PH->QTadsOutput, true);
    std::shared_ptr<dap::Writer> out = dap::file(PH->QTadsInput, true);
    // session->bind(in, out);

    std::shared_ptr<dap::Writer> log;
    log = dap::file("TestLog.log");

    session->bind(spy(in, log), spy(out, log));
}

// Test the ProcessHandler class. It should both start and stop QTADS
TEST(StartingAndStopping, StartAndStopQTADS)
{
    ProcessHandler PH;
    EXPECT_TRUE(PH.StartQTADS("BreakpointTest.t3")) << "QTADS failed to start";
    Sleep(100);
    PH.StopQTADS();
    Sleep(100);
    EXPECT_FALSE(PH.QTADSRunning()) << "QTADS is still running after calling PH.StopQTADS()";
}
// Test the DAP lifecycle. We should be able to bind to the QTads input and output streams using a
// session, and then close that session along with QTads.
TEST(StartingAndStopping, StartAndStopDapSession)
{
    ProcessHandler PH;
    EXPECT_TRUE(PH.StartQTADS("BreakpointTest.t3")) << "QTADS failed to start";
    auto session = dap::Session::create();
    SetupSession(session.get(), &PH);
    PH.StopQTADS();
    session.reset();
    EXPECT_FALSE(PH.QTADSRunning()) << "QTADS is still running after calling PH.StopQTADS()";
}
} // namespace LifecycleTests

namespace TestInputAndOutput {
// Tests that we can send messages to QTADS, and that we get a responce
TEST(StartingAndStopping, TestInputAndOutput)
{
    ProcessHandler PH;
    EXPECT_TRUE(PH.StartQTADS("BreakpointTest.t3")) << "QTADS failed to start";
    auto session = dap::Session::create();
    LifecycleTests::SetupSession(session.get(), &PH);
    // Send the Initialize Request. As we are just testing that we can send and recive messages,
    // we do not configure the request
    auto future = session->send(dap::InitializeRequest{});
    // Wait on the response.
    auto responseStatus = future.wait_for(std::chrono::milliseconds(100000));
    EXPECT_EQ(responseStatus, dap::future_status::ready) << "The Initialize request timed out";
    if (responseStatus == dap::future_status::ready) {
        auto response = future.get();
        EXPECT_FALSE(response.error.operator bool())
            << "The Initialize request failed with message:" << response.error.message;
    }
    // Close QTADS before calling reset on session to delete the dap::Session object.
    // If we were to do this the other way, there may be a hang when the session finilizer
    // tries to close the io streams
    PH.StopQTADS();
    EXPECT_FALSE(PH.QTADSRunning()) << "QTADS is still running after calling PH.StopQTADS()";
    session.reset();
}

} // namespace LifecycleTests