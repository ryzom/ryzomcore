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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
#include "contextual_cursor.h"
#include "interface_v3/interface_manager.h"


///////////
// USING //
///////////
using namespace NLMISC;

////////////
// EXTERN //
////////////


////////////
// GLOBAL //
////////////


///////////////
// FUNCTIONS //
///////////////
//-----------------------------------------------
// CContextualCursor :
// Constructor.
//-----------------------------------------------
CContextualCursor::CContextualCursor()
{
}// CContextualCursor //


//-----------------------------------------------
// check :
// Check if there is an entity under the cursor.
//-----------------------------------------------
void CContextualCursor::check()
{
	// Call the function associated.
	TContext::iterator it = _Contexts.find(_Context);
	if(it != _Contexts.end())
	{
		TFunctions &functions =  (*it).second;
		if(functions.checkFunc)
			functions.checkFunc();
	}
}// check //

//-----------------------------------------------
// execute :
// Execute the Action according to the cursor state.
//-----------------------------------------------
void CContextualCursor::execute(bool rightClick, bool dblClick)
{
	// Call the function associated.
	TContext::iterator it = _Contexts.find(_Context);
	if(it != _Contexts.end())
	{
		TFunctions &functions =  (*it).second;
		if(functions.execFunc)
			functions.execFunc(rightClick, dblClick);
	}
}// execute //


//-----------------------------------------------
// add :
// Insert a context and associate the function. If the context already exist -> replace the function if 'replace' = true.
//-----------------------------------------------
void CContextualCursor::add(bool isString, const std::string &contextName, const std::string &texture, float distMax, TFunc checkFunc, TFunc2 execFunc, bool replace)
{
	TFunctions functions;

	// Replace the old associated texture if needed.
	if(replace)
		del(contextName);

	// Set the max distance for the context.
	functions.distMax = distMax;
	if(functions.distMax<0.f)
		functions.distMax = 0.f;

	functions.checkFunc = checkFunc;
	functions.execFunc  = execFunc;
	functions.cursor	= texture;
	functions.isString	= isString;
	_Contexts.insert(TContext::value_type(contextName, functions));
}// add //

//-----------------------------------------------
// del :
// Remove a context.
//-----------------------------------------------
void CContextualCursor::del(const std::string &contextName)
{
	// Delete the context.
	TContext::iterator it = _Contexts.find(contextName);
	if(it != _Contexts.end())
		_Contexts.erase(it);
}// del //


//-----------------------------------------------
// context :
// Select a nex context.
//-----------------------------------------------
bool CContextualCursor::context(const std::string &contextName, float dist, const ucstring &cursName)
{
	// Delete the context.
	TContext::iterator it = _Contexts.find(contextName);
	if(it == _Contexts.end())
		return false;
	// Get a reference on the structure.
	TFunctions &functions =  (*it).second;
	// Check the distance Max.
	if(functions.distMax>0.0f && dist>functions.distMax)
		return false;
	// Change the context name.
	_Context = contextName;
	// Change the cursor.
	CInterfaceManager *IM = CInterfaceManager::getInstance();
	if(IM)
	{
		CViewPointer *cursor = static_cast< CViewPointer* >( CWidgetManager::getInstance()->getPointer() );
		if(cursor)
		{
			if (!functions.isString)
			{
				// Is not a string cursor
				cursor->setStringMode(false);
				cursor->setCursor(functions.cursor);
			}
			else
			{
				// Is a string cursor
				cursor->setStringMode(true);
				if(cursName.empty())
					cursor->setString(CI18N::get(functions.cursor));
				else
					cursor->setString(cursName);
			}
		}
	}
	return true;
}// context //

//-----------------------------------------------
void CContextualCursor::release()
{
	_Contexts.clear ();
	_Context = "";
}
