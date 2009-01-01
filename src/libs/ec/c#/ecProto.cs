//
// This file is part of the aMule Project.
//
// Copyright (c) 2003-2008 aMule Team ( admin@amule.org / http://www.amule.org )
// Copyright (c) 2003-2008 Froenchenko Leonid ( lfroen@gmail.com / http://www.amule.org )
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

using System;
using System.IO;
using System.Security.Cryptography;
using System.Collections.Generic;
using System.Text;

namespace amule.net
{
    public enum EcTagTypes {
        EC_TAGTYPE_UNKNOWN = 0,
        EC_TAGTYPE_CUSTOM = 1,
        EC_TAGTYPE_UINT8 = 2,
        EC_TAGTYPE_UINT16 = 3,
        EC_TAGTYPE_UINT32 = 4,
        EC_TAGTYPE_UINT64 = 5,
        EC_TAGTYPE_STRING = 6,
        EC_TAGTYPE_DOUBLE = 7,
        EC_TAGTYPE_IPV4 = 8,
        EC_TAGTYPE_HASH16 = 9
    };

    public class ecProto {

        public class ecTag {
            protected int m_size;
            protected EcTagTypes m_type;
            protected ECTagNames m_name;
            protected LinkedList<ecTag> m_subtags;

            public ecTag(ECTagNames n, EcTagTypes t)
            {
                m_name = n;
                m_type = t;
                m_subtags = new LinkedList<ecTag>();
            }
            public ecTag(ECTagNames n, EcTagTypes t, LinkedList<ecTag> subtags)
            {
                m_name = n;
                m_type = t;
                m_subtags = subtags; ;
            }

            public ecTag()
            {
                m_subtags = new LinkedList<ecTag>();
            }

            public int SubtagCount()
            {
                return m_subtags.Count;
            }

            protected void WriteSubtags(BinaryWriter wr)
            {
                Int16 count16 = (Int16)m_subtags.Count;
                if (count16 != 0) {
                    wr.Write(System.Net.IPAddress.HostToNetworkOrder(count16));
                    foreach (ecTag t in m_subtags)
                    {
                        t.Write(wr);
                    }
                }
            }

            public ECTagNames Name()
            {
                return m_name;
            }

            public virtual void Write(BinaryWriter wr)
            {
                Int16 name16 = (Int16)(m_name);
                name16 <<= 1;
                byte type8 = (byte)m_type;
                Int32 size32 = (Int32)Size();
                if (m_subtags.Count != 0) {
                    name16 |= 1;
                }
                wr.Write(System.Net.IPAddress.HostToNetworkOrder(name16));
                wr.Write(type8);
                wr.Write(System.Net.IPAddress.HostToNetworkOrder(size32));

                WriteSubtags(wr);
                //
                // here derived class will put actual data
                //
            }

            protected int Size()
            {
                int total_size = m_size;
                foreach (ecTag t in m_subtags) {
                    total_size += t.Size();
                    // name + type + size for each tag
                    total_size += (2 + 1 + 4);
                    if (t.HaveSubtags()) {
                        total_size += 2;
                    }
                }
                return total_size;
            }

            public LinkedList<ecTag>.Enumerator GetTagIterator()
            {
                return m_subtags.GetEnumerator();
            }

            public void AddSubtag(ecTag t)
            {
                m_subtags.AddLast(t);
            }

            bool HaveSubtags()
            {
                return (m_subtags.Count != 0);
            }

            public ecTag SubTag(ECTagNames name)
            {
                foreach (ecTag t in m_subtags) {
                    if (t.m_name == name) {
                        return t;
                    }
                }
                return null;
            }
        }

        public class ecTagInt : ecTag {
            protected Int64 m_val;
            public ecTagInt(ECTagNames n, byte v)
                : base(n, EcTagTypes.EC_TAGTYPE_UINT8)
            {
                m_val = v;
                m_size = 1;
            }

            public ecTagInt(ECTagNames n, Int16 v)
                : base(n, EcTagTypes.EC_TAGTYPE_UINT16)
            {
                m_val = v;
                m_size = 2;
            }

            public ecTagInt(ECTagNames n, Int32 v)
                : base(n, EcTagTypes.EC_TAGTYPE_UINT32)
            {
                m_val = v;
                m_size = 4;
            }

            public ecTagInt(ECTagNames n, Int64 v)
                : base(n, EcTagTypes.EC_TAGTYPE_UINT64)
            {
                m_val = v;
                m_size = 8;
            }

            public ecTagInt(ECTagNames n, Int32 tag_size, BinaryReader br, LinkedList<ecTag> subtags)
                : base(n, EcTagTypes.EC_TAGTYPE_UINT8, subtags)
            {
                m_size = tag_size;
                UInt64 raw_val;
                Int32 hi, lo, v32;
                Int16 v16;
                switch ( m_size ) {
                    case 8:
                        m_type = EcTagTypes.EC_TAGTYPE_UINT64;
                        lo = System.Net.IPAddress.NetworkToHostOrder(br.ReadInt32());
                        hi = System.Net.IPAddress.NetworkToHostOrder(br.ReadInt32());
                        raw_val = ((UInt64)hi) << 32 | (UInt32)lo;
                        break;
                    case 4:
                        m_type = EcTagTypes.EC_TAGTYPE_UINT32;
                        v32 = (System.Net.IPAddress.NetworkToHostOrder(br.ReadInt32()));
                        raw_val = (UInt32)v32;
                        break;
                    case 2:
                        m_type = EcTagTypes.EC_TAGTYPE_UINT16;
                        v16 = System.Net.IPAddress.NetworkToHostOrder(br.ReadInt16());
                        raw_val = (UInt16)v16;
                        break;
                    case 1:
                        m_type = EcTagTypes.EC_TAGTYPE_UINT8;
                        raw_val = (UInt64)br.ReadByte();
                        break;
                    default:
                        throw new Exception("Unexpected size of data in integer tag");
                }
                m_val = (Int64)raw_val;
                if ( m_val < 0 ) {
                    throw new Exception("WTF - typecasting is broken?!");
                }
            }

            public int ValueInt()
            {
                return (int)m_val;
            }

            public Int64 Value64()
            {
                return m_val;
            }
            
            public override void Write(BinaryWriter wr)
            {
                base.Write(wr);
                
                switch ( m_size ) {
                    case 8:
                        Int32 val32 = (Int32)(m_val >> 32);
                        wr.Write(System.Net.IPAddress.HostToNetworkOrder(val32));

                        val32 = (Int32)(m_val & 0xffffffff);
                        wr.Write(System.Net.IPAddress.HostToNetworkOrder(val32));
                        break;
                    case 4:
                        val32 = (Int32)(m_val & 0xffffffff);
                        wr.Write(System.Net.IPAddress.HostToNetworkOrder(val32));
                        break;
                    case 2:
                        Int16 val16 = (Int16)(m_val & 0xffff);
                        wr.Write(System.Net.IPAddress.HostToNetworkOrder(val16));
                        break;
                    case 1:
                        wr.Write((byte)(m_val & 0xff));
                        break;
                }

            }
        }

        public class ecMD5 : IComparable<ecMD5>, IEquatable<ecMD5> {
            Int64 m_lo, m_hi;
            public ecMD5(Int64 lo, Int64 hi)
            {
                m_hi = hi;
                m_lo = lo;
            }

            //
            // actual byte order doesn't matter, but conversion must be consistant
            //
            public ecMD5(byte [] v)
            {
                m_lo = ((Int64)v[0] << 0) | ((Int64)v[1] << 8) | ((Int64)v[2] << 16) | ((Int64)v[3] << 24) |
                    ((Int64)v[4] << 32) | ((Int64)v[5] << 40) | ((Int64)v[6] << 48) | ((Int64)v[7] << 56);
                m_hi = ((Int64)v[8] << 0) | ((Int64)v[9] << 8) | ((Int64)v[10] << 16) | ((Int64)v[11] << 24) |
                    ((Int64)v[12] << 32) | ((Int64)v[13] << 40) | ((Int64)v[14] << 48) | ((Int64)v[15] << 56);
            }

            public byte [] ByteValue()
            {
                byte[] v = {
                    (byte)(m_lo >> 0), (byte)(m_lo >> 8), (byte)(m_lo >> 16), (byte)(m_lo >> 24),
                    (byte)(m_lo >> 32), (byte)(m_lo >> 40), (byte)(m_lo >> 48), (byte)(m_lo >> 56),
                    (byte)(m_hi >> 0), (byte)(m_hi >> 8), (byte)(m_hi >> 16), (byte)(m_hi >> 24),
                    (byte)(m_hi >> 32), (byte)(m_hi >> 40), (byte)(m_hi >> 48), (byte)(m_hi >> 56),
                };
                return v;
            }

            public override int GetHashCode()
            {
                return (int)m_lo ^ (int)m_hi;
            }

            public bool Equals(ecMD5 i)
            {
                return (m_hi == i.m_hi) && (m_lo == i.m_lo);
            }

            public int CompareTo(ecMD5 i)
            {
                Int64 r = ((m_hi == i.m_hi) ? (m_lo - i.m_lo) : (m_hi - i.m_hi));
                return r > 0 ? 1 : (r < 0 ? -1 : 0);
            }
        }

        public class ecTagMD5 : ecTag {
            byte[] m_val;

            public ecTagMD5(ECTagNames n, ecMD5 value)
                : base(n, EcTagTypes.EC_TAGTYPE_HASH16)
            {
                m_val = value.ByteValue();
                m_size = 16;
            }

            public ecTagMD5(ECTagNames n, string s, bool string_is_hash)
                : base(n, EcTagTypes.EC_TAGTYPE_HASH16)
            {
                if ( string_is_hash ) {
                    // in this case hash is passed as hex string
                    if ( s.Length != 16*2 ) {
                        throw new Exception("md5 hash of proto version have incorrect length");
                    }
                    //byte[] hash_str = s.ToCharArray();
                    for (int i = 0; i < 16; i++) {
                        string v = s.Substring(i * 2, 2);
                        
                    }
                    m_val = new byte[16];
                } else {
                    MD5CryptoServiceProvider p = new MD5CryptoServiceProvider();
                    byte[] bs = System.Text.Encoding.UTF8.GetBytes(s);
                    m_val = p.ComputeHash(bs);
                }
                m_size = 16;
            }

            public ecTagMD5(ECTagNames name, byte[] hash_data)
                : base(name, EcTagTypes.EC_TAGTYPE_HASH16)
            {
                m_val = hash_data;
                m_size = 16;
            }

            public ecTagMD5(ECTagNames name, BinaryReader br, LinkedList<ecTag> subtags)
                : base(name, EcTagTypes.EC_TAGTYPE_HASH16, subtags)
            {
                m_size = 16;
                m_val = br.ReadBytes(16);
            }

            public override void Write(BinaryWriter wr)
            {
                base.Write(wr);
                wr.Write(m_val);
            }

            public ecMD5 ValueMD5()
            {
                return new ecMD5(m_val);
            }
        }

        public class ecTagIPv4 : ecTag {
            Int32 m_addr;
            Int16 m_port;
            public ecTagIPv4(ECTagNames name, BinaryReader br, LinkedList<ecTag> subtags)
                : base(name, EcTagTypes.EC_TAGTYPE_IPV4, subtags)
            {
                m_size = 4+2;
                m_addr = System.Net.IPAddress.NetworkToHostOrder(br.ReadInt32());
                m_port = System.Net.IPAddress.NetworkToHostOrder(br.ReadInt16());
            }
        }

        public class ecTagCustom : ecTag {
            byte[] m_val;
            public ecTagCustom(ECTagNames n, Int32 tag_size, BinaryReader br, LinkedList<ecTag> subtags)
                : base(n, EcTagTypes.EC_TAGTYPE_CUSTOM, subtags)
            {
                m_val= br.ReadBytes(tag_size);
                m_size = tag_size;
            }
            public byte [] Value()
            {
            	return m_val;
            }

        }

        public class ecTagString : ecTag {
            byte[] m_val;
            public ecTagString(ECTagNames n, string s)
                : base(n, EcTagTypes.EC_TAGTYPE_STRING)
            {
                m_val = System.Text.Encoding.UTF8.GetBytes(s);
                m_size = m_val.GetLength(0) + 1;
            }

            public ecTagString(ECTagNames n, Int32 tag_size, BinaryReader br, LinkedList<ecTag> subtags)
                : base(n, EcTagTypes.EC_TAGTYPE_STRING, subtags)
            {
                byte[] buf = br.ReadBytes(tag_size-1);
                // discard trailing '0'
                br.ReadBytes(1);

                m_size = tag_size;
                m_val = buf;
            }

            public override void Write(BinaryWriter wr)
            {
                base.Write(wr);
                wr.Write(m_val);
                byte zero_byte = 0;
                wr.Write(zero_byte);
            }
            public string StringValue()
            {
                Encoding u8 = Encoding.UTF8;
                string s = u8.GetString(m_val);
                return s;
            }
        }

        public class ecPacket : ecTag {
            // since I have no zlib here, effectively disable compression
            const int MaxUncompressedPacket = 0x6666;

            private ECOpCodes m_opcode;
            protected Int32 m_flags;

            //
            // Parsing ctor
            //
            ecTag ReadTag(BinaryReader br)
            {
                ecTag t = null;
                Int16 tag_name16 = System.Net.IPAddress.NetworkToHostOrder(br.ReadInt16());
                bool have_subtags = ((tag_name16 & 1) != 0);
                ECTagNames tag_name = (ECTagNames)(tag_name16 >> 1);

                byte tag_type8 = br.ReadByte();
                Int32 tag_size32 = System.Net.IPAddress.NetworkToHostOrder(br.ReadInt32());
                LinkedList<ecTag> subtags = null;
                if ( have_subtags ) {
                    subtags = ReadSubtags(br);
                }
                EcTagTypes tag_type = (EcTagTypes)tag_type8;
                switch (tag_type) {
                    case EcTagTypes.EC_TAGTYPE_UNKNOWN:
                        break;
                    case EcTagTypes.EC_TAGTYPE_CUSTOM:
                        t = new ecTagCustom(tag_name, tag_size32, br, subtags);
                        break;

                    case EcTagTypes.EC_TAGTYPE_UINT8:
                        t = new ecTagInt(tag_name, 1, br, subtags);
                        break;
                    case EcTagTypes.EC_TAGTYPE_UINT16:
                        t = new ecTagInt(tag_name, 2, br, subtags);
                        break;
                    case EcTagTypes.EC_TAGTYPE_UINT32:
                        t = new ecTagInt(tag_name, 4, br, subtags);
                        break;
                    case EcTagTypes.EC_TAGTYPE_UINT64:
                        t = new ecTagInt(tag_name, 8, br, subtags);
                        break;

                    case EcTagTypes.EC_TAGTYPE_STRING:
                        t = new ecTagString(tag_name, tag_size32, br, subtags);
                        break;
                    case EcTagTypes.EC_TAGTYPE_DOUBLE:
                        break;
                    case EcTagTypes.EC_TAGTYPE_IPV4:
                        t = new ecTagIPv4(tag_name, br, subtags);
                        break;
                    case EcTagTypes.EC_TAGTYPE_HASH16:
                        t = new ecTagMD5(tag_name, br, subtags);
                        break;
                    default:
                        break;
                }
                if ( t == null ) {
                    throw new Exception("Unexpected tag type");
                }
                return t;
            }

            LinkedList<ecTag> ReadSubtags(BinaryReader br)
            {
                Int16 count16 = System.Net.IPAddress.NetworkToHostOrder(br.ReadInt16());
                LinkedList<ecTag> taglist = new LinkedList<ecTag>();
                for (int i = 0; i < count16;i++) {
                    ecTag st = ReadTag(br);
                    taglist.AddLast(st);
                }
                return taglist;
            }

            public ecPacket()
            {
                m_flags = 0x20;
            }

            public ecPacket(BinaryReader br)
            {
                m_flags = System.Net.IPAddress.NetworkToHostOrder(br.ReadInt32());
                Int32 packet_size = System.Net.IPAddress.NetworkToHostOrder(br.ReadInt32());
                m_opcode = (ECOpCodes)br.ReadByte();

                Int16 tags_count = System.Net.IPAddress.NetworkToHostOrder(br.ReadInt16());

                if ( tags_count != 0 ) {
                    for (int i = 0; i < tags_count; i++) {
                        ecTag t = ReadTag(br);
                        AddSubtag(t);
                    }
                }
            }

            //
            // Default ctor - for tx packets
            public ecPacket(ECOpCodes cmd)
            {
                m_flags = 0x20;
                m_opcode = cmd;
            }
            
            public ecPacket(ECOpCodes cmd, EC_DETAIL_LEVEL detail_level)
            {
                m_flags = 0x20;
                m_opcode = cmd;
                if ( detail_level != EC_DETAIL_LEVEL.EC_DETAIL_FULL ) {
                    AddSubtag(new ecTagInt(ECTagNames.EC_TAG_DETAIL_LEVEL, (Int64)detail_level));
                }
            }

            //
            // Size of data for TX, not of payload
            //
            public int PacketSize()
            {
                int packet_size = Size();
                if ((m_flags & (UInt32)ECFlags.EC_FLAG_ACCEPTS) != 0) {
                    packet_size += 4;
                }
                // 1 (command) + 2 (tag count) + 4 (flags) + 4 (total size)
                return packet_size + 1 + 2 + 4 + 4;
            }

            public ECOpCodes Opcode()
            {
                return m_opcode;
            }

            public override void Write(BinaryWriter wr)
            {
                // 1 (command) + 2 (tag count)
                int packet_size = Size() + 1 + 2;
                if ( packet_size > MaxUncompressedPacket ) {
                    m_flags |= (Int32)ECFlags.EC_FLAG_ZLIB;
                }

                if ((m_flags & (UInt32)ECFlags.EC_FLAG_ZLIB) != 0) {
                    throw new NotImplementedException("no zlib compression yet");
                }


                wr.Write(System.Net.IPAddress.HostToNetworkOrder((Int32)(m_flags)));
                if ((m_flags & (UInt32)ECFlags.EC_FLAG_ACCEPTS) != 0) {
                    wr.Write(System.Net.IPAddress.HostToNetworkOrder((Int32)(m_flags)));
                }

                wr.Write(System.Net.IPAddress.HostToNetworkOrder((Int32)(packet_size)));
                wr.Write((byte)m_opcode);
                if ( m_subtags.Count != 0 ) {
                    WriteSubtags(wr);
                } else {
                    wr.Write((Int16)(0));
                }
            }
            
        }

        //
        // Specific - purpose tags
        //

        public class ecLoginPacket : ecPacket {
            public ecLoginPacket(string client_name, string version, string pass)
                : base(ECOpCodes.EC_OP_AUTH_REQ)
            {
                m_flags |= 0x20 | (Int32)ECFlags.EC_FLAG_ACCEPTS;

                AddSubtag(new ecTagString(ECTagNames.EC_TAG_CLIENT_NAME, client_name));
                AddSubtag(new ecTagString(ECTagNames.EC_TAG_CLIENT_VERSION, version));
                AddSubtag(new ecTagInt(ECTagNames.EC_TAG_PROTOCOL_VERSION,
                    (Int64)ProtocolVersion.EC_CURRENT_PROTOCOL_VERSION));

                AddSubtag(new ecTagMD5(ECTagNames.EC_TAG_PASSWD_HASH, pass, false));

                // discussion is ongoing
                //AddSubtag(new ecTagMD5(ECTagNames.EC_TAG_VERSION_ID, EC_VERSION_ID, true));
            }
        }

        public class ecDownloadsInfoReq : ecPacket {
            public ecDownloadsInfoReq() : base(ECOpCodes.EC_OP_GET_DLOAD_QUEUE)
            {
            }
        }

        //
        // Class exists only for parsing purpose
        //
        public class ecConnStateTag {
            ecTagInt m_tag;
            Int32 m_tag_val;
            public ecConnStateTag(ecTagInt tag)
            {
                m_tag = tag;
                //m_tag_val = (Int32)tag.Value64();
                m_tag_val = 0xfff;
            }

            //public static explicit operator ecConnStateTag(ecTagInt t)
            //{
            //    return new ecConnStateTag(t.ValueInt64());
            //}

            public bool IsConnected()
            {
                return IsConnectedED2K() || IsConnectedKademlia();
            }

            public bool IsConnectedED2K()
            {
                return (m_tag_val & 0x01) != 0; 
            }

            public bool IsConnectingED2K()
            {
                return (m_tag_val & 0x02) != 0; 
            }

            public bool IsConnectedKademlia()
            {
                return (m_tag_val & 0x04) != 0; 
            }

            public bool IsKadFirewalled()
            {
                return (m_tag_val & 0x08) != 0; 
            }

            public bool IsKadRunning()
            {
                return (m_tag_val & 0x10) != 0; 
            }

            public ecProto.ecTag Server()
            {
                return m_tag.SubTag(ECTagNames.EC_TAG_SERVER);
            }
        }

    }
}
