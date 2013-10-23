
//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"

// game share
#include "game_share/utils.h"
#include "game_share/persistent_data.h"

// ai share
#include "ai_share.h"
#include "ai_actions.h"

#include "ai_actions_dr.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace AI_SHARE;

//---------------------------------------------------------------------------------------
// Stuff used for management of log messages

static bool VerboseLog=false;
#define LOG if (!VerboseLog) {} else NLMISC::InfoLog->displayNL


//-------------------------------------------------------------------------------------------------
// GLOBALS
//-------------------------------------------------------------------------------------------------


extern NLLIGO::CLigoConfig LigoConfig;



//-------------------------------------------------------------------------------------------------
// methods CAIActionsDataRecordPtr
//-------------------------------------------------------------------------------------------------

void CAIActionsDataRecordPtr::clear()
{
	_PdrPtr->clear();
}

void CAIActionsDataRecordPtr::readFile(const std::string &fileName)
{
	_PdrPtr->readFromFile(fileName.c_str());
}

void CAIActionsDataRecordPtr::writeFile(const std::string &fileName)
{	
	_PdrPtr->writeToFile(fileName.c_str());
}

void CAIActionsDataRecordPtr::display()
{
	NLMISC::CSString s;
	_PdrPtr->toLines(s);
	CVectorSString lines;
	s.splitLines(lines);
	for (uint32 i=0;i<lines.size();++i)
	{
		LOG("AIActions: %s",lines[i].c_str());
	}
}

void CAIActionsDataRecordPtr::serial(NLMISC::IStream &stream)
{
	if (!stream.isReading())
	{
		// calculate and write the data block size
		uint32 buffSize= _PdrPtr->totalDataSize();
		stream.serial(buffSize);

		// allocate a buffer to store the data block temporarily and dump data into it
		char* buffer= (char*)malloc(buffSize);
		nlassert(buffer!=NULL);
		_PdrPtr->toBuffer(buffer,buffSize);

		// write the buffer to the stream
		stream.serialBuffer((uint8*)buffer,buffSize);
	}
	else
	{
		// read the buffer size from the stream
		uint32 buffSize;
		stream.serial(buffSize);

		// allocate a temporary memory buffer and copy out the serialised data block
		char* buffer= (char*)malloc(buffSize);
		nlassert(buffer!=NULL);
		stream.serialBuffer((uint8*)buffer,buffSize);

		// decode the buffer contents to PDR representation
		_PdrPtr->fromBuffer(buffer,buffSize);
	}
}

void CAIActionsDataRecordPtr::addOpenFile(const std::string &fileName)
{
	LOG("AIActions: openFile(\"%s\")",fileName.c_str());

	_PdrPtr->push(_PdrPtr->addString("fileName"),fileName);
	_PdrPtr->pushStructBegin(_PdrPtr->addString("file"));
}

void CAIActionsDataRecordPtr::addCloseFile(const std::string &fileName)
{
	LOG("AIActions: closeFile(\"%s\")",fileName.c_str());

	_PdrPtr->pushStructEnd(_PdrPtr->addString("file"));
}

void CAIActionsDataRecordPtr::addExecute(uint64 action,const std::vector <CAIActions::CArg> &args)
{
	// convert the 64 bit action ID to a text string
	char actionName[9];
	actionName[8]=0;
	*(uint64*)&actionName[0]=action;

	LOG("AIActions: execute(\"%s\" with %d args)",actionName,args.size());

	// push any arguments belonging to the action
	for (uint32 i=0;i<args.size();++i)
	{
		args[i].pushToPdr(*_PdrPtr);
	}

	// push the action itself
	_PdrPtr->push(_PdrPtr->addString(actionName));
}

void CAIActionsDataRecordPtr::addBegin(uint32 contextAlias)
{
	LOG("AIActions: begin(%i)",contextAlias);

	_PdrPtr->push(_PdrPtr->addString("contextAlias"),contextAlias);
	_PdrPtr->pushStructBegin(_PdrPtr->addString("context"));
}

void CAIActionsDataRecordPtr::addEnd(uint32 contextAlias)
{
	LOG("AIActions: end(%i)",contextAlias);

	_PdrPtr->pushStructEnd(_PdrPtr->addString("context"));
}

static uint64 stringToInt64(const char *str)
{
	uint64 id=0;
	uint i;
	for (i=0;i<8 && str[i]!=0;++i)
		((char *)&id)[i]=str[i];

	// the following assert ensures that we never try to cram >8 letters into an int64
	nlassert(str[i]==0);

	return id;
}

void CAIActionsDataRecordPtr::applyToExecutor(CAIActions::IExecutor& executor)
{
	std::vector<CAIActions::CArg> args;
	std::vector<uint32> contextStack;
	std::string fileName;
	uint32 contextAlias=~0u;
	
	_PdrPtr->rewind();
	while (!_PdrPtr->isEndOfData())
	{
		
		CPersistentDataRecord::TToken token = _PdrPtr->peekNextToken();

		const std::string& tokenName= _PdrPtr->peekNextTokenName();

		if (_PdrPtr->isStartOfStruct())
		{
			if (tokenName=="file")
			{
				BOMB_IF(!args.empty(),"Found args with unmatched action in pdr",return);
				BOMB_IF(contextAlias!=~0u,"Found context alias without matching context clause",return);

				executor.openFile(fileName);
				//fileName = "";
				_PdrPtr->popStructBegin(token);
				continue;
			}

			if (tokenName=="context")
			{
				BOMB_IF(!args.empty(),"Found args with unmatched action in pdr",return);
			//	BOMB_IF(!fileName.empty(),"Found file name without matching file clause",return);

				executor.begin(contextAlias);
				contextStack.push_back(contextAlias);
				contextAlias=~0u;
				_PdrPtr->popStructBegin(token);
				continue;
			}

			// if it's not a special case then it must be an argument
			//BOMB_IF(!fileName.empty(),"Found file name without matching file clause",return);
			BOMB_IF(contextAlias!=~0u,"Found context alias without matching context clause",return);
			vectAppend(args).popFromPdr(*_PdrPtr);
			continue;
		}

		//BOMB_IF(!fileName.empty(),"Found file name without matching file clause",return);
		BOMB_IF(contextAlias!=~0u,"Found context alias without matching context clause",return);

		if (_PdrPtr->isEndOfStruct())
		{
			BOMB_IF(!args.empty(),"Found args with unmatched action in pdr",return);

			if (tokenName=="file")
			{
				BOMB_IF(fileName.empty(),"PDR file invalid: found end of file marker but no file open",return);
				executor.closeFile(fileName);
				fileName.clear();
				_PdrPtr->popStructEnd(token);
				continue;
			}

			if (tokenName=="context")
			{
				BOMB_IF(contextStack.empty(),"PDR file invalid: found more section closes than opens",return);
				executor.end(contextStack.back());
				contextStack.pop_back();
				_PdrPtr->popStructEnd(token);
				continue;
			}

			BOMB("I'm not sure we shold be here .... :( bug ???",return);
		}

		if (_PdrPtr->isTokenWithNoData())
		{
			BOMB_IF(tokenName.size()>8,"The action name '"+tokenName+"' is invalid - it must be <= 8 characters long",return);
			executor.execute(stringToInt64(tokenName.c_str()),args);
			args.clear();
			_PdrPtr->pop(token);
			continue;
		}

		if (tokenName=="contextAlias")
		{
			BOMB_IF(!args.empty(),"Found args with unmatched action in pdr",return);

			contextAlias= (uint32)_PdrPtr->popNextArg(token).asUint();
			continue;
		}

		if (tokenName=="fileName")
		{
			BOMB_IF(!fileName.empty(),"Found a file clause within another file clause!",return);
			BOMB_IF(!args.empty(),"Found args with unmatched action in pdr",return);

			fileName= _PdrPtr->popNextArg(token).asString();
			continue;
		}
		// contxt wiht no value
		if (tokenName == "context")
		{		
		    BOMB_IF(contextAlias==~0u,"error",return);
			executor.begin(contextAlias);
			executor.end(contextAlias);
			contextAlias=~0u;
			_PdrPtr->pop(token);
			continue;
		}

		// if it's not a special case then it must be an argument
		vectAppend(args).popFromPdr(*_PdrPtr);
	}
}

//---------------------------------------------------------------------------------------
// Control over verbose nature of logging
//---------------------------------------------------------------------------------------

NLMISC_COMMAND(verboseActionsParseLog,"Turn on or off or check the state of verbose parser activity logging","")
{
	if(args.size()>1)
		return false;

	if(args.size()==1)
	{
		if(args[0]==string("on")||args[0]==string("ON")||args[0]==string("true")||args[0]==string("TRUE")||args[0]==string("1"))
			VerboseLog=true;

		if(args[0]==string("off")||args[0]==string("OFF")||args[0]==string("false")||args[0]==string("FALSE")||args[0]==string("0"))
			VerboseLog=false;
	}

	nlinfo("verboseLogging is %s",VerboseLog?"ON":"OFF");
	return true;
}

//---------------------------------------------------------------------------------------
