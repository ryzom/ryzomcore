// State.h: interface for the CState class.
// 2001 Fleury David, Nevrax
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STATE_H__8501C0A2_320A_42B9_BD7E_3D77F27301ED__INCLUDED_)
#define AFX_STATE_H__8501C0A2_320A_42B9_BD7E_3D77F27301ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <afxtempl.h>


namespace NLLOGIC
{
class CLogicState;
}

/**
 * class CEvent
 */
class CEvent
{
public:
	CEvent()
	{
		m_bActionIsMessage = FALSE;
	}

	virtual ~CEvent()
	{
	}

	const CString &getEventAsString() const;

//attributes:
public:
	CString		m_sConditionName;

	/// bool, true if the event action is sending a message, false if it's changing state
	bool		m_bActionIsMessage;

	/// \name event message
	//@{
	///destination of the message (name of a .bot, .flaura, .var... file)
	CString		m_sMessageDestination; 
	/// message ID (selected from valid ID for given destination (file extension))
	CString		m_sMessageID;
	/// message arguments, syntax defined for given message ID
	CString		m_sArguments;
	//@}

	/// \name state change event
	//@{
	/// destination state (if m_bActionIsMessage == false)
	CString		m_sStateChange;
	//@]

	/// temp string used to store the event as a string when requested
	mutable CString eventString;

	friend bool operator==( const CEvent &ev1, const CEvent &ev2);
};


typedef CList< CEvent *, CEvent *&> TPEventList;





/**
 * class State
 */
class CState  
{
public:
	/// constructor
	CState( const CString &name = CString("") );

	/// copy constructor
	CState( const CState &state );

	virtual ~CState();

	/// add an event to this State
	inline void addEvent( CEvent *event) { m_evEvents.AddTail( event ) ; }

	/// remove the specified event from the state object, return TRUE if done, FALSE if event not found
	BOOL removeEvent( CEvent *event);

	
// attributes
//private:
public:
	CString		m_sName;

	/// list of pointers on CEvent objects
	TPEventList	m_evEvents;
};


/**
 * Set a CLogicState from a CState 
 */
void cStateToCLogicState( CState& state, NLLOGIC::CLogicState& logicState );

/**
 * Set a CState from a CLogicState 
 */
void cLogicStateToCState( const NLLOGIC::CLogicState& logicState, CState& state );


#endif // !defined(AFX_STATE_H__8501C0A2_320A_42B9_BD7E_3D77F27301ED__INCLUDED_)
