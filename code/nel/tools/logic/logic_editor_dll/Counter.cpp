// Counter.cpp: implementation of the CCounter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "logic_editor.h"
#include "Counter.h"

#include "nel/logic/logic_variable.h"

#include <string>

using namespace std;
using namespace NLLOGIC;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCounter::CCounter(const CString &name)
{
	m_sName = name;
	m_sMode = "Loop";
	m_sWay = "up";

	m_nUpperLimit = 0;
	m_nLowerLimit = 0;
}

CCounter::~CCounter()
{

}

//-----------------------------------------------------
//	cCounterToCLogicCounter
//
//-----------------------------------------------------
void cCounterToCLogicCounter( CCounter& counter, CLogicCounter& logicCounter )
{
	// counter name
	logicCounter.setName( string((LPCSTR)counter.m_sName) );
	
	// running mode
	if( counter.m_sMode == "Shuttle" )
	{
		logicCounter.Mode.setValue( CLogicCounter::SHUTTLE );
	}
	else
	if( counter.m_sMode == "Loop" )
	{
		logicCounter.Mode.setValue( CLogicCounter::LOOP );
	}
	else
	if( counter.m_sMode == "Stop on arrival" )
	{
		logicCounter.Mode.setValue( CLogicCounter::STOP_AT_LIMIT );
	}
	else
	{
		// default value
		logicCounter.Mode.setValue( CLogicCounter::STOP_AT_LIMIT );
	}
	
	// running state
	logicCounter.Control.setValue( CLogicCounter::RUN );

	/// lower limit for counter
	logicCounter.LowLimit.setValue( counter.m_nLowerLimit );

	/// higher limit for counter
	logicCounter.HighLimit.setValue( counter.m_nUpperLimit );
	
	// TODO : phase, period,...

} // cCounterToCLogicCounter //


//-----------------------------------------------------
//	cLogicCounterToCCounter
//
//-----------------------------------------------------
void cLogicCounterToCCounter( const CLogicCounter& logicCounter, CCounter& counter )
{
	// counter name
	counter.m_sName = CString( logicCounter.getName().c_str() );

	// running mode
	switch( logicCounter.Mode.getValue() )
	{
		case CLogicCounter::SHUTTLE			: counter.m_sMode = "Shuttle";			break;
		case CLogicCounter::LOOP			: counter.m_sMode = "Loop";				break;
		case CLogicCounter::STOP_AT_LIMIT	: counter.m_sMode = "Stop on arrival";	break;
	}

	// lower limit for counter
	counter.m_nLowerLimit = (long)logicCounter.LowLimit.getValue();

	// higher limit for counter
	counter.m_nUpperLimit = (long)logicCounter.HighLimit.getValue();



} // cLogicCounterToCCounter //