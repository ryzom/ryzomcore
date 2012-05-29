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

#include "pd_messages.h"

#include <nel/misc/path.h>
#include <nel/misc/file.h>

namespace RY_PDS
{

/*
 * Build Log string
 */
std::string	CDbMessage::buildLogString(const CDBDescriptionParser& description) const
{
	if (getType() != Log)
		return "";

	const CDatabaseNode&	db = description.getDatabaseNode();

	// check overflow
	if (getLogId() >= db.Logs.size())
	{
		return NLMISC::toString("<invalid log id %u>", getLogId());;
	}

	const std::string&			logmsg = db.Logs[getLogId()].Message;
	const std::vector<uint8>&	logbuffer = getLogBuffer();
	uint						i;

	std::string	result;

	for (i=0; i<logmsg.size(); )
	{
		if (logmsg[i] == '$')
		{
			++i;
			uint	param;
			NLMISC::fromString(logmsg.substr(i), param);
			while (i<logmsg.size() && logmsg[i]>='0' && logmsg[i]<='9')
				++i;

			// check no overflow
			if (param >= db.Logs[getLogId()].Parameters.size())
				break;

			const CLogNode::CParameter&	lparam = db.Logs[getLogId()].Parameters[param];

			// check no overflow
			if (lparam.ByteOffset+lparam.ByteSize > logbuffer.size())
				break;

			const uint8*				dataptr = (&(logbuffer[0])) + lparam.ByteOffset;

			if (lparam.DataType == ExtLogTypeString)
			{
				uint16	stringOffset = *(uint16*)dataptr;
				if (stringOffset < _ExtLogBuffer.size())
					result += (char*)(&(_ExtLogBuffer[stringOffset]));
			}
			else
			{
				const CTypeNode&			typenode = db.Types[lparam.TypeId];

				switch (typenode.DataType)
				{
				case PDS_bool:		result += (*dataptr != 0 ? "true" : "false");			break;
				case PDS_char:		result += *(char*)dataptr;								break;
				case PDS_ucchar:	result += NLMISC::toString(*(ucchar*)dataptr);			break;
				case PDS_uint8:		result += NLMISC::toString(*(uint8*)dataptr);			break;
				case PDS_uint16:	result += NLMISC::toString(*(uint16*)dataptr);			break;
				case PDS_uint32:	result += NLMISC::toString(*(uint32*)dataptr);			break;
				case PDS_uint64:	result += NLMISC::toString(*(uint64*)dataptr);			break;
				case PDS_sint8:		result += NLMISC::toString(*(sint8*)dataptr);			break;
				case PDS_sint16:	result += NLMISC::toString(*(sint16*)dataptr);			break;
				case PDS_sint32:	result += NLMISC::toString(*(sint32*)dataptr);			break;
				case PDS_sint64:	result += NLMISC::toString(*(sint64*)dataptr);			break;
				case PDS_float:		result += NLMISC::toString(*(float*)dataptr);			break;
				case PDS_double:	result += NLMISC::toString(*(double*)dataptr);			break;
				case PDS_CSheetId:	result += NLMISC::toString(*(uint32*)dataptr);			break;
				case PDS_CEntityId:	result += ((NLMISC::CEntityId*)dataptr)->toString();	break;
				case PDS_enum:
					{
						result += typenode.getEnumName(*(uint32*)dataptr);
					}
					break;
				case PDS_dimension:
					{
						if (typenode.ByteSize == 1)			result += NLMISC::toString(*(uint8*)dataptr);
						else if (typenode.ByteSize == 2)	result += NLMISC::toString(*(uint16*)dataptr);
						else if (typenode.ByteSize == 4)	result += NLMISC::toString(*(uint32*)dataptr);
					}
					break;
				default:
					break;
				}
			}
		}
		else
		{
			result += logmsg[i++];
		}
	}
	
	return result;
}

/*
 * Dump Message content to string as a human readable message
 */
void	CDbMessage::getHRContent(const CDBDescriptionParser& description, std::string& result) const
{
	const CDatabaseNode&	db = description.getDatabaseNode();

	switch (getType())
	{
	case UpdateValue:
		{
			if (getTable() >= db.Tables.size())
				return;
			const CTableNode&	table = db.Tables[getTable()];
			if (getColumn() >= table.Columns.size())
				return;
			const CColumnNode&	column = table.Columns[getColumn()];
			std::string			strValue;
			switch (column.DataType)
			{
			case PDS_bool:		strValue = (asBool() ? "true" : "false");	break;
			case PDS_char:		strValue = asChar();						break;
			case PDS_ucchar:	strValue = NLMISC::toString(asUCChar());	break;
			case PDS_uint8:		strValue = NLMISC::toString(asUint8());		break;
			case PDS_uint16:	strValue = NLMISC::toString(asUint16());	break;
			case PDS_uint32:	strValue = NLMISC::toString(asUint32());	break;
			case PDS_uint64:	strValue = NLMISC::toString(asUint64());	break;
			case PDS_sint8:		strValue = NLMISC::toString(asSint8());		break;
			case PDS_sint16:	strValue = NLMISC::toString(asSint16());	break;
			case PDS_sint32:	strValue = NLMISC::toString(asSint32());	break;
			case PDS_sint64:	strValue = NLMISC::toString(asSint64());	break;
			case PDS_float:		strValue = NLMISC::toString(asFloat());		break;
			case PDS_double:	strValue = NLMISC::toString(asDouble());	break;
			case PDS_CSheetId:	strValue = NLMISC::toString(asUint32());	break;
			case PDS_CEntityId:	strValue = asEntityId().toString();			break;
			case PDS_enum:
				{
					if (column.TypeId >= db.Types.size())
						return;
					const CTypeNode&	type = db.Types[column.TypeId];
					strValue = type.getEnumName(asUint32());
				}
				break;
			case PDS_dimension:
				{
					if (column.ByteSize == 1)		strValue = NLMISC::toString(asUint8());
					else if (column.ByteSize == 2)	strValue = NLMISC::toString(asUint16());
					else if (column.ByteSize == 4)	strValue = NLMISC::toString(asUint32());
				}
				break;
			default:
				break;
			}

			result = NLMISC::toString("%-12s: ", "UpdateValue");

			if (objectEntityIdPresent())
			{
				result += getObjectId().toString();
				result += ' ';
			}

			result += NLMISC::toString("%s:%u:%s = %s", table.Name.c_str(), getRow(), column.Name.c_str(), strValue.c_str());
		}
		break;

	case SetParent:
		{
			if (getTable() >= db.Tables.size())
				return;
			const CTableNode&	table = db.Tables[getTable()];
			if (getColumn() >= table.Columns.size())
				return;
			const CColumnNode&	column = table.Columns[getColumn()];

			result = NLMISC::toString("%-12s: ", "SetParent");

			if (objectEntityIdPresent())
			{
				result += getObjectId().toString();
				result += ' ';
			}

			result += NLMISC::toString("%s:%u:%s = %s", table.Name.c_str(), getRow(), column.Name.c_str(), getObjectIndex().toString().c_str());

			if (parentsEntityIdPresent())
				result += NLMISC::toString(" new=%s previous=%s", getNewParentId().toString().c_str(), getPreviousParentId().toString().c_str());
		}
		break;

	case AllocRow:
		{
			if (getTable() >= db.Tables.size())
				return;
			const CTableNode&	table = db.Tables[getTable()];
			/// \todo display mapping value

			if (objectEntityIdPresent())
			{
				result = getObjectId().toString();
				result += ' ';
			}

			result = NLMISC::toString("%-12s: %s%s:%u", "AllocRow", result.c_str(), table.Name.c_str(), getRow());
		}
		break;

	case DeallocRow:
		{
			if (getTable() >= db.Tables.size())
				return;
			const CTableNode&	table = db.Tables[getTable()];

			if (objectEntityIdPresent())
			{
				result = getObjectId().toString();
				result += ' ';
			}

			result = NLMISC::toString("%-12s: %s%s:%u", "DeallocRow", result.c_str(), table.Name.c_str(), getRow());
		}
		break;

	case LoadRow:
		{
			if (getTable() >= db.Tables.size())
				return;
			const CTableNode&	table = db.Tables[getTable()];
			result = NLMISC::toString("%-12s: %s %"NL_I64"X %s", "LoadRow", table.Name.c_str(), asUint64(), asEntityId().toString().c_str());
		}
		break;

	case AddString:
		{
			result = NLMISC::toString("%-12s: %s='%s'", "AddString", asEntityId().toString().c_str(), getString().toString().c_str());
		}
		break;

	case UnmapString:
		{
			result = NLMISC::toString("%-12s: %s", "UnmapString", asEntityId().toString().c_str());
		}
		break;

	case ReleaseRow:
		{
			if (getTable() >= db.Tables.size())
				return;
			const CTableNode&	table = db.Tables[getTable()];
			result = NLMISC::toString("%-12s: %s:%u", "ReleaseRow", table.Name.c_str(), getRow());
		}
		break;


	case Log:
		{
			result = NLMISC::toString("%-12s: ", "Log")+buildLogString(description);
		}
		break;

	case PushContext:
		{
			result = NLMISC::toString("%-12s:", "PushContext");
			//indent += "..";
		}
		break;

	case PopContext:
		{
			//if (indent.size() >= 2)
			//	indent = indent.substr(0, indent.size()-2);
			result = NLMISC::toString("%-12s:", "PopContext");
		}
		break;

	case LogChat:
		{
			result = NLMISC::toString("%-12s: %s says '%s' to", "LogChat", asEntityId().toString().c_str(), _String.toString().c_str());
			if (_LogBuffer.empty())
			{
				result += " no one";
			}
			else
			{
				if (_LogBuffer.empty())
					return;
				const NLMISC::CEntityId*	ptr = (const NLMISC::CEntityId*)(&(_LogBuffer[0]));
				uint	num = (uint)_LogBuffer.size()/sizeof(NLMISC::CEntityId);
				uint	i;
				for (i=0; i<num; ++i)
				{
					result += ' ';
					result += ptr[i].toString();
				}
			}
		}
		break;

	default:
		result = NLMISC::toString("**** Unknown message type %d ****", getType());
		break;
	}
}


inline bool		compareEId(const NLMISC::CEntityId& id1, const NLMISC::CEntityId& id2)
{
	return id1.getType() == id2.getType() && id1.getShortId() == id2.getShortId();
}


/*
 * Does message contains CEntityId?
 */
bool	CDbMessage::contains(const CDBDescriptionParser& description, const NLMISC::CEntityId& id)
{
	const CDatabaseNode&	db = description.getDatabaseNode();

	try
	{

		switch (getType())
		{
		case UpdateValue:
			{
				if (objectEntityIdPresent() && compareEId(getObjectId(), id))
					return true;

				if (getTable() >= db.Tables.size())
					return false;
				const CTableNode&	table = db.Tables[getTable()];
				if (getColumn() >= table.Columns.size())
					return false;
				const CColumnNode&	column = table.Columns[getColumn()];

				if (column.DataType == PDS_CEntityId && compareEId(asEntityId(), id))
					return true;
			}
			break;

		case AllocRow:
		case DeallocRow:
			{
				if (objectEntityIdPresent() && compareEId(getObjectId(), id))
					return true;
			}
			break;

		case SetParent:
			if ((objectEntityIdPresent() && compareEId(getObjectId(), id)) ||
				(parentsEntityIdPresent() && (compareEId(getNewParentId(), id) || compareEId(getPreviousParentId(), id))))
				return true;
			break;

		case UnmapString:
		case AddString:
			if (compareEId(asEntityId(), id))
				return true;
			break;

		case Log:
			{
				if (getLogId() >= db.Logs.size())
					return false;
				const CLogNode&				log = db.Logs[getLogId()];
				const std::vector<uint8>&	logbuffer = getLogBuffer();
				uint	i;
				for (i=0; i<log.Parameters.size(); ++i)
				{
					const CLogNode::CParameter&	lparam = log.Parameters[i];

					if (lparam.DataType == PDS_CEntityId)
					{
						if (lparam.ByteOffset+lparam.ByteSize > logbuffer.size())
							continue;

						const uint8*	dataptr = (&(logbuffer[0])) + lparam.ByteOffset;

						if (compareEId(*((NLMISC::CEntityId*)dataptr), id))
							return true;
					}
				}
			}
			break;

		case LogChat:
			{
				if (compareEId(asEntityId(), id))
					return true;

				if (_LogBuffer.empty())
					break;

				const NLMISC::CEntityId*	ptr = (const NLMISC::CEntityId*)(&(_LogBuffer[0]));
				uint	num = (uint)_LogBuffer.size()/sizeof(NLMISC::CEntityId);
				uint	i;
				for (i=0; i<num; ++i)
					if (compareEId(ptr[i], id))
						return true;
			}
			break;

		default:
			break;
		}
	}
	catch (...)
	{
	}

	return false;
}


/*
 * Does message contains CEntityId?
 */
bool	CDbMessage::contains(const CDBDescriptionParser& description, const std::string& str)
{
	switch (getType())
	{
	case AddString:
	case LogChat:
		return getString().toString().find(str) != std::string::npos;
		break;

	case Log:
		return buildLogString(description).find(str) != std::string::npos;
		break;

	default:
		break;
	}

	return false;
}



/*
 * Display UpdateLog content (using a database description)
 */
void	CUpdateLog::display(const CDBDescriptionParser& description, NLMISC::CLog& log, bool onlySelected)
{
	if (_Updates == NULL)
	{
		log.displayNL("#? <empty content>");
		return;
	}

	const CDatabaseNode&	db = description.getDatabaseNode();

	uint		contextIndent = 0;
	uint		msg;
	std::string	indent;
	for (msg=0; msg<_Updates->getNumMessages(); ++msg)
	{
		CDbMessage&	message = _Updates->getMessage(msg);

		if (onlySelected && !message.Selected)
			continue;

		std::string	result;
		message.getHRContent(description, result);
		log.displayNL("#$ %04X:%02X: %s", msg, message.ContextDepth, result.c_str());
	}
}


/*
 * Select contexts and messages containing a given entityId
 */
bool	CUpdateLog::selectMessages(const CDBDescriptionParser& description, const NLMISC::CEntityId& id)
{
	bool	selected = false;

	std::vector<std::pair<uint, bool> >	contextsStart;

	uint	msg;
	for (msg=0; msg<_Updates->getNumMessages(); ++msg)
	{
		CDbMessage&	message = _Updates->getMessage(msg);

		if (message.getType() == CDbMessage::PushContext)
		{
			contextsStart.push_back(std::make_pair<uint, bool>(msg, false));
		}
		else if (message.getType() == CDbMessage::PopContext)
		{
			uint	contextStart = contextsStart.back().first;
			uint	contextEnd = msg;
			bool	contextSelected = contextsStart.back().second;

			contextsStart.pop_back();

			if (contextSelected)
			{
				if (!contextsStart.empty())
					contextsStart.back().second = true;

				uint	i;
				uint	level = 0;
				for (i=contextStart; i<=contextEnd; ++i)
				{
					CDbMessage&	message = _Updates->getMessage(i);
					if (message.getType() == CDbMessage::PushContext)		++level;
					else if (message.getType() == CDbMessage::PopContext)	--level;

					if (level <= 1)
						message.Selected = true;
				}
			}
		}
		else if (message.contains(description, id))
		{
			if (!contextsStart.empty())
				contextsStart.back().second = true;

			message.Selected = true;
			selected = true;
		}
	}

	return selected;
}

/*
 * Select contexts and messages containing a given entityId
 */
bool	CUpdateLog::selectMessages(const CDBDescriptionParser& description, const std::string& str)
{
	bool	selected = false;

	std::vector<std::pair<uint, bool> >	contextsStart;

	uint	msg;
	for (msg=0; msg<_Updates->getNumMessages(); ++msg)
	{
		CDbMessage&	message = _Updates->getMessage(msg);

		if (message.getType() == CDbMessage::PushContext)
		{
			contextsStart.push_back(std::make_pair<uint, bool>(msg, false));
		}
		else if (message.getType() == CDbMessage::PopContext)
		{
			uint	contextStart = contextsStart.back().first;
			uint	contextEnd = msg;
			bool	contextSelected = contextsStart.back().second;

			contextsStart.pop_back();

			if (contextSelected)
			{
				if (!contextsStart.empty())
					contextsStart.back().second = true;

				uint	i;
				uint	level = 0;
				for (i=contextStart; i<=contextEnd; ++i)
				{
					CDbMessage&	message = _Updates->getMessage(i);
					if (message.getType() == CDbMessage::PushContext)		++level;
					else if (message.getType() == CDbMessage::PopContext)	--level;

					if (level <= 1)
						message.Selected = true;
				}
			}
		}
		else if (message.contains(description, str))
		{
			if (!contextsStart.empty())
				contextsStart.back().second = true;

			message.Selected = true;
			selected = true;
		}
	}

	return selected;
}

/*
 * Select contexts and messages containing modification of a value for a given entityId
 * return true if there were at least one message selected
 */
bool	CUpdateLog::selectMessages(const CDBDescriptionParser& description, const NLMISC::CEntityId& id, const std::string& valuePath)
{
	bool	selected = false;

	std::string::size_type pos = valuePath.find('.');
	if (pos == std::string::npos)
		return false;

	std::string	tableName = valuePath.substr(0, pos);
	std::string	columnName = valuePath.substr(pos+1);

	const CDatabaseNode&	db = description.getDatabaseNode();

	uint	table;
	for (table=0; table<db.Tables.size(); ++table)
		if (db.Tables[table].Name == tableName)
			break;

	if (table == db.Tables.size())
		return false;

	uint	column;
	for (column=0; column<db.Tables[table].Columns.size(); ++column)
		if (db.Tables[table].Columns[column].Name == columnName)
			break;

	if (column == db.Tables[table].Columns.size())
		return false;

	std::vector<std::pair<uint, bool> >	contextsStart;

	uint	msg;
	for (msg=0; msg<_Updates->getNumMessages(); ++msg)
	{
		CDbMessage&	message = _Updates->getMessage(msg);

		if (message.getType() == CDbMessage::PushContext)
		{
			contextsStart.push_back(std::make_pair<uint, bool>(msg, false));
		}
		else if (message.getType() == CDbMessage::PopContext)
		{
			uint	contextStart = contextsStart.back().first;
			uint	contextEnd = msg;
			bool	contextSelected = contextsStart.back().second;

			contextsStart.pop_back();

			if (contextSelected)
			{
				if (!contextsStart.empty())
					contextsStart.back().second = true;

				uint	i;
				uint	level = 0;
				for (i=contextStart; i<=contextEnd; ++i)
				{
					CDbMessage&	message = _Updates->getMessage(i);
					if (message.getType() == CDbMessage::PushContext)		++level;
					else if (message.getType() == CDbMessage::PopContext)	--level;

					if (level <= 1)
						message.Selected = true;
				}
			}
		}
		else if (message.contains(description, id) && message.valueModified(table, column))
		{
			if (!contextsStart.empty())
				contextsStart.back().second = true;

			message.Selected = true;
			selected = true;
		}
	}

	return selected;
}


class CSelectContext
{
public:

	CSelectContext() : Start(0), End(0), Selects(0)		{ }
	~CSelectContext()
	{
		for (uint i=0; i<SubContexts.size(); ++i)
			delete SubContexts[i];
	}

	uint						Start;			// Start Message
	uint						End;			// End Message

	uint32						Selects;		// Selection Mask

	std::vector<CSelectContext*>	SubContexts;

	void	spreadSelects()
	{
		uint	i;
		for (i=0; i<SubContexts.size(); ++i)
		{
			// prevent from spreading selection to unselected branches
			if (SubContexts[i]->Selects != 0)
			{
				SubContexts[i]->Selects |= Selects;
				SubContexts[i]->spreadSelects();
			}
		}
	}

	bool	select(CDbMessageQueue* queue, uint32 mask, bool first = false, bool forceEval = false)
	{
		// context is selected ?
		if (Selects != mask && !forceEval)
			return false;

		bool	selected = false;

		uint	msg, sub = 0;
		for (msg=Start; msg<End; ++msg)
		{
			// skip to next context if current is passed
			while (sub < SubContexts.size() && SubContexts[sub]->End > msg)
				++sub;

			// if msg not in a sub context, select it
			if ((sub >= SubContexts.size() || msg < SubContexts[sub]->Start) && !first)
				selected |= (queue->getMessage(msg).Selected = true);
		}

		// select sub contexts
		for (sub=0; sub<SubContexts.size(); ++sub)
			selected |= SubContexts[sub]->select(queue, mask);

		return selected;
	}
};

/*
 * Select contexts and messages containing a list of entityIds
 * return true if there were at least one message selected
 */
bool	CUpdateLog::selectMessages(const CDBDescriptionParser& description, const std::vector<NLMISC::CEntityId>& ids)
{
	typedef std::pair<CSelectContext*, uint>	TEvalSub;
	typedef std::vector<TEvalSub>				TContextStack;
	CSelectContext	root;
	CSelectContext*	current = &root;
	TContextStack	contextStack;

	contextStack.push_back(TEvalSub(&root, 0));

	root.Start = 0;
	root.End = _Updates->getNumMessages();

	uint32	globalMask = (1 << ids.size())-1;

	// rebuild context hierarchy
	// and select contexts depending on ids appearing in message
	uint	m;
	for (m=0; m<_Updates->getNumMessages(); ++m)
	{
		CDbMessage&	msg = _Updates->getMessage(m);

		// push a new context
		if (msg.getType() == CDbMessage::PushContext)
		{
			current->SubContexts.push_back(new CSelectContext());
			current = current->SubContexts.back();

			contextStack.push_back(TEvalSub(current, 0));
			current->Start = m;
		}
		// pop the current context
		else if (msg.getType() == CDbMessage::PopContext)
		{
			contextStack.pop_back();

			uint32	contextMask = current->Selects;

			current->End = m+1;
			current = contextStack.back().first;
			++(contextStack.back().second);

			current->Selects |= contextMask;
		}
		else
		{
			uint32	idMask = 1;
			uint32	msgMask = 0;
			uint	i;
			for (i=0; i<ids.size(); ++i, idMask <<= 1)
			{
				if (msg.contains(description, ids[i]))
					msgMask |= idMask;
			}

			// only select in subcontexts (not in root)
			if (contextStack.size() > 1)
				current->Selects |= msgMask;

			if (msgMask == globalMask)
				msg.Selected = true;
		}
	}

	root.Selects = 0;

	root.spreadSelects();
	return root.select(_Updates, globalMask, true, true);
}



/*
 * Display log for a given entity id, between 2 dates
 */
void	CUpdateLog::displayLogs(const CDBDescriptionParser& description,
								const NLMISC::CEntityId& id,
								const CTimestamp& begin,
								const CTimestamp& end,
								const std::string& path,
								NLMISC::CLog& log,
								float* progress)
{
	if (end-begin > 86400)
	{
		log.displayNL("#! time interval exceeds 1 day");
		return;
	}

	std::vector<std::string>	files;
	NLMISC::CPath::getPathContent(path, false, false, true, files);

	std::sort(files.begin(), files.end());

	uint	i;
	// preselect files
	for (i=0; i<files.size(); ++i)
	{
		char	buffer[64];
		if (NLMISC::CFile::getExtension(files[i]) != "pd_log" ||
			sscanf(NLMISC::CFile::getFilenameWithoutExtension(files[i]).c_str(), "%28s", buffer) != 1)
		{
			files.erase(files.begin()+i);
			--i;
			continue;
		}

		CTimestamp	timestamp;
		timestamp.fromString(buffer);

		if (timestamp < begin || timestamp > end)
		{
			files.erase(files.begin()+i);
			--i;
			continue;
		}
	}

	for (i=0; i<files.size(); ++i)
	{
		if (progress)
			*progress = (i*100.0f)/files.size();

		char	buffer[64];
		if (NLMISC::CFile::getExtension(files[i]) != "pd_log" ||
			sscanf(NLMISC::CFile::getFilenameWithoutExtension(files[i]).c_str(), "%28s", buffer) != 1)
			continue;

		CTimestamp	timestamp;
		timestamp.fromString(buffer);

		if (timestamp < begin || timestamp > end)
			continue;

		std::vector<CUpdateLog>	logs;

		NLMISC::CIFile	file;
		if (!file.open(files[i]))
			continue;

		bool	displayedFile = false;

		while (!file.eof())
		{
			try
			{
				file.serialCont(logs);
			}
			catch (const NLMISC::Exception&)
			{
				break;
			}

			uint	k;
			for (k=0; k<logs.size(); ++k)
				if (logs[k].checkTimestampBoundaries(begin, end) && logs[k].selectMessages(description, id))
					logs[k].display(description, *NLMISC::InfoLog, true);
		}
	}

}


class CDescriptionItem
{
public:

	CDescriptionItem() : Loaded(false)	{}

	CTimestamp				Start;
	std::string				Filename;
	CDBDescriptionParser	Description;
	bool					Loaded;
};


void	selectLogFiles(const std::string& path,
					   const CTimestamp& begin,
					   const CTimestamp& end,
					   std::vector<std::string>& files,
					   std::vector<CDescriptionItem>& descriptions)
{
	NLMISC::CPath::getPathContent(path, false, false, true, files);

	std::sort(files.begin(), files.end());

	uint	i;

	// preselect description files
	char	buffer[64];
	std::vector<CDescriptionItem>	Descriptions;
	for (i=0; i<files.size(); ++i)
	{
		if (NLMISC::CFile::getExtension(files[i]) == "description")
		{
			CDescriptionItem	item;
			item.Start.fromString(NLMISC::CFile::getFilenameWithoutExtension(files[i]).c_str());
			item.Filename = files[i];

			descriptions.push_back(item);
		}

		if (NLMISC::CFile::getExtension(files[i]) != "pd_log" ||
			sscanf(NLMISC::CFile::getFilenameWithoutExtension(files[i]).c_str(), "%28s", buffer) != 1)
		{
			files.erase(files.begin()+i);
			--i;
			continue;
		}
	}

	while (files.size() > 1)
	{
		// drop first file if next one is older than begin
		sscanf(NLMISC::CFile::getFilenameWithoutExtension(files[1]).c_str(), "%28s", buffer);
		CTimestamp	timestamp;
		timestamp.fromString(buffer);

		if (timestamp >= begin)
			break;

		files.erase(files.begin());
	}

	while (!files.empty())
	{
		sscanf(NLMISC::CFile::getFilenameWithoutExtension(files.back()).c_str(), "%28s", buffer);
		CTimestamp	timestamp;
		timestamp.fromString(buffer);

		if (timestamp <= end)
			break;

		files.pop_back();
	}
}


/*
 * Display log for a given entity id, between 2 dates
 */
void	CUpdateLog::processLogs(const std::string& path,
								const CTimestamp& begin,
								const CTimestamp& end,
								NLMISC::CLog& log,
								CLogProcessor* processor,
								float* progress)
{
	if (end-begin > 86400)
	{
		log.displayNL("#! time interval exceeds 1 day");
		return;
	}

	uint	i;
	std::vector<std::string>		files;
	std::vector<CDescriptionItem>	Descriptions;

	selectLogFiles(path, begin, end, files, Descriptions);

	bool	selectedFiles = false;
	bool	selectedMessages = false;

	for (i=0; i<files.size(); ++i)
	{
		if (progress)
			*progress = (i*100.0f)/files.size();

		std::vector<CUpdateLog>	logs;

		NLMISC::CIFile	file;
		if (!file.open(files[i]))
			continue;

		bool	displayedFile = false;

		while (!file.eof())
		{
			try
			{
				file.serialCont(logs);
			}
			catch (const NLMISC::Exception&)
			{
				break;
			}

			if (logs.empty())
				continue;

			// select matching description
			uint	j;
			for (j=0; j<Descriptions.size(); ++j)
				if (logs[0].StartStamp >= Descriptions[j].Start && (j+1 >= Descriptions.size() || logs[0].StartStamp < Descriptions[j+1].Start))
					break;

			if (j >= Descriptions.size())
				continue;

			// load description (if needed)
			if (!Descriptions[j].Loaded)
			{
				if (!Descriptions[j].Description.loadDescriptionFile(Descriptions[j].Filename))
					continue;

				if (!Descriptions[j].Description.buildColumns())
					continue;

				Descriptions[j].Loaded = true;
			}

			if (!Descriptions[j].Loaded)
				continue;

			selectedFiles = true;

			uint	k;
			for (k=0; k<logs.size(); ++k)
			{
				if (!logs[k].checkTimestampBoundaries(begin, end))
					continue;

				if (processor != NULL && !processor->processLog(logs[k], Descriptions[j].Description))
					continue;

				if (!displayedFile)
				{
					log.displayNL("#? In '%s':", files[i].c_str());
					displayedFile = true;
				}

				log.displayNL("## - at %s/%s", logs[k].StartStamp.toString().c_str(), logs[k].EndStamp.toString().c_str());

				selectedMessages = true;
				logs[k].display(Descriptions[j].Description, log, processor != NULL);
			}
		}
	}

	if (!selectedFiles)
		log.displayNL("#! No file selected");
	else if (!selectedMessages)
		log.displayNL("#! No message selected");
}

/*
 * Display log for a given entity id, between 2 dates
 */

class CSelectIdProcessor : public CUpdateLog::CLogProcessor
{
public:

	NLMISC::CEntityId	Id;

	virtual bool	processLog(CUpdateLog& log, const CDBDescriptionParser& description)
	{
		return log.selectMessages(description, Id);
	}
};

void	CUpdateLog::displayLogs(const std::string& path,
								const NLMISC::CEntityId& id,
								const CTimestamp& begin,
								const CTimestamp& end,
								NLMISC::CLog& log,
								float* progress)
{
	CSelectIdProcessor	p;
	p.Id = id;
	processLogs(path, begin, end, log, &p, progress);
}



/*
 * Display log for a given entity id, between 2 dates
 */
void	CUpdateLog::displayLogs(const std::string& path,
								const CTimestamp& begin,
								const CTimestamp& end,
								NLMISC::CLog& log,
								float* progress)
{
	processLogs(path, begin, end, log, NULL, progress);
}



/*
 * Display log for a given entity id, between 2 dates
 */

class CSelectIdValProcessor : public CUpdateLog::CLogProcessor
{
public:

	NLMISC::CEntityId	Id;
	std::string			Val;

	virtual bool	processLog(CUpdateLog& log, const CDBDescriptionParser& description)
	{
		return log.selectMessages(description, Id, Val);
	}
};

void	CUpdateLog::displayLogs(const std::string& path,
								const NLMISC::CEntityId& id,
								const std::string& valuePath,
								const CTimestamp& begin,
								const CTimestamp& end,
								NLMISC::CLog& log,
								float* progress)
{
	CSelectIdValProcessor	p;
	p.Id = id;
	p.Val = valuePath;
	processLogs(path, begin, end, log, &p, progress);
}



/*
 * Display log for a given entity id, between 2 dates
 */

class CSelectIdsProcessor : public CUpdateLog::CLogProcessor
{
public:

	std::vector<NLMISC::CEntityId>	Ids;

	virtual bool	processLog(CUpdateLog& log, const CDBDescriptionParser& description)
	{
		return log.selectMessages(description, Ids);
	}
};

void	CUpdateLog::displayLogs(const std::string& path,
								const std::vector<NLMISC::CEntityId>& ids,
								const CTimestamp& begin,
								const CTimestamp& end,
								NLMISC::CLog& log,
								float* progress)
{
	CSelectIdsProcessor	p;
	p.Ids = ids;
	processLogs(path, begin, end, log, &p, progress);
}



/*
 * Display log for a given entity id, between 2 dates
 */

class CSelectStrProcessor : public CUpdateLog::CLogProcessor
{
public:

	std::string	Str;

	virtual bool	processLog(CUpdateLog& log, const CDBDescriptionParser& description)
	{
		return log.selectMessages(description, Str);
	}
};

void	CUpdateLog::displayLogs(const std::string& path,
								const std::string& str,
								const CTimestamp& begin,
								const CTimestamp& end,
								NLMISC::CLog& log,
								float* progress)
{
	CSelectStrProcessor	p;
	p.Str = str;
	processLogs(path, begin, end, log, &p, progress);
}



/*
 * Elect matching description
 */
std::string	CUpdateLog::electDescription(const std::string& logFile)
{
	std::string	path = NLMISC::CFile::getPath(logFile);
	std::string	file = NLMISC::CFile::getFilenameWithoutExtension(logFile);


	CTimestamp	timestamp;
	timestamp.fromString(file.c_str());


	std::vector<std::string>	files;
	NLMISC::CPath::getPathContent(path, false, false, true, files);

	std::sort(files.begin(), files.end());

	uint	elected = 0xffffffff;
	for (uint i=0; i<files.size(); ++i)
	{
		if (NLMISC::CFile::getExtension(files[i]) != "description")
			continue;

		std::string	descr = NLMISC::CFile::getFilenameWithoutExtension(files[i]);

		CTimestamp	descts;
		descts.fromString(descr.c_str());

		if (descts <= timestamp)
			elected = i;
	}

	return (elected != 0xffffffff) ? files[elected] : std::string("");
}


/*
 * Check log timestamp boundaries
 */
bool	CUpdateLog::checkTimestampBoundaries(const CTimestamp& begin, const CTimestamp& end)
{
	return !(StartStamp > end || EndStamp < begin);
}

/*
 * Is Empty
 */
bool	CUpdateLog::isEmpty()
{
	return _Updates == NULL || _Updates->getNumMessages() == 0;
}


CUpdateLog::~CUpdateLog()
{
	releaseUpdates();
}

/*
 * Set updates
 */
void	CUpdateLog::setUpdates(CDbMessageQueue* updates)
{
	releaseUpdates();
	_Updates = updates;
	_OwnUpdates = false;
}

/*
 * Create updates
 */
void	CUpdateLog::createUpdates()
{
	releaseUpdates();
	_Updates = new CDbMessageQueue();
	_OwnUpdates = true;
}


}

