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

#ifndef _BACKUP_SERVICE_INTERFACE_SINGLETON_H
#define	_BACKUP_SERVICE_INTERFACE_SINGLETON_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "singleton_registry.h"
#include "backup_service_interface.h"
#include "_backup_service_interface_implementation.h"


//-------------------------------------------------------------------------------------------------
// class CBackupInterfaceSingleton
//-------------------------------------------------------------------------------------------------

class CBackupInterfaceSingleton: public IServiceSingleton, public NLMISC::IVariableChangedCallback
{
	NL_INSTANCE_COUNTER_DECL(CBackupInterfaceSingleton);
public:
	// singleton interface
	static CBackupInterfaceSingleton* getInstance();

public:
	// the instantiated class interface

	// specialisation of IServiceSingleton
	void init();
	void serviceUpdate();

	// methods for registering callbacks associated with given requests
	uint32		pushFileCallback(NLMISC::CSmartPtr<IBackupFileReceiveCallback>& callback, CBackupServiceInterface* itf );
	uint32		pushFileClassCallback(NLMISC::CSmartPtr<IBackupFileClassReceiveCallback>& callback, CBackupServiceInterface* itf);
	uint32		pushGenericAckCallback(NLMISC::CSmartPtr<IBackupGenericAckCallback>& callback, CBackupServiceInterface* itf);

	// methods for getting the callback associated with a request and removing it from the associated callbacks container
	NLMISC::CSmartPtr<IBackupFileReceiveCallback>		popFileCallback(uint32 requestId, CBackupServiceInterface*& itf);
	NLMISC::CSmartPtr<IBackupFileClassReceiveCallback>	popFileClassCallback(uint32 requestId, CBackupServiceInterface*& itf);
	NLMISC::CSmartPtr<IBackupGenericAckCallback>		popGenericCallback(uint32 requestId, CBackupServiceInterface*& itf);

	// methods for checking callback remaining
	bool		fileCallbackDone(uint32 requestId);
	bool		fileClassCallbackDone(uint32 requestId);
	bool		genericCallbackDone(uint32 requestId);

	// routine used to react to changes in key configuration variables
	void		onVariableChanged( NLMISC::IVariable& var );

	// setup a callback to be invoked on connection of backup system (modules or services)
	void		pushBSConnectCallback(IBackupServiceConnection*);

	// register connecteion or deconnection of backup system (called at first connection and last disconnection)
	void		connect();
	void		disconnect();
	bool		isConnected() const;

	// accessors for the currently active BSI implmentation
	IBackupServiceInterfaceImplementation* getBSIImplementation();
	void setBSIImplementation(IBackupServiceInterfaceImplementation* bsii);


private:
	// prohibit construction outside own scope
	CBackupInterfaceSingleton();

public:

	// maps of request ids to callbacks for different types of action
	template <class T>
	struct TBSCallbackInfo
	{
		NLMISC::CSmartPtr<T>		Callback;
		CBackupServiceInterface*	Interface;
		uint32						RequestTime; // allows to measure the response time
	};
	std::map<uint32, TBSCallbackInfo<IBackupFileReceiveCallback> >		_FileResponses;
	std::map<uint32, TBSCallbackInfo<IBackupFileClassReceiveCallback> >	_FileClassResponses;
	std::list<std::pair<uint32, TBSCallbackInfo<IBackupGenericAckCallback> > >	_GenericResponses;

	// container of callbacks for backup system connection events (for first service or module up and last service or module down)
	std::vector<IBackupServiceConnection*> _BSConnectCallbacks;

	// a handy global to track state of the backup system
	bool _IsConnected;

	// the current active IBackupServiceInterfaceImplementation object (initially NULL)
	IBackupServiceInterfaceImplementation* _BackupServiceInterfaceImplementation;

	// a counter used to allocate unique ids to requests
	uint32 _Counter;

	// the backup service interfaces used for shard dependent and global stuff - these are accessible via Bsi and BsiGlobal macros
	// th e_PDBsi is used by the pd lib for its logs
	CBackupServiceInterface	_ShardDependentBsi;
	CBackupServiceInterface	_GlobalBsi;
//	CBackupServiceInterface	_PDBsi;
};


//-------------------------------------------------------------------------------------------------
#endif
