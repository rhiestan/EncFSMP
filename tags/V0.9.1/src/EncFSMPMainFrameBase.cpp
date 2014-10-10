///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "CommonIncludes.h"

#include "EncFSMPMainFrameBase.h"

///////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( EncFSMPMainFrameBase, wxFrame )
	EVT_CLOSE( EncFSMPMainFrameBase::_wxFB_OnMainFrameClose )
	EVT_ICONIZE( EncFSMPMainFrameBase::_wxFB_OnMainFrameIconize )
	EVT_MENU( wxID_EXIT, EncFSMPMainFrameBase::_wxFB_OnExitMenuItem )
	EVT_MENU( ID_EXPORTMENUITEM, EncFSMPMainFrameBase::_wxFB_OnExportMenuItem )
	EVT_MENU( wxID_ABOUT, EncFSMPMainFrameBase::_wxFB_OnAboutMenuItem )
	EVT_BUTTON( ID_CREATEMOUNTBUTTON, EncFSMPMainFrameBase::_wxFB_OnCreateMountButton )
	EVT_BUTTON( ID_OPENEXISTINGENCFS, EncFSMPMainFrameBase::_wxFB_OnOpenExistingEncFSButton )
	EVT_LIST_COL_CLICK( ID_MOUNTSLISTCTRL, EncFSMPMainFrameBase::_wxFB_OnMountsListColClick )
	EVT_LIST_ITEM_DESELECTED( ID_MOUNTSLISTCTRL, EncFSMPMainFrameBase::_wxFB_OnMountsListItemDeselected )
	EVT_LIST_ITEM_SELECTED( ID_MOUNTSLISTCTRL, EncFSMPMainFrameBase::_wxFB_OnMountsListItemSelected )
	EVT_LIST_KEY_DOWN( ID_MOUNTSLISTCTRL, EncFSMPMainFrameBase::_wxFB_OnMountsListKeyDown )
	EVT_BUTTON( ID_MOUNTBUTTON, EncFSMPMainFrameBase::_wxFB_OnMountButton )
	EVT_BUTTON( ID_REMOVEBUTTON, EncFSMPMainFrameBase::_wxFB_OnRemoveButton )
	EVT_BUTTON( ID_EDITBUTTON, EncFSMPMainFrameBase::_wxFB_OnEditButton )
	EVT_BUTTON( ID_BROWSEBUTTON, EncFSMPMainFrameBase::_wxFB_OnBrowseButton )
	EVT_BUTTON( ID_INFOBUTTON, EncFSMPMainFrameBase::_wxFB_OnInfoButton )
	EVT_BUTTON( ID_CHANGEPASSWORDBUTTON, EncFSMPMainFrameBase::_wxFB_OnChangePasswordButton )
END_EVENT_TABLE()

EncFSMPMainFrameBase::EncFSMPMainFrameBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 300,200 ), wxDefaultSize );
	
	pMainMenuBar_ = new wxMenuBar( 0 );
	pFileMenu_ = new wxMenu();
	pExitMenuItem_ = new wxMenuItem( pFileMenu_, wxID_EXIT, wxString( wxT("Exit") ) , wxEmptyString, wxITEM_NORMAL );
	pFileMenu_->Append( pExitMenuItem_ );
	
	pMainMenuBar_->Append( pFileMenu_, wxT("File") ); 
	
	pToolsMenu_ = new wxMenu();
	pExportMenuItem_ = new wxMenuItem( pToolsMenu_, ID_EXPORTMENUITEM, wxString( wxT("Export") ) , wxEmptyString, wxITEM_NORMAL );
	pToolsMenu_->Append( pExportMenuItem_ );
	
	pMainMenuBar_->Append( pToolsMenu_, wxT("Tools") ); 
	
	pHelpMenu_ = new wxMenu();
	pHelpAboutMenuItem_ = new wxMenuItem( pHelpMenu_, wxID_ABOUT, wxString( wxT("About") ) , wxEmptyString, wxITEM_NORMAL );
	pHelpMenu_->Append( pHelpAboutMenuItem_ );
	
	pMainMenuBar_->Append( pHelpMenu_, wxT("Help") ); 
	
	this->SetMenuBar( pMainMenuBar_ );
	
	wxBoxSizer* pMainBoxSizer_;
	pMainBoxSizer_ = new wxBoxSizer( wxVERTICAL );
	
	pMainPanel_ = new wxPanel( this, ID_MAINPANEL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	wxGridSizer* gSizer2;
	gSizer2 = new wxGridSizer( 1, 2, 0, 0 );
	
	pCreateMountButton_ = new wxButton( pMainPanel_, ID_CREATEMOUNTBUTTON, wxT("Create new EncFS"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer2->Add( pCreateMountButton_, 0, wxALIGN_CENTER|wxALL, 3 );
	
	pOpenExistingEncFS_ = new wxButton( pMainPanel_, ID_OPENEXISTINGENCFS, wxT("Open existing EncFS"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer2->Add( pOpenExistingEncFS_, 0, wxALIGN_CENTER|wxALL, 3 );
	
	bSizer4->Add( gSizer2, 1, wxEXPAND, 5 );
	
	bSizer2->Add( bSizer4, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( pMainPanel_, wxID_ANY, wxT("Mounts") ), wxVERTICAL );
	
	pMountsListCtrl_ = new wxListCtrl( pMainPanel_, ID_MOUNTSLISTCTRL, wxDefaultPosition, wxDefaultSize, wxLC_NO_SORT_HEADER|wxLC_REPORT|wxLC_SINGLE_SEL );
	sbSizer1->Add( pMountsListCtrl_, 1, wxALL|wxEXPAND, 3 );
	
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 2, 3, 0, 0 );
	
	pMountButton_ = new wxButton( pMainPanel_, ID_MOUNTBUTTON, wxT("Mount"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( pMountButton_, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	pRemoveButton_ = new wxButton( pMainPanel_, ID_REMOVEBUTTON, wxT("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( pRemoveButton_, 1, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	pEditButton_ = new wxButton( pMainPanel_, ID_EDITBUTTON, wxT("Edit"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( pEditButton_, 1, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	pBrowseButton_ = new wxButton( pMainPanel_, ID_BROWSEBUTTON, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( pBrowseButton_, 1, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	pInfoButton_ = new wxButton( pMainPanel_, ID_INFOBUTTON, wxT("Show Info"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( pInfoButton_, 1, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	pChangePasswordButton_ = new wxButton( pMainPanel_, ID_CHANGEPASSWORDBUTTON, wxT("Change password"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( pChangePasswordButton_, 1, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	sbSizer1->Add( gSizer1, 0, wxEXPAND, 3 );
	
	bSizer2->Add( sbSizer1, 1, wxALL|wxEXPAND, 3 );
	
	pMainPanel_->SetSizer( bSizer2 );
	pMainPanel_->Layout();
	bSizer2->Fit( pMainPanel_ );
	pMainBoxSizer_->Add( pMainPanel_, 1, wxEXPAND | wxALL, 0 );
	
	this->SetSizer( pMainBoxSizer_ );
	this->Layout();
	pStatusBar_ = this->CreateStatusBar( 1, wxST_SIZEGRIP, ID_STATUSBAR );
	
	this->Centre( wxBOTH );
}

EncFSMPMainFrameBase::~EncFSMPMainFrameBase()
{
}

BEGIN_EVENT_TABLE( OpenExistingFSDialogBase, wxDialog )
	EVT_INIT_DIALOG( OpenExistingFSDialogBase::_wxFB_OnInitDialog )
	EVT_CHECKBOX( ID_STOREPASSWORDCHECKBOX, OpenExistingFSDialogBase::_wxFB_OnStorePasswordCheckBox )
	EVT_BUTTON( wxID_CANCEL, OpenExistingFSDialogBase::_wxFB_OnCancelButton )
	EVT_BUTTON( wxID_OK, OpenExistingFSDialogBase::_wxFB_OnOKButton )
END_EVENT_TABLE()

OpenExistingFSDialogBase::OpenExistingFSDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 200,120 ), wxDefaultSize );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	pFlexGridSizer_ = new wxFlexGridSizer( 8, 2, 0, 0 );
	pFlexGridSizer_->AddGrowableCol( 1 );
	pFlexGridSizer_->SetFlexibleDirection( wxBOTH );
	pFlexGridSizer_->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("Mount name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	pFlexGridSizer_->Add( m_staticText1, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pMountNameTextCtrl_ = new wxTextCtrl( this, ID_MOUNTNAMETEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pMountNameTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &mountName_ ) );
	pMountNameTextCtrl_->SetToolTip( wxT("Enter a unique, alphanumeric name for this mount") );
	
	pFlexGridSizer_->Add( pMountNameTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("EncFS path:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	pFlexGridSizer_->Add( m_staticText2, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pEncFSPathDirPicker_ = new wxDirPickerCtrl( this, ID_ENCFSPATHDIRPICKER, wxEmptyString, wxT("Select a folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE );
	pFlexGridSizer_->Add( pEncFSPathDirPicker_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	pDriveLetterStaticText_ = new wxStaticText( this, ID_DRIVELETTERSTATICTEXT, wxT("Drive letter:"), wxDefaultPosition, wxDefaultSize, 0 );
	pDriveLetterStaticText_->Wrap( -1 );
	pFlexGridSizer_->Add( pDriveLetterStaticText_, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	wxArrayString pDriveLetterChoice_Choices;
	pDriveLetterChoice_ = new wxChoice( this, ID_DRIVELETTERCHOICE, wxDefaultPosition, wxDefaultSize, pDriveLetterChoice_Choices, 0 );
	pDriveLetterChoice_->SetSelection( 0 );
	pFlexGridSizer_->Add( pDriveLetterChoice_, 0, wxALL, 3 );
	
	m_staticText30 = new wxStaticText( this, wxID_ANY, wxT("Store password:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText30->Wrap( -1 );
	pFlexGridSizer_->Add( m_staticText30, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 2 );
	
	pStorePasswordCheckBox_ = new wxCheckBox( this, ID_STOREPASSWORDCHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pStorePasswordCheckBox_->SetValidator( wxGenericValidator( &storePassword_ ) );
	pStorePasswordCheckBox_->SetToolTip( wxT("Stores the password in cleartext on this computer. This is a potential security risk!") );
	
	pFlexGridSizer_->Add( pStorePasswordCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText28 = new wxStaticText( this, wxID_ANY, wxT("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText28->Wrap( -1 );
	pFlexGridSizer_->Add( m_staticText28, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pPasswordTextCtrl_ = new wxTextCtrl( this, ID_PASSWORDTEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
	pPasswordTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &password_ ) );
	
	pFlexGridSizer_->Add( pPasswordTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText29 = new wxStaticText( this, wxID_ANY, wxT("Retype password:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText29->Wrap( -1 );
	pFlexGridSizer_->Add( m_staticText29, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pPasswordRetypeTextCtrl_ = new wxTextCtrl( this, ID_PASSWORDRETYPETEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
	pPasswordRetypeTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &passwordRetype_ ) );
	
	pFlexGridSizer_->Add( pPasswordRetypeTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	pWorldWritableStaticText_ = new wxStaticText( this, ID_WORLDWRITABLESTATICTEXT, wxT("World writable:"), wxDefaultPosition, wxDefaultSize, 0 );
	pWorldWritableStaticText_->Wrap( -1 );
	pFlexGridSizer_->Add( pWorldWritableStaticText_, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pWorldWritableCheckBox_ = new wxCheckBox( this, ID_WORLDWRITABLECHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pWorldWritableCheckBox_->SetValidator( wxGenericValidator( &worldWritable_ ) );
	pWorldWritableCheckBox_->SetToolTip( wxT("Make files and directories writable for users other than the owner") );
	
	pFlexGridSizer_->Add( pWorldWritableCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	pSystemVisibleStaticText_ = new wxStaticText( this, ID_SYSTEMVISIBLESTATICTEXT, wxT("System visible:"), wxDefaultPosition, wxDefaultSize, 0 );
	pSystemVisibleStaticText_->Wrap( -1 );
	pFlexGridSizer_->Add( pSystemVisibleStaticText_, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pSystemVisibleCheckBox_ = new wxCheckBox( this, ID_SYSTEMVISIBLECHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pSystemVisibleCheckBox_->SetValidator( wxGenericValidator( &systemVisible_ ) );
	pSystemVisibleCheckBox_->SetToolTip( wxT("Make system visible. Used for installations and sharing a folder via network. Needs administrative rights") );
	
	pFlexGridSizer_->Add( pSystemVisibleCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	bSizer4->Add( pFlexGridSizer_, 0, wxALL|wxEXPAND, 3 );
	
	
	bSizer4->Add( 0, 20, 1, wxEXPAND, 5 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	bSizer4->Add( m_sdbSizer1, 0, wxALL|wxEXPAND, 3 );
	
	this->SetSizer( bSizer4 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

OpenExistingFSDialogBase::~OpenExistingFSDialogBase()
{
}

BEGIN_EVENT_TABLE( CreateNewEncFSDialogBase, wxDialog )
	EVT_RADIOBOX( ID_ENCFSCONFIGURATIONRADIOBUTTON, CreateNewEncFSDialogBase::_wxFB_OnEncFSConfigurationRadioBox )
	EVT_CHOICE( ID_CIPHERALGORITHMCHOICE, CreateNewEncFSDialogBase::_wxFB_OnCipherAlgorithmChoice )
	EVT_BUTTON( wxID_CANCEL, CreateNewEncFSDialogBase::_wxFB_OnCancel )
	EVT_BUTTON( wxID_OK, CreateNewEncFSDialogBase::_wxFB_OnOK )
END_EVENT_TABLE()

CreateNewEncFSDialogBase::CreateNewEncFSDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );
	
	pFlexGridSizer_ = new wxFlexGridSizer( 8, 2, 0, 0 );
	pFlexGridSizer_->AddGrowableCol( 1 );
	pFlexGridSizer_->SetFlexibleDirection( wxBOTH );
	pFlexGridSizer_->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("Mount name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	pFlexGridSizer_->Add( m_staticText1, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pMountNameTextCtrl_ = new wxTextCtrl( this, ID_MOUNTNAMETEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pMountNameTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &mountName_ ) );
	pMountNameTextCtrl_->SetToolTip( wxT("Enter a unique, alphanumeric name for this mount") );
	
	pFlexGridSizer_->Add( pMountNameTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("EncFS path:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	pFlexGridSizer_->Add( m_staticText2, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pEncFSPathDirPicker_ = new wxDirPickerCtrl( this, ID_ENCFSPATHDIRPICKER, wxEmptyString, wxT("Select a folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE );
	pFlexGridSizer_->Add( pEncFSPathDirPicker_, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText25 = new wxStaticText( this, wxID_ANY, wxT("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText25->Wrap( -1 );
	pFlexGridSizer_->Add( m_staticText25, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pPasswordTextCtrl_ = new wxTextCtrl( this, ID_PASSWORDTEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
	pPasswordTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &password_ ) );
	
	pFlexGridSizer_->Add( pPasswordTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText26 = new wxStaticText( this, wxID_ANY, wxT("Retype Password:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText26->Wrap( -1 );
	pFlexGridSizer_->Add( m_staticText26, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pPasswordRetypeTextCtrl_ = new wxTextCtrl( this, ID_PASSWORDRETYPETEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
	pPasswordRetypeTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &passwordRetype_ ) );
	
	pFlexGridSizer_->Add( pPasswordRetypeTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText27 = new wxStaticText( this, wxID_ANY, wxT("Store password:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText27->Wrap( -1 );
	pFlexGridSizer_->Add( m_staticText27, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pStorePasswordCheckBox_ = new wxCheckBox( this, ID_STOREPASSWORDCHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pStorePasswordCheckBox_->SetValidator( wxGenericValidator( &storePassword_ ) );
	pStorePasswordCheckBox_->SetToolTip( wxT("Stores the password in cleartext on this computer. This is a potential security risk!") );
	
	pFlexGridSizer_->Add( pStorePasswordCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	pDriveLetterStaticText_ = new wxStaticText( this, ID_DRIVELETTERSTATICTEXT, wxT("Drive letter:"), wxDefaultPosition, wxDefaultSize, 0 );
	pDriveLetterStaticText_->Wrap( -1 );
	pFlexGridSizer_->Add( pDriveLetterStaticText_, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	wxArrayString pDriveLetterChoice_Choices;
	pDriveLetterChoice_ = new wxChoice( this, ID_DRIVELETTERCHOICE, wxDefaultPosition, wxDefaultSize, pDriveLetterChoice_Choices, 0 );
	pDriveLetterChoice_->SetSelection( 0 );
	pFlexGridSizer_->Add( pDriveLetterChoice_, 1, wxALL, 3 );
	
	pWorldWritableStaticText_ = new wxStaticText( this, ID_WORLDWRITABLESTATICTEXT, wxT("World writable:"), wxDefaultPosition, wxDefaultSize, 0 );
	pWorldWritableStaticText_->Wrap( -1 );
	pFlexGridSizer_->Add( pWorldWritableStaticText_, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pWorldWritableCheckBox_ = new wxCheckBox( this, ID_WORLDWRITABLECHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pWorldWritableCheckBox_->SetValidator( wxGenericValidator( &worldWritable_ ) );
	pWorldWritableCheckBox_->SetToolTip( wxT("Make files and directories writable for users other than the owner") );
	
	pFlexGridSizer_->Add( pWorldWritableCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	pSystemVisibleStaticText_ = new wxStaticText( this, ID_SYSTEMVISIBLESTATICTEXT, wxT("System visible:"), wxDefaultPosition, wxDefaultSize, 0 );
	pSystemVisibleStaticText_->Wrap( -1 );
	pFlexGridSizer_->Add( pSystemVisibleStaticText_, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pSystemVisibleCheckBox_ = new wxCheckBox( this, ID_SYSTEMVISIBLECHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pSystemVisibleCheckBox_->SetValidator( wxGenericValidator( &systemVisible_ ) );
	pSystemVisibleCheckBox_->SetToolTip( wxT("Make system visible. Used for installations and sharing a folder via network. Needs administrative rights") );
	
	pFlexGridSizer_->Add( pSystemVisibleCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	bSizer5->Add( pFlexGridSizer_, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("EncFS parameters") ), wxVERTICAL );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxHORIZONTAL );
	
	wxString pEncFSConfigurationRadioButton_Choices[] = { wxT("Standard"), wxT("Paranoia"), wxT("Expert") };
	int pEncFSConfigurationRadioButton_NChoices = sizeof( pEncFSConfigurationRadioButton_Choices ) / sizeof( wxString );
	pEncFSConfigurationRadioButton_ = new wxRadioBox( this, ID_ENCFSCONFIGURATIONRADIOBUTTON, wxT("Configuration"), wxDefaultPosition, wxDefaultSize, pEncFSConfigurationRadioButton_NChoices, pEncFSConfigurationRadioButton_Choices, 1, wxRA_SPECIFY_COLS );
	pEncFSConfigurationRadioButton_->SetSelection( 1 );
	bSizer6->Add( pEncFSConfigurationRadioButton_, 0, wxALL, 3 );
	
	
	bSizer6->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 9, 2, 0, 0 );
	fgSizer6->AddGrowableCol( 1 );
	fgSizer6->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText16 = new wxStaticText( this, wxID_ANY, wxT("Cipher algorithm:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText16->Wrap( -1 );
	fgSizer6->Add( m_staticText16, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	wxArrayString pCipherAlgorithmChoice_Choices;
	pCipherAlgorithmChoice_ = new wxChoice( this, ID_CIPHERALGORITHMCHOICE, wxDefaultPosition, wxDefaultSize, pCipherAlgorithmChoice_Choices, 0 );
	pCipherAlgorithmChoice_->SetSelection( 0 );
	fgSizer6->Add( pCipherAlgorithmChoice_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText17 = new wxStaticText( this, wxID_ANY, wxT("Cipher keysize:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText17->Wrap( -1 );
	fgSizer6->Add( m_staticText17, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	wxArrayString pCipherKeysizeChoice_Choices;
	pCipherKeysizeChoice_ = new wxChoice( this, ID_CIPHERKEYSIZECHOICE, wxDefaultPosition, wxDefaultSize, pCipherKeysizeChoice_Choices, 0 );
	pCipherKeysizeChoice_->SetSelection( 0 );
	fgSizer6->Add( pCipherKeysizeChoice_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText18 = new wxStaticText( this, wxID_ANY, wxT("Cipher blocksize:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText18->Wrap( -1 );
	fgSizer6->Add( m_staticText18, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	wxArrayString pCipherBlocksizeChoice_Choices;
	pCipherBlocksizeChoice_ = new wxChoice( this, ID_CIPHERBLOCKSIZECHOICE, wxDefaultPosition, wxDefaultSize, pCipherBlocksizeChoice_Choices, 0 );
	pCipherBlocksizeChoice_->SetSelection( 0 );
	fgSizer6->Add( pCipherBlocksizeChoice_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText19 = new wxStaticText( this, wxID_ANY, wxT("Name encoding:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText19->Wrap( -1 );
	fgSizer6->Add( m_staticText19, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	wxArrayString pNameEncodingChoice_Choices;
	pNameEncodingChoice_ = new wxChoice( this, ID_NAMEENCODINGCHOICE, wxDefaultPosition, wxDefaultSize, pNameEncodingChoice_Choices, 0 );
	pNameEncodingChoice_->SetSelection( 0 );
	fgSizer6->Add( pNameEncodingChoice_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText20 = new wxStaticText( this, wxID_ANY, wxT("Per-block HMAC:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText20->Wrap( -1 );
	fgSizer6->Add( m_staticText20, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pPerBlockHMACCheckBox_ = new wxCheckBox( this, ID_PERBLOCKHMACCHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pPerBlockHMACCheckBox_->SetValidator( wxGenericValidator( &perBlockHMAC_ ) );
	pPerBlockHMACCheckBox_->SetToolTip( wxT("Enable block authentication code headers on every block in a file?\nThis adds about 12 bytes per block to the storage requirements for a file, and significantly affects performance but it also means [almost] any modifications or errors within a block will be caught and will cause a read error.") );
	
	fgSizer6->Add( pPerBlockHMACCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText21 = new wxStaticText( this, wxID_ANY, wxT("Unique IV:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText21->Wrap( -1 );
	fgSizer6->Add( m_staticText21, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pUniqueIVCheckBox_ = new wxCheckBox( this, ID_UNIQUEIVCHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pUniqueIVCheckBox_->SetValidator( wxGenericValidator( &uniqueIV_ ) );
	pUniqueIVCheckBox_->SetToolTip( wxT("Enable per-file initialization vectors?\nThis adds about 8 bytes per file to the storage requirements. It should not affect performance except possibly with applications which rely on block-aligned file io for performance.") );
	
	fgSizer6->Add( pUniqueIVCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText22 = new wxStaticText( this, wxID_ANY, wxT("Chained IV:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText22->Wrap( -1 );
	fgSizer6->Add( m_staticText22, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pChainedIVCheckBox_ = new wxCheckBox( this, ID_CHAINEDIVCHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pChainedIVCheckBox_->SetValidator( wxGenericValidator( &chainedIV_ ) );
	pChainedIVCheckBox_->SetToolTip( wxT("Enable filename initialization vector chaining?\nThis makes filename encoding dependent on the complete path, rather then encoding each path element individually.") );
	
	fgSizer6->Add( pChainedIVCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText23 = new wxStaticText( this, wxID_ANY, wxT("External IV:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText23->Wrap( -1 );
	fgSizer6->Add( m_staticText23, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pExternalIVCheckBox_ = new wxCheckBox( this, ID_EXTERNALIVCHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pExternalIVCheckBox_->SetValidator( wxGenericValidator( &externalIV_ ) );
	pExternalIVCheckBox_->SetToolTip( wxT("Enable filename to IV header chaining?\nThis makes file data encoding dependent on the complete file path. If a file is renamed, it will not decode sucessfully unless it was renamed by encfs with the proper key. If this option is enabled, then hard links will not be supported in the filesystem.") );
	
	fgSizer6->Add( pExternalIVCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText24 = new wxStaticText( this, wxID_ANY, wxT("Key derivation duration (ms):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText24->Wrap( -1 );
	fgSizer6->Add( m_staticText24, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	wxArrayString pKeyDerivationDurationChoice_Choices;
	pKeyDerivationDurationChoice_ = new wxChoice( this, ID_KEYDERIVATIONDURATIONCHOICE, wxDefaultPosition, wxDefaultSize, pKeyDerivationDurationChoice_Choices, 0 );
	pKeyDerivationDurationChoice_->SetSelection( 0 );
	fgSizer6->Add( pKeyDerivationDurationChoice_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	bSizer6->Add( fgSizer6, 0, wxEXPAND, 5 );
	
	sbSizer2->Add( bSizer6, 1, wxEXPAND, 5 );
	
	bSizer5->Add( sbSizer2, 0, wxALL|wxEXPAND, 3 );
	
	
	bSizer5->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_sdbSizer2 = new wxStdDialogButtonSizer();
	m_sdbSizer2OK = new wxButton( this, wxID_OK );
	m_sdbSizer2->AddButton( m_sdbSizer2OK );
	m_sdbSizer2Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer2->AddButton( m_sdbSizer2Cancel );
	m_sdbSizer2->Realize();
	bSizer5->Add( m_sdbSizer2, 0, wxALL|wxEXPAND, 3 );
	
	this->SetSizer( bSizer5 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

CreateNewEncFSDialogBase::~CreateNewEncFSDialogBase()
{
}

ShowEncFSInfoDialog::ShowEncFSInfoDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 300,300 ), wxDefaultSize );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("EncFS Info") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 13, 2, 0, 0 );
	fgSizer4->AddGrowableCol( 1 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText26 = new wxStaticText( this, wxID_ANY, wxT("Mount name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText26->Wrap( -1 );
	fgSizer4->Add( m_staticText26, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pMountNameTextCtrl_ = new wxTextCtrl( this, ID_MOUNTNAMETEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	pMountNameTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &mountName_ ) );
	
	fgSizer4->Add( pMountNameTextCtrl_, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText27 = new wxStaticText( this, wxID_ANY, wxT("EncFS path:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText27->Wrap( -1 );
	fgSizer4->Add( m_staticText27, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pEncFSPathTextCtrl_ = new wxTextCtrl( this, ID_ENCFSPATHTEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	pEncFSPathTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &encFSPath_ ) );
	
	fgSizer4->Add( pEncFSPathTextCtrl_, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText28 = new wxStaticText( this, wxID_ANY, wxT("EncFS config version:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText28->Wrap( -1 );
	fgSizer4->Add( m_staticText28, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pEncFSConfigVersionTextCtrl_ = new wxTextCtrl( this, ID_ENCFSCONFIGVERSIONTEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	pEncFSConfigVersionTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &configVersion_ ) );
	
	fgSizer4->Add( pEncFSConfigVersionTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText29 = new wxStaticText( this, wxID_ANY, wxT("Cipher algorithm:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText29->Wrap( -1 );
	fgSizer4->Add( m_staticText29, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pCipherAlgorithmTextCtrl_ = new wxTextCtrl( this, ID_CIPHERALGORITHMTEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	pCipherAlgorithmTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &cipherAlgorithm_ ) );
	
	fgSizer4->Add( pCipherAlgorithmTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText30 = new wxStaticText( this, wxID_ANY, wxT("Cipher keysize:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText30->Wrap( -1 );
	fgSizer4->Add( m_staticText30, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pCipherKeySizeTextCtrl_ = new wxTextCtrl( this, ID_CIPHERKEYSIZETEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	pCipherKeySizeTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &cipherKeySize_ ) );
	
	fgSizer4->Add( pCipherKeySizeTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText36 = new wxStaticText( this, wxID_ANY, wxT("Cipher blocksize:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText36->Wrap( -1 );
	fgSizer4->Add( m_staticText36, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pCipherBlocksizeTextCtrl_ = new wxTextCtrl( this, ID_CIPHERBLOCKSIZETEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	pCipherBlocksizeTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &cipherBlocksize_ ) );
	
	fgSizer4->Add( pCipherBlocksizeTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText37 = new wxStaticText( this, wxID_ANY, wxT("Name encoding:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText37->Wrap( -1 );
	fgSizer4->Add( m_staticText37, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pNameEncodingTextCtrl_ = new wxTextCtrl( this, ID_NAMEENCODINGTEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	pNameEncodingTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &nameEncoding_ ) );
	
	fgSizer4->Add( pNameEncodingTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText38 = new wxStaticText( this, wxID_ANY, wxT("PBKDF2 iterations:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText38->Wrap( -1 );
	fgSizer4->Add( m_staticText38, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pKeyDerivIterationsTextCtrl_ = new wxTextCtrl( this, ID_KEYDERIVITERATIONSTEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	pKeyDerivIterationsTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &keyDerivIterations_ ) );
	
	fgSizer4->Add( pKeyDerivIterationsTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText39 = new wxStaticText( this, wxID_ANY, wxT("Salt size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText39->Wrap( -1 );
	fgSizer4->Add( m_staticText39, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pSaltSizeTextCtrl_ = new wxTextCtrl( this, ID_SALTSIZETEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	pSaltSizeTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &saltSize_ ) );
	
	fgSizer4->Add( pSaltSizeTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText40 = new wxStaticText( this, wxID_ANY, wxT("Per-block HMAC:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText40->Wrap( -1 );
	fgSizer4->Add( m_staticText40, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pPerBlockHMACCheckBox_ = new wxCheckBox( this, ID_PERBLOCKHMACCHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pPerBlockHMACCheckBox_->SetValidator( wxGenericValidator( &perBlockHMAC_ ) );
	
	fgSizer4->Add( pPerBlockHMACCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText41 = new wxStaticText( this, wxID_ANY, wxT("Unique IV:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText41->Wrap( -1 );
	fgSizer4->Add( m_staticText41, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pUniqueIVCheckBox_ = new wxCheckBox( this, ID_UNIQUEIVCHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pUniqueIVCheckBox_->SetValidator( wxGenericValidator( &uniqueIV_ ) );
	
	fgSizer4->Add( pUniqueIVCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText42 = new wxStaticText( this, wxID_ANY, wxT("Chained IV:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText42->Wrap( -1 );
	fgSizer4->Add( m_staticText42, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pChainedIVCheckBox_ = new wxCheckBox( this, ID_CHAINEDIVCHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pChainedIVCheckBox_->SetValidator( wxGenericValidator( &chainedIV_ ) );
	
	fgSizer4->Add( pChainedIVCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText43 = new wxStaticText( this, wxID_ANY, wxT("External IV:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText43->Wrap( -1 );
	fgSizer4->Add( m_staticText43, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pExternalIVCheckBox_ = new wxCheckBox( this, ID_EXTERNALIVCHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pExternalIVCheckBox_->SetValidator( wxGenericValidator( &externalIV_ ) );
	
	fgSizer4->Add( pExternalIVCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	sbSizer3->Add( fgSizer4, 1, wxEXPAND, 5 );
	
	bSizer7->Add( sbSizer3, 1, wxALL|wxEXPAND, 3 );
	
	m_sdbSizer3 = new wxStdDialogButtonSizer();
	m_sdbSizer3OK = new wxButton( this, wxID_OK );
	m_sdbSizer3->AddButton( m_sdbSizer3OK );
	m_sdbSizer3->Realize();
	bSizer7->Add( m_sdbSizer3, 0, wxALL|wxEXPAND, 3 );
	
	this->SetSizer( bSizer7 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

ShowEncFSInfoDialog::~ShowEncFSInfoDialog()
{
}

BEGIN_EVENT_TABLE( ChangePasswordDialogBase, wxDialog )
	EVT_BUTTON( wxID_OK, ChangePasswordDialogBase::_wxFB_OnOKButtonClick )
END_EVENT_TABLE()

ChangePasswordDialogBase::ChangePasswordDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 4, 2, 0, 0 );
	fgSizer5->AddGrowableCol( 1 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText44 = new wxStaticText( this, wxID_ANY, wxT("Old password:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText44->Wrap( -1 );
	fgSizer5->Add( m_staticText44, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pOldPasswordTextCtrl_ = new wxTextCtrl( this, ID_OLDPASSWORDTEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
	pOldPasswordTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &oldPassword_ ) );
	
	fgSizer5->Add( pOldPasswordTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText45 = new wxStaticText( this, wxID_ANY, wxT("New password:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText45->Wrap( -1 );
	fgSizer5->Add( m_staticText45, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pNewPasswordTextCtrl_ = new wxTextCtrl( this, ID_NEWPASSWORDTEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
	pNewPasswordTextCtrl_->SetValidator( wxTextValidator( wxFILTER_NONE, &newPassword_ ) );
	
	fgSizer5->Add( pNewPasswordTextCtrl_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText46 = new wxStaticText( this, wxID_ANY, wxT("Retype new password:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText46->Wrap( -1 );
	fgSizer5->Add( m_staticText46, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pRetypeNewPassword_ = new wxTextCtrl( this, ID_RETYPENEWPASSWORD, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
	pRetypeNewPassword_->SetValidator( wxTextValidator( wxFILTER_NONE, &retypeNewPassword_ ) );
	
	fgSizer5->Add( pRetypeNewPassword_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText47 = new wxStaticText( this, wxID_ANY, wxT("Store new password:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText47->Wrap( -1 );
	fgSizer5->Add( m_staticText47, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	pStoreNewPasswordCheckBox_ = new wxCheckBox( this, ID_PSTORENEWPASSWORDCHECKBOX_, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	pStoreNewPasswordCheckBox_->SetValidator( wxGenericValidator( &storeNewPassword_ ) );
	pStoreNewPasswordCheckBox_->SetToolTip( wxT("Stores the password in cleartext on this computer. This is a potential security risk!") );
	
	fgSizer5->Add( pStoreNewPasswordCheckBox_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	bSizer8->Add( fgSizer5, 1, wxEXPAND, 5 );
	
	m_sdbSizer4 = new wxStdDialogButtonSizer();
	m_sdbSizer4OK = new wxButton( this, wxID_OK );
	m_sdbSizer4->AddButton( m_sdbSizer4OK );
	m_sdbSizer4Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer4->AddButton( m_sdbSizer4Cancel );
	m_sdbSizer4->Realize();
	bSizer8->Add( m_sdbSizer4, 0, wxALL|wxEXPAND, 3 );
	
	this->SetSizer( bSizer8 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

ChangePasswordDialogBase::~ChangePasswordDialogBase()
{
}
