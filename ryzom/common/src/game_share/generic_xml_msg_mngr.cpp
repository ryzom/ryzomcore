// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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



#include "stdpch.h"

#include "generic_xml_msg_mngr.h"
#include "stream_record.h"

#include "nel/misc/file.h"
#include "nel/misc/path.h"

#include <algorithm>
#include <cstring>
#include "nel/misc/file.h"
#include "nel/misc/xml_auto_ptr.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLMISC;

/*
 * CGenericXmlMsgHeaderManager
 */

/*
 * Constructor
 */
CGenericXmlMsgHeaderManager::CGenericXmlMsgHeaderManager()
{
	_Root = nullptr;
	_StreamClassError = false;
}

// Destructor
CGenericXmlMsgHeaderManager::~CGenericXmlMsgHeaderManager()
{
	delete _Root;
	_Root = nullptr;
}


// init
void	CGenericXmlMsgHeaderManager::init(const string &filename)
{
	// open xml file
	CIFile file;
	if (!file.open (filename))
	{
		nlwarning("Cannot open xml file '%s', unable to initialize generic messages", filename.c_str());
		return;
	}

	// Init an xml stream
	CIXml read;
	read.init (file);

	// The classes of messages that go on streams, which the nodes then refer to by name
	_StreamClasses.clear();
	_StreamClassError = false;
	for (xmlNodePtr xmlChild = read.getRootNode()->children; xmlChild != nullptr; xmlChild = xmlChild->next)
	{
		if (strcmp((const char *)xmlChild->name, "stream_class") != 0)
			continue;
		CXMLAutoPtr name(xmlGetProp(xmlChild, (xmlChar *)"name"));
		std::string className = name ? (const char *)name : "";
		if (className.empty())
		{
			nlwarning("A stream_class element of '%s' has no name", filename.c_str());
			_StreamClassError = true;
		}
		else if (std::find(_StreamClasses.begin(), _StreamClasses.end(), className) != _StreamClasses.end())
		{
			nlwarning("'%s' declares the stream class '%s' twice", filename.c_str(), className.c_str());
			_StreamClassError = true;
		}
		else if (_StreamClasses.size() >= CLFECOMMON::StreamClassMax)
		{
			nlwarning("'%s' declares more than %u stream classes, '%s' is one too many", filename.c_str(), CLFECOMMON::StreamClassMax, className.c_str());
			_StreamClassError = true;
		}
		else
		{
			_StreamClasses.push_back(className);
		}
	}

	// create root node from root xml node
	_Root = new CNode(read.getRootNode(), 0, CLFECOMMON::NoStreamClass, _StreamClasses, _StreamClassError);
}

// the text of a msg.xml without what declares the stream classes
void	CGenericXmlMsgHeaderManager::removeStreamClasses(std::vector<uint8> &text)
{
	static const char declaration[] = "<stream_class";
	static const char commentStart[] = "<!-- stream_class";
	static const char commentEnd[] = "-->";
	static const char attribute[] = " stream=\"";
	const size_t declarationSize = sizeof(declaration) - 1;
	const size_t commentStartSize = sizeof(commentStart) - 1;
	const size_t commentEndSize = sizeof(commentEnd) - 1;
	const size_t attributeSize = sizeof(attribute) - 1;

	std::vector<uint8> out;
	out.reserve(text.size());
	bool inComment = false;
	size_t pos = 0;
	while (pos < text.size())
	{
		// One line, with its end
		size_t end = pos;
		while (end < text.size() && text[end] != '\n')
			++end;
		if (end < text.size())
			++end;
		const uint8 *line = &text[pos];
		const size_t size = end - pos;
		pos = end;

		// What the line starts with, after its indentation
		size_t first = 0;
		while (first < size && (line[first] == ' ' || line[first] == '\t'))
			++first;
		const size_t left = size - first;

		bool removeLine = false;
		if (inComment)
		{
			removeLine = true;
		}
		else if (left >= commentStartSize && !memcmp(line + first, commentStart, commentStartSize))
		{
			inComment = true;
			removeLine = true;
		}
		else if (left >= declarationSize && !memcmp(line + first, declaration, declarationSize))
		{
			removeLine = true;
		}
		if (inComment)
		{
			// The comment ends on the line that has its end
			for (size_t i = first; i + commentEndSize <= size; ++i)
			{
				if (!memcmp(line + i, commentEnd, commentEndSize))
				{
					inComment = false;
					break;
				}
			}
		}
		if (removeLine)
			continue;

		// The line without its stream attributes
		size_t i = 0;
		while (i < size)
		{
			if (size - i >= attributeSize && !memcmp(line + i, attribute, attributeSize))
			{
				size_t close = i + attributeSize;
				while (close < size && line[close] != '"' && line[close] != '\n')
					++close;
				if (close < size && line[close] == '"')
				{
					i = close + 1;
					continue;
				}
			}
			out.push_back(line[i]);
			++i;
		}
	}
	text.swap(out);
}

// the signature of a msg.xml
NLMISC::CHashKeyMD5	CGenericXmlMsgHeaderManager::getSignature(const string &filename, bool removeCarriageReturns)
{
	NLMISC::CHashKeyMD5 signature;
	signature.clear();

	CIFile file;
	if (!file.open(CPath::lookup(filename, false)))
	{
		nlwarning("Cannot open '%s' for its signature", filename.c_str());
		return signature;
	}
	std::vector<uint8> text(file.getFileSize());
	if (!text.empty())
		file.serialBuffer(&text[0], (uint)text.size());

	removeStreamClasses(text);
	if (removeCarriageReturns)
		text.erase(std::remove(text.begin(), text.end(), (uint8)'\r'), text.end());

	static const uint8 nothing = 0;
	return NLMISC::getMD5(text.empty() ? &nothing : &text[0], (uint32)text.size());
}

// set callback
bool	CGenericXmlMsgHeaderManager::setCallback(const string &msgName, TMsgHeaderCallback callback)
{
	// check root
	if (_Root == nullptr)
	{
		nlwarning("Can't set callback for message '%s', Root not properly initialized.", msgName.c_str());
		return false;
	}

	// search for msg node
	CNode	*node = _Root->select(msgName.c_str());

	// check node
	if (node == nullptr)
	{
		nlwarning("Can't set callback for message '%s', message not found.", msgName.c_str());
		return false;
	}

	// set callback
	node->Callback = callback;
	return true;
}

bool	CGenericXmlMsgHeaderManager::setUserData(const string &msgName, const uint64 &data, uint index)
{
	// check root
	if (_Root == nullptr)
	{
		nlwarning("Can't set user data for message '%s', Root not properly initialized.", msgName.c_str());
		return false;
	}

	// search for msg node
	CNode	*node = _Root->select(msgName.c_str());

	// check node
	if (node == nullptr)
	{
		nlwarning("Can't set user data for message '%s', message not found.", msgName.c_str());
		return false;
	}

	// set callback
	node->UserData[index] = data;
	return true;
}

// execute
void	CGenericXmlMsgHeaderManager::execute(CBitMemStream &strm)
{
	// check root
	if (_Root == nullptr)
	{
		nlwarning("Can't execute message , Root not properly initialized.");
		return;
	}

	CNode	*node = _Root->select(strm);

	// check node
	if (node == nullptr)
	{
		nlwarning("Can't execute stream, no valid sequence found");
	}
	// check callback
	else if (node->Callback == nullptr)
	{
		nlwarning("Can't execute msg '%s', no callback set", node->Name.c_str());
	}
	// execute callback
	else
	{
		node->Callback(strm);
	}
}

//
bool	CGenericXmlMsgHeaderManager::pushNameToStream(const string &msgName, CBitMemStream &strm)
{
	bool res = (_Root->select(msgName.c_str(), strm) != nullptr);

	if (!res) nlwarning("pushNameToStream failed: Unknown message name '%s'", msgName.c_str());

	return res;
}

//
bool	CGenericXmlMsgHeaderManager::pushNameToStream(const char *msgName, CBitMemStream &strm)
{
	bool res = (_Root->select(msgName, strm) != nullptr);

	if (!res) nlwarning("pushNameToStream failed: Unknown message name '%s'", msgName);

	return res;
}

//
void	CGenericXmlMsgHeaderManager::popNameFromStream(string &resultName, CBitMemStream &strm)
{
	_Root->select(strm, resultName);
}

//
void	CGenericXmlMsgHeaderManager::popNameAndDescriptionFromStream(string &resultName, string& description, CBitMemStream &strm)
{
	CNode	*node = _Root->select(strm, resultName);
	if (node != nullptr)
	{
		description = node->Description;
	}
}



/*
 * CGenericXmlMsgHeaderManager::CNode
 */

/*
 * Constructor
 */
CGenericXmlMsgHeaderManager::CNode::CNode(xmlNodePtr xmlNode, uint32 value, uint8 parentStreamClass, const std::vector<std::string> &streamClasses, bool &streamClassError)
    : Value(value)
    , UseCycle(false)
    , NbBits(0)
    , Callback(nullptr)
    , StreamClass(parentStreamClass)
{
	UserData[0] = 0;
	UserData[1] = 0;
	UserData[2] = 0;
	UserData[3] = 0;

	// setup node name
	CXMLAutoPtr name(xmlGetProp (xmlNode, (xmlChar*)"name"));
	if (name)
	{
		Name = (const char*)name;
	}

	// The class of the node, which the nodes below it have too unless they name another
	CXMLAutoPtr stream(xmlGetProp(xmlNode, (xmlChar *)"stream"));
	if (stream)
	{
		std::vector<std::string>::const_iterator it = std::find(streamClasses.begin(), streamClasses.end(), std::string((const char *)stream));
		if (it != streamClasses.end())
		{
			StreamClass = (uint8)(it - streamClasses.begin());
		}
		else
		{
			nlwarning("Node '%s' names the stream class '%s', which is not declared (a stream_class element)", Name.c_str(), (const char *)stream);
			StreamClass = CLFECOMMON::NoStreamClass;
			streamClassError = true;
		}
	}

	uint32		childValue = 0;

	if (!strcmp((const char*)xmlNode->name, "leaf"))
	{
		// only setup description and format if leaf

		// setup node description
		CXMLAutoPtr description(xmlGetProp (xmlNode, (xmlChar*)"description"));
		if (description)
		{
			Description = (const char*)description;
		}

		// setup node description
		CXMLAutoPtr sendto(xmlGetProp (xmlNode, (xmlChar*)"sendto"));
		if (sendto)
		{
			SendTo = (const char*)sendto;
		}

		// setup node description
		CXMLAutoPtr usecycle(xmlGetProp (xmlNode, (xmlChar*)"usecycle"));

		if (bool(usecycle) && !strcmp((const char*)usecycle, "yes"))
		{
			UseCycle = true;
		}

		// setup node format
		CXMLAutoPtr format(xmlGetProp (xmlNode, (xmlChar*)"format"));
		static char buf[256];
		if (format)
		{
			char *scan = &buf[0];
			nlassert( strlen((const char *)format) < 256 );
			strcpy( scan, (const char *)format );
			while (*scan != '\0')
			{
				switch (tolower(*scan++))
				{
				case 's':
					{
						uint	numBits = 0;
						while (isdigit(*scan))
							numBits = numBits*10 + (*(scan++) - '0');
						if (numBits == 0)
						// here consider s as string
							Format.push_back(CMessageField(String, numBits));
						else if (numBits == 8)
						// here consider s as sint
							Format.push_back(CMessageField(Sint8, numBits));
						else if (numBits == 16)
							Format.push_back(CMessageField(Sint16, numBits));
						else if (numBits == 32)
							Format.push_back(CMessageField(Sint32, numBits));
						else if (numBits == 64)
							Format.push_back(CMessageField(Sint64, numBits));
						else
						{
							nlwarning("Can't use sint in format with other size than 8, 16, 32 or 64");
						}
					}
					break;
				case 'u':
					{
						// consider uc as ucstring
						if (*scan == 'c')
							Format.push_back(CMessageField(UCString, 0));
						else
						{
							// here consider u as uint
							uint	numBits = 0;
							while (isdigit(*scan))
								numBits = numBits*10 + *(scan++) -'0';
							if (numBits == 8)
								Format.push_back(CMessageField(Uint8, numBits));
							else if (numBits == 16)
								Format.push_back(CMessageField(Uint16, numBits));
							else if (numBits == 32 || numBits == 0)
								Format.push_back(CMessageField(Uint32, 32));
							else if (numBits == 64)
								Format.push_back(CMessageField(Uint64, numBits));
							else
								Format.push_back(CMessageField(BitSizedUint, numBits));
						}
					}
					break;
				case 'f':
					// here consider f as float
					Format.push_back(CMessageField(Float, 32));
					break;
				case 'd':
					// here consider d as double
					Format.push_back(CMessageField(Double, 64));
					break;
				case 'e':
					// here consider e as CEntityId
					Format.push_back(CMessageField(EntityId, 64));
					break;
				case 'b':
					// here consider b as bool
					Format.push_back(CMessageField(Bool, 1));
					break;
				default:
					// don't consider other characters
					break;
				}
			}
		}
	}
	else
	{
		// only parse children if not leaf
		xmlNodePtr	xmlChild = xmlNode->children;

		while (xmlChild != nullptr)
		{
			// check node is leaf or branch
			if (!strcmp((const char*)xmlChild->name, "branch") || !strcmp((const char*)xmlChild->name, "leaf"))
			{
				// create a node from the child xml node
				CNode	*child = new CNode(xmlChild, childValue, StreamClass, streamClasses, streamClassError);

				// check node doesn't exist yet in parent
				if (NodesByName.find(child->Name) == NodesByName.end())
				{
					// add it to parent's children
					NodesByName.insert(TNodesByName::value_type(child->Name, child));
					Nodes.push_back(child);
					++childValue;
				}
				else
				{
					nlwarning("Child '%s' in node '%s' already exists, unable to add it", child->Name.c_str(), Name.c_str());
					delete child;
				}
			}

			// next child
			xmlChild = xmlChild->next;
		}
	}

	// compute number of bits from the number of children
	NbBits = (childValue == 0) ? 0 : NLMISC::getPowerOf2(childValue);
}

CGenericXmlMsgHeaderManager::CNode::~CNode()
{
	uint	i;
	for (i=0; i<Nodes.size(); ++i)
	{
		delete Nodes[i];
		Nodes[i] = nullptr;
	}
}
