///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "Layout.gen.h"

///////////////////////////////////////////////////////////////////////////

MainFrameLayout::MainFrameLayout( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 200,200 ), wxDefaultSize );

	wxBoxSizer* topSizer;
	topSizer = new wxBoxSizer( wxVERTICAL );

	m_mainPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );

	m_serverListSimplebook = new wxSimplebook( m_mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_serverListPanel = new wxPanel( m_serverListSimplebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* serverDataSizer;
	serverDataSizer = new wxBoxSizer( wxVERTICAL );

	m_revisionLabel = new wxStaticText( m_serverListPanel, wxID_ANY, _("Server List Revision #"), wxDefaultPosition, wxDefaultSize, 0 );
	m_revisionLabel->Wrap( -1 );
	serverDataSizer->Add( m_revisionLabel, 0, wxALL, 5 );

	m_serverDataView = new wxDataViewListCtrl( m_serverListPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_ROW_LINES|wxDV_VERT_RULES );
	m_serverDataIdCol = m_serverDataView->AppendTextColumn( _("Identifier"), wxDATAVIEW_CELL_INERT, 100, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE );
	m_serverDataDescCol = m_serverDataView->AppendTextColumn( _("Description"), wxDATAVIEW_CELL_INERT, 200, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE );
	m_serverDataBlockedCol = m_serverDataView->AppendToggleColumn( _("Blocked"), wxDATAVIEW_CELL_ACTIVATABLE, 80, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE );
	m_serverDataPingCol = m_serverDataView->AppendTextColumn( _("Ping"), wxDATAVIEW_CELL_INERT, 80, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE );
	serverDataSizer->Add( m_serverDataView, 2, wxALL|wxEXPAND, 5 );


	m_serverListPanel->SetSizer( serverDataSizer );
	m_serverListPanel->Layout();
	serverDataSizer->Fit( m_serverListPanel );
	m_serverListSimplebook->AddPage( m_serverListPanel, _("a page"), false );
	m_emptyListPanel = new wxPanel( m_serverListSimplebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );


	bSizer10->Add( 0, 0, 1, wxEXPAND, 5 );

	m_emptyListLabel = new wxStaticText( m_emptyListPanel, wxID_ANY, _("Server list is not synced yet. Press the \"Sync server list\" button below to sync server list."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL );
	m_emptyListLabel->Wrap( -1 );
	bSizer10->Add( m_emptyListLabel, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_syncServersGauge = new wxGauge( m_emptyListPanel, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_syncServersGauge->SetValue( 0 );
	bSizer10->Add( m_syncServersGauge, 0, wxALL|wxEXPAND, 5 );


	bSizer10->Add( 0, 0, 1, wxEXPAND, 5 );


	m_emptyListPanel->SetSizer( bSizer10 );
	m_emptyListPanel->Layout();
	bSizer10->Fit( m_emptyListPanel );
	m_serverListSimplebook->AddPage( m_emptyListPanel, _("a page"), false );

	mainSizer->Add( m_serverListSimplebook, 1, wxEXPAND | wxALL, 0 );

	wxBoxSizer* bottomSizer;
	bottomSizer = new wxBoxSizer( wxHORIZONTAL );

	m_syncServersButton = new wxButton( m_mainPanel, wxID_ANY, _("Sync server list"), wxDefaultPosition, wxDefaultSize, 0 );
	bottomSizer->Add( m_syncServersButton, 0, wxALL, 5 );


	bottomSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_testPingButton = new wxButton( m_mainPanel, wxID_ANY, _("Test ping"), wxDefaultPosition, wxDefaultSize, 0 );
	m_testPingButton->Enable( false );
	m_testPingButton->SetToolTip( _("Show IP addresses and other detailed information on selected location") );

	bottomSizer->Add( m_testPingButton, 0, wxALL, 5 );

	m_blockAllButton = new wxButton( m_mainPanel, wxID_ANY, _("Block all"), wxDefaultPosition, wxDefaultSize, 0 );
	m_blockAllButton->Enable( false );
	m_blockAllButton->SetToolTip( _("Block selected locations") );

	bottomSizer->Add( m_blockAllButton, 0, wxALL, 5 );

	m_unblockAllButton = new wxButton( m_mainPanel, wxID_ANY, _("Unblock all"), wxDefaultPosition, wxDefaultSize, 0 );
	m_unblockAllButton->Enable( false );
	m_unblockAllButton->SetToolTip( _("Block selected locations") );

	bottomSizer->Add( m_unblockAllButton, 0, wxALL, 5 );


	mainSizer->Add( bottomSizer, 0, wxEXPAND, 5 );

	wxBoxSizer* infoSizer;
	infoSizer = new wxBoxSizer( wxHORIZONTAL );

	m_infoLabel = new wxStaticText( m_mainPanel, wxID_ANY, _("Deadlock Server Picker"), wxDefaultPosition, wxDefaultSize, 0 );
	m_infoLabel->Wrap( -1 );
	infoSizer->Add( m_infoLabel, 0, wxALL, 5 );


	infoSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	projectPageLink = new wxHyperlinkCtrl( m_mainPanel, wxID_ANY, _("GitHub Page"), wxT("https://github.com/tuorqai/dlserverpicker"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	infoSizer->Add( projectPageLink, 0, wxALL, 5 );


	mainSizer->Add( infoSizer, 0, wxEXPAND, 5 );


	m_mainPanel->SetSizer( mainSizer );
	m_mainPanel->Layout();
	mainSizer->Fit( m_mainPanel );
	topSizer->Add( m_mainPanel, 1, wxEXPAND | wxALL, 0 );


	this->SetSizer( topSizer );
	this->Layout();

	this->Centre( wxBOTH );
}

MainFrameLayout::~MainFrameLayout()
{
}
