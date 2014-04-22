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

#ifndef R2_ACTION_HISTORIC_H
#define R2_ACTION_HISTORIC_H

#include "nel/misc/historic.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/ucstring.h"
//
#include "game_share/object.h"
#include "game_share/scenario.h"

namespace R2
{

class IDynamicMapClient;

// An undo / redo historic
class CActionHistoric
{
public:
	CActionHistoric();
	~CActionHistoric();
	void setDMC(IDynamicMapClient *dmc) { _DMC = dmc; }
	/** Signal the beginning of a new action. Calling this twice in a row will not create
	  * an empty action.
	  */
	void newSingleAction(const ucstring &name);
	/** Begin a new action composed of several subactions. 'numSubActions' calls to 'newAction' will be expected
	  * before the merge of the subactions is pushed
	  */
	void newMultiAction(const ucstring &name, uint actionCount);
	/** Begin a new action composed of several subactions. 'numSubActions' calls to 'newAction' will be expected
	  * before the merge of the subactions is pushed
	  */
	void newPendingMultiAction(const ucstring &name, uint actionCount);
	/** Begin a new 'pending' action
	  * Unlike 'newAction', requests of the actions are sent at each call
	  * to the 'request' commands, not when the action is finished.
	  * Use this if you can't issue an action in a single row (may happen
	  * if some requests in the action depends on notifications sent by previous requests)
	  */
	void newPendingAction(const ucstring &name);

	// force to end a multi action, event if the action count hasnt been reached
	void forceEndMultiAction();

	// End last action / pending action
	void endAction();
	// cancel any action, including multi action
	void cancelAction();
	// If there's a pending action for this frame, flush its content
	void flushPendingAction();
	// Test if a pending action has been begun
	bool isPendingActionInProgress() const { return _NewActionIsPending; }
	// get number of completed actions (do not include the new action being built)
	uint getNumActions() const { return _Actions.getSize(); }
	bool isNewActionBeingRecorded() const { return _NewAction != NULL; }
	uint getMaxNumActions() const { return _Actions.getMaxSize(); }
	void setMaxNumActions(uint count);
	// clear
	void clear(CObject *newScenario = NULL);
	// get name of next action that can be redone
	const ucstring *getNextActionName() const;
	// get name of previous action that can be undone
	const ucstring *getPreviousActionName() const;
	// Test if an action can be undone
	bool canUndo() const;
	// Test if an action can be redo
	bool canRedo() const;
	// undo last action, if possible (return true on success)
	bool undo();
	// redo last action, if possible (return true on success)
	bool redo();
	// push requests into current action
	void requestInsertNode(const std::string& instanceId, const std::string& name, sint32 position, const std::string& key, CObject* value);
	void requestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value);
	void requestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position);
	void requestMoveNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition);
	//Uundo supported only if 'clear' was called with a scenario pointer
	//Only 'redo' supported else
	bool isUndoSupported() const { return _Scenario.getHighLevel() != NULL; }
private:
	// base for all requests
	class CRequestBase : public NLMISC::CRefCount
	{
	public:
		typedef NLMISC::CSmartPtr<CRequestBase> TSmartPtr;
		virtual ~CRequestBase() {}
		virtual void redo(IDynamicMapClient *dmc, CScenario &scenario) = 0;
		virtual void undo(IDynamicMapClient *dmc, CScenario &scenario) = 0;
	protected:
		// clone an object, and disable its refids objects (these object would send
		// creation notifications otherwise)
	public:
		static CObject *cloneObject(const CObject *src);
	};
	/** An atomic list of requests.
	  * Once redo / undo have been called, no other requests can be pushed into this action,
	  * so calls to 'pushRequest' will assert.
	  */
	class CAction : public NLMISC::CRefCount
	{
	public:
		typedef NLMISC::CSmartPtr<CAction> TSmartPtr;
		CAction(const ucstring &name);
		~CAction();
		void setCompleted() { _Completed = true; }
		void pushRequest(CRequestBase *req);
		// For pending action : flush the content that has already been pushed using 'pushRequest'
		// This should be called before 'setCompleted'
		void flush(IDynamicMapClient *dmc, CScenario &scenario);
		// If content has been flushed, rollback the modifications
		void rollback(IDynamicMapClient *dmc, CScenario &scenario);
		//
		void redo(IDynamicMapClient *dmc, CScenario &scenario);
		void undo(IDynamicMapClient *dmc, CScenario &scenario);
		const ucstring &getName() const { return _Name; }
	private:
		ucstring _Name;
		std::vector<CRequestBase::TSmartPtr> _Requests;
		uint								 _FlushedCount;
		bool _Completed;
		bool _Flushing;
	};
	//////////////
	// REQUESTS //
	//////////////
	class CRequestSetNode : public CRequestBase
	{
	public:
		CRequestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value);
		// from CRequestBase
		virtual void redo(IDynamicMapClient *dmc, CScenario &scenario);
		virtual void undo(IDynamicMapClient *dmc, CScenario &scenario);
	private:
		std::string			_InstanceId;
		std::string			_AttrName;
		CObject::TSmartPtr	_NewValue;
		CObject::TSmartPtr	_OldValue;
	};
	//
	class CRequestEraseNode : public CRequestBase
	{
	public:
		CRequestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position);
		// from CRequestBase
		virtual void redo(IDynamicMapClient *dmc, CScenario &scenario);
		virtual void undo(IDynamicMapClient *dmc, CScenario &scenario);
	private:
		// point of deletion
		std::string			_InstanceId;
		std::string			_AttrName;
		sint32				_Position;
		CObject::TSmartPtr	_OldValue;
		// name in parent
		std::string			_ParentInstanceId;
		std::string			_AttrNameInParent;
		sint32				_PositionInParent;
	};
	//
	class CRequestInsertNode : public CRequestBase
	{
	public:
		CRequestInsertNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& key, CObject* value);
		// from CRequestBase
		virtual void redo(IDynamicMapClient *dmc, CScenario &scenario);
		virtual void undo(IDynamicMapClient *dmc, CScenario &scenario);
	private:
		std::string			_InstanceId;
		std::string			_AttrName;
		sint32				_Position;
		std::string			_Key;
		CObject::TSmartPtr	_Value;
	};
	//
	class CRequestMoveNode : public CRequestBase
	{
	public:
		CRequestMoveNode(const std::string& srcInstanceId,
						const std::string& srcAttrName,
						sint32			   srcPosition,
						const std::string& destInstanceId,
						const std::string& destAttrName,
						sint32 destPosition);
		// from CRequestBase
		virtual void redo(IDynamicMapClient *dmc, CScenario &scenario);
		virtual void undo(IDynamicMapClient *dmc, CScenario &scenario);
	private:
		std::string			_SrcInstanceId;
		std::string			_SrcAttrName;
		sint32				_SrcPosition;
		// name of source in parent for reciprocal move
		std::string			_SrcInstanceIdInParent;
		std::string			_SrcAttrNameInParent;
		sint32				_SrcPositionInParent;
		//
		std::string			_DestInstanceId;
		std::string			_DestAttrName;
		sint32				_DestPosition;
		// name of dest
		std::string			_DestInstanceIdAfterMove;
		std::string			_DestAttrNameAfterMove;
		sint32				_DestPositionAfterMove;
	};
private:
	IDynamicMapClient					  *_DMC;
	NLMISC::CHistoric<CAction::TSmartPtr> _Actions;
	/** Index of current action relative to the end of the stack (-1 for the last action)
	  * Undo applies to this action, redo applies to the next action
	  */
	sint								  _CurrActionIndex;
	CAction::TSmartPtr					  _NewAction;
	ucstring							  _NewActionName;
	uint								  _SubActionCount;
	bool								  _NewActionIsPending;
	// anticipated scenario state,
	CScenario							  _Scenario;
	//
private:
	// return previous action absolute index, or -1 if there's no previous action that can be undone.
	sint getPreviousActionIndex() const;
	// return next action absolute index, or -1 if there's no next action that can be undone.
	sint getNextActionIndex() const;
};

} // R2

#endif
