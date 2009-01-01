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

#ifndef CLIENTLISTCTRL_H
#define CLIENTLISTCTRL_H

#include "MuleListCtrl.h"		// Needed for CMuleListCtrl
#include "Constants.h"			// Needed for ViewType
#include <wx/brush.h>

class CUpDownClient;


/**
 * This class represents a number of ways of displaying clients, currently
 * supporting 3 different "ViewTypes". In other words, this class superseeds
 * the 3 widgets that were used to display clients beforehand:
 *  - CUploadListCtrl
 *  - CQueueListCtrl
 *  - CClientListCtrl
 *
 * This is done in an modular fashion, which means that adding new views is a 
 * rather trivial task. This approch has the advantage that only one widget exists
 * and thus only one actual list is maintained at a time, even though the number
 * of calls to the list wont decrease. This however can be trivially fixed if needed.
 */
class CClientListCtrl : public CMuleListCtrl
{
public:
	/**
	 * Constructor.
	 *
	 * @see CMuleListCtrl::CMuleListCtrl
	 */
 	CClientListCtrl(
	            wxWindow *parent,
                wxWindowID winid = -1,
                const wxPoint &pos = wxDefaultPosition,
                const wxSize &size = wxDefaultSize,
                long style = wxLC_ICON,
                const wxValidator& validator = wxDefaultValidator,
                const wxString &name = wxT("clientlistctrl") );
		
	/**
	 * Destructor.
	 */	 
	~CClientListCtrl();


	/**
	 * Returns the current view-type. 
	 * 
	 * @return The viewtype set by the user or the default value.
	 */
	ViewType GetListView();

	/**
	 * Sets another view-type.
	 *
	 * @param newView A view-mode different from the current view-type.
	 *
	 * Calling this function resets the list and re-initializes it to display
	 * clients acording to the specifications of that view-mode. If you wish
	 * to susspend the list, then use vtNone as the argument.
	 */
	void	SetListView( ViewType newView );

	
	/**
	 * Adds a new client to the list.
	 *
	 * @param client The client to be added.
	 * @param view The view where the client should be displayed.
	 *
	 * This function adds the specified client to the list, provided that the 
	 * view parameter matches the currently selected view-mode.
	 */
	void	InsertClient( CUpDownClient* client, ViewType view );
	
	/**
	 * Removes a client from the list.
	 *
	 * @param client The client to be removed.
	 * @param view The view where the client is being displayed.
	 *
	 * This function removes the specified client from the list, provided that 
	 * the view parameter matches the currently selected view-mode.
	 */
	void	RemoveClient( CUpDownClient* client, ViewType view );

	/**
	 * Updates a client on the list.
	 *
	 * @param client The client to be updated.
	 * @param view The view where the client is being displayed.
	 *
	 * This function updates (redraws) the specified client on the list, provided
	 * that the view parameter matches the currently selected view-mode. Clients
	 * that are outside the currently visible range will also be ignored.
	 */
	void	UpdateClient( CUpDownClient* client, ViewType view );


	/**
	 * This function toggles between the different view-types.
	 *
	 * Calling this function makes the list switch to the next view-type
	 * available, provided that the current view-type isn't vtNone. The
	 * sequence is as specified in the ViewType enum, with the exception
	 * that vtNone will be skipped.
	 */
	void	ToggleView();

	
protected:
	/// Return old column order.
	wxString GetOldColumnOrder() const;

private:
	/**
	 * Custom cell-drawing function.
	 */
	virtual void OnDrawItem(int item, wxDC* dc, const wxRect& rc, const wxRect& rectHL, bool hl);
	
	/**
	 * @see CMuleListCtrl::GetTTSText
	 */
	virtual wxString GetTTSText(unsigned item) const;
	
	
	/**
	 * Event-handler for displaying a menu at right-clicks.
	 */
	void	OnRightClick( wxMouseEvent& event );
	
	/**
	 * Event-handler for displaying the client-details dialog upon middle-clicks.
	 */
	void	OnMiddleClick( wxListEvent& event );
	
	/**
	 * Event-handler for switching between the different view-types.
	 */
	void	OnChangeView( wxCommandEvent& event );
	
	/**
	 * Event-handler for adding a client on the list to the list of friends.
	 */
	void	OnAddFriend( wxCommandEvent& event );
	
	/**
	 * Event-handler for showing details about a client.
	 */
	void	OnShowDetails( wxCommandEvent& event );
	
	/**
	 * Event-handler for requesting the sharedfiles-list of a client.
	 */
	void	OnViewFiles( wxCommandEvent& event );
	
	/**
	 * Event-handler for sending a message to a specific client.
	 */
	void	OnSendMessage( wxCommandEvent& event );
	
	/**
	 * Event-handler for un-banning a client.
	 */
	void	OnUnbanClient( wxCommandEvent& event );
	

	//! The current view-type. The default value is vtUploading.
	ViewType	m_viewType;
	
	//! A pointer to the displayed menu, used to ensure that only one menu is displayed at a time.
	wxMenu*		m_menu;

	//! One of the two most used brushes, cached for performance reasons.
	wxBrush	m_hilightBrush;
	
	//! One of the two most used brushes, cached for performance reasons.
	wxBrush	m_hilightUnfocusBrush;
	

	DECLARE_EVENT_TABLE()
};



/**
 * This is the default view for the list, representing a list of clients recieving files.
 *
 * This struct contains the functions needed to realize the uploading-view. It contains
 * the functions used by the CClientListCtrl to prepare, sort and draw the list when the
 * Uploading-view is enabled.
 */
struct CUploadingView
{
	/**
	 * Initializes the view.
	 *
	 * @param list The list which wants to make use of the view.
	 *
	 * This function is called when the CClientListCtrl changes to this view-type,
	 * and it is responsible for setting the initial columns and contents. By the 
	 * time this function is called, the list will already be completly empty.
	 */
	static void Initialize( CClientListCtrl* list );

	/**
	 * Draws a specific cell.
	 *
	 * @param client The client used as a reference.
	 * @param column The column to be drawn.
	 * @param dc The device-context to draw onto.
	 * @param rect The rectangle to draw in.
	 *
	 * This function is used to draw the contents of each row, and is called for
	 * every visible column. 
	 */
	static void DrawCell( CUpDownClient* client, int column, wxDC* dc, const wxRect& rect );
	
	/**
	 * This is the sorter-function used by the listctrl to sort the contents.
	 *
	 * @see wxListCtrl::SortItems
	 */
	static int wxCALLBACK SortProc(wxUIntPtr item1, wxUIntPtr item2, long sortData);
	
	/**
	 * Helperfunction which draws a simple bar-span over the clients requested file.
	 */
	static void DrawStatusBar( CUpDownClient* client, wxDC* dc, const wxRect &rect );

	/**
	 * Return the old column order for this view.
	 */
	static wxString GetOldColumnOrder();
};


/**
 * This struct contains the functions needed to realize the Queued-clients view.
 *
 * @see CUploadingView
 */
struct CQueuedView
{
	/**
	 * @see CUploadingView::Initialize
	 */
	static void Initialize( CClientListCtrl* list );

	/**
	 * @see CUploadingView::DrawCell
	 */
	static void DrawCell( CUpDownClient* client, int column, wxDC* dc, const wxRect& rect );

	/**
	 * @see CUploadingView::SortProc
	 */
	static int wxCALLBACK SortProc(wxUIntPtr item1, wxUIntPtr item2, long sortData);

	/**
	 * Return the old column order for this view.
	 */
	static wxString GetOldColumnOrder();
};


/**
 * This struct contains the functions needed to realize the Clients view.
 *
 * @see CUploadingView
 */
struct CClientsView
{
	/**
	 * @see CUploadingView::Initialize
	 */
	static void Initialize( CClientListCtrl* list );

	/**
	 * @see CUploadingView::DrawCell
	 */
	static void DrawCell( CUpDownClient* client, int column, wxDC* dc, const wxRect& rect );

	/**
	 * @see CUploadingView::SortProc
	 */
	static int wxCALLBACK SortProc(wxUIntPtr item1, wxUIntPtr item2, long sortData);

	/**
	 * Return the old column order for this view.
	 */
	static wxString GetOldColumnOrder();
};

#endif
// File_checked_for_headers
