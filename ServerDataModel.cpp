
#include "ServerDataModel.h"
#include <wx/msgdlg.h>
#include "FirewallManager.h"

enum ServerDataColumn
{
    SERVER_DATA_COL_ID,
    SERVER_DATA_COL_DESC,
    SERVER_DATA_COL_BLOCKED,
    SERVER_DATA_COL_PING,
};

wxDataViewItem ServerLocationToItem(std::size_t index)
{
    return wxDataViewItem(reinterpret_cast<void *>(index + 1));
}

std::size_t ServerLocationFromItem(wxDataViewItem item)
{
    return reinterpret_cast<std::size_t>(item.GetID()) - 1;
}

ServerDataModel::ServerDataModel(ServerData const &data)
    : m_data(data)
{
}

bool ServerDataModel::IsContainer(wxDataViewItem const &item) const
{
    if (item.GetID() == nullptr) {
        return true;
    }

    return false;
}

wxDataViewItem ServerDataModel::GetParent(wxDataViewItem const &item) const
{
    return wxDataViewItem(nullptr);
}

unsigned int ServerDataModel::GetChildren(wxDataViewItem const &item, wxDataViewItemArray &array) const
{
    if (item.GetID() != nullptr) {
        return 0;
    }

    for (std::size_t i = 0; i < m_data.NumLocations(); i++) {
        array.Add(ServerLocationToItem(i));
    }

    return static_cast<unsigned int>(m_data.NumLocations());
}

void ServerDataModel::GetValue(wxVariant &variant, wxDataViewItem const &item, unsigned int col) const
{
    ServerData::Location const &location = m_data.GetLocation(ServerLocationFromItem(item));

    switch (col) {
    case SERVER_DATA_COL_ID:
        variant = location.identifier;
        break;
    case SERVER_DATA_COL_DESC:
        variant = location.description;
        break;
    case SERVER_DATA_COL_BLOCKED:
        variant = FirewallManager::Get()->IsLocationBlocked(location);
        break;
    case SERVER_DATA_COL_PING:
        if (location.ping == -1) {
            variant = "N/A";
        } else if (location.ping <= 1) {
            variant = "<1 ms";
        } else {
            variant = wxString::Format("%d ms", location.ping);
        }
        break;
    default:
        break;
    }
}

bool ServerDataModel::SetValue(wxVariant const &variant, wxDataViewItem const &item, unsigned int col)
{
    if (col != SERVER_DATA_COL_BLOCKED) {
        return false;
    }

    ServerData::Location const &location = m_data.GetLocation(ServerLocationFromItem(item));

    if (location.relays.empty()) {
        wxMessageBox(_("This location doesn't have any relay servers for whatever reason."), _("Warning"), wxOK | wxICON_WARNING);
        return false;
    }

    try {
        if (FirewallManager::Get()->IsLocationBlocked(location)) {
            FirewallManager::Get()->UnblockLocation(location);
        } else {
            FirewallManager::Get()->BlockLocation(location);
        }
    } catch (std::exception const &exception) {
        wxMessageBox(exception.what(), _("Firewall Error"), wxOK | wxICON_ERROR);
        return false;
    }

    return true;
}

int ServerDataModel::Compare(wxDataViewItem const &a, wxDataViewItem const &b, unsigned int col, bool asc) const
{
    if (col != SERVER_DATA_COL_PING) {
        return wxDataViewModel::Compare(a, b, col, asc);
    }

    ServerData::Location const &locA = m_data.GetLocation(ServerLocationFromItem(a));
    ServerData::Location const &locB = m_data.GetLocation(ServerLocationFromItem(b));

    return asc ? (locA.ping - locB.ping) : (locB.ping - locA.ping);
}
