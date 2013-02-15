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



#include "stdpch.h"

#include "generic_xml_msg_mngr.h"
#include "nel/misc/file.h"
#include "nel/misc/xml_auto_ptr.h"

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
	_Root = NULL;
}

// Destructor
CGenericXmlMsgHeaderManager::~CGenericXmlMsgHeaderManager()
{
	delete _Root;
	_Root = NULL;
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

	// create root node from root xml node
	_Root = new CNode(read.getRootNode(), 0);
}

// set callback
bool	CGenericXmlMsgHeaderManager::setCallback(const string &msgName, TMsgHeaderCallback callback)
{
	// check root
	if (_Root == NULL)
	{
		nlwarning("Can't set callback for message '%s', Root not properly initialized.", msgName.c_str());
		return false;
	}

	// search for msg node
	CNode	*node = _Root->select(msgName.c_str());

	// check node
	if (node == NULL)
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
	if (_Root == NULL)
	{
		nlwarning("Can't set user data for message '%s', Root not properly initialized.", msgName.c_str());
		return false;
	}

	// search for msg node
	CNode	*node = _Root->select(msgName.c_str());

	// check node
	if (node == NULL)
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
	if (_Root == NULL)
	{
		nlwarning("Can't execute message , Root not properly initialized.");
		return;
	}

	CNode	*node = _Root->select(strm);

	// check node
	if (node == NULL)
	{
		nlwarning("Can't execute stream, no valid sequence found");
	}
	// check callback
	else if (node->Callback == NULL)
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
	bool res = (_Root->select(msgName.c_str(), strm) != NULL);

	if (!res) nlwarning("pushNameToStream failed: Unknown message name '%s'", msgName.c_str());

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
	if (node != NULL)
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
CGenericXmlMsgHeaderManager::CNode::CNode(xmlNodePtr xmlNode, uint32 value) : Value(value), UseCycle(false), NbBits(0), Callback(NULL)
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

		while (xmlChild != NULL)
		{
			// check node is leaf or branch
			if (!strcmp((const char*)xmlChild->name, "branch") || !strcmp((const char*)xmlChild->name, "leaf"))
			{
				// create a node from the child xml node
				CNode	*child = new CNode(xmlChild, childValue);

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
		Nodes[i] = NULL;
	}
}
