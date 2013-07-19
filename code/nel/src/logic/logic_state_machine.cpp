// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#include "nel/logic/logic_state_machine.h"

#include "nel/net/service.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace NLLOGIC
{

// test if a string is valid considering a filter and a motif
bool testNameWithFilter( sint8 filter, string motif, string varName );

void xmlCheckNodeName (xmlNodePtr &node, const char *nodeName)
{
	// Check node name
	if ( node == NULL || ((const char*)node->name == NULL) || (strcmp ((const char*)node->name, nodeName) != 0) )
	{

		// try to find a child
		if (node != NULL)
		{
			node = CIXml::getFirstChildNode (node, nodeName);
			if ( node != NULL && ((const char*)node->name != NULL) && (strcmp ((const char*)node->name, nodeName) == 0) )
			{
				nlinfo ("check node %s ok in the child", nodeName);
				return;
			}
		}

		// Make an error message
		char tmp[512];
		smprintf (tmp, 512, "LogicStateMachine STATE_MACHINE XML Syntax error in block line %d, node %s should be %s",
			(int)node->line, node->name, nodeName);

		nlinfo (tmp);
		nlstop;
		throw EXmlParsingError (tmp);
	}

	nlinfo ("check node %s ok", nodeName);
}

std::string getXMLProp (xmlNodePtr node, const char *propName)
{
	const char *name = (const char*)xmlGetProp (node, (xmlChar*)propName);
	if (name)
	{
		nlinfo ("get prop %s = %s", propName, name);
		string n = name;
		xmlFree ((void*)name);
		return n;
	}
	else
	{
		// Make an error message
		char tmp[512];
		smprintf (tmp, 512, "LogicStateMachine XML Syntax error in block %s line %d, aguments Name not found",
			node->name, (int)node->line);
		throw EXmlParsingError (tmp);
		return "";
	}
}


//---------------------------------------------------
// setCurrentState :
//
//---------------------------------------------------
void CLogicStateMachine::setCurrentState( string stateName )
{
	map<string,CLogicState>::iterator itStates = _States.find( stateName );
	if( itStates != _States.end() )
	{
		(*itStates).second.exitState();

		_CurrentState = stateName;

		(*itStates).second.enterState();

		nlinfo("Switching to state \"%s\"",_CurrentState.c_str());
	}
	else
	{
		nlwarning("(LOGIC)<CLogicStateMachine::setCurrentState> The state \"%s\" is not in the state machine \"%s\"",stateName.c_str(),_Name.c_str());
	}

} // setCurrentState //



//---------------------------------------------------
// addCondition :
//
//---------------------------------------------------
void CLogicStateMachine::addCondition( CLogicCondition condition )
{
	condition.setLogicStateMachine(this);
	_Conditions.insert(make_pair(condition.getName(),condition));

} // addCondition //



//---------------------------------------------------
// addState :
//
//---------------------------------------------------
void CLogicStateMachine::addState( CLogicState logicState )
{
	logicState.setLogicStateMachine( this );
	_States.insert( std::make_pair(logicState.getName(),logicState) );

} // addState //




//---------------------------------------------------
// addSIdMap :
//
//---------------------------------------------------
void CLogicStateMachine::addSIdMap( const TSIdMap& sIdMap )
{
	// call addSIdMap for each state
	map<string,CLogicState>::iterator itStates;
	for( itStates = _States.begin(); itStates != _States.end(); ++itStates )
	{
		(*itStates).second.addSIdMap( sIdMap );
	}

} // addSIdMap //



//---------------------------------------------------
// processLogic :
//
//---------------------------------------------------
void CLogicStateMachine::processLogic()
{
	// call processLogic for the current state
	map<string,CLogicState>::iterator itStates = _States.find( _CurrentState );
	nlassert( itStates != _States.end() );
	(*itStates).second.processLogic();

	// update the counters
	map<string,CLogicCounter>::iterator itCount;
	for( itCount = _Counters.begin(); itCount != _Counters.end(); ++itCount )
	{
		(*itCount).second.update();
	}

} // processLogic //



//---------------------------------------------------
// getMessagesToSend :
//
//---------------------------------------------------
void CLogicStateMachine::getMessagesToSend( multimap<CEntityId,CMessage>& msgs )
{
	map<std::string, CLogicState>::iterator itState;
	for( itState = _States.begin(); itState != _States.end(); ++itState )
	{
		(*itState).second.getMessagesToSend( msgs );
	}

} // getMessagesToSend //




//---------------------------------------------------
// getVariable :
//
//---------------------------------------------------
bool CLogicStateMachine::getVariable( std::string& varName, CLogicVariable& var )
{
	map<string,CLogicVariable>::iterator itVar = _Variables.find( varName );
	if( itVar != _Variables.end() )
	{
		var = (*itVar).second;
		return true;
	}

	map<string,CLogicCounter>::iterator itCount = _Counters.find( varName );
	if( itCount != _Counters.end() )
	{
		var = (*itCount).second;
		return true;
	}

	return false;

} // getVariable //



//---------------------------------------------------
// getCondition :
//
//---------------------------------------------------
bool CLogicStateMachine::getCondition( const std::string& condName, CLogicCondition& cond )
{
	map<string,CLogicCondition>::iterator itCond = _Conditions.find( condName );
	if( itCond != _Conditions.end() )
	{
		cond = (*itCond).second;
		return true;
	}
	else
	{
		return false;
	}

} // getCondition //


//---------------------------------------------------
// modifyVariable :
//
//---------------------------------------------------
void CLogicStateMachine::modifyVariable( string varName, string modifOperator, sint64 value )
{
	map<string,CLogicVariable>::iterator itVar = _Variables.find( varName );
	if( itVar != _Variables.end() )
	{
		(*itVar).second.applyModification( modifOperator, value );
		return;
	}
	map<string,CLogicCounter>::iterator itCount = _Counters.find( varName );
	if( itCount != _Counters.end() )
	{
		(*itCount).second.applyModification( modifOperator, value );
		return;
	}

	nlwarning("(LOGIC)<CLogicStateMachine::modifyVariable> The variable \"%s\" is not in the state machine \"%s\"",varName.c_str(),_Name.c_str());

} // modifyVariable //



//---------------------------------------------------
// serial :
//
//---------------------------------------------------
/*void CLogicStateMachine::serial( IStream &f )
{
	f.xmlPush("STATE_MACHINE");


	f.serialCont( _Variables );
	f.serialCont( _Counters );
	f.serialCont( _Conditions );
	f.serialCont( _States );
	f.serial( _CurrentState );
	f.serial( _Name );

	if( f.isReading() )
	{
		// set the logic state machine addr in each state
		map<string,CLogicState>::iterator itStates;
		for( itStates = _States.begin(); itStates != _States.end(); ++itStates )
		{
			(*itStates).second.setLogicStateMachine( this );
		}

		// set the logic state machine addr in each conditions
		map<string,CLogicCondition>::iterator itCond;
		for( itCond = _Conditions.begin(); itCond != _Conditions.end(); ++itCond )
		{
			(*itCond).second.setLogicStateMachine( this );
		}
	}

	f.xmlPop();

} // serial //*/


//---------------------------------------------------
//	Display the variables
//
//---------------------------------------------------
void CLogicStateMachine::displayVariables()
{
	multimap<CEntityId,string> allVariables;

	// // get vars referenced in the states
	map<string, CLogicState>::iterator itS;
	for( itS = _States.begin(); itS != _States.end(); ++itS )
	{
		(*itS).second.fillVarMap( allVariables );
	}

	// extract the unclaimed variables from all the variables
	vector<string> unclaimedVariables;
	CEntityId unknown;
	unknown.setType( 0xfe );
	unknown.setCreatorId( 0 );
	unknown.setDynamicId( 0 );
	pair<multimap<CEntityId,string>::iterator,multimap<CEntityId,string>::iterator> itVarsRng = allVariables.equal_range(unknown);
	multimap<CEntityId,string>::iterator itVars;

	for( itVars = itVarsRng.first; itVars != itVarsRng.second; )
	{
		multimap<CEntityId,string>::iterator itDel = itVars++;
		unclaimedVariables.push_back( (*itDel).second );
		allVariables.erase( itDel );
	}
	/*
	if( itVarsRng.first != allVariables.end() )
	{
		itVars = itVarsRng.first;
		do
		{
			multimap<CEntityId,string>::iterator itDel = itVars++;
			unclaimedVariables.push_back( (*itDel).second );
			allVariables.erase( itDel );
		}
		while( itVars != itVarsRng.second );
	}
	*/


	nlinfo("VARIABLES/COUNTERS in %s : %d/%d are registered : ",_Name.c_str(),allVariables.size(),allVariables.size()+unclaimedVariables.size());
	// display the registered variables
	for( itVars = allVariables.begin(); itVars != allVariables.end(); ++itVars )
	{
		map<string, CLogicVariable>::const_iterator itV = _Variables.find( (*itVars).second );
		nlinfo("[%d] %s = %f",(uint8)(*itVars).first.getDynamicId(),(*itV).first.c_str(),(double)(*itV).second.getValue());
	}

	// display the unclaimed variables
	sort( unclaimedVariables.begin(), unclaimedVariables.end() );
	vector<string>::iterator itUV;
	for( itUV = unclaimedVariables.begin(); itUV != unclaimedVariables.end(); ++itUV )
	{
		map<string, CLogicVariable>::const_iterator itV = _Variables.find( *itUV );
		nlinfo("(-)%s = %f",(*itV).first.c_str(),(double)(*itV).second.getValue());
	}

} // displayVariables //


//---------------------------------------------------
//	Display the states
//
//---------------------------------------------------
void CLogicStateMachine::displayStates()
{
	nlinfo("There are %d STATES in the state machine \"%s\": ",_States.size(),_Name.c_str());
	map<string, CLogicState>::const_iterator itS;
	for( itS = _States.begin(); itS != _States.end(); ++itS )
	{
		nlinfo("%s",(*itS).first.c_str());
	}
	nlinfo("The current state is : \"%s\"",_CurrentState.c_str());

} // displayStates //


//---------------------------------------------------
//	Set the verbose mode for the variable
//
//---------------------------------------------------
void CLogicStateMachine::setVerbose( string varName, bool b )
{
	if( varName == "all" )
	{
		map<string, CLogicVariable>::iterator itV;
		for( itV = _Variables.begin(); itV != _Variables.end(); ++itV )
		{
			(*itV).second.setVerbose( b );
		}
		map<string, CLogicCounter>::iterator itC;
		for( itC = _Counters.begin(); itC != _Counters.end(); ++itC )
		{
			(*itC).second.setVerbose( b );
		}
		if(b)
		{
			nlinfo("the verbose mode has been activated for all the variables",varName.c_str());
		}
		else
		{
			nlinfo("the verbose mode has been desactivated for all the variables",varName.c_str());
		}
		return;
	}

	sint8 filter = -1;
	string motif;
	// *xxx* => we look for a string with xxx inside
	if( varName[0]=='*' && varName[varName.size()-1]=='*')
	{
		motif = varName.substr(1,varName.size()-2);
		filter = 0;
	}
	else
	// *xxx => we look for a string with xxx at the end
	if( varName[0]=='*' )
	{
		motif = varName.substr(1,varName.size()-1);
		filter = 1;
	}
	else
	// xxx* => we look for a string with xxx at the beginning
	if( varName[varName.size()-1]=='*' )
	{
		motif = varName.substr(0,varName.size()-1);
		filter = 2;
	}

	// if no filter
	if( filter == -1 )
	{
		map<string, CLogicVariable>::iterator itV = _Variables.find( varName );
		if( itV != _Variables.end() )
		{
			(*itV).second.setVerbose( b );
		}
		map<string, CLogicCounter>::iterator itC = _Counters.find( varName );
		if( itC != _Counters.end() || varName =="all" )
		{
			(*itC).second.setVerbose( b );
		}
	}
	// if filter
	else
	{
		map<string, CLogicVariable>::iterator itV;
		for( itV = _Variables.begin(); itV != _Variables.end(); ++itV )
		{
			if( testNameWithFilter( filter, motif,(*itV).second.getName()) )
			{
				(*itV).second.setVerbose( b );
			}
		}
		map<string, CLogicCounter>::iterator itC;
		for( itC = _Counters.begin(); itC != _Counters.end(); ++itC )
		{
			if( testNameWithFilter( filter, motif,(*itC).second.getName()) )
			{
				(*itC).second.setVerbose( b );
			}
		}
	}
	if(b)
	{
		nlinfo("the verbose mode for variable \"%s\" has been activated",varName.c_str());
	}
	else
	{
		nlinfo("the verbose mode for variable \"%s\" has been desactivated",varName.c_str());
	}

} // setVerbose //



//---------------------------------------------------
//	testNameWithFilter :
//
//---------------------------------------------------
bool testNameWithFilter( sint8 filter, string motif, string varName )
{
	if( varName.size() > motif.size() )
	{
		switch( filter )
		{
			// *xxx*
			case 0 :
			{
				if(varName.find(motif) != string::npos)
				{
					return true;
				}
			}
			break;

			// *xxx
			case 1 :
			{
				sint beginIndex = (sint)(varName.size() - motif.size() - 1);
				string endOfVarName = varName.substr(beginIndex,motif.size());
				if( endOfVarName == motif )
				{
					return true;
				}
			}
			break;

			// xxx*
			case 2 :
			{
				string beginOfVarName = varName.substr(0,motif.size());
				if( beginOfVarName == motif )
				{
					return true;
				}
			}
			break;
		}
	}

	return false;

} // testNameWithFilter //

void CLogicStateMachine::write (xmlDocPtr doc) const
{
	// Create the first node
	xmlNodePtr node = xmlNewDocNode (doc, NULL, (const xmlChar*)"STATE_MACHINE", NULL);
	xmlDocSetRootElement (doc, node);
	xmlSetProp (node, (const xmlChar*)"Name", (const xmlChar*)_Name.c_str());
	xmlSetProp (node, (const xmlChar*)"CurrentState", (const xmlChar*)_CurrentState.c_str());

	for (std::map<std::string, CLogicVariable>::const_iterator vit = _Variables.begin(); vit != _Variables.end(); vit++)
	{
		(*vit).second.write(node);
	}

	for (std::map<std::string, CLogicCounter>::const_iterator cit = _Counters.begin(); cit != _Counters.end(); cit++)
	{
		(*cit).second.write(node);
	}

	for (std::map<std::string, CLogicCondition>::const_iterator c2it = _Conditions.begin(); c2it != _Conditions.end(); c2it++)
	{
		(*c2it).second.write(node);
	}

	for (std::map<std::string, CLogicState>::const_iterator sit = _States.begin(); sit != _States.end(); sit++)
	{
		(*sit).second.write(node);
	}
}

void CLogicStateMachine::read (xmlNodePtr node)
{
	xmlCheckNodeName (node, "STATE_MACHINE");

	setName (getXMLProp (node, "Name"));

	{
		// Count the parent
		uint nb = CIXml::countChildren (node, "VARIABLE");
		uint i = 0;
		xmlNodePtr parent = CIXml::getFirstChildNode (node, "VARIABLE");
		while (i<nb)
		{
			CLogicVariable v;
			v.read(parent);
			_Variables.insert (make_pair(v.getName(), v));

			// Next parent
			parent = CIXml::getNextChildNode (parent, "VARIABLE");
			i++;
		}
	}

	{
		// Count the parent
		uint nb = CIXml::countChildren (node, "COUNTER");
		uint i = 0;
		xmlNodePtr parent = CIXml::getFirstChildNode (node, "COUNTER");
		while (i<nb)
		{
			CLogicCounter v;
			v.read(parent);
			_Counters.insert (make_pair(v.getName(), v));

			// Next parent
			parent = CIXml::getNextChildNode (parent, "COUNTER");
			i++;
		}
	}

	{
		// Count the parent
		uint nb = CIXml::countChildren (node, "CONDITION");
		uint i = 0;
		xmlNodePtr parent = CIXml::getFirstChildNode (node, "CONDITION");
		while (i<nb)
		{
			CLogicCondition v;
			v.read(parent);
			_Conditions.insert (make_pair(v.getName(), v));

			// Next parent
			parent = CIXml::getNextChildNode (parent, "CONDITION");
			i++;
		}
	}

	{
		// Count the parent
		uint nb = CIXml::countChildren (node, "STATE");
		uint i = 0;
		xmlNodePtr parent = CIXml::getFirstChildNode (node, "STATE");
		while (i<nb)
		{
			CLogicState v;
			v.read(parent);
			_States.insert (make_pair(v.getName(), v));

			// Next parent
			parent = CIXml::getNextChildNode (parent, "STATE");
			i++;
		}
	}

	setCurrentState (getXMLProp (node, "CurrentState"));
}

} // NLLOGIC
