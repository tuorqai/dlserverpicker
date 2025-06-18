//------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------

#include <wx/dataview.h>
#include <wx/webrequest.h>
#include "Layout.gen.h"
#include "ServerData.h"

//------------------------------------------------------------------------------

class MainFrame : public MainFrameLayout
{
public:
    MainFrame();

private:
    void OnServerSyncInvoked(wxCommandEvent const &event);
    void OnExitRequested(wxCommandEvent const &event);

    void OnPingTestProgress(wxCommandEvent const &event);
    void OnPingTestCompleted(wxCommandEvent const &event);

    void OnTestPingButtonClicked(wxCommandEvent const &event);
    void OnBlockAllButtonClicked(wxCommandEvent const &event);
    void OnUnblockAllButtonClicked(wxCommandEvent const &event);

    void ServerSyncCompleted(wxWebResponse const &response);
    void ServerSyncFailed(wxString const &errDesc);

    void OnServerDataUpdate();

    wxString m_requestResult;
    ServerData m_serverData;
    wxObjectDataPtr<wxDataViewModel> m_serverDataModel;
    int m_savedGameChoice;
};
