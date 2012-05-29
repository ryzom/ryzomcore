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
#include "nel/logic/logic_variable.h"

using namespace std;
using namespace NLMISC;


namespace NLLOGIC
{


//---------------------------------------------------
// CLogicVariable :
//
//---------------------------------------------------
CLogicVariable::CLogicVariable()
{
	_Value = 0;
	_Name = "unamed_var";
	_Verbose = false;

} // CLogicVariable //



//---------------------------------------------------
// setValue :
//
//---------------------------------------------------
void CLogicVariable::setValue( sint64 value )
{
	_Value = value;

	if( _Verbose )
	{
		nlinfo("variable \"%s\" value is now %f",_Name.c_str(),(double)_Value);
	}

} // setValue //



//---------------------------------------------------
// applyModification :
//
//---------------------------------------------------
void CLogicVariable::applyModification( string op, sint64 value )
{
	if( op == "SET" || op == "set" )
	{
		_Value = value;
	}
	else
	if( op == "ADD" || op == "add" )
	{
		_Value += value;
	}
	else
	if( op == "SUB" || op == "sub" )
	{
		_Value -= value;
	}
	else
	if( op == "MUL" || op == "mul")
	{
		_Value *= value;
	}
	else
	if( op == "DIV" || op == "div")
	{
		if( value != 0 ) _Value /= value;
	}
	else
	{
		nlwarning("(LGCS)<CLogicVariable::applyModification> The operator \"%s\" is unknown",op.c_str());
		return;
	}

	if( _Verbose )
	{
		nlinfo("variable \"%s\" value is now %f",_Name.c_str(),(double)_Value);
	}

} // applyModification //


//---------------------------------------------------
// processLogic :
//
//---------------------------------------------------
void CLogicVariable::processLogic()
{


} // processLogic //


//---------------------------------------------------
// serial :
//
//---------------------------------------------------
/*void CLogicVariable::serial( IStream &f )
{
	f.xmlPush( "VARIABLE");

	f.serial( _Value );
	f.serial( _Name );

	f.xmlPop();

} // serial //*/

void CLogicVariable::write (xmlNodePtr node) const
{
	xmlNodePtr elmPtr = xmlNewChild ( node, NULL, (const xmlChar*)"VARIABLE", NULL);
	xmlSetProp (elmPtr, (const xmlChar*)"Name", (const xmlChar*)_Name.c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"Value", (const xmlChar*)toString(_Value).c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"Verbose", (const xmlChar*)toString(_Verbose).c_str());
}

void CLogicVariable::read (xmlNodePtr node)
{
	xmlCheckNodeName (node, "VARIABLE");

	_Name = getXMLProp (node, "Name");
	_Value = atoiInt64 (getXMLProp (node, "Value").c_str());
	NLMISC::fromString(getXMLProp(node, "Verbose"), _Verbose);
}

//---------------------------------------------------
// CLogicCounter :
//
//---------------------------------------------------
CLogicCounter::CLogicCounter()
{
	_TickCount = 0;
	_Value = 0;
	_Name = "unamed_counter";

	Period.setValue( 10 );
	Period.setName("Period");

	Phase.setValue( 0 );
	Phase.setName("Phase");

	Step.setValue( 1 );
	Step.setName("Step");

	LowLimit.setValue( 0 );
	LowLimit.setName("LowLimit");

	HighLimit.setValue( 100 );
	HighLimit.setName("HighLimit");

	Mode.setValue( STOP_AT_LIMIT );
	Mode.setName("Mode");

	Control.setValue( RUN );
	Control.setName("Control");

} // CLogicCounter //


//---------------------------------------------------
// update :
//
//---------------------------------------------------
void CLogicCounter::update()
{
	if( Control.getValue() == STOPPED )
	{
		return;
	}

	_TickCount++;
	if( _TickCount < Period.getValue() )
	{
		return;
	}
	else
	{
		_TickCount = 0;
	}

	switch( Control.getValue() )
	{
		case RUN :
		{
			_Value += Step.getValue();
			manageRunningMode();
		}
		break;

		case REWIND :
		{
			_Value = LowLimit.getValue();
			Control.setValue( RUN );
		}
		break;

		case FAST_FORWARD :
		{
			_Value = HighLimit.getValue();
			Control.setValue( RUN );
		}
		break;

	}

	if( _Verbose )
	{
		nlinfo("variable \"%s\" value is now %f",_Name.c_str(),(double)_Value);
	}

} // update //


//---------------------------------------------------
// manageRunningMode :
//
//---------------------------------------------------
void CLogicCounter::manageRunningMode()
{
	// loop on one value
	if( HighLimit.getValue() == LowLimit.getValue() )
	{
		_Value = HighLimit.getValue();
		return;
	}

	switch( Mode.getValue() )
	{
		case STOP_AT_LIMIT :
		{
			if( _Value > HighLimit.getValue() )
			{
				_Value = HighLimit.getValue();
			}
			if( _Value < LowLimit.getValue() )
			{
				_Value = LowLimit.getValue();
			}
		}
		break;

		case LOOP :
		{
			// value is higher than high limit
			if( _Value > HighLimit.getValue() )
			{
				_Value = LowLimit.getValue() + _Value - HighLimit.getValue() - 1;
			}
			// value is lower than low limit
			else
			{
				if( _Value < LowLimit.getValue() )
				{
					_Value = HighLimit.getValue() - (LowLimit.getValue() -_Value) + 1;
				}
			}
		}
		break;

		case SHUTTLE :
		{
			// value is higher than high limit
			if( _Value > HighLimit.getValue() )
			{
				_Value = HighLimit.getValue() - (_Value - HighLimit.getValue());
				Step.setValue( -Step.getValue() );
			}
			// value is lower than low limit
			else
			{
				if( _Value < LowLimit.getValue() )
				{
					_Value = LowLimit.getValue() + LowLimit.getValue() - _Value;
					Step.setValue( -Step.getValue() );
				}
			}
		}
		break;

		case DOWN_UP :
		{
			// low limit reached, we go up
			if( _Value < LowLimit.getValue() )
			{
				_Value = LowLimit.getValue() + LowLimit.getValue() - _Value;
				Step.setValue( -Step.getValue() );
			}
			else
			{
				// high limit reached we stop
				if( _Value > HighLimit.getValue() )
				{
					_Value = HighLimit.getValue();
				}
			}
		}
		break;

		case UP_DOWN :
		{
			// high limit reached, we go down
			if( _Value > HighLimit.getValue() )
			{
				_Value = HighLimit.getValue() - (_Value - HighLimit.getValue());
				Step.setValue( -Step.getValue() );
			}
			else
			{
				// low limit reached, we stop
				if( _Value < LowLimit.getValue() )
				{
					_Value = LowLimit.getValue();
				}
			}
		}
	}

} // manageRunningMode //


//---------------------------------------------------
// serial :
//
//---------------------------------------------------
/*void CLogicCounter::serial( IStream &f )
{
	f.xmlPush( "COUNTER");

	f.serial( _Value );
	f.serial( _Name );
	f.serial( Period );
	f.serial( Phase );
	f.serial( Step );
	f.serial( LowLimit );
	f.serial( HighLimit );
	f.serial( Mode );

	f.xmlPop();

} // serial //*/

void CLogicCounter::write (xmlNodePtr node) const
{
	xmlNodePtr elmPtr = xmlNewChild ( node, NULL, (const xmlChar*)"COUNTER", NULL);
	xmlSetProp (elmPtr, (const xmlChar*)"Name", (const xmlChar*)_Name.c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"Value", (const xmlChar*)toString(_Value).c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"Verbose", (const xmlChar*)toString(_Verbose).c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"Period", (const xmlChar*)toString(Period.getValue()).c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"Phase", (const xmlChar*)toString(Phase.getValue()).c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"Step", (const xmlChar*)toString(Step.getValue()).c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"LowLimit", (const xmlChar*)toString(LowLimit.getValue()).c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"HighLimit", (const xmlChar*)toString(HighLimit.getValue()).c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"Mode", (const xmlChar*)toString(Mode.getValue()).c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"Control", (const xmlChar*)toString(Control.getValue()).c_str());
}

void CLogicCounter::read (xmlNodePtr node)
{
	xmlCheckNodeName (node, "COUNTER");

	_Name = getXMLProp (node, "Name");
	_Value = atoiInt64 (getXMLProp (node, "Value").c_str());
	NLMISC::fromString(getXMLProp (node, "Verbose"), _Verbose);
	Period.setValue(atoiInt64(getXMLProp (node, "Period").c_str()));
	Phase.setValue(atoiInt64(getXMLProp (node, "Phase").c_str()));
	Step.setValue(atoiInt64(getXMLProp (node, "Step").c_str()));
	LowLimit.setValue(atoiInt64(getXMLProp (node, "LowLimit").c_str()));
	HighLimit.setValue(atoiInt64(getXMLProp (node, "HighLimit").c_str()));
	Mode.setValue(atoiInt64(getXMLProp (node, "Mode").c_str()));
	Control.setValue(atoiInt64(getXMLProp (node, "Control").c_str()));
}


}
