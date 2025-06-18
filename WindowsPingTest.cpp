//------------------------------------------------------------------------------
// WindowsPingTest.cpp: implementation of PingTest for Windows
//------------------------------------------------------------------------------

#include "PingTest.h"
#include <thread>
#include <Windows.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>

//------------------------------------------------------------------------------

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

//------------------------------------------------------------------------------

wxDEFINE_EVENT(EVT_PING_TEST_PROGRESS, wxCommandEvent);
wxDEFINE_EVENT(EVT_PING_TEST_FAILED, wxCommandEvent);
wxDEFINE_EVENT(EVT_PING_TEST_COMPLETED, wxCommandEvent);

class WindowsPingTest : public PingTest
{
public:
	WindowsPingTest();
	~WindowsPingTest();

	void Invoke(wxEvtHandler &evtHandler, ServerData const &serverData) override;

private:
	HANDLE				m_hIcmpFile;
	char				m_sendData[32];
	DWORD				m_replySize;
	LPVOID				m_replyBuffer;
	std::atomic<bool>	m_isActive;
};

//------------------------------------------------------------------------------

WindowsPingTest::WindowsPingTest()
{
	m_hIcmpFile = IcmpCreateFile();

	if (m_hIcmpFile == INVALID_HANDLE_VALUE) {
		throw std::runtime_error("IcmpCreateFile() failed");
	}

	std::memset(m_sendData, 0, sizeof(m_sendData));

	m_replySize = sizeof(ICMP_ECHO_REPLY) + sizeof(m_sendData) + 8;
	m_replyBuffer = new std::uint8_t[m_replySize];
}

WindowsPingTest::~WindowsPingTest()
{
	if (m_hIcmpFile != INVALID_HANDLE_VALUE) {
		IcmpCloseHandle(m_hIcmpFile);
	}

	delete[] m_replyBuffer;
}

void WindowsPingTest::Invoke(wxEvtHandler &evtHandler, ServerData const &serverData)
{
	if (m_isActive) {
		return;
	}

	std::thread thread([&]() {
		m_isActive = true;

		for (std::size_t i = 0; i < serverData.NumLocations(); i++) {
			auto &location = serverData.GetLocation(i);

			if (location.relays.empty()) {
				continue;
			}

			auto addr = inet_addr(location.relays[0].ipv4);

			if (addr == INADDR_NONE) {
				continue;
			}

			wxLongLong startTimeMillis = wxGetUTCTimeMillis();

			DWORD dwResult = IcmpSendEcho2(m_hIcmpFile, 0, nullptr, nullptr, addr,
										   m_sendData, sizeof(m_sendData), nullptr,
										   m_replyBuffer, m_replySize, 1000);

			wxLongLong endTimeMillis = wxGetUTCTimeMillis();

			if (dwResult == 0) {
				continue;
			}

			wxCommandEvent *event = new wxCommandEvent(EVT_PING_TEST_PROGRESS);
			
			event->SetInt(static_cast<int>(i));
			event->SetExtraLong(static_cast<long>((endTimeMillis - startTimeMillis).GetValue()));

			evtHandler.QueueEvent(event);
		}

		m_isActive = false;

		evtHandler.QueueEvent(new wxCommandEvent(EVT_PING_TEST_COMPLETED));
	});

	thread.detach();
}

PingTest *PingTest::Get()
{
	static WindowsPingTest instance;
	return &instance;
}
