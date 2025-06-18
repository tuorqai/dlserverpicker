//------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------

#include <functional>
#include <wx/event.h>
#include <wx/time.h>
#include "ServerData.h"

//------------------------------------------------------------------------------

wxDECLARE_EVENT(EVT_PING_TEST_PROGRESS, wxCommandEvent);
wxDECLARE_EVENT(EVT_PING_TEST_FAILED, wxCommandEvent);
wxDECLARE_EVENT(EVT_PING_TEST_COMPLETED, wxCommandEvent);

//------------------------------------------------------------------------------

class PingTest
{
public:
	static PingTest *Get();

	virtual void Invoke(wxEvtHandler &evtHandler, ServerData const &serverData) = 0;
};
