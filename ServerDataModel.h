
#pragma once

#include <cstdint>
#include <wx/dataview.h>
#include "ServerData.h"

wxDataViewItem ServerLocationToItem(std::size_t index);
std::size_t ServerLocationFromItem(wxDataViewItem item);

class ServerDataModel : public wxDataViewModel
{
public:
    explicit ServerDataModel(ServerData const &data);

    bool IsContainer(wxDataViewItem const &item) const override;
    wxDataViewItem GetParent(wxDataViewItem const &item) const override;
    unsigned int GetChildren(wxDataViewItem const &item, wxDataViewItemArray &array) const override;
    void GetValue(wxVariant &variant, wxDataViewItem const &item, unsigned int col) const override;
    bool SetValue(wxVariant const &variant, wxDataViewItem const &item, unsigned int col) override;

    int Compare(wxDataViewItem const &a, wxDataViewItem const &b, unsigned int col, bool asc) const override;

private:
    ServerData const &m_data;
};
