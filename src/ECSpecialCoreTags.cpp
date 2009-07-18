//
// This file is part of the aMule Project.
//
// Copyright (c) 2003-2008 Angel Vidal ( kry@amule.org )
// Copyright (c) 2003-2008 aMule Team ( admin@amule.org / http://www.amule.org )
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


#include <ec/cpp/ECTag.h>		// Needed for CECTag
#include <ec/cpp/ECSpecialTags.h>	// Needed for special EC tag creator classes


// Since there are only constructors defined here,
// removing everything from non-local builds.

#include "amule.h"
#include "Server.h"		// Needed for CServer
#include "PartFile.h"		// Needed for CPartFile
#include "ServerConnect.h"	// Needed for CServerConnect
#include "updownclient.h"
#include "UploadQueue.h"	// Needed for CUploadQueue
#include "SharedFileList.h"
#include "SearchList.h"

#include "kademlia/kademlia/Kademlia.h"


CEC_Server_Tag::CEC_Server_Tag(const CServer *server, EC_DETAIL_LEVEL detail_level) :
	CECTag(EC_TAG_SERVER, EC_IPv4_t(server->GetIP(), server->GetPort()))
{
	wxString tmpStr;
	uint32 tmpInt;
	uint8 tmpShort;

	switch (detail_level) {
		case EC_DETAIL_INC_UPDATE:
			// should not get here
			wxFAIL;
			break;
		case EC_DETAIL_UPDATE:
			if ((tmpInt = server->GetPing()) != 0) {
				AddTag(CECTag(EC_TAG_SERVER_PING, tmpInt));
			}
			if ((tmpShort = (uint8)server->GetFailedCount()) != 0) {
				AddTag(CECTag(EC_TAG_SERVER_FAILED, tmpShort));
			}
			break;
		case EC_DETAIL_WEB:
		case EC_DETAIL_FULL:
			if ((tmpInt = server->GetPing()) != 0) {
				AddTag(CECTag(EC_TAG_SERVER_PING, tmpInt));
			}
			if ((tmpShort = (uint8)server->GetPreferences()) != SRV_PR_NORMAL) {
				AddTag(CECTag(EC_TAG_SERVER_PRIO, tmpShort));
			}
			if ((tmpShort = (uint8)server->GetFailedCount()) != 0) {
				AddTag(CECTag(EC_TAG_SERVER_FAILED, tmpShort));
			}
			if ((tmpShort = (server->IsStaticMember() ? 1 : 0)) != 0) {
				AddTag(CECTag(EC_TAG_SERVER_STATIC, tmpShort));
			}
			if (!(tmpStr = server->GetVersion()).IsEmpty()) {
				AddTag(CECTag(EC_TAG_SERVER_VERSION, tmpStr));
			}
			if (!(tmpStr = server->GetDescription()).IsEmpty()) {
				AddTag(CECTag(EC_TAG_SERVER_DESC, tmpStr));
			}
			if ((tmpInt = server->GetUsers()) != 0) {
				AddTag(CECTag(EC_TAG_SERVER_USERS, tmpInt));
			}
			if ((tmpInt = server->GetMaxUsers()) != 0) {
				AddTag(CECTag(EC_TAG_SERVER_USERS_MAX, tmpInt));
			}
			if ((tmpInt = server->GetFiles()) != 0) {
				AddTag(CECTag(EC_TAG_SERVER_FILES, tmpInt));
			}
		case EC_DETAIL_CMD:
			if (!(tmpStr = server->GetListName()).IsEmpty()) {
				AddTag(CECTag(EC_TAG_SERVER_NAME, tmpStr));
			}
	}
}

CEC_ConnState_Tag::CEC_ConnState_Tag(EC_DETAIL_LEVEL detail_level) : CECTag(EC_TAG_CONNSTATE,
	(uint8)(
			(theApp->IsConnectedED2K() ? 0x01 : 0x00)
			|
			(theApp->serverconnect->IsConnecting() ? 0x02 : 0x00)
			|
			(theApp->IsConnectedKad() ? 0x04 : 0x00)
			|
			(Kademlia::CKademlia::IsFirewalled() ? 0x08 : 0x00)
			| 
			(Kademlia::CKademlia::IsRunning() ? 0x10 : 0x00)
		))
{
	if (theApp->IsConnectedED2K()) {
		if ( theApp->serverconnect->GetCurrentServer() ) {
			AddTag(CEC_Server_Tag(theApp->serverconnect->GetCurrentServer(), detail_level));
		}
		AddTag(CECTag(EC_TAG_ED2K_ID, theApp->GetED2KID()));
	}

	AddTag(CECTag(EC_TAG_CLIENT_ID, theApp->GetID()));	
}

void CEC_PartFile_Tag::Detail_Tag(CPartFile *file, CValueMap *valuemap)
{
	// Tag for source names
	CECTag sn(EC_TAG_PARTFILE_SOURCE_NAMES, (uint64) 0);
	SourcenameItemList sil;
	const CPartFile::SourceSet &sources = file->GetSourceList();
	for (CPartFile::SourceSet::const_iterator it = sources.begin(); it != sources.end(); ++it) {
		CUpDownClient *cur_src = *it; 
		if (cur_src->GetRequestFile() != file || cur_src->GetClientFilename().Length() == 0) {
			continue;
		}
			
		bool found = false;
		for (SourcenameItemList::iterator its = sil.begin() ; its != sil.end(); ++its ) {
			if (its->name == cur_src->GetClientFilename()) {
				its->count++;
				found = true;
				break;
			}
		}
		if (!found) {
			sil.push_back(SourcenameItem(cur_src->GetClientFilename(), 1));
		}
	}
	for (SourcenameItemList::const_iterator its = sil.begin() ; its != sil.end(); ++its ) {
		// Tag children are evaluated by index, not by name
		sn.AddTag(CECTag(EC_TAG_PARTFILE_SOURCE_NAMES, its->name));
		sn.AddTag(CECTag(EC_TAG_PARTFILE_SOURCE_NAMES, (uint64) its->count));
	}
	AddTag(sn, valuemap);
}	

CEC_PartFile_Tag::CEC_PartFile_Tag(CPartFile *file, EC_DETAIL_LEVEL detail_level, bool detail, CValueMap *valuemap)
:
CECTag(EC_TAG_PARTFILE, file->GetFileHash())
{
	AddTag(EC_TAG_PARTFILE_STATUS, file->GetStatus(), valuemap);
	AddTag(EC_TAG_PARTFILE_STOPPED, file->IsStopped(), valuemap);

	AddTag(EC_TAG_PARTFILE_SOURCE_COUNT, file->GetSourceCount(), valuemap);
	AddTag(EC_TAG_PARTFILE_SOURCE_COUNT_NOT_CURRENT, file->GetNotCurrentSourcesCount(), valuemap);
	AddTag(EC_TAG_PARTFILE_SOURCE_COUNT_XFER, file->GetTransferingSrcCount(), valuemap);
	AddTag(EC_TAG_PARTFILE_SOURCE_COUNT_A4AF, file->GetSrcA4AFCount(), valuemap);
		
	if ( (file->GetTransferingSrcCount() > 0) || (detail_level != EC_DETAIL_UPDATE) || valuemap) {
		
		AddTag(EC_TAG_PARTFILE_SIZE_XFER, file->GetTransferred(), valuemap);
		AddTag(EC_TAG_PARTFILE_SIZE_DONE, file->GetCompletedSize(), valuemap);
		AddTag(EC_TAG_PARTFILE_SPEED, file->GetKBpsDown()*1024, valuemap);
	}
	
	AddTag(EC_TAG_PARTFILE_PRIO, (file->IsAutoDownPriority() ? 
		file->GetDownPriority() + 10 : file->GetDownPriority()), valuemap);

	AddTag(EC_TAG_PARTFILE_CAT, file->GetCategory(), valuemap);
	AddTag(EC_TAG_PARTFILE_LAST_SEEN_COMP, file->lastseencomplete, valuemap);
	AddTag(EC_TAG_PARTFILE_LAST_RECV, file->GetLastChangeDatetime(), valuemap);
	AddTag(EC_TAG_PARTFILE_DOWNLOAD_ACTIVE, file->GetDlActiveTime(), valuemap);

	AddTag(EC_TAG_PARTFILE_LOST_CORRUPTION, file->GetLostDueToCorruption(), valuemap);
	AddTag(EC_TAG_PARTFILE_GAINED_COMPRESSION, file->GetGainDueToCompression(), valuemap);
	AddTag(EC_TAG_PARTFILE_SAVED_ICH, file->TotalPacketsSavedDueToICH(), valuemap);

	if (detail) {
		Detail_Tag(file, valuemap);
	}
	
	// Tag for comments
	CECEmptyTag sc(EC_TAG_PARTFILE_COMMENTS);

	const FileRatingList & list = file->GetRatingAndComments();
	for (FileRatingList::const_iterator it = list.begin(); it != list.end(); ++it) {
		// Tag children are evaluated by index, not by name
		sc.AddTag(CECTag(EC_TAG_PARTFILE_COMMENTS, it->UserName));
		sc.AddTag(CECTag(EC_TAG_PARTFILE_COMMENTS, it->FileName));
		sc.AddTag(CECTag(EC_TAG_PARTFILE_COMMENTS, (uint64) it->Rating));
		sc.AddTag(CECTag(EC_TAG_PARTFILE_COMMENTS, it->Comment));
	}
	AddTag(sc, valuemap);

	if (detail_level == EC_DETAIL_UPDATE) {
		return;
	}
	
	AddTag(EC_TAG_PARTFILE_NAME,file->GetFileName().GetPrintable(), valuemap);

	long l;
	if (file->GetPartMetFileName().RemoveAllExt().GetRaw().ToLong(&l)) {
		AddTag(EC_TAG_PARTFILE_PARTMETID, (uint64)l, valuemap);
	}

	AddTag(EC_TAG_PARTFILE_SIZE_FULL, file->GetFileSize(), valuemap);

	AddTag(EC_TAG_PARTFILE_ED2K_LINK,
		theApp->CreateED2kLink(file, (theApp->IsConnectedED2K() && !theApp->serverconnect->IsLowID())), valuemap);
}

CEC_SharedFile_Tag::CEC_SharedFile_Tag(const CKnownFile *file, CValueMap &valuemap) : CECTag(EC_TAG_KNOWNFILE, file->GetFileHash())
{
	valuemap.CreateTag(EC_TAG_KNOWNFILE_REQ_COUNT, file->statistic.GetRequests(), this);
	valuemap.CreateTag(EC_TAG_KNOWNFILE_REQ_COUNT_ALL, file->statistic.GetAllTimeRequests(), this);
	valuemap.CreateTag(EC_TAG_KNOWNFILE_ACCEPT_COUNT, file->statistic.GetAccepts(), this);
	valuemap.CreateTag(EC_TAG_KNOWNFILE_ACCEPT_COUNT_ALL, file->statistic.GetAllTimeAccepts(), this);
	valuemap.CreateTag(EC_TAG_KNOWNFILE_XFERRED, file->statistic.GetTransferred(), this);
	valuemap.CreateTag(EC_TAG_KNOWNFILE_XFERRED_ALL, file->statistic.GetAllTimeTransferred(), this);
	valuemap.CreateTag(EC_TAG_KNOWNFILE_AICH_MASTERHASH, file->GetAICHMasterHash(), this);
	
	valuemap.CreateTag(EC_TAG_PARTFILE_PRIO,
		(uint64)(file->IsAutoUpPriority() ? file->GetUpPriority() + 10 : file->GetUpPriority()), this);
	
	valuemap.CreateTag(EC_TAG_PARTFILE_NAME, file->GetFileName().GetPrintable(), this);
	valuemap.CreateTag(EC_TAG_PARTFILE_SIZE_FULL, file->GetFileSize(), this);
	valuemap.CreateTag(EC_TAG_PARTFILE_ED2K_LINK,
		theApp->CreateED2kLink(file, (theApp->IsConnectedED2K() && !theApp->serverconnect->IsLowID())), this);
}

CEC_SharedFile_Tag::CEC_SharedFile_Tag(const CKnownFile *file, EC_DETAIL_LEVEL detail_level) : CECTag(EC_TAG_KNOWNFILE, file->GetFileHash())
{
	AddTag(CECTag(EC_TAG_KNOWNFILE_REQ_COUNT, file->statistic.GetRequests()));
	AddTag(CECTag(EC_TAG_KNOWNFILE_REQ_COUNT_ALL, file->statistic.GetAllTimeRequests()));
	
	AddTag(CECTag(EC_TAG_KNOWNFILE_ACCEPT_COUNT, file->statistic.GetAccepts()));
	AddTag(CECTag(EC_TAG_KNOWNFILE_ACCEPT_COUNT_ALL, file->statistic.GetAllTimeAccepts()));

	AddTag(CECTag(EC_TAG_KNOWNFILE_XFERRED, file->statistic.GetTransferred()));
	AddTag(CECTag(EC_TAG_KNOWNFILE_XFERRED_ALL, file->statistic.GetAllTimeTransferred()));
	AddTag(CECTag(EC_TAG_KNOWNFILE_AICH_MASTERHASH, file->GetAICHMasterHash()));
	
	AddTag(CECTag(EC_TAG_PARTFILE_PRIO,
		(uint8)(file->IsAutoUpPriority() ? file->GetUpPriority() + 10 : file->GetUpPriority())));

	if (detail_level == EC_DETAIL_UPDATE) {
			return;
	}
	
	AddTag(CECTag(EC_TAG_PARTFILE_NAME,file->GetFileName().GetPrintable()));

	AddTag(CECTag(EC_TAG_PARTFILE_SIZE_FULL, file->GetFileSize()));

	AddTag(CECTag(EC_TAG_PARTFILE_ED2K_LINK,
			theApp->CreateED2kLink(file, (theApp->IsConnectedED2K() && !theApp->serverconnect->IsLowID()))));
}

CEC_UpDownClient_Tag::CEC_UpDownClient_Tag(const CUpDownClient* client, EC_DETAIL_LEVEL detail_level) :
	CECTag(EC_TAG_CLIENT, client->GetUserIDHybrid())
{
	// General
	AddTag(CECTag(EC_TAG_CLIENT_NAME, client->GetUserName()));
	AddTag(CECTag(EC_TAG_CLIENT_HASH, client->GetUserHash()));
	AddTag(CECTag(EC_TAG_CLIENT_SCORE, client->GetScore(false, client->IsDownloading(), false)));
	AddTag(CECTag(EC_TAG_CLIENT_RATING, client->GetRating()));
	AddTag(CECTag(EC_TAG_CLIENT_SOFTWARE, client->GetClientSoft()));
	AddTag(CECTag(EC_TAG_CLIENT_SOFT_VER_STR, client->GetSoftVerStr()));
	AddTag(CECTag(EC_TAG_CLIENT_USER_IP, client->GetConnectIP()));
	AddTag(CECTag(EC_TAG_CLIENT_USER_PORT, client->GetUserPort()));
	AddTag(CECTag(EC_TAG_CLIENT_FROM, (uint64)client->GetSourceFrom()));
	AddTag(CECTag(EC_TAG_CLIENT_SERVER_IP, client->GetServerIP()));
	AddTag(CECTag(EC_TAG_CLIENT_SERVER_PORT, client->GetServerPort()));
	AddTag(CECTag(EC_TAG_CLIENT_SERVER_NAME, client->GetServerName()));
	
	// Transfers to Client
	AddTag(CECTag(EC_TAG_CLIENT_UP_SPEED, client->GetUploadDatarate()));
	if (client->GetDownloadState() == DS_DOWNLOADING) {
		AddTag(CECTag(EC_TAG_CLIENT_DOWN_SPEED, (uint64)(client->GetKBpsDown()*1024.0)));
	}
	AddTag(CECTag(EC_TAG_CLIENT_UPLOAD_SESSION, client->GetSessionUp()));
	AddTag(CECTag(EC_TAG_PARTFILE_SIZE_XFER, client->GetTransferredDown()));
	AddTag(CECTag(EC_TAG_CLIENT_UPLOAD_TOTAL, client->GetUploadedTotal()));
	AddTag(CECTag(EC_TAG_CLIENT_DOWNLOAD_TOTAL, client->GetDownloadedTotal()));
	
	AddTag(CECTag(EC_TAG_CLIENT_STATE,
		uint64((uint16)client->GetDownloadState() | (((uint16)client->GetUploadState()) << 8) )));
	AddTag(CECTag(EC_TAG_CLIENT_IDENT_STATE, (uint64) client->GetCurrentIdentState()));
	AddTag(CECTag(EC_TAG_CLIENT_OBFUSCATED_CONNECTION, client->HasObfuscatedConnectionBeenEstablished()));
	AddTag(CECTag(EC_TAG_CLIENT_WAIT_TIME, client->GetWaitTime()));
	AddTag(CECTag(EC_TAG_CLIENT_XFER_TIME, client->GetUpStartTimeDelay()));
	AddTag(CECTag(EC_TAG_CLIENT_QUEUE_TIME, (uint64)(::GetTickCount() - client->GetWaitStartTime())));
	AddTag(CECTag(EC_TAG_CLIENT_LAST_TIME, (uint64)(::GetTickCount() - client->GetLastUpRequest())));
	AddTag(CECTag(EC_TAG_CLIENT_WAITING_POSITION, theApp->uploadqueue->GetWaitingPosition(client)));
	
	if (detail_level == EC_DETAIL_UPDATE) {
			return;
	}
	const CKnownFile* file = client->GetUploadFile();
	if (file) {
		AddTag(CECTag(EC_TAG_PARTFILE_NAME, file->GetFileName().GetPrintable()));
		AddTag(CECTag(EC_TAG_KNOWNFILE, file->GetFileHash()));
	}
	
}

CEC_UpDownClient_Tag::CEC_UpDownClient_Tag(const CUpDownClient* client, CValueMap &valuemap) :
	CECTag(EC_TAG_CLIENT, client->GetUserIDHybrid())
{
	// General
	valuemap.CreateTag(EC_TAG_CLIENT_NAME, client->GetUserName(), this);
	valuemap.CreateTag(EC_TAG_CLIENT_HASH, client->GetUserHash(), this);
	valuemap.CreateTag(EC_TAG_CLIENT_SCORE, client->GetScore(false), this);
	valuemap.CreateTag(EC_TAG_CLIENT_SOFTWARE, client->GetClientSoft(), this);
	valuemap.CreateTag(EC_TAG_CLIENT_SOFT_VER_STR, client->GetSoftVerStr(), this);
	valuemap.CreateTag(EC_TAG_CLIENT_USER_IP, client->GetConnectIP(), this);
	valuemap.CreateTag(EC_TAG_CLIENT_USER_PORT, client->GetUserPort(), this);
	valuemap.CreateTag(EC_TAG_CLIENT_FROM, (uint64)client->GetSourceFrom(), this);
	valuemap.CreateTag(EC_TAG_CLIENT_SERVER_IP, client->GetServerIP(), this);
	valuemap.CreateTag(EC_TAG_CLIENT_SERVER_PORT, client->GetServerPort(), this);
	valuemap.CreateTag(EC_TAG_CLIENT_SERVER_NAME, client->GetServerName(), this);
	
	// Transfers to Client
	valuemap.CreateTag(EC_TAG_CLIENT_UP_SPEED, client->GetUploadDatarate(), this);
	valuemap.CreateTag(EC_TAG_CLIENT_DOWN_SPEED, (uint64)(client->GetKBpsDown()*1024.0), this);
	valuemap.CreateTag(EC_TAG_CLIENT_UPLOAD_SESSION, client->GetSessionUp(), this);
	valuemap.CreateTag(EC_TAG_PARTFILE_SIZE_XFER, client->GetTransferredDown(), this);
	valuemap.CreateTag(EC_TAG_CLIENT_UPLOAD_TOTAL, client->GetUploadedTotal(), this);
	valuemap.CreateTag(EC_TAG_CLIENT_DOWNLOAD_TOTAL, client->GetDownloadedTotal(), this);
	
	valuemap.CreateTag(EC_TAG_CLIENT_STATE,
		uint64((uint16)client->GetDownloadState() | (((uint16)client->GetUploadState()) << 8) ), this);
	valuemap.CreateTag(EC_TAG_CLIENT_WAIT_TIME, client->GetWaitTime(), this);
	valuemap.CreateTag(EC_TAG_CLIENT_XFER_TIME, client->GetUpStartTimeDelay(), this);
	valuemap.CreateTag(EC_TAG_CLIENT_QUEUE_TIME, (uint64)(::GetTickCount() - client->GetWaitStartTime()), this);
	valuemap.CreateTag(EC_TAG_CLIENT_LAST_TIME, (uint64)(::GetTickCount() - client->GetLastUpRequest()), this);
	valuemap.CreateTag(EC_TAG_CLIENT_WAITING_POSITION, theApp->uploadqueue->GetWaitingPosition(client), this);
	
	const CKnownFile* file = client->GetUploadFile();
	if (file) {
		valuemap.CreateTag(EC_TAG_PARTFILE_NAME, file->GetFileName().GetPrintable(), this);
		valuemap.CreateTag(EC_TAG_KNOWNFILE, file->GetFileHash(), this);
	}
	
}

//
// Search reply
//
CEC_SearchFile_Tag::CEC_SearchFile_Tag(CSearchFile *file, EC_DETAIL_LEVEL detail_level) : CECTag(EC_TAG_SEARCHFILE, file->GetFileHash())
{
	AddTag(CECTag(EC_TAG_PARTFILE_SOURCE_COUNT, file->GetSourceCount()));
	AddTag(CECTag(EC_TAG_PARTFILE_SOURCE_COUNT_XFER, file->GetCompleteSourceCount()));

	if (detail_level == EC_DETAIL_UPDATE) {
			return;
	}

	AddTag(CECTag(EC_TAG_PARTFILE_NAME, file->GetFileName().GetPrintable()));
	AddTag(CECTag(EC_TAG_PARTFILE_SIZE_FULL, file->GetFileSize()));
	if ( theApp->sharedfiles->GetFileByID(file->GetFileHash()) ) {
		AddTag(CECEmptyTag(EC_TAG_KNOWNFILE));
	}
}

CEC_SearchFile_Tag::CEC_SearchFile_Tag(CSearchFile *file, CValueMap &valuemap) : CECTag(EC_TAG_SEARCHFILE, file->GetFileHash())
{
	valuemap.CreateTag(EC_TAG_PARTFILE_SOURCE_COUNT, file->GetSourceCount(), this);

	valuemap.CreateTag(EC_TAG_PARTFILE_SOURCE_COUNT_XFER, file->GetCompleteSourceCount(), this);

	valuemap.CreateTag(EC_TAG_PARTFILE_NAME, file->GetFileName().GetPrintable(), this);

	valuemap.CreateTag(EC_TAG_PARTFILE_SIZE_FULL, file->GetFileSize(), this);

	if ( theApp->sharedfiles->GetFileByID(file->GetFileHash()) ) {
		AddTag(CECEmptyTag(EC_TAG_KNOWNFILE));
	}
}
// File_checked_for_headers
