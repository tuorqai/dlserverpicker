//------------------------------------------------------------------------------

#include "MainFrame.h"
#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <wx/webrequest.h>
#include "FirewallManager.h"
#include "PingTest.h"
#include "ServerDataModel.h"
#include "Version.h"
#include "DeadlockServerPicker.xpm"

//------------------------------------------------------------------------------

MainFrame::MainFrame()
    : MainFrameLayout(nullptr, wxID_ANY)
    , m_serverDataModel(new ServerDataModel(m_serverData))
    , m_savedGameChoice(0)
{
    SetIcons(wxICON(dlsp));
    
    // [Ping Test] A single location is tested.
    Bind(EVT_PING_TEST_PROGRESS, &MainFrame::OnPingTestProgress, this);

    // [Ping Test] All locations are tested.
    Bind(EVT_PING_TEST_COMPLETED, &MainFrame::OnPingTestCompleted, this);

    // All web requests are handled there.
    Bind(wxEVT_WEBREQUEST_STATE, &MainFrame::OnWebRequestComplete, this);

    // Bottom row of buttons
    m_syncServersButton->Bind(wxEVT_BUTTON, &MainFrame::OnServerSyncInvoked, this);
    m_testPingButton->Bind(wxEVT_BUTTON, &MainFrame::OnTestPingButtonClicked, this);
    m_blockAllButton->Bind(wxEVT_BUTTON, &MainFrame::OnBlockAllButtonClicked, this);
    m_unblockAllButton->Bind(wxEVT_BUTTON, &MainFrame::OnUnblockAllButtonClicked, this);

    // Bind server data view to its model
    m_serverDataView->AssociateModel(m_serverDataModel.get());

    // Restore server data from cache, update layout.
    m_serverData.RestoreFromCache();
    OnServerDataUpdate();

    // In case if server list is empty, we should hide the progress bar.
    m_syncServersGauge->Hide();
    m_emptyListPanel->Layout();

    // Check if allowed to modify firewall rules.
    if (!FirewallManager::Get()->CheckPermissions()) {
        // The message here refers to Linux only which is
        // incredibly broken approach from OOP standpoint.
        // Meh, too lazy to do it better.
        wxMessageBox(
            _("You have no permissions to modify nftables.\n"
              "Either run this program as root (e.g. with sudo) "
              "or use setcaps to permit network administration (refer to README)."),
            _("No permissions to modify nftables"),
            wxOK | wxICON_ASTERISK
        );
        Close();
    }

    // Show a warning if the firewall is disabled.
    if (!FirewallManager::Get()->IsFirewallEnabled()) {
        wxMessageBox(
            _("Firewall seems to be disabled. Make sure to enable it, "
              "otherwise this app won't have any effect."),
            _("Firewall is disabled"),
            wxOK | wxICON_ASTERISK
        );
        Close();
    }

    m_infoLabel->SetLabelText("Deadlock Server Picker v" DL_SERVER_PICKER_VERSION);

    // [Geolocation] This is needed to warn RU users.
    m_geoRequest = wxWebSession::GetDefault().CreateRequest(this, "https://api.country.is/");
    if (m_geoRequest.IsOk()) {
        m_geoRequest.Start();
    }
}

void MainFrame::OnServerSyncInvoked(wxCommandEvent const &event)
{
    if (!FirewallManager::Get()->IsClear()) {
        int answer = wxMessageBox(
            _("All previously blocked servers will be unblocked first. Proceed?"),
            _("Warning"),
            wxYES_NO | wxCANCEL | wxICON_QUESTION,
            this
        );

        if (answer != wxYES) {
            return;
        }
    }

    bool isFailed = false;

    try {
        FirewallManager::Get()->Clear();
    } catch (std::exception const &exception) {
        wxMessageBox(exception.what(), _("Firewall Error"), wxOK | wxICON_ERROR, this);
        isFailed = true;
    }

    m_serverDataModel->Cleared();

    if (isFailed) {
        return;
    }

    wxString url = wxString::Format("https://api.steampowered.com/ISteamApps/GetSDRConfig/v1/?appid=%d", 1422450);
    m_syncRequest = wxWebSession::GetDefault().CreateRequest(this, url);

    if (!m_syncRequest.IsOk()) {
        wxMessageBox(_("Unable to sync server list. Check your Internet connection."),
                     _("Sync Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    m_syncRequest.Start();

    m_serverListSimplebook->SetSelection(1);

    m_emptyListLabel->SetLabelText(_("Syncing server list, please wait..."));
    m_syncServersGauge->Show();
    m_syncServersGauge->Pulse();
    m_emptyListPanel->Layout();

    m_syncServersButton->Disable();
}

void MainFrame::OnExitRequested(wxCommandEvent const &event)
{
    Close();
}

void MainFrame::OnPingTestProgress(wxCommandEvent const &event)
{
    int location = event.GetInt();
    long latency = event.GetExtraLong();

    m_serverData.GetLocation(location).ping = latency;
    m_serverDataModel->ItemChanged(ServerLocationToItem(location));
}

void MainFrame::OnPingTestCompleted(wxCommandEvent const &event)
{
    m_testPingButton->Enable();
}

void MainFrame::OnTestPingButtonClicked(wxCommandEvent const &event)
{
    m_testPingButton->Disable();
    PingTest::Get()->Invoke(*this, m_serverData);
}

void MainFrame::OnBlockAllButtonClicked(wxCommandEvent const &event)
{
    int answer = wxMessageBox(
        _("You are going to block all relay servers. Proceed?"),
        _("Warning"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION,
        this
    );

    if (answer != wxYES) {
        return;
    }

    try {
        for (std::size_t i = 0; i < m_serverData.NumLocations(); i++) {
            if (m_serverData.GetLocation(i).relays.empty()) {
                continue;
            }

            FirewallManager::Get()->BlockLocation(m_serverData.GetLocation(i));
        }
    } catch (std::exception const &exception) {
        wxMessageBox(exception.what(), _("Firewall Error"), wxOK | wxICON_ERROR, this);
    }

    OnServerDataUpdate();
}

void MainFrame::OnUnblockAllButtonClicked(wxCommandEvent const &event)
{
    int answer = wxMessageBox(
        _("You are going to unblock all relay servers. Proceed?"),
        _("Warning"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION,
        this
    );

    if (answer != wxYES) {
        return;
    }

    try {
        FirewallManager::Get()->Clear();
    } catch (std::exception const &exception) {
        wxMessageBox(exception.what(), _("Firewall Error"), wxOK | wxICON_ERROR, this);
    }

    OnServerDataUpdate();
}

void MainFrame::ServerSyncCompleted(wxWebResponse const &response)
{
    m_requestResult.Clear();

    wxStringOutputStream sstream(&m_requestResult);
    response.GetStream()->Read(sstream);

    m_serverData.UpdateFromJSON(m_requestResult);
    OnServerDataUpdate();
}

void MainFrame::ServerSyncFailed(wxString const &errDesc)
{
    wxString message = wxString::Format(
        _("Failed to sync server list. Try again.\nReason: %s"),
        errDesc
    );

    wxMessageBox(message, "Sync failed", wxOK | wxICON_ERROR, this);

    if (m_serverData.NumLocations() == 0) {
        m_emptyListLabel->SetLabelText(_("Server sync has failed. Try again later."));

        m_emptyListLabel->Wrap(m_emptyListPanel->GetSize().x - 50);
        m_syncServersGauge->Hide();
        m_emptyListPanel->Layout();

        m_syncServersButton->Enable();
    }

    OnServerDataUpdate();
}

void MainFrame::OnServerDataUpdate()
{
    m_serverDataModel->Cleared();

    bool isDataEmpty = m_serverData.NumLocations() == 0;

    m_serverListSimplebook->SetSelection(isDataEmpty ? 1 : 0);

    if (!isDataEmpty) {
        wxDateTime dateTime(static_cast<time_t>(m_serverData.GetRevision()));
        wxString text = wxString::Format(_("Server list revision: %s"), dateTime.Format());
        m_revisionLabel->SetLabel(text);
    }

    m_testPingButton->Enable(!isDataEmpty);
    m_blockAllButton->Enable(!isDataEmpty);
    m_unblockAllButton->Enable(!isDataEmpty);
}

void MainFrame::OnWebRequestComplete(wxWebRequestEvent const &event)
{
    if (m_syncRequest.IsOk() && event.GetRequest().GetId() == m_syncRequest.GetId()) {
        switch (event.GetState()) {
        case wxWebRequest::State_Completed:
            ServerSyncCompleted(event.GetResponse());
            break;
        case wxWebRequest::State_Failed:
            ServerSyncFailed(event.GetErrorDescription());
            break;
        default:
            break;
        }
    } else if (m_geoRequest.IsOk() && event.GetRequest().GetId() == m_geoRequest.GetId()) {
        switch (event.GetState()) {
        case wxWebRequest::State_Completed:
            {
                wxWebResponse response = m_geoRequest.GetResponse();
                if (response.GetStatus() == 200) {
                    std::string country;
                    JSON::parse(response.AsString()).at("country").get_to(country);

                    if (country == "RU") {
                        IronCurtainWarning(country);
                    }
                }
            }
            break;
        case wxWebRequest::State_Failed:
            break;
        case wxWebRequest::State_Cancelled:
            break;
        default:
            break;
        }
    }
}

void MainFrame::IronCurtainWarning(wxString const &country)
{
    wxString msg = wxString::Format(_("Your IP is detected to be from %s. "
        "This country is currently region-locked by Valve. "
        "This means you will get matched with players from %s only."),
        country, country);
    
    wxMessageBox(msg, _("You're behind the Iron Curtain"), wxICON_WARNING);
}
