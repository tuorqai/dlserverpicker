///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/dataview.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/gauge.h>
#include <wx/simplebook.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/hyperlink.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class MainFrameLayout
///////////////////////////////////////////////////////////////////////////////
class MainFrameLayout : public wxFrame
{
	private:

	protected:
		wxPanel* m_mainPanel;
		wxSimplebook* m_serverListSimplebook;
		wxPanel* m_serverListPanel;
		wxStaticText* m_revisionLabel;
		wxDataViewListCtrl* m_serverDataView;
		wxDataViewColumn* m_serverDataIdCol;
		wxDataViewColumn* m_serverDataDescCol;
		wxDataViewColumn* m_serverDataBlockedCol;
		wxDataViewColumn* m_serverDataPingCol;
		wxPanel* m_emptyListPanel;
		wxStaticText* m_emptyListLabel;
		wxGauge* m_syncServersGauge;
		wxButton* m_syncServersButton;
		wxButton* m_testPingButton;
		wxButton* m_blockAllButton;
		wxButton* m_unblockAllButton;
		wxStaticText* m_infoLabel;
		wxHyperlinkCtrl* projectPageLink;

	public:

		MainFrameLayout( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Deadlock Server Picker"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,500 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~MainFrameLayout();

};

