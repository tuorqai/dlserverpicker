
//------------------------------------------------------------------------------

#include "PingTest.h"

#include <atomic>
#include <thread>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

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

private:
    std::atomic<bool>	m_isActive;
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
    if (m_isActive) {
        return;
    }

    std::thread thread([&]() {
        m_isActive = true;

        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);

        if (sock < 0) {
            m_isActive = false;
            return;
        }

        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        for (std::size_t i = 0; i < serverData.NumLocations(); i++) {
            auto &location = serverData.GetLocation(i);

            if (location.relays.empty()) {
                wxCommandEvent *event = new wxCommandEvent(EVT_PING_TEST_PROGRESS);

                event->SetInt(static_cast<int>(i));
                event->SetExtraLong(-2);

                evtHandler.QueueEvent(event);
                continue;
            }

            struct sockaddr_in dest = { 0 };
            dest.sin_family = AF_INET;
            inet_pton(AF_INET, location.relays[0].ipv4, &dest.sin_addr);

            char packet[64] = { 0 };

            struct icmphdr *icmp = (struct icmphdr *) &packet;
            icmp->type = ICMP_ECHO;
            icmp->un.echo.id = htons(getpid());
            icmp->un.echo.sequence = htons(i);

            wxLongLong startTimeMillis = wxGetUTCTimeMillis();

            if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *) &dest, sizeof(dest)) < 0) {
                continue;
            }

            char recv[1024];
            struct sockaddr_in src;
            socklen_t srclen;
            if (recvfrom(sock, recv, sizeof(recv), 0, (struct sockaddr *) &src, &srclen) < 0) {
                continue;
            }

            wxLongLong endTimeMillis = wxGetUTCTimeMillis();

            wxCommandEvent *event = new wxCommandEvent(EVT_PING_TEST_PROGRESS);

            event->SetInt(static_cast<int>(i));
            event->SetExtraLong(static_cast<long>((endTimeMillis - startTimeMillis).GetValue()));

            evtHandler.QueueEvent(event);
        }

        close(sock);

        evtHandler.QueueEvent(new wxCommandEvent(EVT_PING_TEST_COMPLETED));
        m_isActive = false;
    });

    thread.detach();
}

//------------------------------------------------------------------------------

PingTest *PingTest::Get()
{
    static LinuxPingTest instance;
    return &instance;
}
