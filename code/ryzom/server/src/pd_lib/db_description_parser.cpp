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

#include "db_description_parser.h"

#include <nel/misc/mem_stream.h>
#include <nel/misc/file.h>
#include "game_share/backup_service_interface.h"

#include "pds_common.h"
//#include "pds_type.h"

/*
 * Globals
 */
namespace RY_PDS
{
	extern NLMISC::CVariable<bool> PDUseBS;
}



using namespace std;
using namespace NLMISC;


using namespace RY_PDS;

/*
 * Constructor
 */
CDBDescriptionParser::CDBDescriptionParser()
{
}


/*
 * Load database description
 */
bool	CDBDescriptionParser::loadDescriptionFile(const string& filename)
{
	CIFile	ifile;

	if (!ifile.open(filename))
		return false;

	uint	sz = ifile.getFileSize();
	uint8*	buffer = new uint8[sz+1];

	ifile.serialBuffer(buffer, sz);
	buffer[sz] = 0;

	bool	success = loadDescription(buffer);

	delete buffer;
	return success;
}

/*
 * Load database description
 */
bool	CDBDescriptionParser::loadDescription(const uint8* description)
{
	uint	sz = (uint)strlen((const char*)description);

	// set description and compute hashkey
	_Description = (const char*)description;
	_HashKey = getSHA1(description, sz);

	// init stream
	CMemStream	stream(true);
	stream.fill(description, sz);

	// check file parsed
	CIXml		xmlStream;
	bool		xmlParsed = false;

	try
	{
		if (xmlStream.init(stream))
			xmlParsed = true;
	}
	catch (const EXmlParsingError &e)
	{
		nlwarning("CDBDescriptionParser::loadDescription(): failed, parse error in xml: %s", e.what());
		return false;
	}

	if (!xmlParsed)
	{
		nlwarning("CDBDescriptionParser::loadDescription(): failed, couldn't init xml stream");
		return false;
	}

	return loadDescription(xmlStream);
}



/*
 * Load database description
 */
bool	CDBDescriptionParser::loadDescription(NLMISC::CIXml& xmlStream)
{
	// check root node
	xmlNodePtr		root = xmlStream.getRootNode();
	if (root == NULL)
	{
		nlwarning("CDBDescriptionParser::loadDescription(): failed, unable to getRootNode in XML stream");
		return false;
	}

	// check db node
	xmlNodePtr		dbNode = CIXml::getFirstChildNode(root, "db");
	if (dbNode == NULL)
	{
		nlwarning("CDBDescriptionParser::loadDescription(): failed, unable to get XML 'db' childNode in XML stream");
		return false;
	}

	return loadDatabase(dbNode);
}


/*
 * Save database description
 */
//bool	CDBDescriptionParser::saveDescription(const std::string& filename)
//{
//	// get a pointer to the buffer that we want to output to file
//	char*	buffer = const_cast<char*>(_Description.c_str());
//
//	// see whether we're using the backup system or not
//	if (!PDUseBS)
//	{
//		// not using BS so write directly to disk
//		COFile	ofile;
//		if (!ofile.open(filename))
//		{
//			nlwarning("CDBDescriptionParser::saveDescription(): open '%s' file", filename.c_str());
//			return false;
//		}
//		ofile.serialBuffer((uint8*)buffer, _Description.size());
//	}
//	else
//	{
//		// using BS so send network message to remote backup service
//		CBackupMsgSaveFile	msgBS( filename, CBackupMsgSaveFile::SaveFileCheck, PDBsi );
//		msgBS.DataMsg.serialBuffer((uint8*)buffer, _Description.size());
//		PDBsi.sendFile(msgBS);
//	}
//
//	return true;
//}



/*
 * Load Database
 */
bool	CDBDescriptionParser::loadDatabase(xmlNodePtr node)
{
	if (!getProperty(node, "name", _Database.Name))
		return false;

	xmlNodePtr		typeNode;
	FOREACH_CHILD(typeNode, node, typedef)
	{
		if (!loadType(typeNode))
		{
			nlwarning("CDBDescriptionParser::loadDatabase(): failed to load type");
			return false;
		}
	}

	xmlNodePtr		tableNode;
	FOREACH_CHILD(tableNode, node, classdef)
	{
		if (!loadTable(tableNode))
		{
			nlwarning("CDBDescriptionParser::loadDatabase(): failed to load table");
			return false;
		}
	}

	xmlNodePtr		logNode;
	FOREACH_CHILD(logNode, node, logmsg)
	{
		if (!loadLog(logNode))
		{
			nlwarning("CDBDescriptionParser::loadDatabase(): failed to load log");
			return false;
		}
	}

	return true;
}

/*
 * Load Type
 */
bool	CDBDescriptionParser::loadType(xmlNodePtr node)
{
	uint	id;
	if (!getProperty(node, "id", id))
	{
		nlwarning("CDBDescriptionParser::loadType(): missing property 'id' in type node");
		return false;
	}

	if (id >= _Database.Types.size())
		_Database.Types.resize(id+1);

	CTypeNode&	typeNode = _Database.Types[id];

	string		storage;
	string		type;

	typeNode.Id = id;

	if (!getProperty(node, "name", typeNode.Name) ||
		!getProperty(node, "size", typeNode.ByteSize) ||
		!getProperty(node, "storage", storage) ||
		!getProperty(node, "type", type))
	{
		nlwarning("CDBDescriptionParser::loadType(): missing property in type node");
		return false;
	}

	typeNode.DataType = getDataTypeFromName(storage);

	if (type == "enum")
	{
		typeNode.Type = CTypeNode::TypeEnum;
		typeNode.Dimension = 0;

		xmlNodePtr	enumNode;
		FOREACH_CHILD(enumNode, node, enumvalue)
		{
			string	name;
			uint32	value;

			if (!getProperty(enumNode, "name", name) ||
				!getProperty(enumNode, "value", value))
			{
				nlwarning("CDBDescriptionParser::loadType(): missing property in enum node");
				return false;
			}

			typeNode.EnumValues.push_back(make_pair<string, uint32>(name, value));

			if (typeNode.Dimension <= value)
				typeNode.Dimension = value;
		}
	}
	else if (type == "dimension")
	{
		typeNode.Type = CTypeNode::TypeDimension;

		if (!getProperty(node, "dimension", typeNode.Dimension))
		{
			nlwarning("CDBDescriptionParser::loadType(): missing property in dimension node");
			return false;
		}
	}
	else if (type == "type")
	{
		typeNode.Type = CTypeNode::TypeSimple;
	}
	else
	{
		nlwarning("type '%s' is unknown", type.c_str());
		return false;
	}

	return true;
}

/*
 * Load Table description
 */
bool	CDBDescriptionParser::loadTable(xmlNodePtr node)
{
	uint	id;
	if (!getProperty(node, "id", id))
	{
		nlwarning("CDBDescriptionParser::loadTable(): missing property 'id' in table node");
		return false;
	}

	if (id >= _Database.Tables.size())
		_Database.Tables.resize(id+1);

	CTableNode&	table = _Database.Tables[id];

	table.Id = id;

	if (!getProperty(node, "name", table.Name))
	{
		nlwarning("CDBDescriptionParser::loadTable(): missing property 'name' in table node");
		return false;
	}

	getProperty(node, "inherit", table.Inherit, -1);
	getProperty(node, "mapped", table.Mapped, -1);
	getProperty(node, "key", table.Key, -1);

	xmlNodePtr	attributeNode;
	FOREACH_CHILD(attributeNode, node, attribute)
	{
		if (!loadAttribute(attributeNode, table))
		{
			nlwarning("CDBDescriptionParser::loadTable(): failed to load attribute");
			return false;
		}
	}

	return true;
}

/**
 * Load attribute description
 */
bool	CDBDescriptionParser::loadAttribute(xmlNodePtr node, CTableNode& table)
{
	uint	id;
	if (!getProperty(node, "id", id))
	{
		nlwarning("CDBDescriptionParser::loadAttribute(): missing property 'id' in attribute node");
		return false;
	}

	if (id >= table.Attributes.size())
		table.Attributes.resize(id+1);

	CAttributeNode&	attribute = table.Attributes[id];

	attribute.Id = id;
	attribute.AllowNull = false;
	string	type;

	if (!getProperty(node, "name", attribute.Name) ||
		!getProperty(node, "columnid", attribute.ColumnId) ||
		!getProperty(node, "columns", attribute.Columns) ||
		!getProperty(node, "type", type))
	{
		nlwarning("CDBDescriptionParser::loadAttribute(): missing property in attribute node");
		return false;
	}

	attribute.MetaType = getMetaTypeFromName(type);

	switch (attribute.MetaType)
	{
	case PDS_ArrayType:
		if (!getProperty(node, "indexid", attribute.Index))
		{
			nlwarning("CDBDescriptionParser::loadAttribute(): missing property 'indexid' in attribute node");
			return false;
		}
	case PDS_Type:
		if (!getProperty(node, "typeid", attribute.TypeId))
		{
			nlwarning("CDBDescriptionParser::loadAttribute(): missing property 'typeid' in attribute node");
			return false;
		}
		break;

	case PDS_ArrayClass:
		if (!getProperty(node, "indexid", attribute.Index))
		{
			nlwarning("CDBDescriptionParser::loadAttribute(): missing property 'indexid' in attribute node");
			return false;
		}
	case PDS_Class:
		if (!getProperty(node, "classid", attribute.TypeId))
		{
			nlwarning("CDBDescriptionParser::loadAttribute(): missing property 'classid' in attribute node");
			return false;
		}
		break;

	case PDS_BackRef:
		if (!getProperty(node, "classid", attribute.TypeId) ||
			!getProperty(node, "backreferentid", attribute.Reference))
		{
			nlwarning("CDBDescriptionParser::loadAttribute(): missing property 'classid' or 'backreferentid' in attribute node");
			return false;
		}
		break;

	case PDS_ArrayRef:
		if (!getProperty(node, "indexid", attribute.Index) ||
			!getProperty(node, "allownull", attribute.AllowNull))
		{
			nlwarning("CDBDescriptionParser::loadAttribute(): missing property 'indexid' in attribute node");
			return false;
		}
	case PDS_ForwardRef:
	case PDS_Set:
		if (!getProperty(node, "classid", attribute.TypeId) ||
			!getProperty(node, "forwardreferedid", attribute.Reference))
		{
			nlwarning("CDBDescriptionParser::loadAttribute(): missing property 'classid' or 'forwardreferedid' in attribute node");
			return false;
		}
		break;
	default:
		break;
	}

	return true;
}

/**
 * Load attribute description
 */
bool	CDBDescriptionParser::loadLog(xmlNodePtr node)
{
	uint	id;
	if (!getProperty(node, "id", id))
	{
		nlwarning("CDBDescriptionParser::loadLog(): missing property 'id' in logmsg node");
		return false;
	}

	if (id >= _Database.Logs.size())
		_Database.Logs.resize(id+1);

	CLogNode&	log = _Database.Logs[id];

	log.Id = id;

	if (!getProperty(node, "context", log.Context))
	{
		nlwarning("CDBDescriptionParser::loadLog(): missing property 'context' in logmsg node");
		return false;
	}

	xmlNodePtr	paramNode;
	FOREACH_CHILD(paramNode, node, param)
	{
		uint	id;
		if (!getProperty(paramNode, "id", id))
		{
			nlwarning("CDBDescriptionParser::loadLog(): missing property 'id' in logmsg.param node");
			return false;
		}

		if (id >= log.Parameters.size())
			log.Parameters.resize(id+1);

		std::string	logType;

		if (getProperty(paramNode, "typeid", logType))
		{
			if (logType == "string")
			{
				log.Parameters[id].TypeId = ExtLogTypeString;
			}
			else
			{
				NLMISC::fromString(logType, log.Parameters[id].TypeId);
			}
		}
		else
		{
			nlwarning("CDBDescriptionParser::loadLog(): missing property 'typeid' in logmsg.param node");
			return false;
		}
	}

	uint	i;
	uint	byteOffset = 0;
	for (i=0; i<log.Parameters.size(); ++i)
	{
		if (log.Parameters[i].TypeId == ExtLogTypeString)
		{
			log.Parameters[i].ByteOffset = byteOffset;
			log.Parameters[i].ByteSize = 2;
			log.Parameters[i].DataType = ExtLogTypeString;
		}
		else
		{
			log.Parameters[i].ByteOffset = byteOffset;
			log.Parameters[i].ByteSize = _Database.Types[log.Parameters[i].TypeId].ByteSize;
			log.Parameters[i].DataType = _Database.Types[log.Parameters[i].TypeId].DataType;
		}
		byteOffset += log.Parameters[i].ByteSize;
	}

	xmlNodePtr	msgNode = CIXml::getFirstChildNode(node, "msg");
	if (msgNode == NULL)
	{
		nlwarning("CDBDescriptionParser::loadLog(): failed to find msg child node in logmsg node");
		return false;
	}

	if (!CIXml::getContentString(log.Message, msgNode))
	{
		nlwarning("CDBDescriptionParser::loadLog(): failed to get msg child node content");
		return false;
	}

	return true;
}


/*
 * Build Columns
 */
bool	CDBDescriptionParser::buildColumns()
{
	bool	success = true;

	uint	i;
	for (i=0; i<_Database.Tables.size(); ++i)
	{
		if (!buildColumns(i))
		{
			success = false;
			nlwarning("CDBDescriptionParser::buildColumns(): failed to build columns for table %d of database '%s'", i, _Database.Name.c_str());
		}
	}

	return success;
}

/*
 * Build Table Columns
 */
bool	CDBDescriptionParser::buildColumns(uint tableIndex)
{
	if (tableIndex >= _Database.Tables.size())
		return false;

	CTableNode&		table = _Database.Tables[tableIndex];

	if (table.ColumnsBuilt)
		return true;

	uint	i;
	for (i=0; i<table.Attributes.size(); ++i)
	{
		CAttributeNode&	attribute = table.Attributes[i];
		CColumnNode		column;

		switch (attribute.MetaType)
		{
		case PDS_Type:
			{
				if (attribute.TypeId >= _Database.Types.size())
				{
					nlwarning("CDBDescriptionParser::buildColumns(): Can't access type %d in database '%s'", attribute.TypeId, _Database.Name.c_str());
					return false;
				}

				column.Index = (uint)table.Columns.size();
				column.Name = attribute.Name;
				column.TypeId = attribute.TypeId;
				column.DataType = _Database.Types[attribute.TypeId].DataType;
				column.ByteSize = _Database.Types[attribute.TypeId].ByteSize;
				table.Columns.push_back(column);
			}
			break;

		case PDS_Class:
			{
				if (attribute.TypeId >= _Database.Tables.size())
				{
					nlwarning("CDBDescriptionParser::buildColumns(): Can't access table %d in database '%s'", attribute.TypeId, _Database.Name.c_str());
					return false;
				}

				if (!buildColumns(attribute.TypeId))
				{
					nlwarning("CDBDescriptionParser::buildColumns(): Failed to build columns for table %d in database '%s'", attribute.TypeId, _Database.Name.c_str());
					return false;
				}

				CTableNode&	subtable = _Database.Tables[attribute.TypeId];

				uint	j;
				for (j=0; j<subtable.Columns.size(); ++j)
				{
					CColumnNode&	copy = subtable.Columns[j];
					column.Index = (uint)table.Columns.size();
					column.Name = attribute.Name+"."+copy.Name;
					column.TypeId = copy.TypeId;
					column.DataType = copy.DataType;
					column.ByteSize = copy.ByteSize;
					table.Columns.push_back(column);
				}
			}
			break;

		case PDS_BackRef:
			{
				column.Index = (uint)table.Columns.size();
				column.Name = attribute.Name;
				column.TypeId = attribute.TypeId;
				column.DataType = PDS_Index;
				column.ByteSize = 8;	/// \todo remove hardcoded value
				table.Columns.push_back(column);
			}
			break;

		case PDS_ForwardRef:
			{
				column.Index = (uint)table.Columns.size();
				column.Name = attribute.Name;
				column.TypeId = attribute.TypeId;
				column.DataType = PDS_Index;
				column.ByteSize = 8;	/// \todo remove hardcoded value
				table.Columns.push_back(column);
			}
			break;

		case PDS_ArrayType:
			{
				if (attribute.TypeId >= _Database.Types.size())
				{
					nlwarning("CDBDescriptionParser::buildColumns(): Can't access type %d in database '%s'", attribute.TypeId, _Database.Name.c_str());
					return false;
				}

				if (attribute.Index >= _Database.Types.size() || (_Database.Types[attribute.Index].Type != CTypeNode::TypeEnum && _Database.Types[attribute.Index].Type != CTypeNode::TypeDimension))
				{
					nlwarning("CDBDescriptionParser::buildColumns(): Can't access index %d (or is not index) in database '%s'", attribute.Index, _Database.Name.c_str());
					return false;
				}

				CTypeNode&	index = _Database.Types[attribute.Index];
				uint	j;
				for (j=0; j<index.Dimension; ++j)
				{
					column.Index = (uint)table.Columns.size();
					column.Name = attribute.Name + '[' + (index.Type == CTypeNode::TypeDimension ? toString(j) : index.getEnumName(j)) + ']';
					column.TypeId = attribute.TypeId;
					column.DataType = _Database.Types[attribute.TypeId].DataType;
					table.Columns.push_back(column);
				}
			}
			break;

		case PDS_ArrayClass:
			{
				if (attribute.TypeId >= _Database.Tables.size())
				{
					nlwarning("CDBDescriptionParser::buildColumns(): Can't access table %d in database '%s'", attribute.TypeId, _Database.Name.c_str());
					return false;
				}

				if (attribute.Index >= _Database.Types.size() || (_Database.Types[attribute.Index].Type != CTypeNode::TypeEnum && _Database.Types[attribute.Index].Type != CTypeNode::TypeDimension))
				{
					nlwarning("CDBDescriptionParser::buildColumns(): Can't access index %d (or is not index) in database '%s'", attribute.Index, _Database.Name.c_str());
					return false;
				}

				if (!buildColumns(attribute.TypeId))
				{
					nlwarning("CDBDescriptionParser::buildColumns(): Failed to build columns for table %d in database '%s'", attribute.TypeId, _Database.Name.c_str());
					return false;
				}

				CTableNode&	subtable = _Database.Tables[attribute.TypeId];
				CTypeNode&	index = _Database.Types[attribute.Index];
				uint	j;
				for (j=0; j<index.Dimension; ++j)
				{
					uint	k;
					for (k=0; k<subtable.Columns.size(); ++k)
					{
						CColumnNode&	copy = subtable.Columns[k];
						column.Index = (uint)table.Columns.size();
						column.Name = attribute.Name + '[' + (index.Type == CTypeNode::TypeDimension ? toString(j) : index.getEnumName(j)) + ']' + '.' + copy.Name;
						column.TypeId = copy.TypeId;
						column.DataType = copy.DataType;
						column.ByteSize = copy.ByteSize;
						table.Columns.push_back(column);
					}
				}

			}
			break;

		case PDS_ArrayRef:
			{
				if (attribute.Index >= _Database.Types.size() || (_Database.Types[attribute.Index].Type != CTypeNode::TypeEnum && _Database.Types[attribute.Index].Type != CTypeNode::TypeDimension))
				{
					nlwarning("CDBDescriptionParser::buildColumns(): Can't access index %d (or is not index) in database '%s'", attribute.Index, _Database.Name.c_str());
					return false;
				}

				CTypeNode&	index = _Database.Types[attribute.Index];
				uint	j;
				for (j=0; j<index.Dimension; ++j)
				{
					column.Index = (uint)table.Columns.size();
					column.Name = attribute.Name + '[' + (index.Type == CTypeNode::TypeDimension ? toString(j) : index.getEnumName(j)) + ']';
					column.TypeId = attribute.TypeId;
					column.DataType = PDS_Index;
					column.ByteSize = 8;	/// \todo remove hardcoded value
					table.Columns.push_back(column);
				}
			}
			break;

		case PDS_Set:
			{
				column.Index = (uint)table.Columns.size();
				column.Name = attribute.Name;
				column.TypeId = attribute.TypeId;
				column.DataType = PDS_List;
				column.ByteSize = 4;	/// \todo remove hardcoded value
				table.Columns.push_back(column);
			}
			break;
		default:
			break;
		}
	}

	table.ColumnsBuilt = true;
	return true;
}


/*
 * Display whole database node info
 */
void	CDBDescriptionParser::display(NLMISC::CLog& log) const
{
	log.displayNL("Display of database '%s'", _Database.Name.c_str());

	uint	i;
	for (i=0; i<_Database.Tables.size(); ++i)
	{
		log.displayNL("%2d: %32s %d columns", i, _Database.Tables[i].Name.c_str(), _Database.Tables[i].Columns.size());
	}

	log.displayNL("End of database '%s'", _Database.Name.c_str());
}

/*
 * Display Table node info
 */
void	CDBDescriptionParser::displayTable(uint table, NLMISC::CLog& log) const
{
	if (table >= _Database.Tables.size())
		return;

	log.displayNL("Display of table '%s'", _Database.Tables[table].Name.c_str());

	uint	i;
	for (i=0; i<_Database.Tables[table].Columns.size(); ++i)
	{
		if (checkStrictDataType(_Database.Tables[table].Columns[i].DataType))
		{
			const CTypeNode&	type = _Database.Types[_Database.Tables[table].Columns[i].TypeId];
			log.displayNL("%5d: %32s %s", i, _Database.Tables[table].Columns[i].Name.c_str(), type.Name.c_str());
		}
		else
		{
			log.displayNL("%5d: %32s %s", i, _Database.Tables[table].Columns[i].Name.c_str(), getNameFromDataType(_Database.Tables[table].Columns[i].DataType).c_str());
		}
	}

	log.displayNL("End of table '%s'", _Database.Tables[table].Name.c_str());
}

