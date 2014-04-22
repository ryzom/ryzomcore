// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.



#ifndef NL_GENERIC_XML_MSG_MNGR_H
#define NL_GENERIC_XML_MSG_MNGR_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/bit_mem_stream.h"

#include <string>
#include <vector>
#include <map>


/**
 * Class to manage headers for the generic messages (xml version).
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2002
 */
class CGenericXmlMsgHeaderManager
{
	friend class CNodeId;

protected:
	class CNode;

public:

	/// The type of callbacks called by execute
	typedef void (*TMsgHeaderCallback)(NLMISC::CBitMemStream &);

	enum TFieldType
	{
		Bool,
		Sint8,
		Sint16,
		Sint32,
		Sint64,
		Uint8,
		Uint16,
		Uint32,
		Uint64,
		BitSizedUint,
		Float,
		Double,
		EntityId,
		String,
		UCString,
	};

	/// A message field
	class CMessageField
	{
	public:
		CMessageField() : Type(Uint32), BitSize(0) {}
		CMessageField(TFieldType type, uint bitSize=0) : Type(type), BitSize(bitSize) {}
		TFieldType	Type;
		uint8		BitSize;
	};

	/**
	 * The message format.
	 * Each field might a bool, a sint32, an uint32, a bitsized uint, a float, a double, a string, or an entity id.
	 * See also TFieldType enum and CMessageField class for more information.
	 */
	typedef std::vector<CMessageField>	TMessageFormat;

	/// A node id, used to retrieve meta information on a message
	class CNodeId
	{
		friend class CGenericXmlMsgHeaderManager;
	protected:
		CNode					*_Node;
		NLMISC::CBitMemStream	*_BitMemStream;
		uint					_CurrentField;

		CNodeId(CNode *node, NLMISC::CBitMemStream *stream) : _Node(node), _BitMemStream(stream), _CurrentField(0)	{}

	public:
		/// Check if node id is valid
		bool	isValid() const		{ return _Node != NULL; }
		/// Check if node has a stream and is ready for further decoding/encoding
		bool	hasStream() const	{ return _BitMemStream != NULL; }
		/// Set stream (if not yet has), and return true if node id hadn't a stream yet
		bool	setStream(NLMISC::CBitMemStream &stream)
		{
			if (hasStream())
				return false;
			_BitMemStream = &stream;
			return true;
		}
		/// Reset internal field counter
		void	reset()				{ _CurrentField = 0; }


		/// \name Message serial
		//@{

		/// serial next field, returns false if failed.
		template<typename T>
		bool					serial(T &b)
		{
			bool	valid = true, stream = true, field = true, stc = true;
			if (!(valid = isValid()) || !(stream = hasStream()) || (field=(_CurrentField>=_Node->Format.size())) || (stc=!serialTypeCheck(b)))
			{
				if (!valid)
					nlwarning("Failed to serialize from CBitMemStream, node is not valid");
				if (!stream)
					nlwarning("Failed to serialize from CBitMemStream, node has no stream");
				if (!field)
					nlwarning("Failed to serialize from CBitMemStream, no more fields to serialize");
				if (!stc)
					nlwarning("Failed to serialize from CBitMemStream, bad type provided");
				return false;
			}
			++_CurrentField;
			return true;
		}

		//@}

	protected:

#define DEFINE_SERIAL_TYPE_CHECK(stc__type, stc__typecheck)\
		bool	serialTypeCheck(stc__type &b)\
		{\
			if (_Node->Format[_CurrentField].Type != stc__typecheck) return false;\
			_BitMemStream->serial(b);\
			return true;\
		}

DEFINE_SERIAL_TYPE_CHECK(bool, Bool)
DEFINE_SERIAL_TYPE_CHECK(sint8, Sint8)
DEFINE_SERIAL_TYPE_CHECK(sint16, Sint16)
DEFINE_SERIAL_TYPE_CHECK(sint32, Sint32)
DEFINE_SERIAL_TYPE_CHECK(sint64, Sint64)
DEFINE_SERIAL_TYPE_CHECK(uint8, Uint8)
DEFINE_SERIAL_TYPE_CHECK(uint16, Uint16)
DEFINE_SERIAL_TYPE_CHECK(uint64, Uint64)

		bool	serialTypeCheck(uint32 &b)
		{
			if (_Node->Format[_CurrentField].Type != Uint32)
				_BitMemStream->serial(b);
			else if (_Node->Format[_CurrentField].Type != BitSizedUint)
				_BitMemStream->serial(b, _Node->Format[_CurrentField].BitSize);
			else
				return false;
			return true;
		}

DEFINE_SERIAL_TYPE_CHECK(float, Float)
DEFINE_SERIAL_TYPE_CHECK(double, Double)
DEFINE_SERIAL_TYPE_CHECK(std::string, String)
DEFINE_SERIAL_TYPE_CHECK(ucstring, UCString)
DEFINE_SERIAL_TYPE_CHECK(NLMISC::CEntityId, EntityId)


	};

public:

	/// Constructor
	CGenericXmlMsgHeaderManager();

	/// Destructor
	~CGenericXmlMsgHeaderManager();

	/**
	 * Initialize the class.
	 * \param string filename : file to describe all messages.
	 */
	void	init(const std::string &filename);

	/**
	 * display
	 */
	void	xmlDisplay()	{ if (_Root != NULL) _Root->xmlDisplay(); }

	/**
	 * associate a callback to a message.
	 * \param string msgName : message that will take the callback.
	 * \param TMsgHeaderCallback callback : the callback to associate to the message 'msgName'.
	 * \return bool : 'false' if the message does not exist.
	 */
	bool	setCallback(const std::string &msgName, TMsgHeaderCallback callback);

	/**
	 * associate a user data to a message.
	 * \param string msgName : message that will take the callback.
	 * \param uint64 data : the user data to associate to the message 'msgName'.
	 * \return bool : 'false' if the message does not exist.
	 */
	bool	setUserData(const std::string &msgName, const uint64 &data, uint index=0);

	/**
	 * Decode the stream header and launch the callback associated.
	 * \param CBitMemStream strm : stream to decode.
	 */
	void	execute(NLMISC::CBitMemStream &strm);



	/// \name Old fashion message header decoding/encoding.
	//@{

	/**
	 * Convert and write a Message Name into a stream.
	 * \param string msgName : Message Name to convert and write into the stream.
	 * \param CBitMemStream strm : the stream to receive the Message Name.
	 * \return bool : 'false' if the method cannot write the message Name into the stream (probably because de message name is wrong).
	 */
	bool	pushNameToStream(const std::string &msgName, NLMISC::CBitMemStream &strm);

	/**
	 * Convert and return the Message Name from a stream.
	 * \param string resultName: The result for the Message Name.
	 * \param CBitMemStream strm : the stream with the Message Name.
	 */
	void	popNameFromStream(std::string &resultName, NLMISC::CBitMemStream &strm);

	/**
	 * Convert and return the Message Name from a stream.
	 * \param resultName: The result for the Message Name.
	 * \param description: The description of the message
	 * \param strm : the stream with the Message Name.
	 */
	void	popNameAndDescriptionFromStream(std::string &resultName, std::string& description, NLMISC::CBitMemStream &strm);

	//@}



	/// \name New fashion message header decoding/encoding.
	//@{

	/**
	 * Get the message node id from a stream.
	 * The stream is serialised to extract node route
	 */
	CNodeId		getNodeId(NLMISC::CBitMemStream &strm)
	{
		CNode	*node = _Root == NULL ? NULL : _Root->select(strm);
		return CNodeId(node, &strm);
	}

	/**
	 * Get the message node id from a stream and build message name.
	 * The stream is serialised to extract node route
	 */
	CNodeId		getNodeId(NLMISC::CBitMemStream &strm, std::string &name)
	{
		CNode	*node = _Root == NULL ? NULL : _Root->select(strm, name);
		return CNodeId(node, &strm);
	}

	/**
	 * Get the message node id from a string
	 */
	CNodeId		getNodeId(const std::string &name)
	{
		CNode	*node = _Root == NULL ? NULL : _Root->select(name.c_str());
		return CNodeId(node, NULL);
	}

	/**
	 * Get the message node id from a string and stream message header to stream
	 */
	CNodeId		getNodeId(const std::string &name, NLMISC::CBitMemStream &strm)
	{
		CNode	*node = _Root == NULL ? NULL : _Root->select(name.c_str(), strm);
		return CNodeId(node, NULL);
	}

	/**
	 * Get the message node name
	 */
	const std::string		&getNodeName(CNodeId &nodeId)				{ return nodeId._Node->Name; }

	/**
	 * Get the message node description
	 */
	const std::string		&getNodeDescription(CNodeId &nodeId)		{ return nodeId._Node->Description; }

	/**
	 * Get the message node format
	 */
	const TMessageFormat	&getNodeFormat(CNodeId &nodeId)				{ return nodeId._Node->Format; }

	/**
	 * Get the message node send to
	 */
	const std::string		&getNodeSendTo(CNodeId &nodeId)				{ return nodeId._Node->SendTo; }

	/**
	 * checks if message node uses cycle information
	 */
	bool					nodeUsesCycle(CNodeId &nodeId)				{ return nodeId._Node->UseCycle; }

	/**
	 * Set node user data
	 */
	void					setNodeUserData(CNodeId &nodeId, const uint64 &data, uint index=0)	{ nodeId._Node->UserData[index] = data; }

	/**
	 * Set node user data
	 */
	const uint64			&getNodeUserData(CNodeId &nodeId, uint index=0)			{ return nodeId._Node->UserData[index]; }

protected:
	/**
	 * A generic message node
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CNode
	{
	public:
		typedef std::vector<CNode*>					TNodes;
		typedef std::map<std::string, CNode*>		TNodesByName;

		TNodes				Nodes;
		TNodesByName		NodesByName;
		uint32				Value;
		std::string			Name;
		std::string			Description;
		std::string			SendTo;
		bool				UseCycle;
		uint64				UserData[4];
		TMessageFormat		Format;
		uint				NbBits;
		TMsgHeaderCallback	Callback;

		/// Constructor
		CNode(xmlNodePtr xmlNode, uint32 value);

		/// Destructor
		~CNode();

		/// select node using name, no other action performed
		CNode	*select(const char *name)
		{
			CNode					*node = this;
			std::string				sub;
			TNodesByName::iterator	it;

			for(;;)
			{
				sub.resize(0);
				while (*name != '\0' && *name != ':')
					sub += *name++;
				it = node->NodesByName.find(sub);
				if (it == node->NodesByName.end())
				{
					nlwarning("Couldn't select node '%s', not found in parent '%s'", sub.c_str(), node->Name.c_str());
					return NULL;
				}
				node = (*it).second;
				if (*name == '\0')
					return node;
				++name;
			}
			return NULL;
		}

		/// select node using name, and write bits in stream
		CNode	*select(const char *name, NLMISC::CBitMemStream &strm)
		{
			CNode					*node = this;
			std::string				sub;
			TNodesByName::iterator	it;

			for(;;)
			{
				sub.resize(0);
				while (*name != '\0' && *name != ':')
					sub += *name++;

				if (node->NbBits == 0)
				{
					nlwarning("Couldn't select node '%s', parent '%s' has no bit per child", sub.c_str(), node->Name.c_str());
					return 0;
				}

				it = node->NodesByName.find(sub);
				if (it == node->NodesByName.end())
				{
					nlwarning("Couldn't select node '%s', not found in parent '%s'", sub.c_str(), node->Name.c_str());
					return NULL;
				}

				strm.serialAndLog2((*it).second->Value, node->NbBits);

				node = (*it).second;
				if (*name == '\0')
					return node;
				++name;
			}
			return NULL;
		}

		/// select node using bits stream
		CNode	*select(NLMISC::CBitMemStream &strm)
		{
			CNode	*node = this;

			while (node != NULL && node->NbBits != 0)
			{
				uint32	index = 0;
				strm.serialAndLog2(index, node->NbBits);
				if (index >= node->Nodes.size())
				{
					nlwarning("Couldn't select node from stream, invalid index %d in parent '%s'", index, node->Name.c_str());
					return NULL;
				}
				node = node->Nodes[index];
			}

			return node;
		}

		/// select node using bits stream, and build string
		CNode	*select(NLMISC::CBitMemStream &strm, std::string &str)
		{
			CNode	*node = this;

			str.resize(0);
			bool	first = true;

			while (node != NULL && node->NbBits != 0)
			{
				uint32	index = 0;
				strm.serialAndLog2(index, node->NbBits);
				if (index >= node->Nodes.size())
				{
					nlwarning("Couldn't select node from stream, invalid index %d in parent '%s'", index, node->Name.c_str());
					return NULL;
				}
				node = node->Nodes[index];

				if (!first)
					str += ':';
				first = false;
				str += node->Name;
			}

			return node;
		}

		/// display nodes
		void	xmlDisplay()
		{
			std::string	ntype = (Name.empty() ? "client_messages_description" : NbBits == 0 ? "leaf" : "branch");

			nlinfo("<%s name=\"%s\" description=\"%s\">", ntype.c_str(), Name.c_str(), Description.c_str());

			uint	i;
			for (i=0; i<Nodes.size(); ++i)
				Nodes[i]->xmlDisplay();

			nlinfo("</%s>", ntype.c_str());
		}
	};

protected:
	CNode	*_Root;

};


#endif // NL_GENERIC_XML_MSG_MNGR_H

/* End of generic_xml_msg_mngr.h */
