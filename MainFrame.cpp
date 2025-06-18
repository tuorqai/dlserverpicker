//------------------------------------------------------------------------------

#include "MainFrame.h"
#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <wx/webrequest.h>
#include "FirewallManager.h"
#include "PingTest.h"
#include "ServerDataModel.h"
#include "Version.h"

//------------------------------------------------------------------------------

MainFrame::MainFrame()
    : MainFrameLayout(nullptr, wxID_ANY)
    , m_serverDataModel(new ServerDataModel(m_serverData))
    , m_savedGameChoice(0)
{
    // [Ping Test] A single location is tested.
    Bind(EVT_PING_TEST_PROGRESS, &MainFrame::OnPingTestProgress, this);

    // [Ping Test] All locations are tested.
    Bind(EVT_PING_TEST_COMPLETED, &MainFrame::OnPingTestCompleted, this);

    // Server sync web request are handled here
    Bind(wxEVT_WEBREQUEST_STATE, [this](wxWebRequestEvent const &event) {
        switch (event.GetState()) {
        case wxWebRequest::State_Completed:
            ServerSyncCompleted(event.GetResponse());
            break;
        case wxWebRequest::State_Failed:
            ServerSyncFailed(event.GetErrorDescription());
            break;
        }
    });

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

    // Show a warning if the firewall is disabled.
    if (!FirewallManager::Get()->IsFirewallEnabled()) {
        wxMessageBox(
            _("Firewall seems to be disabled. Make sure to enable it, "
              "otherwise this app won't have any effect."),
            _("Firewall is disabled"),
            wxOK | wxICON_ASTERISK
        );
    }

    m_infoLabel->SetLabelText("Deadlock Server Picker v" DL_SERVER_PICKER_VERSION);
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
    wxWebRequest request = wxWebSession::GetDefault().CreateRequest(this, url);

    if (!request.IsOk()) {
        wxMessageBox(_("Unable to sync server list. Check your Internet connection."),
                     _("Sync Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    request.Start();

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
