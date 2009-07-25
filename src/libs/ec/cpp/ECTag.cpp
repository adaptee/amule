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

#ifdef __DEBUG__
#define DEBUG_EC_IMPLEMENTATION

#include <common/Format.h>  // Needed for CFormat
#endif

#include "ECTag.h"	// Needed for ECTag
#include "ECSocket.h"	// Needed for CECSocket
#include "ECSpecialTags.h"	// Needed for CValueMap

/**********************************************************
 *							  *
 *	CECTag class					  *
 *							  *
 **********************************************************/

//! Defines the Null tag which may be returned by GetTagByNameSafe.
const CECTag CECTag::s_theNullTag;

//! Defines the data for the Null tag.  Large enough (16 bytes) for GetMD4Data.
const uint32 CECTag::s_theNullTagData[4] = { 0, 0, 0, 0 };

/**
 * Creates a new null-valued CECTag instance
 *
 * @see s_theNullTag
 * @see s_theNullTagData
 * @see GetTagByNameSafe
 */
CECTag::CECTag() :
	m_error(0),
	m_tagData(s_theNullTagData),
	m_tagName(0),
	m_dataLen(0),
	m_dataType(EC_TAGTYPE_UNKNOWN),
	m_dynamic(false),
	m_tagList(),
	m_haschildren(false)
{
}

/**
 * Creates a new CECTag instance from the given data
 *
 * @param name	 TAG name
 * @param length length of data buffer
 * @param data	 TAG data
 * @param copy	 whether to create a copy of the TAG data at \e *data, or should use the provided pointer.
 *
 * \note When you set \e copy to \b false, the provided data buffer must exist
 * in the whole lifetime of the packet.
 */
CECTag::CECTag(ec_tagname_t name, unsigned int length, const void *data, bool copy) : m_tagName(name), m_dynamic(copy), m_haschildren( false )
{
	m_error = 0;
	m_dataLen = length;
	if (copy && (data != NULL)) {
		m_tagData = malloc(m_dataLen);
		if (m_tagData != NULL) {
			memcpy((void *)m_tagData, data, m_dataLen);
		} else {
			m_dataLen = 0;
			m_error = 1;
		}
	} else {
		m_tagData = data;
	}
	m_dataType = EC_TAGTYPE_CUSTOM;
}

/**
 * Creates a new CECTag instance for custom data
 *
 * @param name	 	TAG name
 * @param length 	length of data buffer that will be alloc'ed
 * @param dataptr	pointer to internal TAG data buffer
 *
 * 
 */
CECTag::CECTag(ec_tagname_t name, unsigned int length, void **dataptr)  : m_tagName(name), m_dynamic(true), m_haschildren( false )
{
	m_error = 0;
	m_dataLen = length;
	m_tagData = malloc(m_dataLen);
	if ( !m_tagData ) {
		m_dataLen = 0;
		m_error = 1;
	}
	*dataptr = (void *)m_tagData;
	m_dataType = EC_TAGTYPE_CUSTOM;
}

/**
 * Creates a new CECTag instance, which contains an IPv4 address.
 *
 * This function takes care of the endianness of the port number.
 *
 * @param name TAG name
 * @param data The EC_IPv4_t class containing the IPv4 address.
 *
 * @see GetIPv4Data()
 */
CECTag::CECTag(ec_tagname_t name, const EC_IPv4_t& data) : m_tagName(name), m_dynamic(true), m_haschildren( false )
{

	m_dataLen = sizeof(EC_IPv4_t);
	m_tagData = malloc(sizeof(EC_IPv4_t));
	if (m_tagData != NULL) {
		RawPokeUInt32( ((EC_IPv4_t *)m_tagData)->m_ip, RawPeekUInt32( data.m_ip ) );
		((EC_IPv4_t *)m_tagData)->m_port = ENDIAN_HTONS(data.m_port);
		m_error = 0;
		m_dataType = EC_TAGTYPE_IPV4;
	} else {
		m_error = 1;
	}
}

/**
 * Creates a new CECTag instance, which contains a MD4 hash.
 *
 * This function takes care to store hash in network byte order.
 *
 * @param name TAG name
 * @param data The CMD4Hash class containing the MD4 hash.
 *
 * @see GetMD4Data()
 */
CECTag::CECTag(ec_tagname_t name, const CMD4Hash& data) : m_tagName(name), m_dynamic(true), m_haschildren( false )
{

	m_dataLen = 16;
	m_tagData = malloc(16);
	if (m_tagData != NULL) {
		RawPokeUInt64( (char*)m_tagData,		RawPeekUInt64( data.GetHash() ) );
		RawPokeUInt64( (char*)m_tagData + 8,	RawPeekUInt64( data.GetHash() + 8 ) );
		m_error = 0;
		m_dataType = EC_TAGTYPE_HASH16;
	} else {
		m_error = 1;
	}
}

/**
 * Creates a new CECTag instance, which contains a string
 *
 * @param name TAG name
 * @param data wxString object, it's contents are converted to UTF-8.
 *
 * @see GetStringDataSTL()
 */
CECTag::CECTag(ec_tagname_t name, const std::string& data) : m_tagName(name), m_dynamic(true), m_haschildren( false )
{
	ConstructStringTag(name,data);
}

/**
 * Creates a new CECTag instance, which contains a string
 *
 * @param name TAG name
 * @param data wxString object, it's contents are converted to UTF-8.
 *
 * @see GetStringData()
 */
CECTag::CECTag(ec_tagname_t name, const wxString& data) : m_tagName(name), m_dynamic(true), m_haschildren( false )
{
	ConstructStringTag(name, (const char*)unicode2UTF8(data));
}
CECTag::CECTag(ec_tagname_t name, const wxChar* data) : m_tagName(name), m_dynamic(true), m_haschildren( false )
{
	ConstructStringTag(name, (const char*)unicode2UTF8(data));
}

/**
 * Copy constructor
 */
CECTag::CECTag(const CECTag& tag) : m_state( tag.m_state ), m_tagName( tag.m_tagName ), m_dynamic( tag.m_dynamic ), m_haschildren( tag.m_haschildren )
{
	m_error = 0;
	m_dataLen = tag.m_dataLen;
	m_dataType = tag.m_dataType;
	if (m_dataLen != 0) {
		if (m_dynamic) {
			m_tagData = malloc(m_dataLen);
			if (m_tagData != NULL) {
				memcpy((void *)m_tagData, tag.m_tagData, m_dataLen);
			} else {
				m_dataLen = 0;
				m_error = 1;
				return;
			}
		} else {
			m_tagData = tag.m_tagData;
		}
	} else m_tagData = NULL;
	if (!tag.m_tagList.empty()) {
		m_tagList.reserve(tag.m_tagList.size());
		for (TagList::size_type i=0; i<tag.m_tagList.size(); i++) {
			m_tagList.push_back(tag.m_tagList[i]);
			if (m_tagList.back().m_error != 0) {
				m_error = m_tagList.back().m_error;
#ifndef KEEP_PARTIAL_PACKETS
				m_tagList.pop_back();
#endif
				break;
			}
		}
	}
}

/**
 * Creates a new CECTag instance, which contains an int value.
 *
 * This takes care of endianness problems with numbers.
 *
 * @param name TAG name.
 * @param data number.
 *
 * @see GetInt()
 */
CECTag::CECTag(ec_tagname_t name, bool data) : m_tagName(name), m_dynamic(true)
{
	InitInt(data);
}
CECTag::CECTag(ec_tagname_t name, uint8 data) : m_tagName(name), m_dynamic(true)
{
	InitInt(data);
}
CECTag::CECTag(ec_tagname_t name, uint16 data) : m_tagName(name), m_dynamic(true)
{
	InitInt(data);
}
CECTag::CECTag(ec_tagname_t name, uint32 data) : m_tagName(name), m_dynamic(true)
{
	InitInt(data);
}
CECTag::CECTag(ec_tagname_t name, uint64 data) : m_tagName(name), m_dynamic(true)
{
	InitInt(data);
}

void CECTag::InitInt(uint64 data)
{
	if (data <= 0xFF) {
		m_dataType = EC_TAGTYPE_UINT8;
		m_dataLen = 1;
	} else if (data <= 0xFFFF) {
		m_dataType = EC_TAGTYPE_UINT16;
		m_dataLen = 2;
	} else if (data <= 0xFFFFFFFF) {
		m_dataType = EC_TAGTYPE_UINT32;
		m_dataLen = 4;
	} else {
		m_dataType = EC_TAGTYPE_UINT64;
		m_dataLen = 8;
	}	
	
	m_tagData = malloc(m_dataLen);
	
	if (m_tagData != NULL) {
		switch (m_dataType) {
			case EC_TAGTYPE_UINT8:
				PokeUInt8( (void*)m_tagData, (uint8) data );
				break;
			case EC_TAGTYPE_UINT16:
				PokeUInt16( (void*)m_tagData, wxUINT16_SWAP_ALWAYS((uint16) data ));
				break;
			case EC_TAGTYPE_UINT32:
				PokeUInt32( (void*)m_tagData, wxUINT32_SWAP_ALWAYS((uint32) data ));
				break;
			case EC_TAGTYPE_UINT64:
				PokeUInt64( (void*)m_tagData, wxUINT64_SWAP_ALWAYS(data) );
				break;
			default:
				/* WTF?*/
				EC_ASSERT(0);
				free((void*)m_tagData);
				m_error = 1;
				return;
		}
		m_error = 0;
	} else {
		m_error = 1;
	}
}

/**
 * Creates a new CECTag instance, which contains a double precision floating point number
 *
 * @param name TAG name
 * @param data double number
 *
 * @note The actual data is converted to string representation, because we have not found
 * yet an effective and safe way to transmit floating point numbers.
 *
 * @see GetDoubleData()
 */
CECTag::CECTag(ec_tagname_t name, double data) : m_tagName(name), m_dynamic(true)
{
	std::ostringstream double_str;
	double_str << data;
	m_dataLen = (ec_taglen_t)strlen(double_str.str().c_str()) + 1;
	m_tagData = malloc(m_dataLen);
	if (m_tagData != NULL) {
		memcpy((void *)m_tagData, double_str.str().c_str(), m_dataLen);
		m_dataType = EC_TAGTYPE_DOUBLE;
		m_error = 0;
	} else {
		m_error = 1;
	}
}

/**
 * Destructor - frees allocated data and deletes child TAGs.
 */
CECTag::~CECTag(void)
{
	if (m_dynamic) free((void *)m_tagData);
}

/**
 * Copy assignment operator.
 *
 * std::vector uses this, but the compiler-supplied version wouldn't properly
 * handle m_dynamic and m_tagData.  This wouldn't be necessary if m_tagData
 * was a smart pointer (Hi, Kry!).
 */
CECTag& CECTag::operator=(const CECTag& rhs)
{
	if (&rhs != this)
	{
		// This is a trick to reuse the implementation of the copy constructor
		// so we don't have to duplicate it here.  temp is constructed as a
		// copy of rhs, which properly handles m_dynamic and m_tagData.  Then
		// temp's members are swapped for this object's members.  So,
		// effectively, this object has been made a copy of rhs, which is the
		// point.  Then temp is destroyed as it goes out of scope, so its
		// destructor cleans up whatever data used to belong to this object.
		CECTag temp(rhs);
		std::swap(m_error,	temp.m_error);
		std::swap(m_tagData,	temp.m_tagData);
		std::swap(m_tagName,	temp.m_tagName);
		std::swap(m_dataLen,	temp.m_dataLen);
		std::swap(m_dynamic,	temp.m_dynamic);
		std::swap(m_tagList,	temp.m_tagList);
		std::swap(m_state,	temp.m_state);
	}

	return *this;
}

/**
 * Compare operator.
 *
 */
bool CECTag::operator==(const CECTag& tag) const
{
	return	m_dataType == tag.m_dataType
			&& m_dataLen == tag.m_dataLen
			&&	(m_dataLen == 0
				|| !memcmp(m_tagData, tag.m_tagData, m_dataLen))
			&& m_tagList == tag.m_tagList;
}

/**
 * Add a child tag to this one.
 *
 * Be very careful that this creates a copy of \e tag. Thus, the following code won't work as expected:
 * \code
 * {
 *	CECPacket *p = new CECPacket(whatever);
 *	CECTag *t1 = new CECTag(whatever);
 *	CECTag *t2 = new CECTag(whatever);
 *	p.AddTag(*t1);
 *	t1.AddTag(*t2);	// t2 won't be part of p !!!
 * }
 * \endcode
 *
 * To get the desired results, the above should be replaced with something like:
 *
 * \code
 * {
 *	CECPacket *p = new CECPacket(whatever);
 *	CECTag *t1 = new CECTag(whatever);
 *	CECTag *t2 = new CECTag(whatever);
 *	t1.AddTag(*t2);
 *	delete t2;	// we can safely delete t2 here, because t1 holds a copy
 *	p.AddTag(*t1);
 *	delete t1;	// now p holds a copy of both t1 and t2
 * }
 * \endcode
 *
 * Then why copying? The answer is to enable simplifying the code like this:
 *
 * \code
 * {
 *	CECPacket *p = new CECPacket(whatever);
 *	CECTag t1(whatever);
 *	t1.AddTag(CECTag(whatever));	// t2 is now created on-the-fly
 *	p.AddTag(t1);	// now p holds a copy of both t1 and t2
 * }
 * \endcode
 *
 * @param tag a CECTag class instance to add.
 * @return \b true on succcess, \b false when an error occured
 */
bool CECTag::AddTag(const CECTag& tag, CValueMap* valuemap)
{
	if (valuemap) {
		return valuemap->AddTag(tag, this);
	}
	// cannot have more than 64k tags
	wxASSERT(m_tagList.size() < 0xffff);

	m_tagList.push_back(tag);
	if (m_tagList.back().m_error == 0) {
		return true;
	} else {
		m_error = m_tagList.back().m_error;
#ifndef KEEP_PARTIAL_PACKETS
		m_tagList.pop_back();
#endif
		return false;
	}
}

void CECTag::AddTag(ec_tagname_t name, uint64_t data, CValueMap* valuemap)
{
	if (valuemap) {
		valuemap->CreateTag(name, data, this);
	} else {
		AddTag(CECTag(name, data));
	}
}

void CECTag::AddTag(ec_tagname_t name, const wxString& data, CValueMap* valuemap)
{
	if (valuemap) {
		valuemap->CreateTag(name, data, this);
	} else {
		AddTag(CECTag(name, data));
	}
}

bool CECTag::ReadFromSocket(CECSocket& socket)
{
	if (m_state == bsName) {
		ec_tagname_t tmp_tagName;
		if (!socket.ReadNumber(&tmp_tagName, sizeof(ec_tagname_t))) {
			m_tagName = 0;
			return false;
		} else {
			m_tagName = tmp_tagName >> 1;
			m_haschildren = (tmp_tagName & 0x01) ? true : false;
			m_state = bsType;
		}
	}
	
	if (m_state == bsType) {
		ec_tagtype_t type;
		if (!socket.ReadNumber(&type, sizeof(ec_tagtype_t))) {
			m_dataType = EC_TAGTYPE_UNKNOWN;
			return false;
		} else {
			m_dataType = type;
			if (m_haschildren) {
				m_state = bsLengthChld;
			} else {
				m_state = bsLength;
			}
		}
	}
	
	if (m_state == bsLength || m_state == bsLengthChld) {
		ec_taglen_t tagLen;
		if (!socket.ReadNumber(&tagLen, sizeof(ec_taglen_t))) {
			return false;
		} else {
			m_dataLen = tagLen;
			if (m_state == bsLength) {
				m_state = bsData1;
			} else {
				m_state = bsChildCnt;
			}
		}
	}
	
	if (m_state == bsChildCnt || m_state == bsChildren) {
		if (!ReadChildren(socket)) {
			return false;
		}
	}
	
	if (m_state == bsData1) {
		unsigned int tmp_len = m_dataLen;
		m_dataLen = 0;
		m_dataLen = tmp_len - GetTagLen();
		if (m_dataLen > 0) {
			m_tagData = malloc(m_dataLen);
			m_state = bsData2;
		} else {
			m_tagData = NULL;
			m_state = bsFinished;
		}
	}
	
	if (m_state == bsData2) {
		if (m_tagData != NULL) {
			if (!socket.ReadBuffer((void *)m_tagData, m_dataLen)) {
				return false;
			} else {
				m_state = bsFinished;
			}
		} else {
			m_error = 1;
			return false;
		}
	}
	
	return true;
}


bool CECTag::WriteTag(CECSocket& socket) const
{
	ec_tagname_t tmp_tagName = (m_tagName << 1) | (m_tagList.empty() ? 0 : 1);
	ec_tagtype_t type = m_dataType;
	ec_taglen_t tagLen = GetTagLen();
	wxASSERT(type != EC_TAGTYPE_UNKNOWN);
	
	if (!socket.WriteNumber(&tmp_tagName, sizeof(ec_tagname_t))) return false;
	if (!socket.WriteNumber(&type, sizeof(ec_tagtype_t))) return false;
	if (!socket.WriteNumber(&tagLen, sizeof(ec_taglen_t))) return false;
	
	if (!m_tagList.empty()) {
		if (!WriteChildren(socket)) return false;
	}
	
	if (m_dataLen > 0) {
		if (m_tagData != NULL) {	// This is here only to make sure everything, it should not be NULL at this point
			if (!socket.WriteBuffer(m_tagData, m_dataLen)) return false;
		}
	}
	
	return true;
}

bool CECTag::ReadChildren(CECSocket& socket)
{
	if (m_state == bsChildCnt) {
		uint16 tmp_tagCount;
		if (!socket.ReadNumber(&tmp_tagCount, sizeof(uint16))) {
			return false;
		} else {
			m_tagList.clear();
			if (tmp_tagCount > 0) {
				m_tagList.reserve(tmp_tagCount);
				for (int i=0; i<tmp_tagCount; i++) {
					m_tagList.push_back(CECTag(socket));
				}
				m_haschildren = true;
				m_state = bsChildren;
			} else {
				m_state = bsData1;
			}
		}
	}
	
	if (m_state == bsChildren) {
		for (unsigned int i=0; i<m_tagList.size(); i++) {
			CECTag& tag = m_tagList[i];
			if (!tag.IsOk()) {
				if (!tag.ReadFromSocket(socket)) {
					if (tag.m_error != 0) {
						m_error = tag.m_error;
					}
					return false;
				}
			}
		}
		m_state = bsData1;
	}
	
	return true;
}

bool CECTag::WriteChildren(CECSocket& socket) const
{
	wxASSERT(m_tagList.size() < 0xFFFF);
    uint16 tmp = (uint16)m_tagList.size();
	if (!socket.WriteNumber(&tmp, sizeof(tmp))) return false;
	if (!m_tagList.empty()) {
		for (TagList::size_type i=0; i<m_tagList.size(); i++) {
			if (!m_tagList[i].WriteTag(socket)) return false;
		}
	}
	return true;
}

/**
 * Finds the (first) child tag with given name.
 *
 * @param name TAG name to look for.
 * @return the tag found, or NULL.
 */
const CECTag* CECTag::GetTagByName(ec_tagname_t name) const
{
	for (TagList::size_type i=0; i<m_tagList.size(); i++)
		if (m_tagList[i].m_tagName == name) return &m_tagList[i];
	return NULL;
}

/**
 * Finds the (first) child tag with given name.
 *
 * @param name TAG name to look for.
 * @return the tag found, or NULL.
 */
CECTag* CECTag::GetTagByName(ec_tagname_t name)
{
	for (TagList::size_type i=0; i<m_tagList.size(); i++)
		if (m_tagList[i].m_tagName == name) return &m_tagList[i];
	return NULL;
}

/**
 * Finds the (first) child tag with given name.
 *
 * @param name TAG name to look for.
 * @return the tag found, or a special null-valued tag otherwise.
 *
 * @see s_theNullTag
 */
const CECTag* CECTag::GetTagByNameSafe(ec_tagname_t name) const
{
	const CECTag* result = GetTagByName(name);
	if (result == NULL)
		result = &s_theNullTag;
	return result;
}

/**
 * Query TAG length that is suitable for the TAGLEN field (i.e.\ 
 * without it's own header size).
 *
 * @return Tag length, containing its childs' length.
 */
uint32 CECTag::GetTagLen(void) const
{
	uint32 length = m_dataLen;
	for (TagList::size_type i=0; i<m_tagList.size(); i++) {
		length += m_tagList[i].GetTagLen();
		length += sizeof(ec_tagname_t) + sizeof(ec_tagtype_t) + sizeof(ec_taglen_t) + ((m_tagList[i].GetTagCount() > 0) ? 2 : 0);
	}
	return length;
}


uint64_t CECTag::GetInt() const
{
	if (m_tagData == NULL) {
		// Empty tag - This is NOT an error.
		EC_ASSERT(m_dataType == EC_TAGTYPE_UNKNOWN);
		return 0;
	}

	switch (m_dataType) {
		case EC_TAGTYPE_UINT8:
			return PeekUInt8(m_tagData);
		case EC_TAGTYPE_UINT16:
			return ENDIAN_NTOHS( RawPeekUInt16( m_tagData ) );
		case EC_TAGTYPE_UINT32:
			return ENDIAN_NTOHL( RawPeekUInt32( m_tagData ) );
		case EC_TAGTYPE_UINT64:
			return ENDIAN_NTOHLL( RawPeekUInt64( m_tagData ) );
		case EC_TAGTYPE_UNKNOWN:
			// Empty tag - This is NOT an error.
			return 0;
		default:
			EC_ASSERT(0);
			return 0;
	}
}


std::string CECTag::GetStringDataSTL() const
{ 
	if (m_dataType != EC_TAGTYPE_STRING) {
		EC_ASSERT(m_dataType == EC_TAGTYPE_UNKNOWN);
		return std::string();
	} else if (m_tagData == NULL) {
		EC_ASSERT(false);
		return std::string();
	}

	return std::string((const char*)m_tagData);
}


#ifdef USE_WX_EXTENSIONS
wxString CECTag::GetStringData() const
{ 
	return UTF82unicode(GetStringDataSTL().c_str());
}
#endif


CMD4Hash CECTag::GetMD4Data() const
{ 
	if (m_dataType != EC_TAGTYPE_HASH16) {
		EC_ASSERT(m_dataType == EC_TAGTYPE_UNKNOWN);
		return CMD4Hash();
	}

	EC_ASSERT(m_tagData != NULL);

	// Doesn't matter if m_tagData is NULL in CMD4Hash(), 
	// that'll just result in an empty hash.
	return CMD4Hash((const unsigned char *)m_tagData); 
}


/**
 * Returns an EC_IPv4_t class.
 *
 * This function takes care of the enadianness of the port number.
 *
 * @return EC_IPv4_t class.
 *
 * @see CECTag(ec_tagname_t, const EC_IPv4_t&)
 */
EC_IPv4_t CECTag::GetIPv4Data() const
{
	EC_IPv4_t p(0, 0);
	
	if (m_tagData == NULL) {
		EC_ASSERT(false);
	} else if (m_dataType != EC_TAGTYPE_IPV4) {
		EC_ASSERT(false);
	} else {
		RawPokeUInt32( p.m_ip, RawPeekUInt32( ((EC_IPv4_t *)m_tagData)->m_ip ) );
		p.m_port = ENDIAN_NTOHS(((EC_IPv4_t *)m_tagData)->m_port);
	}

	return p;
}

/**
 * Returns a double value.
 *
 * @note The returned value is what we get by converting the string form
 * of the number to a double.
 *
 * @return The double value of the tag.
 *
 * @see CECTag(ec_tagname_t, double)
 */
double CECTag::GetDoubleData(void) const
{
	if (m_dataType != EC_TAGTYPE_DOUBLE) {
		EC_ASSERT(m_dataType == EC_TAGTYPE_UNKNOWN);
		return 0;
	} else if (m_tagData == NULL) {
		EC_ASSERT(false);
		return 0;
	}
	
	std::istringstream double_str((const char*)m_tagData);
	
	double data;
	double_str >> data;
	return data;
}


void CECTag::ConstructStringTag(ec_tagname_t /*name*/, const std::string& data)
{
	m_dataLen = (ec_taglen_t)strlen(data.c_str()) + 1;
	m_tagData = malloc(m_dataLen);
	if (m_tagData != NULL) {
		memcpy((void *)m_tagData, data.c_str(), m_dataLen);
		m_error = 0;
		m_dataType = EC_TAGTYPE_STRING;
	} else {
		m_error = 1;
	}	
}

#if	__DEBUG__
void CECTag::DebugPrint(int level, bool print_empty) const
{
	if (m_dataLen || print_empty) {
		wxString space;
		for (int i = level; i--;) space += wxT("  ");
		wxString s1 = CFormat(wxT("%s%s %d = ")) % space % GetDebugNameECTagNames(m_tagName) % m_dataLen;
		wxString s2;
		switch (m_tagName) {
			case EC_TAG_DETAIL_LEVEL:
				s2 = GetDebugNameEC_DETAIL_LEVEL(GetInt()); break;
			case EC_TAG_SEARCH_TYPE:
				s2 = GetDebugNameEC_SEARCH_TYPE(GetInt()); break;
			case EC_TAG_STAT_VALUE_TYPE:
				s2 = GetDebugNameEC_STATTREE_NODE_VALUE_TYPE(GetInt()); break;
			default:
				switch (m_dataType) {
					case EC_TAGTYPE_UINT8:
					case EC_TAGTYPE_UINT16:
					case EC_TAGTYPE_UINT32:
					case EC_TAGTYPE_UINT64:
						s2 = CFormat(wxT("%d")) % GetInt(); break;
					case EC_TAGTYPE_STRING:
						s2 = GetStringData(); break;
					case EC_TAGTYPE_DOUBLE:
						s2 = CFormat(wxT("%.1f")) % GetDoubleData(); break;
					case EC_TAGTYPE_HASH16:
						s2 = GetMD4Data().Encode(); break;
					default:
						s2 = GetDebugNameECTagTypes(m_dataType);
				}
		}
		DoECLogLine(s1 + s2);
	}
	for (TagList::const_iterator it = m_tagList.begin(); it != m_tagList.end(); ++it) {
		it->DebugPrint(level + 1, true);
	}
}
#endif

/*!
 * \fn CMD4Hash CECTag::GetMD4Data(void) const
 *
 * \brief Returns a CMD4Hash class.
 *
 * This function takes care of converting from MSB to LSB as necessary.
 *
 * \return CMD4Hash class.
 *
 * \sa CECTag(ec_tagname_t, const CMD4Hash&)
 */

/*!
 * \fn CECTag *CECTag::GetTagByIndex(size_t index) const
 *
 * \brief Finds the indexth child tag.
 *
 * \param index 0-based index, 0 <= \e index < GetTagCount()
 *
 * \return The child tag, or NULL if index out of range.
 */

/*!
 * \fn CECTag *CECTag::GetTagByIndexSafe(size_t index) const
 *
 * \brief Finds the indexth child tag.
 *
 * \param index 0-based index, 0 <= \e index < GetTagCount()
 *
 * \return The child tag, or a special null-valued tag if index out of range.
 */

/*!
 * \fn size_t CECTag::GetTagCount(void) const
 *
 * \brief Returns the number of child tags.
 *
 * \return The number of child tags.
 */

/*!
 * \fn const void *CECTag::GetTagData(void) const
 *
 * \brief Returns a pointer to the TAG DATA.
 *
 * \return A pointer to the TAG DATA. (As specified with the data field of the constructor.)
*/

/*!
 * \fn uint16 CECTag::GetTagDataLen(void) const
 *
 * \brief Returns the length of the data buffer.
 *
 * \return The length of the data buffer.
 */

/*!
 * \fn ec_tagname_t CECTag::GetTagName(void) const
 *
 * \brief Returns TAGNAME.
 *
 * \return The name of the tag.
 */

/*!
 * \fn wxString CECTag::GetStringData(void) const
 *
 * \brief Returns the string data of the tag.
 *
 * Returns a wxString created from TAGDATA. It is automatically
 * converted from UTF-8 to the internal application encoding.
 * Should be used with care (only on tags created with the
 * CECTag(ec_tagname_t, const wxString&) constructor),
 * becuse it does not perform any check to see if the tag really contains a
 * string object.
 *
 * \return The string data of the tag.
 *
 * \sa CECTag(ec_tagname_t, const wxString&)
 */

/*!
 * \fn uint8 CECTag::GetInt(void) const
 *
 * \brief Returns the uint8 data of the tag.
 *
 * This function takes care of the endianness problems with numbers.
 *
 * \return The uint8 data of the tag.
 *
 * \sa CECTag(ec_tagname_t, uint8)
 */

/*!
 * \fn uint16 CECTag::GetInt(void) const
 *
 * \brief Returns the uint16 data of the tag.
 *
 * This function takes care of the endianness problems with numbers.
 *
 * \return The uint16 data of the tag.
 *
 * \sa CECTag(ec_tagname_t, uint16)
 */

/*!
 * \fn uint32 CECTag::GetInt(void) const
 *
 * \brief Returns the uint32 data of the tag.
 *
 * This function takes care of the endianness problems with numbers.
 *
 * \return The uint32 data of the tag.
 *
 * \sa CECTag(ec_tagname_t, uint32)
 */

/*!
 * \fn uint64 CECTag::GetInt(void) const
 *
 * \brief Returns the uint64 data of the tag.
 *
 * This function takes care of the endianness problems with numbers.
 *
 * \return The uint64 data of the tag.
 *
 * \sa CECTag(ec_tagname_t, uint64)
 */
// File_checked_for_headers
