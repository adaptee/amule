//
// This file is part of the aMule Project.
//
// Copyright (c) 2004-2008 aMule Team ( admin@amule.org / http://www.amule.org )
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

#ifndef ECSPECIALTAGS_H
#define ECSPECIALTAGS_H

//#warning Kry - Preferences packet derived from packet, and that's ok, but shouldn't be here because this is a tag file and forces a stupid include
#include "ECPacket.h"	// Needed for CECPacket
#include "../../../NetworkFunctions.h" // Needed for IsLowID


#include <map>

/*
 * Specific tags for specific requests
 *
 * \note EC remote end does not need to create these packets,
 * only using the getter functions.
 *
 * Regarding this, those classes are removed from remote build,
 * that have only a constructor.
 */

class CServer;
class CKnownFile;
class CPartFile;
class CSearchFile;
class CUpDownClient;

/*
 * EC tags encoder. Idea: if for an object <X>, client <Z> tag <Y> have value equal to previous
 * request, skip this tag.
 */

class CValueMap {
		/*
		 * Tag -> LastValue map. Hold last value that transmitted to remote side
		 */
		std::map<ec_tagname_t, uint8> m_map_uint8;
		std::map<ec_tagname_t, uint16> m_map_uint16;
		std::map<ec_tagname_t, uint32> m_map_uint32;
		std::map<ec_tagname_t, uint64> m_map_uint64;
		std::map<ec_tagname_t, CMD4Hash> m_map_md4;
		std::map<ec_tagname_t, wxString> m_map_string;
		
		template <class T>
		void CreateTagT(ec_tagname_t tagname, T value, std::map<ec_tagname_t, T> &map, CECTag *parent)
		{
			if ( (map.count(tagname) == 0) || (map[tagname] != value) ) {
				parent->AddTag(CECTag(tagname, value));
				map[tagname] = value;
			}
		}
	public:
		CValueMap()
		{
		}
		
		CValueMap(const CValueMap &valuemap)
		{
			m_map_uint8 = valuemap.m_map_uint8;
			m_map_uint16 = valuemap.m_map_uint16;
			m_map_uint32 = valuemap.m_map_uint32;
			m_map_uint64 = valuemap.m_map_uint64;
			m_map_md4 = valuemap.m_map_md4;
			m_map_string = valuemap.m_map_string;
		}
		
		~CValueMap()
		{
		}
		
		void CreateTag(ec_tagname_t tagname, uint8 value, CECTag *parent)
		{
			CreateTagT<uint8>(tagname, value, m_map_uint8, parent);
		}
		
		void CreateTag(ec_tagname_t tagname, uint16 value, CECTag *parent)
		{
			CreateTagT<uint16>(tagname, value, m_map_uint16, parent);
		}
		
		void CreateTag(ec_tagname_t tagname, uint32 value, CECTag *parent)
		{
			CreateTagT<uint32>(tagname, value, m_map_uint32, parent);
		}
		
		void CreateTag(ec_tagname_t tagname, uint64 value, CECTag *parent)
		{
			CreateTagT<uint64>(tagname, value, m_map_uint64, parent);
		}
		
		void CreateTag(ec_tagname_t tagname, CMD4Hash value, CECTag *parent)
		{
			CreateTagT<CMD4Hash>(tagname, value, m_map_md4, parent);
		}
		
		void CreateTag(ec_tagname_t tagname, wxString value, CECTag *parent)
		{
			CreateTagT<wxString>(tagname, value, m_map_string, parent);
		}
};

class CEC_Category_Tag : public CECTag {
 	public:
 		CEC_Category_Tag(uint32 cat_index, EC_DETAIL_LEVEL detail_level = EC_DETAIL_FULL);
 		// for create-upate commands
 		CEC_Category_Tag(uint32 cat_index, wxString name, wxString path,
			wxString comment, uint32 color, uint8 prio);
 		
 		void Apply();
 		void Create();
 		
 		wxString Name() { return GetTagByNameSafe(EC_TAG_CATEGORY_TITLE)->GetStringData(); }
 		wxString Path() { return GetTagByNameSafe(EC_TAG_CATEGORY_PATH)->GetStringData(); }
 		wxString Comment() { return GetTagByNameSafe(EC_TAG_CATEGORY_COMMENT)->GetStringData(); }
 		uint8 Prio() { return GetTagByNameSafe(EC_TAG_CATEGORY_PRIO)->GetInt(); }
 		uint32 Color() { return GetTagByNameSafe(EC_TAG_CATEGORY_COLOR)->GetInt(); }
 		
};

class CEC_Prefs_Packet : public CECPacket {
 	public:
 		CEC_Prefs_Packet(uint32 selection, EC_DETAIL_LEVEL prefs_detail = EC_DETAIL_FULL, EC_DETAIL_LEVEL cat_details = EC_DETAIL_FULL);

 		void Apply();
};

class CEC_Server_Tag : public CECTag {
 	public:
 		CEC_Server_Tag(const CServer *, EC_DETAIL_LEVEL);
 		
 		wxString ServerName() { return GetTagByNameSafe(EC_TAG_SERVER_NAME)->GetStringData(); }
 		wxString ServerDesc() { return GetTagByNameSafe(EC_TAG_SERVER_DESC)->GetStringData(); }

 		uint8 GetPrio() { return GetTagByNameSafe(EC_TAG_SERVER_PRIO)->GetInt(); }
 		bool GetStatic() { return (GetTagByNameSafe(EC_TAG_SERVER_STATIC)->GetInt() == 1); }

 		uint32 GetPing() { return GetTagByNameSafe(EC_TAG_SERVER_PING)->GetInt(); }
 		uint8 GetFailed() { return GetTagByNameSafe(EC_TAG_SERVER_FAILED)->GetInt(); }

 		uint32 GetFiles() { return GetTagByNameSafe(EC_TAG_SERVER_FILES)->GetInt(); }
 		uint32 GetUsers() { return GetTagByNameSafe(EC_TAG_SERVER_USERS)->GetInt(); }
 		uint32 GetMaxUsers() { return GetTagByNameSafe(EC_TAG_SERVER_USERS_MAX)->GetInt(); }
 		
 		// we're not using incremental update on server list,
 		// but template code needs it
 		uint32 ID() { return 0; }
};


class CEC_ConnState_Tag : public CECTag {
 	public:
 		CEC_ConnState_Tag(EC_DETAIL_LEVEL);

		uint32	GetEd2kId()		{ return GetTagByNameSafe(EC_TAG_ED2K_ID)->GetInt(); }
		uint32	GetClientId()		{ return GetTagByNameSafe(EC_TAG_CLIENT_ID)->GetInt(); }
 		bool	HasLowID()		{ return GetEd2kId() < HIGHEST_LOWID_ED2K_KAD; }
 		bool	IsConnected()		const { return IsConnectedED2K() || IsConnectedKademlia(); }
 		bool	IsConnectedED2K()	const { return (GetInt() & 0x01) != 0; }
 		bool	IsConnectingED2K()	const { return (GetInt() & 0x02) != 0; }
		bool	IsConnectedKademlia()	const { return (GetInt() & 0x04) != 0; }
		bool	IsKadFirewalled()	const { return (GetInt() & 0x08) != 0; }
		bool	IsKadRunning()	const { return (GetInt() & 0x10) != 0; }
};

class CEC_PartFile_Tag : public CECTag {
 	public:
 		CEC_PartFile_Tag(CPartFile *file, EC_DETAIL_LEVEL detail_level, bool detail = false);
		CEC_PartFile_Tag(CPartFile *file, CValueMap &valuemap);
		void Detail_Tag(CPartFile *file);
 		
		// template needs it
		CMD4Hash		ID()	{ return GetMD4Data(); }

 		CMD4Hash	FileHash()	{ return GetMD4Data(); }
		wxString	FileHashString() { return GetMD4Data().Encode(); }

 		wxString	FileName()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_NAME)->GetStringData(); }
 		uint64		SizeFull()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_SIZE_FULL)->GetInt(); }
 		uint64		SizeXfer()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_SIZE_XFER)->GetInt(); }
  		uint64		SizeDone()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_SIZE_DONE)->GetInt(); }
 		wxString	FileEd2kLink()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_ED2K_LINK)->GetStringData(); }
 		uint8		FileStatus()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_STATUS)->GetInt(); }
  		uint16		SourceCount()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_SOURCE_COUNT)->GetInt(); }
  		uint16		SourceNotCurrCount()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_SOURCE_COUNT_NOT_CURRENT)->GetInt(); }
  		uint16		SourceXferCount()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_SOURCE_COUNT_XFER)->GetInt(); }
  		uint16		SourceCountA4AF()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_SOURCE_COUNT_A4AF)->GetInt(); }
  		uint32		Speed()		{ return GetTagByNameSafe(EC_TAG_PARTFILE_SPEED)->GetInt(); }
  		uint8		Prio()		{ return GetTagByNameSafe(EC_TAG_PARTFILE_PRIO)->GetInt(); }

 		uint8		FileCat()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_CAT)->GetInt(); }
		time_t		LastSeenComplete() { return (time_t)GetTagByNameSafe(EC_TAG_PARTFILE_LAST_SEEN_COMP)->GetInt(); }

		wxString	PartMetName()
			{
				CECTag* tmp = GetTagByName(EC_TAG_PARTFILE_PARTMETID);
				if (tmp) {
					return wxString::Format(wxT("%03u.part.met"), tmp->GetInt());
				} else {
					return wxEmptyString;
				}
			}
		
		void SetSizeXfer(uint64 value) { AssignIfExist(EC_TAG_PARTFILE_SIZE_XFER, value); }
		void SetSizeDone(uint64 value) { AssignIfExist(EC_TAG_PARTFILE_SIZE_DONE, value); }

		void SetFileEd2kLink(uint32 value) { AssignIfExist(EC_TAG_PARTFILE_ED2K_LINK, value); }

		void SetFileStatus(uint8 &value) { AssignIfExist(EC_TAG_PARTFILE_STATUS, value); }

		void SetSourceCount(uint16 &value) { AssignIfExist(EC_TAG_PARTFILE_SOURCE_COUNT, value); }
		void SetSourceNotCurrCount(uint16 &value) { AssignIfExist(EC_TAG_PARTFILE_SOURCE_COUNT_NOT_CURRENT, value); }
		void SetSourceXferCount(uint16 &value) { AssignIfExist(EC_TAG_PARTFILE_SOURCE_COUNT_XFER, value); }
		void SetSourceCountA4AF(uint16 &value) { AssignIfExist(EC_TAG_PARTFILE_SOURCE_COUNT_A4AF, value); }

		void SetSpeed(uint32 &value) { AssignIfExist(EC_TAG_PARTFILE_SPEED, value); }
		void SetPrio(uint8 &value) { AssignIfExist(EC_TAG_PARTFILE_PRIO, value); }
		void SetFileCat(uint8 &value) { AssignIfExist(EC_TAG_PARTFILE_CAT, value); }
		void SetLastSeenComplete(time_t &value)
			{
				CECTag *tag = GetTagByName(EC_TAG_PARTFILE_LAST_SEEN_COMP);
				if ( tag ) {
					value = tag->GetInt();
				}
			}
		
		wxString	GetFileStatusString();
};

class CEC_SharedFile_Tag : public CECTag {
	public:
 		CEC_SharedFile_Tag(const CKnownFile *file, EC_DETAIL_LEVEL detail_level);
		CEC_SharedFile_Tag(const CKnownFile *file, CValueMap &valuemap);
 		
		// template needs it
 		CMD4Hash	ID()		{ return GetMD4Data(); }
		
 		CMD4Hash	FileHash()	{ return GetMD4Data(); }
		wxString	FileHashString(){ return GetMD4Data().Encode(); }

 		wxString	FileName()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_NAME)->GetStringData(); }
 		uint64		SizeFull()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_SIZE_FULL)->GetInt(); }
 		wxString	FileEd2kLink()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_ED2K_LINK)->GetStringData(); }

  		uint8		Prio()		{ return GetTagByNameSafe(EC_TAG_PARTFILE_PRIO)->GetInt(); }
 		uint16		GetRequests()	{ return GetTagByNameSafe(EC_TAG_KNOWNFILE_REQ_COUNT)->GetInt(); }
 		uint16		GetAllRequests()	{ return GetTagByNameSafe(EC_TAG_KNOWNFILE_REQ_COUNT_ALL)->GetInt(); }

 		uint16		GetAccepts()	{ return GetTagByNameSafe(EC_TAG_KNOWNFILE_ACCEPT_COUNT)->GetInt(); }
 		uint16		GetAllAccepts()	{ return GetTagByNameSafe(EC_TAG_KNOWNFILE_ACCEPT_COUNT_ALL)->GetInt(); }

 		uint64		GetXferred()	{ return GetTagByNameSafe(EC_TAG_KNOWNFILE_XFERRED)->GetInt(); }
 		uint64		GetAllXferred()	{ return GetTagByNameSafe(EC_TAG_KNOWNFILE_XFERRED_ALL)->GetInt(); }

		wxString	GetAICHHash()	{ return GetTagByNameSafe(EC_TAG_KNOWNFILE_AICH_MASTERHASH)->GetStringData(); }
 		
 		void SetPrio(uint8 &val) { AssignIfExist(EC_TAG_PARTFILE_PRIO, val); }
 		
 		void SetRequests(uint16 &val) { AssignIfExist(EC_TAG_KNOWNFILE_REQ_COUNT, val); }
 		void SetAllRequests(uint32 &val) { AssignIfExist(EC_TAG_KNOWNFILE_REQ_COUNT_ALL, val); }
 		
 		void SetAccepts(uint16 &val) { AssignIfExist(EC_TAG_KNOWNFILE_ACCEPT_COUNT, val); }
 		void SetAllAccepts(uint32 &val) { AssignIfExist(EC_TAG_KNOWNFILE_ACCEPT_COUNT_ALL, val); }
 		
 		void SetXferred(uint64 &val) { AssignIfExist(EC_TAG_KNOWNFILE_XFERRED, val); }
 		void SetAllXferred(uint64 &val) { AssignIfExist(EC_TAG_KNOWNFILE_XFERRED_ALL, val); }
};

class CEC_UpDownClient_Tag : public CECTag {
	public:
		CEC_UpDownClient_Tag(const CUpDownClient* client, EC_DETAIL_LEVEL detail_level);
		CEC_UpDownClient_Tag(const CUpDownClient* client, CValueMap &valuemap);

		uint32 ID() { return GetInt(); }
		
 		CMD4Hash FileID() { return GetTagByNameSafe(EC_TAG_KNOWNFILE)->GetMD4Data(); }
 		CMD4Hash UserID() { return GetTagByNameSafe(EC_TAG_CLIENT_HASH)->GetMD4Data(); }
 		
 		bool HaveFile() { return GetTagByName(EC_TAG_KNOWNFILE) != NULL; }

 		wxString ClientName() { return GetTagByNameSafe(EC_TAG_CLIENT_NAME)->GetStringData(); }
 		uint32 SpeedUp() { return GetTagByNameSafe(EC_TAG_CLIENT_UP_SPEED)->GetInt(); }
 		uint32 SpeedDown() { return GetTagByNameSafe(EC_TAG_CLIENT_DOWN_SPEED)->GetInt(); }
 		
 		uint64 XferUp() { return GetTagByNameSafe(EC_TAG_CLIENT_UPLOAD_TOTAL)->GetInt(); };
 		uint64 XferDown() { return GetTagByNameSafe(EC_TAG_CLIENT_DOWNLOAD_TOTAL)->GetInt(); }
 		uint32 XferUpSession() { return GetTagByNameSafe(EC_TAG_CLIENT_UPLOAD_SESSION)->GetInt(); }
 		
 		bool IsFriend() { return (GetTagByName(EC_TAG_CLIENT_FRIEND) != 0); }
 		
 		uint8 ClientSoftware() { return GetTagByNameSafe(EC_TAG_CLIENT_SOFTWARE)->GetInt(); }
 		
 		uint8 ClientState() { return GetTagByNameSafe(EC_TAG_CLIENT_STATE)->GetInt(); }
 		
 		uint32 WaitTime() { return GetTagByNameSafe(EC_TAG_CLIENT_WAIT_TIME)->GetInt(); }
 		uint32 XferTime() { return GetTagByNameSafe(EC_TAG_CLIENT_XFER_TIME)->GetInt(); }
 		uint32 LastReqTime() { return GetTagByNameSafe(EC_TAG_CLIENT_LAST_TIME)->GetInt(); }
 		uint32 QueueTime() { return GetTagByNameSafe(EC_TAG_CLIENT_QUEUE_TIME)->GetInt(); }
		uint8 GetSourceFrom() { return GetTagByNameSafe(EC_TAG_CLIENT_FROM)->GetInt(); }
		uint32 UserIP() { return GetTagByNameSafe(EC_TAG_CLIENT_USER_IP)->GetInt(); }
		uint16 UserPort() { return GetTagByNameSafe(EC_TAG_CLIENT_USER_PORT)->GetInt(); }
		uint32 ServerIP() { return GetTagByNameSafe(EC_TAG_CLIENT_SERVER_IP)->GetInt(); }
		uint16 ServerPort() { return GetTagByNameSafe(EC_TAG_CLIENT_SERVER_PORT)->GetInt(); }
		wxString ServerName() { return GetTagByNameSafe(EC_TAG_CLIENT_SERVER_NAME)->GetStringData(); }
		wxString SoftVerStr() { return GetTagByNameSafe(EC_TAG_CLIENT_SOFT_VER_STR)->GetStringData(); }
		uint32 Score() { return GetTagByNameSafe(EC_TAG_CLIENT_SCORE)->GetInt(); }
		uint16 WaitingPosition() { return GetTagByNameSafe(EC_TAG_CLIENT_WAITING_POSITION)->GetInt(); }
};

class CEC_SearchFile_Tag : public CECTag {
	public:
		CEC_SearchFile_Tag(CSearchFile *file, EC_DETAIL_LEVEL detail_level);
		CEC_SearchFile_Tag(CSearchFile *file, CValueMap &valuemap);

		// template needs it
 		CMD4Hash	ID()	{ return GetMD4Data(); }

 		CMD4Hash	FileHash()	{ return GetMD4Data(); }
		wxString	FileHashString() { return GetMD4Data().Encode(); }

 		wxString	FileName()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_NAME)->GetStringData(); }
 		uint64		SizeFull()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_SIZE_FULL)->GetInt(); }
  		uint32		SourceCount()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_SOURCE_COUNT)->GetInt(); }
  		uint32		CompleteSourceCount()	{ return GetTagByNameSafe(EC_TAG_PARTFILE_SOURCE_COUNT_XFER)->GetInt(); }
  		bool		AlreadyHave()	{ return GetTagByName(EC_TAG_KNOWNFILE) != 0; }
};

class CEC_Search_Tag : public CECTag {
	public:
		// search request
		CEC_Search_Tag(const wxString &name, EC_SEARCH_TYPE search_type, const wxString &file_type,
			const wxString &extension, uint32 avail, uint64 min_size, uint64 max_size);
			
		wxString SearchText() { return GetTagByNameSafe(EC_TAG_SEARCH_NAME)->GetStringData(); }
		EC_SEARCH_TYPE SearchType() { return (EC_SEARCH_TYPE)GetInt(); }
		uint64 MinSize() { return GetTagByNameSafe(EC_TAG_SEARCH_MIN_SIZE)->GetInt(); }
		uint64 MaxSize() { return GetTagByNameSafe(EC_TAG_SEARCH_MAX_SIZE)->GetInt(); }
		uint32 Avail() { return GetTagByNameSafe(EC_TAG_SEARCH_AVAILABILITY)->GetInt(); }
		wxString SearchExt() { return GetTagByNameSafe(EC_TAG_SEARCH_EXTENSION)->GetStringData(); }
		wxString SearchFileType() { return GetTagByNameSafe(EC_TAG_SEARCH_FILE_TYPE)->GetStringData(); }
};

class CEC_StatTree_Node_Tag : public CECTag {
	public:
		CEC_StatTree_Node_Tag();	// just to keep compiler happy
		wxString GetDisplayString() const;
};

#endif /* ECSPEACIALTAGS_H */
// File_checked_for_headers
