//------------------------------------------------------------------------------
// Main application class
//------------------------------------------------------------------------------

#include "DeadlockServerPicker.h"
#include "MainFrame.h"

//------------------------------------------------------------------------------

wxIMPLEMENT_APP(DeadlockServerPicker);

bool DeadlockServerPicker::OnInit()
{
    m_frame = new MainFrame();
    m_frame->Show(true);

    return true;
}

int DeadlockServerPicker::OnExit()
{
    return 0;
}
