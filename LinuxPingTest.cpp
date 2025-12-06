
//------------------------------------------------------------------------------

#include "PingTest.h"

//------------------------------------------------------------------------------

wxDEFINE_EVENT(EVT_PING_TEST_PROGRESS, wxCommandEvent);
wxDEFINE_EVENT(EVT_PING_TEST_FAILED, wxCommandEvent);
wxDEFINE_EVENT(EVT_PING_TEST_COMPLETED, wxCommandEvent);

//------------------------------------------------------------------------------

class LinuxPingTest : public PingTest
{
public:
    LinuxPingTest();
    ~LinuxPingTest();

    void Invoke(wxEvtHandler &evtHandler, ServerData const &serverData) override;
};

//------------------------------------------------------------------------------

LinuxPingTest::LinuxPingTest()
{
}

LinuxPingTest::~LinuxPingTest()
{
}

void LinuxPingTest::Invoke(wxEvtHandler &evtHandler, ServerData const &serverData)
{
}

//------------------------------------------------------------------------------

PingTest *PingTest::Get()
{
    static LinuxPingTest instance;
    return &instance;
}
