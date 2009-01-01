//
// This file is part of the aMule Project.
//
// Copyright (c) 2003-2008 aMule Team ( admin@amule.org / http://www.amule.org )
// Copyright (c) 2002-2008 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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

#ifndef PARTFILECONVERT_H
#define PARTFILECONVERT_H

#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/gauge.h>


#include "Types.h"

struct ConvertJob;
class CPartFileConvertDlg;
class CPath;


class CPartFileConvert : private wxThread
{
public:
	static int	ScanFolderToAdd(const CPath& folder, bool deletesource = false);
	static void	ConvertToeMule(const CPath& file, bool deletesource = false);
	static void	StartThread();
	static void	StopThread();

	static void	ShowGUI(wxWindow *parent);
	static void	UpdateGUI(float percent, wxString text, bool fullinfo = false);	// current file information
	static void	UpdateGUI(ConvertJob* job); // listcontrol update
	static void	CloseGUI();

	static void	RemoveAllJobs();
	static void	RemoveAllSuccJobs();
	static void	RemoveJob(ConvertJob* job);
	static wxString	GetReturncodeText(int ret);

	static wxMutex	s_mutex;

private:
	CPartFileConvert() : wxThread(wxTHREAD_DETACHED) {}

	static int	performConvertToeMule(const CPath& file);
	virtual ExitCode Entry();

	static wxThread*		s_convertPfThread;
	static std::list<ConvertJob*>	s_jobs;
	static ConvertJob*		s_pfconverting;

	static CPartFileConvertDlg*	s_convertgui;

};

class CConvertListCtrl : public wxListCtrl
{
public:
	CConvertListCtrl(
			 wxWindow* parent,
			 wxWindowID winid = -1,
			 const wxPoint& pos = wxDefaultPosition,
			 const wxSize& size = wxDefaultSize,
			 long style = wxLC_ICON,
			 const wxValidator& validator = wxDefaultValidator,
			 const wxString& name = wxT("convertlistctrl"))
			 ;
};

class CPartFileConvertDlg : public wxDialog
{
	friend class CPartFileConvert;
public:
	CPartFileConvertDlg(wxWindow *parent);

	void	AddJob(ConvertJob* job);
	void	RemoveJob(ConvertJob* job);
	void	UpdateJobInfo(ConvertJob* job);

protected:
	wxGauge*		m_pb_current;
	CConvertListCtrl*	m_joblist;

	void	OnAddFolder(wxCommandEvent& event);
	void	OnClose(wxCloseEvent& event);
	void	OnCloseButton(wxCommandEvent& event);
	void	RetrySel(wxCommandEvent& event);
	void	RemoveSel(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif /* PARTFILECONVERT_H */
// File_checked_for_headers
