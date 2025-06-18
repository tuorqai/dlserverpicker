//------------------------------------------------------------------------------
// Main application class
//------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------

#include <wx/app.h>

//------------------------------------------------------------------------------

class DeadlockServerPicker : public wxApp
{
public:
    bool OnInit() override;
    int OnExit() override;

private:
    wxFrame *m_frame;
};

wxDECLARE_APP(DeadlockServerPicker);
