///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __EncFSMPMainFrameBase__
#define __EncFSMPMainFrameBase__

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/statbox.h>
#include <wx/panel.h>
#include <wx/statusbr.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>
#include <wx/filepicker.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/valgen.h>
#include <wx/dialog.h>
#include <wx/radiobox.h>

///////////////////////////////////////////////////////////////////////////

#define ID_ENCFSMPMAINFRAMEBASE 1000
#define ID_MAINMENUBAR 1001
#define ID_EXPORTMENUITEM 1002
#define ID_MAINPANEL 1003
#define ID_CREATEMOUNTBUTTON 1004
#define ID_OPENEXISTINGENCFS 1005
#define ID_MOUNTSLISTCTRL 1006
#define ID_MOUNTBUTTON 1007
#define ID_REMOVEBUTTON 1008
#define ID_EDITBUTTON 1009
#define ID_BROWSEBUTTON 1010
#define ID_INFOBUTTON 1011
#define ID_CHANGEPASSWORDBUTTON 1012
#define ID_STATUSBAR 1013
#define ID_OPENEXISTINGFSDIALOG 1014
#define ID_MOUNTNAMETEXTCTRL 1015
#define ID_ENCFSPATHDIRPICKER 1016
#define ID_DRIVELETTERSTATICTEXT 1017
#define ID_DRIVELETTERCHOICE 1018
#define ID_STOREPASSWORDCHECKBOX 1019
#define ID_PASSWORDTEXTCTRL 1020
#define ID_PASSWORDRETYPETEXTCTRL 1021
#define ID_WORLDWRITABLESTATICTEXT 1022
#define ID_WORLDWRITABLECHECKBOX 1023
#define ID_SYSTEMVISIBLESTATICTEXT 1024
#define ID_SYSTEMVISIBLECHECKBOX 1025
#define ID_CREATENEWENCFSDIALOG 1026
#define ID_ENCFSCONFIGURATIONRADIOBUTTON 1027
#define ID_CIPHERALGORITHMCHOICE 1028
#define ID_CIPHERKEYSIZECHOICE 1029
#define ID_CIPHERBLOCKSIZECHOICE 1030
#define ID_NAMEENCODINGCHOICE 1031
#define ID_PERBLOCKHMACCHECKBOX 1032
#define ID_UNIQUEIVCHECKBOX 1033
#define ID_CHAINEDIVCHECKBOX 1034
#define ID_EXTERNALIVCHECKBOX 1035
#define ID_KEYDERIVATIONDURATIONCHOICE 1036
#define ID_SHOWENCFSINFODIALOG 1037
#define ID_ENCFSPATHTEXTCTRL 1038
#define ID_ENCFSCONFIGVERSIONTEXTCTRL 1039
#define ID_CIPHERALGORITHMTEXTCTRL 1040
#define ID_CIPHERKEYSIZETEXTCTRL 1041
#define ID_CIPHERBLOCKSIZETEXTCTRL 1042
#define ID_NAMEENCODINGTEXTCTRL 1043
#define ID_KEYDERIVITERATIONSTEXTCTRL 1044
#define ID_SALTSIZETEXTCTRL 1045
#define ID_CHANGEPASSWORDDIALOGBASE 1046
#define ID_OLDPASSWORDTEXTCTRL 1047
#define ID_NEWPASSWORDTEXTCTRL 1048
#define ID_RETYPENEWPASSWORD 1049
#define ID_PSTORENEWPASSWORDCHECKBOX_ 1050

///////////////////////////////////////////////////////////////////////////////
/// Class EncFSMPMainFrameBase
///////////////////////////////////////////////////////////////////////////////
class EncFSMPMainFrameBase : public wxFrame 
{
	DECLARE_EVENT_TABLE()
	private:
		
		// Private event handlers
		void _wxFB_OnMainFrameClose( wxCloseEvent& event ){ OnMainFrameClose( event ); }
		void _wxFB_OnMainFrameIconize( wxIconizeEvent& event ){ OnMainFrameIconize( event ); }
		void _wxFB_OnExitMenuItem( wxCommandEvent& event ){ OnExitMenuItem( event ); }
		void _wxFB_OnExportMenuItem( wxCommandEvent& event ){ OnExportMenuItem( event ); }
		void _wxFB_OnAboutMenuItem( wxCommandEvent& event ){ OnAboutMenuItem( event ); }
		void _wxFB_OnCreateMountButton( wxCommandEvent& event ){ OnCreateMountButton( event ); }
		void _wxFB_OnOpenExistingEncFSButton( wxCommandEvent& event ){ OnOpenExistingEncFSButton( event ); }
		void _wxFB_OnMountsListColClick( wxListEvent& event ){ OnMountsListColClick( event ); }
		void _wxFB_OnMountsListItemDeselected( wxListEvent& event ){ OnMountsListItemDeselected( event ); }
		void _wxFB_OnMountsListItemSelected( wxListEvent& event ){ OnMountsListItemSelected( event ); }
		void _wxFB_OnMountsListKeyDown( wxListEvent& event ){ OnMountsListKeyDown( event ); }
		void _wxFB_OnMountButton( wxCommandEvent& event ){ OnMountButton( event ); }
		void _wxFB_OnRemoveButton( wxCommandEvent& event ){ OnRemoveButton( event ); }
		void _wxFB_OnEditButton( wxCommandEvent& event ){ OnEditButton( event ); }
		void _wxFB_OnBrowseButton( wxCommandEvent& event ){ OnBrowseButton( event ); }
		void _wxFB_OnInfoButton( wxCommandEvent& event ){ OnInfoButton( event ); }
		void _wxFB_OnChangePasswordButton( wxCommandEvent& event ){ OnChangePasswordButton( event ); }
		
	
	protected:
		wxMenuBar* pMainMenuBar_;
		wxMenu* pFileMenu_;
		wxMenuItem* pExitMenuItem_;
		wxMenu* pToolsMenu_;
		wxMenuItem* pExportMenuItem_;
		wxMenu* pHelpMenu_;
		wxMenuItem* pHelpAboutMenuItem_;
		wxPanel* pMainPanel_;
		wxButton* pCreateMountButton_;
		wxButton* pOpenExistingEncFS_;
		wxListCtrl* pMountsListCtrl_;
		wxButton* pMountButton_;
		wxButton* pRemoveButton_;
		wxButton* pEditButton_;
		wxButton* pBrowseButton_;
		wxButton* pInfoButton_;
		wxButton* pChangePasswordButton_;
		wxStatusBar* pStatusBar_;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnMainFrameClose( wxCloseEvent& event ) = 0;
		virtual void OnMainFrameIconize( wxIconizeEvent& event ) = 0;
		virtual void OnExitMenuItem( wxCommandEvent& event ) = 0;
		virtual void OnExportMenuItem( wxCommandEvent& event ) = 0;
		virtual void OnAboutMenuItem( wxCommandEvent& event ) = 0;
		virtual void OnCreateMountButton( wxCommandEvent& event ) = 0;
		virtual void OnOpenExistingEncFSButton( wxCommandEvent& event ) = 0;
		virtual void OnMountsListColClick( wxListEvent& event ) = 0;
		virtual void OnMountsListItemDeselected( wxListEvent& event ) = 0;
		virtual void OnMountsListItemSelected( wxListEvent& event ) = 0;
		virtual void OnMountsListKeyDown( wxListEvent& event ) = 0;
		virtual void OnMountButton( wxCommandEvent& event ) = 0;
		virtual void OnRemoveButton( wxCommandEvent& event ) = 0;
		virtual void OnEditButton( wxCommandEvent& event ) = 0;
		virtual void OnBrowseButton( wxCommandEvent& event ) = 0;
		virtual void OnInfoButton( wxCommandEvent& event ) = 0;
		virtual void OnChangePasswordButton( wxCommandEvent& event ) = 0;
		
	
	public:
		
		EncFSMPMainFrameBase( wxWindow* parent, wxWindowID id = ID_ENCFSMPMAINFRAMEBASE, const wxString& title = wxT("EncFS MP"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~EncFSMPMainFrameBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class OpenExistingFSDialogBase
///////////////////////////////////////////////////////////////////////////////
class OpenExistingFSDialogBase : public wxDialog 
{
	DECLARE_EVENT_TABLE()
	private:
		
		// Private event handlers
		void _wxFB_OnInitDialog( wxInitDialogEvent& event ){ OnInitDialog( event ); }
		void _wxFB_OnStorePasswordCheckBox( wxCommandEvent& event ){ OnStorePasswordCheckBox( event ); }
		void _wxFB_OnCancelButton( wxCommandEvent& event ){ OnCancelButton( event ); }
		void _wxFB_OnOKButton( wxCommandEvent& event ){ OnOKButton( event ); }
		
	
	protected:
		wxFlexGridSizer* pFlexGridSizer_;
		wxStaticText* m_staticText1;
		wxStaticText* m_staticText2;
		wxDirPickerCtrl* pEncFSPathDirPicker_;
		wxStaticText* pDriveLetterStaticText_;
		wxChoice* pDriveLetterChoice_;
		wxStaticText* m_staticText30;
		wxStaticText* m_staticText28;
		wxStaticText* m_staticText29;
		wxTextCtrl* pPasswordRetypeTextCtrl_;
		wxStaticText* pWorldWritableStaticText_;
		wxStaticText* pSystemVisibleStaticText_;
		
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnInitDialog( wxInitDialogEvent& event ) = 0;
		virtual void OnStorePasswordCheckBox( wxCommandEvent& event ) = 0;
		virtual void OnCancelButton( wxCommandEvent& event ) = 0;
		virtual void OnOKButton( wxCommandEvent& event ) = 0;
		
	
	public:
		wxTextCtrl* pMountNameTextCtrl_;
		wxCheckBox* pStorePasswordCheckBox_;
		wxTextCtrl* pPasswordTextCtrl_;
		wxCheckBox* pWorldWritableCheckBox_;
		wxCheckBox* pSystemVisibleCheckBox_;
		wxString mountName_; 
		bool storePassword_; 
		wxString password_; 
		wxString passwordRetype_; 
		bool worldWritable_; 
		bool systemVisible_; 
		
		OpenExistingFSDialogBase( wxWindow* parent, wxWindowID id = ID_OPENEXISTINGFSDIALOG, const wxString& title = wxT("Open existing EncFS"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 343,324 ), long style = wxDEFAULT_DIALOG_STYLE );
		~OpenExistingFSDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class CreateNewEncFSDialogBase
///////////////////////////////////////////////////////////////////////////////
class CreateNewEncFSDialogBase : public wxDialog 
{
	DECLARE_EVENT_TABLE()
	private:
		
		// Private event handlers
		void _wxFB_OnEncFSConfigurationRadioBox( wxCommandEvent& event ){ OnEncFSConfigurationRadioBox( event ); }
		void _wxFB_OnCipherAlgorithmChoice( wxCommandEvent& event ){ OnCipherAlgorithmChoice( event ); }
		void _wxFB_OnCancel( wxCommandEvent& event ){ OnCancel( event ); }
		void _wxFB_OnOK( wxCommandEvent& event ){ OnOK( event ); }
		
	
	protected:
		wxFlexGridSizer* pFlexGridSizer_;
		wxStaticText* m_staticText1;
		wxStaticText* m_staticText2;
		wxDirPickerCtrl* pEncFSPathDirPicker_;
		wxStaticText* m_staticText25;
		wxStaticText* m_staticText26;
		wxTextCtrl* pPasswordRetypeTextCtrl_;
		wxStaticText* m_staticText27;
		wxStaticText* pDriveLetterStaticText_;
		wxChoice* pDriveLetterChoice_;
		wxStaticText* pWorldWritableStaticText_;
		wxStaticText* pSystemVisibleStaticText_;
		wxRadioBox* pEncFSConfigurationRadioButton_;
		
		wxStaticText* m_staticText16;
		wxChoice* pCipherAlgorithmChoice_;
		wxStaticText* m_staticText17;
		wxChoice* pCipherKeysizeChoice_;
		wxStaticText* m_staticText18;
		wxChoice* pCipherBlocksizeChoice_;
		wxStaticText* m_staticText19;
		wxChoice* pNameEncodingChoice_;
		wxStaticText* m_staticText20;
		wxStaticText* m_staticText21;
		wxStaticText* m_staticText22;
		wxCheckBox* pChainedIVCheckBox_;
		wxStaticText* m_staticText23;
		wxCheckBox* pExternalIVCheckBox_;
		wxStaticText* m_staticText24;
		wxChoice* pKeyDerivationDurationChoice_;
		
		wxStdDialogButtonSizer* m_sdbSizer2;
		wxButton* m_sdbSizer2OK;
		wxButton* m_sdbSizer2Cancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnEncFSConfigurationRadioBox( wxCommandEvent& event ) = 0;
		virtual void OnCipherAlgorithmChoice( wxCommandEvent& event ) = 0;
		virtual void OnCancel( wxCommandEvent& event ) = 0;
		virtual void OnOK( wxCommandEvent& event ) = 0;
		
	
	public:
		wxTextCtrl* pMountNameTextCtrl_;
		wxTextCtrl* pPasswordTextCtrl_;
		wxCheckBox* pStorePasswordCheckBox_;
		wxCheckBox* pWorldWritableCheckBox_;
		wxCheckBox* pSystemVisibleCheckBox_;
		wxCheckBox* pPerBlockHMACCheckBox_;
		wxCheckBox* pUniqueIVCheckBox_;
		wxString mountName_; 
		wxString password_; 
		wxString passwordRetype_; 
		bool storePassword_; 
		bool worldWritable_; 
		bool systemVisible_; 
		bool perBlockHMAC_; 
		bool uniqueIV_; 
		bool chainedIV_; 
		bool externalIV_; 
		
		CreateNewEncFSDialogBase( wxWindow* parent, wxWindowID id = ID_CREATENEWENCFSDIALOG, const wxString& title = wxT("Create New EncFS"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 475,587 ), long style = wxDEFAULT_DIALOG_STYLE );
		~CreateNewEncFSDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ShowEncFSInfoDialog
///////////////////////////////////////////////////////////////////////////////
class ShowEncFSInfoDialog : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText26;
		wxStaticText* m_staticText27;
		wxStaticText* m_staticText28;
		wxStaticText* m_staticText29;
		wxStaticText* m_staticText30;
		wxStaticText* m_staticText36;
		wxStaticText* m_staticText37;
		wxStaticText* m_staticText38;
		wxStaticText* m_staticText39;
		wxStaticText* m_staticText40;
		wxStaticText* m_staticText41;
		wxStaticText* m_staticText42;
		wxStaticText* m_staticText43;
		wxStdDialogButtonSizer* m_sdbSizer3;
		wxButton* m_sdbSizer3OK;
	
	public:
		wxTextCtrl* pMountNameTextCtrl_;
		wxTextCtrl* pEncFSPathTextCtrl_;
		wxTextCtrl* pEncFSConfigVersionTextCtrl_;
		wxTextCtrl* pCipherAlgorithmTextCtrl_;
		wxTextCtrl* pCipherKeySizeTextCtrl_;
		wxTextCtrl* pCipherBlocksizeTextCtrl_;
		wxTextCtrl* pNameEncodingTextCtrl_;
		wxTextCtrl* pKeyDerivIterationsTextCtrl_;
		wxTextCtrl* pSaltSizeTextCtrl_;
		wxCheckBox* pPerBlockHMACCheckBox_;
		wxCheckBox* pUniqueIVCheckBox_;
		wxCheckBox* pChainedIVCheckBox_;
		wxCheckBox* pExternalIVCheckBox_;
		wxString mountName_; 
		wxString encFSPath_; 
		wxString configVersion_; 
		wxString cipherAlgorithm_; 
		wxString cipherKeySize_; 
		wxString cipherBlocksize_; 
		wxString nameEncoding_; 
		wxString keyDerivIterations_; 
		wxString saltSize_; 
		bool perBlockHMAC_; 
		bool uniqueIV_; 
		bool chainedIV_; 
		bool externalIV_; 
		
		ShowEncFSInfoDialog( wxWindow* parent, wxWindowID id = ID_SHOWENCFSINFODIALOG, const wxString& title = wxT("EncFS Info"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 516,453 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
		~ShowEncFSInfoDialog();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ChangePasswordDialogBase
///////////////////////////////////////////////////////////////////////////////
class ChangePasswordDialogBase : public wxDialog 
{
	DECLARE_EVENT_TABLE()
	private:
		
		// Private event handlers
		void _wxFB_OnOKButtonClick( wxCommandEvent& event ){ OnOKButtonClick( event ); }
		
	
	protected:
		wxStaticText* m_staticText44;
		wxStaticText* m_staticText45;
		wxStaticText* m_staticText46;
		wxTextCtrl* pRetypeNewPassword_;
		wxStaticText* m_staticText47;
		wxStdDialogButtonSizer* m_sdbSizer4;
		wxButton* m_sdbSizer4OK;
		wxButton* m_sdbSizer4Cancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnOKButtonClick( wxCommandEvent& event ) = 0;
		
	
	public:
		wxTextCtrl* pOldPasswordTextCtrl_;
		wxTextCtrl* pNewPasswordTextCtrl_;
		wxCheckBox* pStoreNewPasswordCheckBox_;
		wxString oldPassword_; 
		wxString newPassword_; 
		wxString retypeNewPassword_; 
		bool storeNewPassword_; 
		
		ChangePasswordDialogBase( wxWindow* parent, wxWindowID id = ID_CHANGEPASSWORDDIALOGBASE, const wxString& title = wxT("Change password"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 360,204 ), long style = wxDEFAULT_DIALOG_STYLE );
		~ChangePasswordDialogBase();
	
};

#endif //__EncFSMPMainFrameBase__
