//
// This file is part of the aMule Project.
//
// Copyright (c) 2003-2008 aMule Team ( admin@amule.org / http://www.amule.org )
// Copyright (c) 2002 Merkur ( devs@emule-project.net / http://www.emule-project.net )
//
// Any parts of this program derived from the xMule, lMule or eMule project,
// or contributed by third-party developers are copyrighted by their
// respective authors.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA
//

#include <protocol/ed2k/ClientSoftware.h>
#include <common/MenuIDs.h>

#include <common/Format.h>	// Needed for CFormat
#include "amule.h"		// Needed for theApp
#include "amuleDlg.h"		// Needed for CamuleDlg
#include "BarShader.h"		// Needed for CBarShader
#include "ClientDetailDialog.h"	// Needed for CClientDetailDialog
#include "ChatWnd.h"		// Needed for CChatWnd
#include "CommentDialogLst.h"	// Needed for CCommentDialogLst
#include "DownloadListCtrl.h"	// Interface declarations
#include "DataToText.h"		// Needed for PriorityToStr
#include "FileDetailDialog.h"	// Needed for CFileDetailDialog
#include "GuiEvents.h"		// Needed for CoreNotify_*
#ifdef ENABLE_IP2COUNTRY
	#include "IP2Country.h"	// Needed for IP2Country
#endif
#include "Logger.h"
#include "muuli_wdr.h"		// Needed for ID_DLOADLIST
#include "PartFile.h"		// Needed for CPartFile
#include "Preferences.h"
#include "SharedFileList.h"	// Needed for CSharedFileList
#include "TerminationProcess.h"	// Needed for CTerminationProcess
#include "updownclient.h"	// Needed for CUpDownClient


class CPartFile;


struct CtrlItem_Struct
{
	CtrlItem_Struct()
		: dwUpdated(0),
		  status(NULL),
		  m_owner(NULL),
		  m_fileValue(NULL),
		  m_sourceValue(NULL),
		  m_type(FILE_TYPE)
	{ }
	
	~CtrlItem_Struct() {
		delete status;
	}

	DownloadItemType GetType() const {
		return m_type;
	}

	CPartFile* GetOwner() const {
		return m_owner;
	}

	CPartFile* GetFile() const {
		return m_fileValue;
	}

	CUpDownClient* GetSource() const {
		return m_sourceValue;
	}

	void SetContents(CPartFile* file) {
		m_owner = NULL;
		m_fileValue = file;
		m_sourceValue = NULL;
		m_owner = NULL;
		m_type = FILE_TYPE;
	}

	void SetContents(CPartFile* owner, CUpDownClient* source, DownloadItemType type) {
		wxCHECK_RET(type != FILE_TYPE, wxT("Invalid type, not a source"));
		
		m_owner = owner;
		m_fileValue = NULL;
		m_sourceValue = source;
		m_type = type;
	}
	
	
	uint32		dwUpdated;
	wxBitmap*	status;

private:
	CPartFile*			m_owner;
	CPartFile*			m_fileValue;
	CUpDownClient*		m_sourceValue;
	DownloadItemType	m_type;
};



#define m_ImageList theApp->amuledlg->m_imagelist


BEGIN_EVENT_TABLE(CDownloadListCtrl, CMuleListCtrl)
	EVT_LIST_ITEM_ACTIVATED(ID_DLOADLIST,	CDownloadListCtrl::OnItemActivated)
	EVT_LIST_ITEM_RIGHT_CLICK(ID_DLOADLIST, CDownloadListCtrl::OnMouseRightClick)
	EVT_LIST_ITEM_MIDDLE_CLICK(ID_DLOADLIST, CDownloadListCtrl::OnMouseMiddleClick)

	EVT_CHAR( CDownloadListCtrl::OnKeyPressed )

	EVT_MENU( MP_CANCEL, 			CDownloadListCtrl::OnCancelFile )
	
	EVT_MENU( MP_PAUSE,			CDownloadListCtrl::OnSetStatus )
	EVT_MENU( MP_STOP,			CDownloadListCtrl::OnSetStatus )
	EVT_MENU( MP_RESUME,			CDownloadListCtrl::OnSetStatus )
	
	EVT_MENU( MP_PRIOLOW,			CDownloadListCtrl::OnSetPriority )
	EVT_MENU( MP_PRIONORMAL,		CDownloadListCtrl::OnSetPriority )
	EVT_MENU( MP_PRIOHIGH,			CDownloadListCtrl::OnSetPriority )
	EVT_MENU( MP_PRIOAUTO,			CDownloadListCtrl::OnSetPriority )

	EVT_MENU( MP_SWAP_A4AF_TO_THIS,		CDownloadListCtrl::OnSwapSources )
	EVT_MENU( MP_SWAP_A4AF_TO_THIS_AUTO,	CDownloadListCtrl::OnSwapSources )
	EVT_MENU( MP_SWAP_A4AF_TO_ANY_OTHER,	CDownloadListCtrl::OnSwapSources )

	EVT_MENU_RANGE( MP_ASSIGNCAT, MP_ASSIGNCAT + 99, CDownloadListCtrl::OnSetCategory )

	EVT_MENU( MP_CLEARCOMPLETED,		CDownloadListCtrl::OnClearCompleted )

	EVT_MENU( MP_GETMAGNETLINK,		CDownloadListCtrl::OnGetLink )
	EVT_MENU( MP_GETED2KLINK,		CDownloadListCtrl::OnGetLink )

	EVT_MENU( MP_METINFO,			CDownloadListCtrl::OnViewFileInfo )
	EVT_MENU( MP_VIEW,			CDownloadListCtrl::OnPreviewFile )
	EVT_MENU( MP_VIEWFILECOMMENTS,		CDownloadListCtrl::OnViewFileComments )

	EVT_MENU( MP_WS,			CDownloadListCtrl::OnGetFeedback )
	EVT_MENU( MP_RAZORSTATS, 		CDownloadListCtrl::OnGetRazorStats )

	EVT_MENU( MP_CHANGE2FILE,		CDownloadListCtrl::OnSwapSource )
	EVT_MENU( MP_SHOWLIST,			CDownloadListCtrl::OnViewFiles )
	EVT_MENU( MP_ADDFRIEND,			CDownloadListCtrl::OnAddFriend )
	EVT_MENU( MP_SENDMESSAGE,		CDownloadListCtrl::OnSendMessage )
	EVT_MENU( MP_DETAIL,			CDownloadListCtrl::OnViewClientInfo )
END_EVENT_TABLE()



//! This listtype is used when gathering the selected items.
typedef std::list<CtrlItem_Struct*>	ItemList;



CDownloadListCtrl::CDownloadListCtrl(
	wxWindow *parent, wxWindowID winid, const wxPoint& pos, const wxSize& size,
	long style, const wxValidator& validator, const wxString& name )
:
CMuleListCtrl( parent, winid, pos, size, style | wxLC_OWNERDRAW, validator, name )
{
	// Setting the sorter function.
	SetSortFunc( SortProc );

	// Set the table-name (for loading and saving preferences).
	SetTableName( wxT("Download") );

	m_menu = NULL;

	m_hilightBrush  = CMuleColour(wxSYS_COLOUR_HIGHLIGHT).Blend(125).GetBrush();

	m_hilightUnfocusBrush = CMuleColour(wxSYS_COLOUR_BTNSHADOW).Blend(125).GetBrush();

	InsertColumn( ColumnPart,			_("Part"),					wxLIST_FORMAT_LEFT,  30, wxT("a") );
	InsertColumn( ColumnFileName,		_("File Name"),				wxLIST_FORMAT_LEFT, 260, wxT("N") );
	InsertColumn( ColumnSize,			_("Size"),					wxLIST_FORMAT_LEFT,  60, wxT("Z") );
	InsertColumn( ColumnTransferred,	_("Transferred"),			wxLIST_FORMAT_LEFT,  65, wxT("T") );
	InsertColumn( ColumnCompleted,		_("Completed"),				wxLIST_FORMAT_LEFT,  65, wxT("C") );
	InsertColumn( ColumnSpeed,			_("Speed"),					wxLIST_FORMAT_LEFT,  65, wxT("S") );
	InsertColumn( ColumnProgress,		_("Progress"),				wxLIST_FORMAT_LEFT, 170, wxT("P") );
	InsertColumn( ColumnSources,		_("Sources"),				wxLIST_FORMAT_LEFT,  50, wxT("u") );
	InsertColumn( ColumnPriority,		_("Priority"),				wxLIST_FORMAT_LEFT,  55, wxT("p") );
	InsertColumn( ColumnStatus,			_("Status"),				wxLIST_FORMAT_LEFT,  70, wxT("s") );
	InsertColumn( ColumnTimeRemaining,  _("Time Remaining"),		wxLIST_FORMAT_LEFT, 110, wxT("r") );
	InsertColumn( ColumnLastSeenComplete, _("Last Seen Complete"),	wxLIST_FORMAT_LEFT, 220, wxT("c") );
	InsertColumn( ColumnLastReception,	_("Last Reception"),		wxLIST_FORMAT_LEFT, 220, wxT("R") );

	m_category = 0;
	m_completedFiles = 0;
	m_filecount = 0;
	LoadSettings();
}

// This is the order the columns had before extendable list-control settings save/load code was introduced.
// Don't touch when inserting new columns!
wxString CDownloadListCtrl::GetOldColumnOrder() const
{
	return wxT("N,Z,T,C,S,P,u,p,s,r,c,R");
}

CDownloadListCtrl::~CDownloadListCtrl()
{
	while ( !m_ListItems.empty() ) {
		delete m_ListItems.begin()->second;
		m_ListItems.erase( m_ListItems.begin() );
	}
}


void CDownloadListCtrl::AddFile( CPartFile* file )
{
	wxASSERT( file );
	
	// Avoid duplicate entries of files
	if ( m_ListItems.find( file ) == m_ListItems.end() ) {
		CtrlItem_Struct* newitem = new CtrlItem_Struct;
		newitem->SetContents(file);
	
		m_ListItems.insert( ListItemsPair( file, newitem ) );
		
		// Check if the new file is visible in the current category
		if ( file->CheckShowItemInGivenCat( m_category ) ) {
			ShowFile( file, true );
			SortList();
		}
	}
}


void CDownloadListCtrl::AddSource(CPartFile* owner, CUpDownClient* source, DownloadItemType type)
{
	wxCHECK_RET(owner, wxT("NULL owner in CDownloadListCtrl::AddSource"));
	wxCHECK_RET(source, wxT("NULL source in CDownloadListCtrl::AddSource"));
	wxCHECK_RET(type != FILE_TYPE, wxT("Invalid type, not a source"));

	// Update the other instances of this source
	bool bFound = false;
	ListIteratorPair rangeIt = m_ListItems.equal_range(source);
	for ( ListItems::iterator it = rangeIt.first; it != rangeIt.second; ++it ) {
		CtrlItem_Struct* cur_item = it->second;

		// Check if this source has been already added to this file => to be sure
		if ( cur_item->GetOwner() == owner ) {
			// Update this instance with its new setting
			cur_item->SetContents(owner, source, type);
			cur_item->dwUpdated = 0;
			bFound = true;
		} else if ( type == AVAILABLE_SOURCE ) {
			// The state 'Available' is exclusive
			cur_item->SetContents(cur_item->GetOwner(), source, A4AF_SOURCE);
			cur_item->dwUpdated = 0;
		}
	}

	if ( bFound ) {
		return;
	}

	if ( owner->ShowSources() ) {
		CtrlItem_Struct* newitem = new CtrlItem_Struct;
		newitem->SetContents(owner, source, type);
		
		m_ListItems.insert( ListItemsPair(source, newitem) );

		// Find the owner-object
		ListItems::iterator it = m_ListItems.find( owner );
	
		if ( it != m_ListItems.end() ) {
			long item = FindItem( -1, reinterpret_cast<wxUIntPtr>(it->second) );
			
			if ( item > -1 ) {
				item = InsertItem( item + 1, wxEmptyString );
				
				SetItemPtrData( item, reinterpret_cast<wxUIntPtr>(newitem) );

				// background.. this should be in a function
				wxListItem listitem;
				listitem.m_itemId = item;

				listitem.SetBackgroundColour( GetBackgroundColour() );
	
				SetItem( listitem );
			}
		}
	}
}


void CDownloadListCtrl::RemoveSource( const CUpDownClient* source, const CPartFile* owner )
{
	wxASSERT( source );
	
	// Retrieve all entries matching the source
	ListIteratorPair rangeIt = m_ListItems.equal_range(source);
	
	for ( ListItems::iterator it = rangeIt.first; it != rangeIt.second; ) {
		ListItems::iterator tmp = it++;
		
		CtrlItem_Struct* item = tmp->second;
		if ( owner == NULL || owner == item->GetOwner() ) {
			// Remove it from the m_ListItems
			m_ListItems.erase( tmp );

			long index = FindItem( -1, reinterpret_cast<wxUIntPtr>(item) );
			
			if ( index > -1 ) {
				DeleteItem( index );
			}
			
			delete item;
		}
	}
}


void CDownloadListCtrl::RemoveFile( CPartFile* file )
{
	wxASSERT( file );
	
	// Ensure that any assosiated sources and list-entries are removed
	ShowFile( file, false );

	// Find the assosiated list-item
	ListItems::iterator it = m_ListItems.find( file );

	if ( it != m_ListItems.end() ) {
		delete it->second;

		m_ListItems.erase( it );
	}
}


void CDownloadListCtrl::UpdateItem(const void* toupdate)
{
	// Retrieve all entries matching the source
	ListIteratorPair rangeIt = m_ListItems.equal_range( toupdate );

	// Visible lines, default to all because not all platforms
	// support the GetVisibleLines function
	long first = 0, last = GetItemCount();

#ifndef __WXMSW__
	// Get visible lines if we need them
	if ( rangeIt.first != rangeIt.second ) {
		GetVisibleLines( &first, &last );
	}
#endif
	
	for ( ListItems::iterator it = rangeIt.first; it != rangeIt.second; ++it ) {
		CtrlItem_Struct* item = it->second;

		long index = FindItem( -1, reinterpret_cast<wxUIntPtr>(item) );

		// Determine if the file should be shown in the current category
		if ( item->GetType() == FILE_TYPE ) {
			CPartFile* file = item->GetFile();
		
			bool show = file->CheckShowItemInGivenCat( m_category );
	
			if ( index > -1 ) {
				if ( show ) {
					item->dwUpdated = 0;

					// Only update visible lines
					if ( index >= first && index <= last) {
						RefreshItem( index );
					}
				} else {
					// Item should no longer be shown in
					// the current category
					ShowFile( file, false );
				}
			} else if ( show ) {
				// Item has been hidden but new status means
				// that it should it should be shown in the
				// current category
				ShowFile( file, true );
			}

			if (file->GetStatus() == PS_COMPLETE) {
				m_completedFiles = true;

				CastByID(ID_BTNCLRCOMPL, GetParent(), wxButton)->Enable(true);
			}
		} else {
			item->dwUpdated = 0;

			// Only update visible lines
			if ( index >= first && index <= last) {
				RefreshItem( index );
			}
		}
	}
}


void CDownloadListCtrl::ShowFile( CPartFile* file, bool show )
{
	wxASSERT( file );
	
	ListItems::iterator it = m_ListItems.find( file );
	
	if ( it != m_ListItems.end() ) { 
		CtrlItem_Struct* item = it->second;
	
		if ( show ) {
			// Check if the file is already being displayed
			long index = FindItem( -1, reinterpret_cast<wxUIntPtr>(item) );
			if ( index == -1 ) {	
				long newitem = InsertItem( GetItemCount(), wxEmptyString );
				
				SetItemPtrData( newitem, reinterpret_cast<wxUIntPtr>(item) );

				wxListItem myitem;
				myitem.m_itemId = newitem;
				myitem.SetBackgroundColour( GetBackgroundColour() );
				
				SetItem(myitem);	
			
				RefreshItem( newitem );

				ShowFilesCount( 1 );
			}
		} else {
			// Ensure sources are hidden
			ShowSources( file, false );

			// Try to find the file and remove it
			long index = FindItem( -1, reinterpret_cast<wxUIntPtr>(item) );
			if ( index > -1 ) {
				DeleteItem( index );
				ShowFilesCount( -1 );
			}
		}
	}
}


void CDownloadListCtrl::ShowSources( CPartFile* file, bool show )
{
	// Check if the current state is the same as the new state
	if ( file->ShowSources() == show ) {
		return;
	}
	
	Freeze();
	
	file->SetShowSources( show );
	
	if ( show ) {
		const CPartFile::SourceSet& normSources = file->GetSourceList();
		const CPartFile::SourceSet& a4afSources = file->GetA4AFList();
			
		// Adding normal sources
		CPartFile::SourceSet::const_iterator it;
		for ( it = normSources.begin(); it != normSources.end(); ++it ) {
			switch ((*it)->GetDownloadState()) {
				case DS_DOWNLOADING:
				case DS_ONQUEUE:
					AddSource( file, *it, AVAILABLE_SOURCE );
				default:
					// Any other state
					AddSource( file, *it, UNAVAILABLE_SOURCE );
			}
			
		}

		// Adding A4AF sources
		for ( it = a4afSources.begin(); it != a4afSources.end(); ++it ) {
			AddSource( file, *it, A4AF_SOURCE );
		}

		SortList();
	} else {
		for ( int i = GetItemCount() - 1; i >= 0; --i ) {
			CtrlItem_Struct* item = (CtrlItem_Struct*)GetItemData(i);
		
			if ( item->GetType() != FILE_TYPE && item->GetOwner() == file ) {
				// Remove from the grand list, this call doesn't remove the source
				// from the listctrl, because ShowSources is now false. This also
				// deletes the item.
				RemoveSource(item->GetSource(), file);
			}
		}
	}
	
	Thaw();
}


void CDownloadListCtrl::ChangeCategory( int newCategory )
{
	Freeze();

	// remove all displayed files with a different cat and show the correct ones
	for (ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++) {
		const CtrlItem_Struct *cur_item = it->second;
		
		if ( cur_item->GetType() == FILE_TYPE ) {
			CPartFile* file = cur_item->GetFile();
	
			bool curVisibility = file->CheckShowItemInGivenCat( m_category );
			bool newVisibility = file->CheckShowItemInGivenCat( newCategory );
		
			// Check if the visibility of the file has changed. However, if the
			// current category is the default (0) category, then we can't use
			// curVisiblity to see if the visibility has changed but instead
			// have to let ShowFile() check if the file is or isn't on the list.
			if ( curVisibility != newVisibility || !newCategory ) {
				ShowFile( file, newVisibility );
			}
		}
	}
	
	Thaw();

	m_category = newCategory;
}


uint8 CDownloadListCtrl::GetCategory() const
{
	return m_category;
}


/*
 *
 */
const int itFILES = 1;
const int itSOURCES = 2;

/**
 * Helper-function: This function is used to gather selected items.
 *
 * @param list A pointer to the list to gather items from.
 * @param types The desired types OR'd together.
 * @return A list containing the selected items of the choosen types.
 */
ItemList GetSelectedItems( CDownloadListCtrl* list, int types )
{
	ItemList results;

	long index = list->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	
	while ( index > -1 ) {
		CtrlItem_Struct* item = (CtrlItem_Struct*)list->GetItemData( index );

		bool add = false;
		add |= ( item->GetType() == FILE_TYPE ) && ( types & itFILES );
		add |= ( item->GetType() != FILE_TYPE ) && ( types & itSOURCES );
		
		if ( add ) {
			results.push_back( item );
		}
		
		index = list->GetNextItem( index, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	}

	return results;
}


void CDownloadListCtrl::OnCancelFile(wxCommandEvent& WXUNUSED(event))
{
	ItemList files = ::GetSelectedItems(this, itFILES);
	if (files.size()) {	
		wxString question;
		if (files.size() == 1) {
			question = _("Are you sure that you wish to delete the selected file?");
		} else {
			question = _("Are you sure that you wish to delete the selected files?");
		}
		if (wxMessageBox( question, _("Cancel"), wxICON_QUESTION | wxYES_NO, this) == wxYES) {
			for (ItemList::iterator it = files.begin(); it != files.end(); ++it) {
				CPartFile* file = (*it)->GetFile();
				if (file) {
					switch (file->GetStatus()) {
					case PS_WAITINGFORHASH:
					case PS_HASHING:
					case PS_COMPLETING:
					case PS_COMPLETE:
						break;
					default:
						CoreNotify_PartFile_Delete(file);
					}
				}
			}
		}
	}
}


void CDownloadListCtrl::OnSetPriority( wxCommandEvent& event )
{
	int priority = 0;
	switch ( event.GetId() ) {
		case MP_PRIOLOW:	priority = PR_LOW;	break;
		case MP_PRIONORMAL:	priority = PR_NORMAL;	break;
		case MP_PRIOHIGH:	priority = PR_HIGH;	break;
		case MP_PRIOAUTO:	priority = PR_AUTO;	break;
		default:
			wxASSERT( false );
	}

	ItemList files = ::GetSelectedItems( this, itFILES );

	for ( ItemList::iterator it = files.begin(); it != files.end(); ++it ) {
		CPartFile* file = (*it)->GetFile();
	
		if ( priority == PR_AUTO ) {
			CoreNotify_PartFile_PrioAuto( file, true );
		} else {
			CoreNotify_PartFile_PrioAuto( file, false );

			CoreNotify_PartFile_PrioSet( file, priority, true );
		}
	}
}


void CDownloadListCtrl::OnSwapSources( wxCommandEvent& event )
{
	ItemList files = ::GetSelectedItems( this, itFILES );

	for ( ItemList::iterator it = files.begin(); it != files.end(); ++it ) {
		CPartFile* file = (*it)->GetFile();

		switch ( event.GetId() ) {
			case MP_SWAP_A4AF_TO_THIS:
				CoreNotify_PartFile_Swap_A4AF( file );
				break;
				
			case MP_SWAP_A4AF_TO_THIS_AUTO:
				CoreNotify_PartFile_Swap_A4AF_Auto( file );
				break;
				
			case MP_SWAP_A4AF_TO_ANY_OTHER:
				CoreNotify_PartFile_Swap_A4AF_Others( file );
				break;
		}
	}
}


void CDownloadListCtrl::OnSetCategory( wxCommandEvent& event )
{
	ItemList files = ::GetSelectedItems( this, itFILES );

	for ( ItemList::iterator it = files.begin(); it != files.end(); ++it ) {
		CPartFile* file = (*it)->GetFile();

		CoreNotify_PartFile_SetCat( file, event.GetId() - MP_ASSIGNCAT );
	}

	ChangeCategory( m_category );
}


void CDownloadListCtrl::OnSetStatus( wxCommandEvent& event )
{
	ItemList files = ::GetSelectedItems( this, itFILES );

	for ( ItemList::iterator it = files.begin(); it != files.end(); ++it ) {
		CPartFile* file = (*it)->GetFile();

		switch ( event.GetId() ) {	
			case MP_PAUSE:
				CoreNotify_PartFile_Pause( file );
				break;
				
			case MP_RESUME:
				CoreNotify_PartFile_Resume( file );
				break;

			case MP_STOP:
				ShowSources(file, false);
				CoreNotify_PartFile_Stop( file );
				break;
		}
	}
}


void CDownloadListCtrl::OnClearCompleted( wxCommandEvent& WXUNUSED(event) )
{
	ClearCompleted();
}


void CDownloadListCtrl::OnGetLink(wxCommandEvent& event)
{
	ItemList files = ::GetSelectedItems( this, itFILES );

	wxString URIs;

	for ( ItemList::iterator it = files.begin(); it != files.end(); ++it ) {
		CPartFile* file = (*it)->GetFile();

		if ( event.GetId() == MP_GETED2KLINK ) {
			URIs += theApp->CreateED2kLink( file ) + wxT("\n");
		} else {
			URIs += theApp->CreateMagnetLink( file ) + wxT("\n");
		}
	}

	if ( !URIs.IsEmpty() ) {
		theApp->CopyTextToClipboard( URIs.BeforeLast(wxT('\n')) );
	}
}


void CDownloadListCtrl::OnGetFeedback(wxCommandEvent& WXUNUSED(event))
{
	wxString feed;
	ItemList files = ::GetSelectedItems(this, itFILES);

	for (ItemList::iterator it = files.begin(); it != files.end(); ++it) {
		if (feed.IsEmpty()) {
			feed = CFormat(_("Feedback from: %s (%s)\n\n")) % thePrefs::GetUserNick() % GetFullMuleVersion();
		} else {
			feed += wxT("\n");
		}
		feed += (*it)->GetFile()->GetFeedback();
	}

	if (!feed.IsEmpty()) {
		theApp->CopyTextToClipboard(feed);
	}
}


void CDownloadListCtrl::OnGetRazorStats( wxCommandEvent& WXUNUSED(event) )
{
	ItemList files = ::GetSelectedItems( this, itFILES );

	if ( files.size() == 1 ) {
		CPartFile* file = files.front()->GetFile();

		theApp->amuledlg->LaunchUrl(
			wxT("http://stats.razorback2.com/ed2khistory?ed2k=") +
			file->GetFileHash().Encode());
	}
}


void CDownloadListCtrl::OnViewFileInfo( wxCommandEvent& WXUNUSED(event) )
{
	ItemList files = ::GetSelectedItems( this, itFILES );

	if ( files.size() == 1 ) {
		CPartFile* file = files.front()->GetFile();

		CFileDetailDialog dialog( this, file );
		dialog.ShowModal();
	}
}


void CDownloadListCtrl::OnViewFileComments( wxCommandEvent& WXUNUSED(event) )
{
	ItemList files = ::GetSelectedItems( this, itFILES );

	if ( files.size() == 1 ) {
		CPartFile* file = files.front()->GetFile();

		CCommentDialogLst dialog( this, file );
		dialog.ShowModal();
	}
}


void CDownloadListCtrl::OnPreviewFile( wxCommandEvent& WXUNUSED(event) )
{
	ItemList files = ::GetSelectedItems( this, itFILES );

	if ( files.size() == 1 ) {
		PreviewFile(files.front()->GetFile());
	}
}

void CDownloadListCtrl::OnSwapSource( wxCommandEvent& WXUNUSED(event) )
{
	ItemList sources = ::GetSelectedItems( this, itSOURCES );

	for ( ItemList::iterator it = sources.begin(); it != sources.end(); ++it ) {
		CPartFile* file = (*it)->GetOwner();
		CUpDownClient* source = (*it)->GetSource();

		source->SwapToAnotherFile( true, false, false, file );
	}
}


void CDownloadListCtrl::OnViewFiles( wxCommandEvent& WXUNUSED(event) )
{
	ItemList sources = ::GetSelectedItems( this, itSOURCES );

	if ( sources.size() == 1 ) {
		CUpDownClient* source = sources.front()->GetSource();
		
		source->RequestSharedFileList();
	}
}


void CDownloadListCtrl::OnAddFriend( wxCommandEvent& WXUNUSED(event) )
{
	ItemList sources = ::GetSelectedItems( this, itSOURCES );

	for ( ItemList::iterator it = sources.begin(); it != sources.end(); ++it ) {
		CUpDownClient* client = (*it)->GetSource();
		if (client->IsFriend()) {
			theApp->amuledlg->m_chatwnd->RemoveFriend(client->GetUserHash(), client->GetIP(), client->GetUserPort());
		} else {
			theApp->amuledlg->m_chatwnd->AddFriend( client );
		}
	}
}


void CDownloadListCtrl::OnSendMessage( wxCommandEvent& WXUNUSED(event) )
{
	ItemList sources = ::GetSelectedItems( this, itSOURCES );

	if ( sources.size() == 1 ) {
		CUpDownClient* source = (sources.front())->GetSource();

		// These values are cached, since calling wxGetTextFromUser will
		// start an event-loop, in which the client may be deleted.
		wxString userName = source->GetUserName();
		uint64 userID = GUI_ID(source->GetIP(), source->GetUserPort());
		
		wxString message = ::wxGetTextFromUser(_("Send message to user"),
			_("Message to send:"));
		if ( !message.IsEmpty() ) {
			theApp->amuledlg->m_chatwnd->SendMessage(message, userName, userID);
		}
	}
}


void CDownloadListCtrl::OnViewClientInfo( wxCommandEvent& WXUNUSED(event) )
{
	ItemList sources = ::GetSelectedItems( this, itSOURCES );

	if ( sources.size() == 1 ) {
		CUpDownClient* source = (sources.front())->GetSource();

		CClientDetailDialog dialog( this, source );
		dialog.ShowModal();
	}
}


void CDownloadListCtrl::OnItemActivated( wxListEvent& evt )
{
	CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData( evt.GetIndex() );
	
	if ( content->GetType() == FILE_TYPE ) {
		CPartFile* file = content->GetFile();

		if ((!file->IsPartFile() || file->GetStatus() == PS_COMPLETE) && file->PreviewAvailable()) {
			PreviewFile( file );
		} else {
			ShowSources( file, !file->ShowSources() );
		}
		
	}
}


void CDownloadListCtrl::OnMouseRightClick(wxListEvent& evt)
{
	long index = CheckSelection(evt);
	if (index < 0) {
		return;
	}	
	
	delete m_menu;
	m_menu = NULL;

	CtrlItem_Struct* item = (CtrlItem_Struct*)GetItemData( index );
	if (item->GetType() == FILE_TYPE) {
		m_menu = new wxMenu( _("Downloads") );

		wxMenu* priomenu = new wxMenu();
		priomenu->AppendCheckItem(MP_PRIOLOW, _("Low"));
		priomenu->AppendCheckItem(MP_PRIONORMAL, _("Normal"));
		priomenu->AppendCheckItem(MP_PRIOHIGH, _("High"));
		priomenu->AppendCheckItem(MP_PRIOAUTO, _("Auto"));

		m_menu->Append(MP_MENU_PRIO, _("Priority"), priomenu);
		m_menu->Append(MP_CANCEL, _("Cancel"));
		m_menu->Append(MP_STOP, _("&Stop"));
		m_menu->Append(MP_PAUSE, _("&Pause"));
		m_menu->Append(MP_RESUME, _("&Resume"));
		m_menu->Append(MP_CLEARCOMPLETED, _("C&lear completed"));
		//-----------------------------------------------------
		m_menu->AppendSeparator();
		//-----------------------------------------------------
		wxMenu* extendedmenu = new wxMenu();
		extendedmenu->Append(MP_SWAP_A4AF_TO_THIS,
			_("Swap every A4AF to this file now"));
		extendedmenu->AppendCheckItem(MP_SWAP_A4AF_TO_THIS_AUTO,
			_("Swap every A4AF to this file (Auto)"));
		//-----------------------------------------------------
		extendedmenu->AppendSeparator();
		//-----------------------------------------------------
		extendedmenu->Append(MP_SWAP_A4AF_TO_ANY_OTHER,
			_("Swap every A4AF to any other file now"));
		//-----------------------------------------------------
		m_menu->Append(MP_MENU_EXTD,
			_("Extended Options"), extendedmenu);
		//-----------------------------------------------------
		m_menu->AppendSeparator();
		//-----------------------------------------------------
/* Commented out till RB2 is back 
		m_menu->Append( MP_RAZORSTATS,
			_("Get Razorback 2's stats for this file"));
		//-----------------------------------------------------
		m_menu->AppendSeparator();
		//-----------------------------------------------------
*/
		m_menu->Append(MP_VIEW, _("Preview"));
		m_menu->Append(MP_METINFO, _("Show file &details"));
		m_menu->Append(MP_VIEWFILECOMMENTS,
			_("Show all comments"));
		//-----------------------------------------------------
		m_menu->AppendSeparator();
		//-----------------------------------------------------
		m_menu->Append(MP_GETMAGNETLINK,
			_("Copy magnet URI to clipboard"));
		m_menu->Append(MP_GETED2KLINK,
			_("Copy eD2k &link to clipboard"));
		m_menu->Append(MP_WS,
			_("Copy feedback to clipboard"));
		//-----------------------------------------------------
		m_menu->AppendSeparator();
		//-----------------------------------------------------	
		// Add dinamic entries
		wxMenu *cats = new wxMenu(_("Category"));
		if (theApp->glob_prefs->GetCatCount() > 1) {
			for (uint32 i = 0; i < theApp->glob_prefs->GetCatCount(); i++) {
				if ( i == 0 ) {
					cats->Append( MP_ASSIGNCAT, _("unassign") );
				} else {
					cats->Append( MP_ASSIGNCAT + i,
						theApp->glob_prefs->GetCategory(i)->title );
				}
			}
		}
		m_menu->Append(MP_MENU_CATS, _("Assign to category"), cats);
		m_menu->Enable(MP_MENU_CATS, (theApp->glob_prefs->GetCatCount() > 1) );

		CPartFile* file = item->GetFile();
		// then set state
		bool canStop;
		bool canPause;
		bool canCancel;
		bool fileResumable;
		if (file->GetStatus(true) != PS_ALLOCATING) {
			const uint8_t fileStatus = file->GetStatus();
			canStop =
				(fileStatus != PS_ERROR) &&
				(fileStatus != PS_COMPLETE) &&
				(file->IsStopped() != true);
			canPause = (file->GetStatus() != PS_PAUSED) && canStop;
			fileResumable =
				(fileStatus == PS_PAUSED) ||
				(fileStatus == PS_ERROR) ||
				(fileStatus == PS_INSUFFICIENT);
			canCancel = fileStatus != PS_COMPLETE;
		} else {
			canStop = canPause = canCancel = fileResumable = false;
		}

		wxMenu* menu = m_menu;
		menu->Enable( MP_CANCEL,	canCancel );
		menu->Enable( MP_PAUSE,		canPause );
		menu->Enable( MP_STOP,		canStop );
		menu->Enable( MP_RESUME, 	fileResumable );
		menu->Enable( MP_CLEARCOMPLETED, m_completedFiles );

		wxString view;
		if (file->IsPartFile() && (file->GetStatus() != PS_COMPLETE)) {
			view = CFormat(wxT("%s [%s]")) % _("Preview")
					% file->GetPartMetFileName().RemoveExt();
		} else if ( file->GetStatus() == PS_COMPLETE ) {
			view = _("&Open the file");
		}
		menu->SetLabel(MP_VIEW, view);
		menu->Enable(MP_VIEW, file->PreviewAvailable() );

		menu->Check(  MP_SWAP_A4AF_TO_THIS_AUTO, 	file->IsA4AFAuto() );

		int priority = file->IsAutoDownPriority() ?
			PR_AUTO : file->GetDownPriority();
		
		priomenu->Check( MP_PRIOHIGH,	priority == PR_HIGH );
		priomenu->Check( MP_PRIONORMAL, priority == PR_NORMAL );
		priomenu->Check( MP_PRIOLOW,	priority == PR_LOW );
		priomenu->Check( MP_PRIOAUTO,	priority == PR_AUTO );

		menu->Enable( MP_MENU_EXTD, canPause );
	
		PopupMenu(m_menu, evt.GetPoint());

	} else {
		CUpDownClient* client = item->GetSource();
		
		m_menu = new wxMenu(wxT("Clients"));
		m_menu->Append(MP_DETAIL, _("Show &Details"));
		m_menu->Append(MP_ADDFRIEND, client->IsFriend() ? _("Remove from friends") : _("Add to Friends"));
		m_menu->Append(MP_SHOWLIST, _("View Files"));
		m_menu->Append(MP_SENDMESSAGE, _("Send message"));
		m_menu->Append(MP_CHANGE2FILE, _("Swap to this file"));
		
		// Only enable the Swap option for A4AF sources
		m_menu->Enable(MP_CHANGE2FILE, (item->GetType() == A4AF_SOURCE));
		// We need a valid IP if we are to message the client
		m_menu->Enable(MP_SENDMESSAGE, (client->GetIP() != 0));
		
		m_menu->Enable(MP_SHOWLIST, !client->HasDisabledSharedFiles());
		
		PopupMenu(m_menu, evt.GetPoint());
					
	}
	
	delete m_menu;
	m_menu = NULL;
	
}


void CDownloadListCtrl::OnMouseMiddleClick(wxListEvent& evt)
{
	// Check if clicked item is selected. If not, unselect all and select it.
	long index = CheckSelection(evt);
	if ( index < 0 ) {
		return;
	}

	CtrlItem_Struct* item = (CtrlItem_Struct*)GetItemData( index );
	
	if ( item->GetType() == FILE_TYPE ) {
		CFileDetailDialog(this, item->GetFile()).ShowModal();
	} else {
		CClientDetailDialog(this, item->GetSource()).ShowModal();
	}
}


void CDownloadListCtrl::OnKeyPressed( wxKeyEvent& event )
{
	// Check if delete was pressed
	switch (event.GetKeyCode()) {
		case WXK_NUMPAD_DELETE:
		case WXK_DELETE: {
			wxCommandEvent evt;
			OnCancelFile( evt );
			break;
		}
		case WXK_F2: {
			ItemList files = ::GetSelectedItems( this, itFILES );
			if (files.size() == 1) {	
				CPartFile* file = files.front()->GetFile();
				
				// Currently renaming of completed files causes problem with kad
				if (file->IsPartFile()) {
					wxString strNewName = ::wxGetTextFromUser(
						_("Enter new name for this file:"),
						_("File rename"), file->GetFileName().GetPrintable());
				
					CPath newName = CPath(strNewName);
					if (newName.IsOk() && (newName != file->GetFileName())) {
						theApp->sharedfiles->RenameFile(file, newName);
					}
				}
			}
			break;
		}
		default:
			event.Skip();
	}
}


void CDownloadListCtrl::OnDrawItem(
	int item, wxDC* dc, const wxRect& rect, const wxRect& rectHL, bool highlighted)
{
	// Don't do any drawing if there's nobody to see it.
	if ( !theApp->amuledlg->IsDialogVisible( CamuleDlg::DT_TRANSFER_WND ) ) {
		return;
	}

	CtrlItem_Struct* content = (CtrlItem_Struct *)GetItemData(item);

	// Define text-color and background
	if ((content->GetType() == FILE_TYPE) && (highlighted)) {
		if (GetFocus()) {
			dc->SetBackground(m_hilightBrush);
			dc->SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
		} else {
			dc->SetBackground(m_hilightUnfocusBrush);
			dc->SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
		}
	} else {
		dc->SetBackground(*(wxTheBrushList->FindOrCreateBrush(
			wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX), wxSOLID)));
		dc->SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	}


	// Define the border of the drawn area
	if ( highlighted ) {
		CMuleColour colour;
		if ( ( content->GetType() == FILE_TYPE ) && !GetFocus() ) {
			colour = m_hilightUnfocusBrush.GetColour();
		} else {
			colour = m_hilightBrush.GetColour();
		}

		dc->SetPen( colour.Blend(65).GetPen() );
	} else {
		dc->SetPen(*wxTRANSPARENT_PEN);
	}


	dc->SetBrush( dc->GetBackground() );
	dc->DrawRectangle( rectHL.x, rectHL.y, rectHL.width, rectHL.height );

	dc->SetPen(*wxTRANSPARENT_PEN);

	if ( content->GetType() == FILE_TYPE && ( !highlighted || !GetFocus() ) ) {
		// If we have category, override textforeground with what category tells us.
		CPartFile *file = content->GetFile();
		if ( file->GetCategory() ) {
			dc->SetTextForeground(
				CMuleColour(theApp->glob_prefs->GetCatColor(file->GetCategory())) );
		}
	}

	// Various constant values we use
	const int iTextOffset = ( rect.GetHeight() - dc->GetCharHeight() ) / 2;
	const int iOffset = 4;

	// The starting end ending position of the tree
	bool tree_show = false;
	int tree_start = 0;
	int tree_end = 0;

	wxRect cur_rec( iOffset, rect.y, 0, rect.height );
	for (int i = 0; i < GetColumnCount(); i++) {
		wxListItem listitem;
		GetColumn(i, listitem);

		if (listitem.GetWidth() > 2*iOffset) {
			cur_rec.width = listitem.GetWidth() - 2*iOffset;

			// Make a copy of the current rectangle so we can apply specific tweaks
			wxRect target_rec = cur_rec;
			if ( i == ColumnProgress ) {
				tree_show = ( listitem.GetWidth() > 0 );

				tree_start = cur_rec.x - iOffset;
				tree_end   = cur_rec.x + iOffset;

				// Double the offset to make room for the cirle-marker
				target_rec.x += iOffset;
				target_rec.width -= iOffset;
			} else {
				// will ensure that text is about in the middle ;)
				target_rec.y += iTextOffset;
			}

			// Draw the item
			if ( content->GetType() == FILE_TYPE ) {
				DrawFileItem(dc, i, target_rec, content);
			} else {
				DrawSourceItem(dc, i, target_rec, content);
			}
		}
		
		// Increment to the next column
		cur_rec.x += listitem.GetWidth();
	}

	// Draw tree last so it draws over selected and focus (looks better)
	if ( tree_show ) {
		// Gather some information
		const bool notLast = item + 1 != GetItemCount();
		const bool notFirst = item != 0;
		const bool hasNext = notLast &&
			((CtrlItem_Struct*)GetItemData(item + 1))->GetType() != FILE_TYPE;
		const bool isOpenRoot = content->GetType() == FILE_TYPE &&
			(content->GetFile())->ShowSources() &&
			((content->GetFile())->GetStatus() != PS_COMPLETE);
		const bool isChild = content->GetType() != FILE_TYPE;

		// Might as well calculate these now
		const int treeCenter = tree_start + 3;
		const int middle = cur_rec.y + ( cur_rec.height + 1 ) / 2;

		// Set up a new pen for drawing the tree
		dc->SetPen( *(wxThePenList->FindOrCreatePen(dc->GetTextForeground(), 1, wxSOLID)) );

		if (isChild) {
			// Draw the line to the status bar
			dc->DrawLine(tree_end, middle, tree_start + 3, middle);

			// Draw the line to the child node
			if (hasNext) {
				dc->DrawLine(treeCenter, middle, treeCenter, cur_rec.y + cur_rec.height + 1);
			}

			// Draw the line back up to parent node
			if (notFirst) {
				dc->DrawLine(treeCenter, middle, treeCenter, cur_rec.y - 1);
			}
		} else if ( isOpenRoot ) {
			// Draw empty circle
			dc->SetBrush(*wxTRANSPARENT_BRUSH);

			dc->DrawCircle( treeCenter, middle, 3 );

			// Draw the line to the child node if there are any children
			if (hasNext) {
				dc->DrawLine(treeCenter, middle + 3, treeCenter, cur_rec.y + cur_rec.height + 1);
			}
		}

	}
}


void CDownloadListCtrl::DrawFileItem( wxDC* dc, int nColumn, const wxRect& rect, CtrlItem_Struct* item ) const
{
	wxDCClipper clipper( *dc, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight() );

	const CPartFile* file = item->GetFile();

	// Used to contain the contenst of cells that dont need any fancy drawing, just text.
	wxString text;

	switch (nColumn) {
	// Part Number
	case ColumnPart: {
	        wxString partno;


		if (file->IsPartFile() && !(file->GetStatus() == PS_COMPLETE)) {
		  partno = CFormat(wxT("%s")) % file->GetPartMetFileName().RemoveAllExt();
		}
		dc->DrawText(partno, rect.GetX(), rect.GetY());
	}
	break;
	// Filename
	case ColumnFileName: {
		wxString filename = file->GetFileName().GetPrintable();

		if (file->HasRating() || file->HasComment()) {
			int image = Client_CommentOnly_Smiley;
			if (file->HasRating()) {
				image = Client_InvalidRating_Smiley + file->UserRating() - 1;
			}	
				
			wxASSERT(image >= Client_InvalidRating_Smiley);
			wxASSERT(image <= Client_CommentOnly_Smiley);
			
			int imgWidth = 16;
			
			// it's already centered by OnDrawItem() ...
			m_ImageList.Draw(image, *dc, rect.GetX(), rect.GetY() - 1,
				wxIMAGELIST_DRAW_TRANSPARENT);
			dc->DrawText(filename, rect.GetX() + imgWidth + 4, rect.GetY());
		} else {
			dc->DrawText(filename, rect.GetX(), rect.GetY());
		}
	}
	break;

	// Filesize
	case ColumnSize:
		text = CastItoXBytes( file->GetFileSize() );
		break;

	// Transferred
	case ColumnTransferred:
		text = CastItoXBytes( file->GetTransferred() );
		break;
	
	// Completed
	case ColumnCompleted:
		text = CastItoXBytes( file->GetCompletedSize() );
		break;
	
	// Speed
	case ColumnSpeed:
		if ( file->GetTransferingSrcCount() ) {
			text = wxString::Format( wxT("%.1f "), file->GetKBpsDown() ) +
				_("kB/s");
		}
		break;

	// Progress	
	case ColumnProgress:
	{
		if (thePrefs::ShowProgBar())
		{
			int iWidth  = rect.GetWidth() - 2;
			int iHeight = rect.GetHeight() - 2;

			// DO NOT DRAW IT ALL THE TIME
			uint32 dwTicks = GetTickCount();
			
			wxMemoryDC cdcStatus;
			
			if ( item->dwUpdated < dwTicks || !item->status || iWidth != item->status->GetWidth() ) {
				if ( item->status == NULL) {
					item->status = new wxBitmap(iWidth, iHeight);
				} else if ( item->status->GetWidth() != iWidth ) {
					// Only recreate if the size has changed
					item->status->Create(iWidth, iHeight);
				}
						
				cdcStatus.SelectObject( *item->status );
				
				if ( thePrefs::UseFlatBar() ) {
					DrawFileStatusBar( file, &cdcStatus,
						wxRect(0, 0, iWidth, iHeight), true);
				} else {
					DrawFileStatusBar( file, &cdcStatus,
						wxRect(1, 1, iWidth - 2, iHeight - 2), false);
		
					// Draw black border
					cdcStatus.SetPen( *wxBLACK_PEN );
					cdcStatus.SetBrush( *wxTRANSPARENT_BRUSH );
					cdcStatus.DrawRectangle( 0, 0, iWidth, iHeight );
				}
			
				item->dwUpdated = dwTicks + 5000; // Plus five seconds
			} else {
				cdcStatus.SelectObject( *item->status );
			}
			
			dc->Blit( rect.GetX(), rect.GetY() + 1, iWidth, iHeight, &cdcStatus, 0, 0);
		}
		
		if (thePrefs::ShowPercent()) {
			// Percentage of completing
			// We strip anything below the first decimal point,
			// to avoid Format doing roundings
			float percent = floor( file->GetPercentCompleted() * 10.0f ) / 10.0f;
		
			wxString buffer = wxString::Format( wxT("%.1f%%"), percent );
			int middlex = (2*rect.GetX() + rect.GetWidth()) >> 1;
			int middley = (2*rect.GetY() + rect.GetHeight()) >> 1;
			
			wxCoord textwidth, textheight;
			
			dc->GetTextExtent(buffer, &textwidth, &textheight);
			wxColour AktColor = dc->GetTextForeground();
			if (thePrefs::ShowProgBar()) {
				dc->SetTextForeground(*wxWHITE);
			} else {
				dc->SetTextForeground(*wxBLACK);
			}
			dc->DrawText(buffer, middlex - (textwidth >> 1), middley - (textheight >> 1));
			dc->SetTextForeground(AktColor);
		}
	}
	break;

	// Sources
	case ColumnSources:	{
		uint16 sc = file->GetSourceCount();
		uint16 ncsc = file->GetNotCurrentSourcesCount();
		if ( ncsc ) {
			text = wxString::Format( wxT("%i/%i" ), sc - ncsc, sc );
		} else {
			text = wxString::Format( wxT("%i"), sc );
		}

		if ( file->GetSrcA4AFCount() ) {
			text += wxString::Format( wxT("+%i"), file->GetSrcA4AFCount() );
		}

		if ( file->GetTransferingSrcCount() ) {
			text += wxString::Format( wxT(" (%i)"), file->GetTransferingSrcCount() );
		}

		break;
	}

	// Priority
	case ColumnPriority:
		text = PriorityToStr( file->GetDownPriority(), file->IsAutoDownPriority() );
		break;
			
	// File-status
	case ColumnStatus:
		text = file->getPartfileStatus();
		break;
	
	// Remaining
	case ColumnTimeRemaining: {
		if ((file->GetStatus() != PS_COMPLETING) && file->IsPartFile()) {
			uint64 remainSize = file->GetFileSize() - file->GetCompletedSize();
			sint32 remainTime = file->getTimeRemaining();
			
			if (remainTime >= 0) {
				text = CastSecondsToHM(remainTime);
			} else {
				text = _("Unknown");
			}

			text += wxT(" (") + CastItoXBytes(remainSize) + wxT(")");
		}
		break;
	}
	
	// Last seen completed
	case ColumnLastSeenComplete: {
		if ( file->lastseencomplete ) {
			text = wxDateTime( file->lastseencomplete ).Format( _("%y/%m/%d %H:%M:%S") );
		} else {
			text = _("Unknown");
		}
		break;
	}
	
	// Last received
	case ColumnLastReception: {
		const time_t lastReceived = file->GetLastChangeDatetime();
		if (lastReceived) {
			text = wxDateTime(lastReceived).Format( _("%y/%m/%d %H:%M:%S") );
		} else {
			text = _("Unknown");
		}
	}
	} // switch

	if ( !text.IsEmpty() ) {
		dc->DrawText( text, rect.GetX(), rect.GetY() );
	}
}


void CDownloadListCtrl::DrawSourceItem(
	wxDC* dc, int nColumn, const wxRect& rect, CtrlItem_Struct* item ) const
{
	wxDCClipper clipper( *dc, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight() );
	wxString buffer;
	
	const CUpDownClient* client = item->GetSource();

	switch (nColumn) {
		// Client name + various icons
		case ColumnFileName: {
			wxRect cur_rec = rect;
			// +3 is added by OnDrawItem()... so take it off
			// Kry - eMule says +1, so I'm trusting it
			wxPoint point( cur_rec.GetX(), cur_rec.GetY()+1 );

			if (item->GetType() != A4AF_SOURCE) {
				uint8 image = 0;
				
				switch (client->GetDownloadState()) {
					case DS_CONNECTING:
					case DS_CONNECTED:
					case DS_WAITCALLBACK:
					case DS_TOOMANYCONNS:
						image = Client_Red_Smiley;
						break;
					case DS_ONQUEUE:
						if (client->IsRemoteQueueFull()) {
							image = Client_Grey_Smiley;
						} else {
							image = Client_Yellow_Smiley;
						}
						break;
					case DS_DOWNLOADING:
					case DS_REQHASHSET:
						image = Client_Green_Smiley;
						break;
					case DS_NONEEDEDPARTS:
					case DS_LOWTOLOWIP:
						image = Client_Grey_Smiley;
						break;
					default: // DS_NONE i.e.
						image = Client_White_Smiley;
					}

					m_ImageList.Draw(image, *dc, point.x, point.y,
						wxIMAGELIST_DRAW_TRANSPARENT);
				} else {
					m_ImageList.Draw(Client_Grey_Smiley, *dc, point.x, point.y,
						wxIMAGELIST_DRAW_TRANSPARENT);
				}

				cur_rec.x += 20;
				wxPoint point2( cur_rec.GetX(), cur_rec.GetY() + 1 );

				uint8 clientImage;
				
				if ( client->IsFriend() ) {
					clientImage = Client_Friend_Smiley;
				} else {
					switch ( client->GetClientSoft() ) {
						case SO_AMULE:
							clientImage = Client_aMule_Smiley;
							break;
						case SO_MLDONKEY:
						case SO_NEW_MLDONKEY:
						case SO_NEW2_MLDONKEY:
							clientImage = Client_mlDonkey_Smiley;
							break;
						case SO_EDONKEY:
						case SO_EDONKEYHYBRID:
							clientImage = Client_eDonkeyHybrid_Smiley;
							break;
						case SO_EMULE:
							clientImage = Client_eMule_Smiley;
							break;
						case SO_LPHANT:
							clientImage = Client_lphant_Smiley;
							break;
						case SO_SHAREAZA:
						case SO_NEW_SHAREAZA:
						case SO_NEW2_SHAREAZA:
							clientImage = Client_Shareaza_Smiley;
							break;
						case SO_LXMULE:
							clientImage = Client_xMule_Smiley;
							break;
						default:
							// cDonkey, Compatible, Unknown
							// No icon for those yet.
							// Using the eMule one + '?'
							clientImage = Client_Unknown;
							break;
					}
				}

				m_ImageList.Draw(clientImage, *dc, point2.x, point.y,
					wxIMAGELIST_DRAW_TRANSPARENT);

				if (client->GetScoreRatio() > 1) {
					// Has credits, draw the gold star
					m_ImageList.Draw(Client_CreditsYellow_Smiley, *dc, point2.x, point.y, 
						wxIMAGELIST_DRAW_TRANSPARENT );
				}	else if ( client->ExtProtocolAvailable() ) {
					// Ext protocol -> Draw the '+'
					m_ImageList.Draw(Client_ExtendedProtocol_Smiley, *dc, point2.x, point.y,
						wxIMAGELIST_DRAW_TRANSPARENT);
				}

				if (client->IsIdentified()) {
					// the 'v'
					m_ImageList.Draw(Client_SecIdent_Smiley, *dc, point2.x, point.y,
						wxIMAGELIST_DRAW_TRANSPARENT);					
				} else if (client->IsBadGuy()) {
					// the 'X'
					m_ImageList.Draw(Client_BadGuy_Smiley, *dc, point2.x, point.y,
						wxIMAGELIST_DRAW_TRANSPARENT);					
				}
							
				if (client->HasObfuscatedConnectionBeenEstablished()) {
					// the "¿" except it's a key
					m_ImageList.Draw(Client_Encryption_Smiley, *dc, point2.x, point.y,
						wxIMAGELIST_DRAW_TRANSPARENT);					
				}				
				
				wxString userName;
#ifdef ENABLE_IP2COUNTRY
				// Draw the flag
				const CountryData& countrydata = theApp->amuledlg->m_IP2Country->GetCountryData(client->GetFullIP());
				dc->DrawBitmap(countrydata.Flag,
					rect.x + 40, rect.y + 5,
					wxIMAGELIST_DRAW_TRANSPARENT);
				
				userName << countrydata.Name;
				
				userName << wxT(" - ");
#endif // ENABLE_IP2COUNTRY
				if (client->GetUserName().IsEmpty()) {
					userName << wxT("?");
				} else {
					userName << client->GetUserName();
				}
				dc->DrawText(userName, rect.GetX() + 60, rect.GetY());
			}
			break;

		case ColumnCompleted:	// completed
			if (item->GetType() != A4AF_SOURCE && client->GetTransferredDown()) {
				buffer = CastItoXBytes(client->GetTransferredDown());
				dc->DrawText(buffer, rect.GetX(), rect.GetY());
			}
			break;

		case ColumnSpeed:	// speed
			if (item->GetType() != A4AF_SOURCE && client->GetKBpsDown() > 0.001) {
				buffer = wxString::Format(wxT("%.1f "),
						client->GetKBpsDown()) + _("kB/s");
				dc->DrawText(buffer, rect.GetX(), rect.GetY());
			}
			break;

		case ColumnProgress:	// file info
			if ( thePrefs::ShowProgBar() ) {
				int iWidth = rect.GetWidth() - 2;
				int iHeight = rect.GetHeight() - 2;

				// don't draw Text beyond the bar
				dc->SetClippingRegion(rect.GetX(), rect.GetY() + 1, iWidth, iHeight);
			
				if ( item->GetType() != A4AF_SOURCE ) {
					uint32 dwTicks = GetTickCount();
					
					wxMemoryDC cdcStatus;

					if ( item->dwUpdated < dwTicks || !item->status || 
							iWidth != item->status->GetWidth() ) {
						
						if (item->status == NULL) {
							item->status = new wxBitmap(iWidth, iHeight);
						} else if ( item->status->GetWidth() != iWidth ) {
							// Only recreate if size has changed
							item->status->Create(iWidth, iHeight);
						}

						cdcStatus.SelectObject(*(item->status));

						if ( thePrefs::UseFlatBar() ) {
							DrawSourceStatusBar( client, &cdcStatus,
								wxRect(0, 0, iWidth, iHeight), true);
						} else {
							DrawSourceStatusBar( client, &cdcStatus,
								wxRect(1, 1, iWidth - 2, iHeight - 2), false);
				
							// Draw black border
							cdcStatus.SetPen( *wxBLACK_PEN );
							cdcStatus.SetBrush( *wxTRANSPARENT_BRUSH );
							cdcStatus.DrawRectangle( 0, 0, iWidth, iHeight );
						}
						
						// Plus ten seconds
						item->dwUpdated = dwTicks + 10000;
					} else {
						cdcStatus.SelectObject(*(item->status));
					}

					dc->Blit(rect.GetX(), rect.GetY() + 1, iWidth, iHeight, &cdcStatus, 0, 0);
				} else {
					wxString a4af;
					CPartFile* p = client->GetRequestFile();
					if (p) {
						a4af = p->GetFileName().GetPrintable();
					} else {
						a4af = wxT("?");
					}
					buffer = CFormat(wxT("%s: %s")) % _("A4AF") % a4af;
					
					int midx = (2*rect.GetX() + rect.GetWidth()) >> 1;
					int midy = (2*rect.GetY() + rect.GetHeight()) >> 1;
					
					wxCoord txtwidth, txtheight;
					
					dc->GetTextExtent(buffer, &txtwidth, &txtheight);
					
					dc->SetTextForeground(*wxBLACK);
					dc->DrawText(buffer, wxMax(rect.GetX() + 2, midx - (txtwidth >> 1)), midy - (txtheight >> 1));

					// Draw black border
					dc->SetPen( *wxBLACK_PEN );
					dc->SetBrush( *wxTRANSPARENT_BRUSH );
					dc->DrawRectangle( rect.GetX(), rect.GetY() + 1, iWidth, iHeight );		
				}
			}
			break;

		case ColumnSources: {
				// Version
				dc->DrawText(client->GetClientVerString(), rect.GetX(), rect.GetY());
				break;
			}

		case ColumnPriority:	// prio
			// We only show priority for sources actually queued for that file
			if (	item->GetType() != A4AF_SOURCE &&
				client->GetDownloadState() == DS_ONQUEUE ) {
				if (client->IsRemoteQueueFull()) {
					buffer = _("Queue Full");
					dc->DrawText(buffer, rect.GetX(), rect.GetY());
				} else {
					if (client->GetRemoteQueueRank()) {
						sint16 qrDiff = client->GetRemoteQueueRank() -
							client->GetOldRemoteQueueRank();
						if(qrDiff == client->GetRemoteQueueRank() ) {
							qrDiff = 0;
						}
						wxColour savedColour = dc->GetTextForeground();
						if( qrDiff < 0 ) {
							dc->SetTextForeground(*wxBLUE);
						}
						if( qrDiff > 0 ) {
							dc->SetTextForeground(*wxRED);
						}
						//if( qrDiff == 0 ) {
						//	dc->SetTextForeground(*wxLIGHT_GREY);
						//}
						buffer = wxString::Format(_("QR: %u (%i)"),
							client->GetRemoteQueueRank(), qrDiff);
						dc->DrawText(buffer, rect.GetX(), rect.GetY());
						dc->SetTextForeground(savedColour);
					}
				}
			}
			break;

		case ColumnStatus:	// status
			if (item->GetType() != A4AF_SOURCE) {
				buffer = DownloadStateToStr( client->GetDownloadState(), 
					client->IsRemoteQueueFull() );
			} else {
				buffer = _("Asked for another file");
				if (	client->GetRequestFile() &&
					client->GetRequestFile()->GetFileName().IsOk()) {
					buffer += CFormat(wxT(" (%s)")) 
						% client->GetRequestFile()->GetFileName();
				}
			}
			dc->DrawText(buffer, rect.GetX(), rect.GetY());
			break;
		// Source comes from?
		case ColumnTimeRemaining: {
			buffer = wxGetTranslation(OriginToText(client->GetSourceFrom()));
			dc->DrawText(buffer, rect.GetX(), rect.GetY());
			break;
		}	
	}
}


wxString CDownloadListCtrl::GetTTSText(unsigned item) const
{
	CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(item);
	
	if (content->GetType() == FILE_TYPE) {
		CPartFile* file = content->GetFile();

		return file->GetFileName().GetPrintable();
	}

	return wxEmptyString;
}


int CDownloadListCtrl::SortProc(wxUIntPtr param1, wxUIntPtr param2, long sortData)
{
	CtrlItem_Struct* item1 = (CtrlItem_Struct*)param1;
	CtrlItem_Struct* item2 = (CtrlItem_Struct*)param2;

	int sortMod = (sortData & CMuleListCtrl::SORT_DES) ? -1 : 1;
	sortData &= CMuleListCtrl::COLUMN_MASK;
	int comp = 0;

	if ( item1->GetType() == FILE_TYPE ) {
		if ( item2->GetType() == FILE_TYPE ) {
			// Both are files, so we just compare them
			comp = Compare( item1->GetFile(), item2->GetFile(), sortData);
		} else {
			// A file and a source, checking if they belong to each other
			if ( item1->GetFile() == item2->GetOwner() ) {
				// A file should always be above its sources
				// Returning directly to avoid the modifier
				return -1;
			} else {
				// Source belongs to anther file, so we compare the files instead
				comp = Compare( item1->GetFile(), item2->GetOwner(), sortData);
			}
		}
	} else {
		if ( item2->GetType() == FILE_TYPE ) {
			// A source and a file, checking if they belong to each other
			if ( item1->GetOwner() == item2->GetFile() ) {
				// A source should always be below its file
				// Returning directly to avoid the modifier
				return 1;
			} else {
				// Source belongs to anther file, so we compare the files instead
				comp = Compare( item1->GetOwner(), item2->GetFile(), sortData);
			}
		} else {
			// Two sources, some different possibilites
			if ( item1->GetOwner() == item2->GetOwner() ) {
				// Avilable sources first, if we have both an
				// available and an unavailable
				comp = ( item2->GetType() - item1->GetType() );

				if (comp) {
					// A4AF and non-A4AF. The order is fixed regardless of sort-order.
					return comp;
				} else {
					comp = Compare(item1->GetSource(), item2->GetSource(), sortData);
				}
			} else {
				// Belongs to different files, so we compare the files
				comp = Compare( item1->GetOwner(), item2->GetOwner(), sortData);
			}
		}
	}

	// We modify the result so that it matches with ascending or decending
	return sortMod * comp;
}


int CDownloadListCtrl::Compare( const CPartFile* file1, const CPartFile* file2, long lParamSort)
{
	int result = 0;

	switch (lParamSort) {
	// Sort by part number
	case ColumnPart:
		result = CmpAny(
			file1->GetPartMetFileName().RemoveAllExt(),
			file2->GetPartMetFileName().RemoveAllExt() );
		break;

	// Sort by filename
	case ColumnFileName:
		result = CmpAny(
			file1->GetFileName(),
			file2->GetFileName() );
		break;

	// Sort by size
	case ColumnSize:
		result = CmpAny(
			file1->GetFileSize(),
			file2->GetFileSize() );
		break;

	// Sort by transferred
	case ColumnTransferred:
		result = CmpAny(
			file1->GetTransferred(),
			file2->GetTransferred() );
		break;

	// Sort by completed
	case ColumnCompleted:
		result = CmpAny(
			file1->GetCompletedSize(),
			file2->GetCompletedSize() );
		break;

	// Sort by speed
	case ColumnSpeed:
		result = CmpAny(
			file1->GetKBpsDown() * 1024,
			file2->GetKBpsDown() * 1024 );
		break;

	// Sort by percentage completed
	case ColumnProgress:
		result = CmpAny(
			file1->GetPercentCompleted(),
			file2->GetPercentCompleted() );
		break;

	// Sort by number of sources
	case ColumnSources:
		result = CmpAny(
			file1->GetSourceCount(),
			file2->GetSourceCount() );
		break;

	// Sort by priority
	case ColumnPriority:
		result = CmpAny(
			file1->GetDownPriority(),
			file2->GetDownPriority() );
		break;

	// Sort by status
	case ColumnStatus:
		result = CmpAny(
			file1->getPartfileStatusRang(),
			file2->getPartfileStatusRang() );
		break;

	// Sort by remaining time
	case ColumnTimeRemaining:
		if (file1->getTimeRemaining() == -1) {
			if (file2->getTimeRemaining() == -1) {
				result = 0;
			} else {
				result = -1;
			}
		} else {
			if (file2->getTimeRemaining() == -1) {
				result = 1;
			} else {
				result = CmpAny(
					file1->getTimeRemaining(),
					file2->getTimeRemaining() );
			}
		}
		break;

	// Sort by last seen complete
	case ColumnLastSeenComplete:
		result = CmpAny(
			file1->lastseencomplete,
			file2->lastseencomplete );
		break;

	// Sort by last reception
	case ColumnLastReception:
		result = CmpAny(
			file1->GetLastChangeDatetime(),
			file2->GetLastChangeDatetime() );
		break;
	}

	return result;
}


int CDownloadListCtrl::Compare(
	const CUpDownClient* client1, const CUpDownClient* client2, long lParamSort)
{
	switch (lParamSort) {
		// Sort by name
		case ColumnPart:
		case ColumnFileName:
			return CmpAny( client1->GetUserName(), client2->GetUserName() );
	
		// Sort by status (size field)
		case ColumnSize:
			return CmpAny( client1->GetDownloadState(), client2->GetDownloadState() );
	
		// Sort by transferred in the following fields
		case ColumnTransferred:	
		case ColumnCompleted:
			return CmpAny( client1->GetTransferredDown(), client2->GetTransferredDown() );

		// Sort by speed
		case ColumnSpeed:
			return CmpAny( client1->GetKBpsDown(), client2->GetKBpsDown() );
		
		// Sort by parts offered (Progress field)
		case ColumnProgress:
			return CmpAny(
				client1->GetAvailablePartCount(),
				client2->GetAvailablePartCount() );
		
		// Sort by client version
		case ColumnSources: {
			if (client1->GetClientSoft() != client2->GetClientSoft()) {
				return client1->GetSoftStr().Cmp(client2->GetSoftStr());
			}

			if (client1->GetVersion() != client2->GetVersion()) {
				return CmpAny(client1->GetVersion(), client2->GetVersion());
			}

			return client1->GetClientModString().Cmp(client2->GetClientModString());
		}
		
		// Sort by Queue-Rank
		case ColumnPriority: {
			// This will sort by download state: Downloading, OnQueue, Connecting ...
			// However, Asked For Another will always be placed last, due to
			// sorting in SortProc
			if ( client1->GetDownloadState() != client2->GetDownloadState() ) {
				return client1->GetDownloadState() - client2->GetDownloadState();
			}

			// Placing items on queue before items on full queues
			if ( client1->IsRemoteQueueFull() ) {
				if ( client2->IsRemoteQueueFull() ) {
					return 0;
				} else {
					return  1;
				}
			} else if ( client2->IsRemoteQueueFull() ) {
				return -1;
			} else {
				if ( client1->GetRemoteQueueRank() ) {
					if ( client2->GetRemoteQueueRank() ) {
						return CmpAny(
							client1->GetRemoteQueueRank(),
							client2->GetRemoteQueueRank() );
					} else {
						return -1;
					}
				} else {
					if ( client2->GetRemoteQueueRank() ) {
						return  1;
					} else {
						return  0;
					}
				}
			}
		}
		
		// Sort by state
		case ColumnStatus: {
			if (client1->GetDownloadState() == client2->GetDownloadState()) {
				return CmpAny(
					client1->IsRemoteQueueFull(),
					client2->IsRemoteQueueFull() );
			} else {
				return CmpAny(
					client1->GetDownloadState(),
					client2->GetDownloadState() );
			}
		}

		// Source of source ;)
		case ColumnTimeRemaining:
			return CmpAny(client1->GetSourceFrom(), client2->GetSourceFrom());
		
		default:
			return 0;
	}
}


void CDownloadListCtrl::ClearCompleted()
{
	m_completedFiles = false;
	CastByID(ID_BTNCLRCOMPL, GetParent(), wxButton)->Enable(false);
	
	// Search for completed files
	for ( ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ) {
		CtrlItem_Struct* item = it->second; ++it;
		
		if ( item->GetType() == FILE_TYPE ) {
			CPartFile* file = item->GetFile();
			
			if ( file->IsPartFile() == false ) {
				RemoveFile(file);
			}
		}
	}
}


void CDownloadListCtrl::ShowFilesCount( int diff )
{
	m_filecount += diff;
	
	wxString str = wxString::Format( _("Downloads (%i)"), m_filecount );
	wxStaticText* label = CastByName( wxT("downloadsLabel"), GetParent(), wxStaticText );

	label->SetLabel( str );
	label->GetParent()->Layout();
}




bool CDownloadListCtrl::ShowItemInCurrentCat(
	const CPartFile* file, int newsel ) const
{
	return 
		((newsel == 0 && !thePrefs::ShowAllNotCats()) ||
		 (newsel == 0 && thePrefs::ShowAllNotCats() && file->GetCategory() == 0)) ||
		(newsel > 0 && newsel == file->GetCategory());
}

static const CMuleColour crHave(104, 104, 104);
static const CMuleColour crFlatHave(0, 0, 0);

static const CMuleColour crPending(255, 208, 0);
static const CMuleColour crFlatPending(255, 255, 100);

static const CMuleColour crProgress(0, 224, 0);
static const CMuleColour crFlatProgress(0, 150, 0);

static const CMuleColour crMissing(255, 0, 0);

void CDownloadListCtrl::DrawFileStatusBar(
	const CPartFile* file, wxDC* dc, const wxRect& rect, bool bFlat ) const
{
	static CBarShader s_ChunkBar(16);
	
	s_ChunkBar.SetHeight(rect.height);
	s_ChunkBar.SetWidth(rect.width); 
	s_ChunkBar.SetFileSize( file->GetFileSize() );
	s_ChunkBar.Set3dDepth( thePrefs::Get3DDepth() );

	if ( file->GetStatus() == PS_COMPLETE || file->GetStatus() == PS_COMPLETING ) {
		s_ChunkBar.Fill( bFlat ? crFlatProgress : crProgress );
		s_ChunkBar.Draw(dc, rect.x, rect.y, bFlat); 
		return;
	}
	
	// Part availability ( of missing parts )
	const CPartFile::CGapPtrList& gaplist = file->GetGapList();
	CPartFile::CGapPtrList::const_iterator it = gaplist.begin();
	uint64 lastGapEnd = 0;
	CMuleColour colour;
	
	for (; it != gaplist.end(); ++it) {
		Gap_Struct* gap = *it;

		// Start position
		uint32 start = ( gap->start / PARTSIZE );
		// fill the Have-Part (between this gap and the last)
		if (gap->start) {
		  s_ChunkBar.FillRange(lastGapEnd + 1, gap->start - 1,  bFlat ? crFlatHave : crHave);
		}
		lastGapEnd = gap->end;
		// End position
		uint32 end   = ( gap->end / PARTSIZE ) + 1;

		// Avoid going past the filesize. Dunno if this can happen, but the old code did check.
		if ( end > file->GetPartCount() ) {
			end = file->GetPartCount();
		}

		// Place each gap, one PART at a time
		for ( uint64 i = start; i < end; ++i ) {
			if ( i < file->m_SrcpartFrequency.size() && file->m_SrcpartFrequency[i]) {
				int blue = 210 - ( 22 * ( file->m_SrcpartFrequency[i] - 1 ) );
				colour.Set(0, ( blue < 0 ? 0 : blue ), 255 );
			} else {
				colour = crMissing;
			}

			if ( file->IsStopped() ) {
				colour.Blend(50);
			}
			
			uint64 gap_begin = ( i == start   ? gap->start : PARTSIZE * i );
			uint64 gap_end   = ( i == end - 1 ? gap->end   : PARTSIZE * ( i + 1 ) - 1 );
		
			s_ChunkBar.FillRange( gap_begin, gap_end,  colour);
		}
	}
	
	// fill the last Have-Part (between this gap and the last)
	s_ChunkBar.FillRange(lastGapEnd + 1, file->GetFileSize() - 1,  bFlat ? crFlatHave : crHave);
	
	// Pending parts
	const CPartFile::CReqBlockPtrList& requestedblocks_list = file->GetRequestedBlockList();
	CPartFile::CReqBlockPtrList::const_iterator it2 = requestedblocks_list.begin();
	// adjacing pending parts must be joined to avoid bright lines between them
	uint64 lastStartOffset = 0;
	uint64 lastEndOffset = 0;
	
	colour = bFlat ? crFlatPending : crPending;
	
	if ( file->IsStopped() ) {
		colour.Blend(50);
	}

	for (; it2 != requestedblocks_list.end(); ++it2) {
		
		if ((*it2)->StartOffset > lastEndOffset + 1) { 
			// not adjacing, draw last block
			s_ChunkBar.FillRange(lastStartOffset, lastEndOffset, colour);
			lastStartOffset = (*it2)->StartOffset;
			lastEndOffset   = (*it2)->EndOffset;
		} else {
			// adjacing, grow block
			lastEndOffset   = (*it2)->EndOffset;
		}
	}
	
	s_ChunkBar.FillRange(lastStartOffset, lastEndOffset, colour);


	// Draw the progress-bar
	s_ChunkBar.Draw( dc, rect.x, rect.y, bFlat );

	
	// Green progressbar width
	int width = (int)(( (float)rect.width / (float)file->GetFileSize() ) *
			file->GetCompletedSize() );

	if ( bFlat ) {
		dc->SetBrush( crFlatProgress.GetBrush() );
		
		dc->DrawRectangle( rect.x, rect.y, width, 3 );
	} else {
		// Draw the two black lines for 3d-effect
		dc->SetPen( *wxBLACK_PEN );
		dc->DrawLine( rect.x, rect.y + 0, rect.x + width, rect.y + 0 );
		dc->DrawLine( rect.x, rect.y + 2, rect.x + width, rect.y + 2 );
		
		// Draw the green line
		dc->SetPen( *(wxThePenList->FindOrCreatePen( crProgress , 1, wxSOLID ) ));
		dc->DrawLine( rect.x, rect.y + 1, rect.x + width, rect.y + 1 );
	}
}

static const CMuleColour crBoth(0, 192, 0);
static const CMuleColour crFlatBoth(0, 150, 0);

static const CMuleColour crNeither(240, 240, 240);
static const CMuleColour crFlatNeither(224, 224, 224);

#define crClientOnly crHave
#define crFlatClientOnly crFlatHave
#define crNextPending crFlatPending

void CDownloadListCtrl::DrawSourceStatusBar(
	const CUpDownClient* source, wxDC* dc, const wxRect& rect, bool bFlat) const
{
	static CBarShader s_StatusBar(16);

	CPartFile* reqfile = source->GetRequestFile();

	s_StatusBar.SetFileSize( reqfile->GetFileSize() );
	s_StatusBar.SetHeight(rect.height);
	s_StatusBar.SetWidth(rect.width);
	s_StatusBar.Set3dDepth( thePrefs::Get3DDepth() );

	// Barry - was only showing one part from client, even when reserved bits from 2 parts
	wxString gettingParts = source->ShowDownloadingParts();

	const BitVector& partStatus = source->GetPartStatus();

	uint64 uEnd = 0;
	for ( uint64 i = 0; i < partStatus.size(); i++ ) {
		uint64 uStart = PARTSIZE * i;
		uEnd = wxMin(reqfile->GetFileSize(), uStart + PARTSIZE) - 1;

		CMuleColour colour;
		if (!partStatus[i]) {
			colour = bFlat ? crFlatNeither : crNeither;
		} else if ( reqfile->IsComplete(uStart, uEnd)) {
			colour = bFlat ? crFlatBoth : crBoth;
		} else if (	source->GetDownloadState() == DS_DOWNLOADING &&
				source->GetLastBlockOffset() <= uEnd &&
				source->GetLastBlockOffset() >= uStart) {
			colour = crPending;
		} else if (gettingParts.GetChar((uint16)i) == 'Y') {
			colour = crNextPending;
		} else {
			colour = bFlat ? crFlatClientOnly : crClientOnly;
		}

		if ( source->GetRequestFile()->IsStopped() ) {
			colour.Blend(50);
		}

		s_StatusBar.FillRange(uStart, uEnd, colour);
	}
	// fill the rest (if partStatus is empty)
	s_StatusBar.FillRange(uEnd + 1, reqfile->GetFileSize() - 1, bFlat ? crFlatNeither : crNeither);

	s_StatusBar.Draw(dc, rect.x, rect.y, bFlat);
}

#ifdef __WXMSW__
#	define QUOTE	wxT("\"")
#else
#	define QUOTE	wxT("\'")
#endif

void CDownloadListCtrl::PreviewFile(CPartFile* file)
{
	wxString command;
	// If no player set in preferences, use mplayer.
	// And please, do a warning also :P
	if (thePrefs::GetVideoPlayer().IsEmpty()) {
		wxMessageBox(_(
			"To prevent this warning to show up in every preview,\nset your preferred video player in preferences (default is mplayer)."),
			_("File preview"), wxOK, this);
		// Since newer versions for some reason mplayer does not automatically
		// select video output device and needs a parameter, go figure...
		command = wxT("xterm -T \"aMule Preview\" -iconic -e mplayer ") QUOTE wxT("$file") QUOTE;
	} else {
		command = thePrefs::GetVideoPlayer();
	}

	// Check if we are (pre)viewing a completed file or not
	if (file->GetStatus() != PS_COMPLETE) {
		// Remove the .met and see if out video player specifiation uses the magic string
		wxString fileWithoutMet = thePrefs::GetTempDir().JoinPaths(
			file->GetPartMetFileName().RemoveExt()).GetRaw();
		if (!command.Replace(wxT("$file"), fileWithoutMet)) {
			// No magic string, so we just append the filename to the player command
			// Need to use quotes in case filename contains spaces
			command << wxT(" ") << QUOTE << fileWithoutMet << QUOTE;
		}
	} else {
		// This is a complete file
		// FIXME: This is probably not going to work if the filenames are mangled ...
		wxString rawFileName = file->GetFullName().GetRaw();
		if (!command.Replace(wxT("$file"), rawFileName)) {
			// No magic string, so we just append the filename to the player command
			// Need to use quotes in case filename contains spaces
			command << wxT(" ") << QUOTE << rawFileName << QUOTE;
		}
	}

	// We can't use wxShell here, it blocks the app
	CTerminationProcess *p = new CTerminationProcess(command);
	int ret = wxExecute(command, wxEXEC_ASYNC, p);
	bool ok = ret > 0;
	if (!ok) {
		delete p;
		AddLogLineM( true,
			CFormat( _("ERROR: Failed to execute external media-player! Command: `%s'") ) %
			command );
	}
}
// File_checked_for_headers
