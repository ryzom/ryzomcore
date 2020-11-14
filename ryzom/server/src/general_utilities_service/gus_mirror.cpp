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

// nel
#include "nel/misc/entity_id.h"
#include "nel/misc/config_file.h"
#include "nel/net/service.h"

// games share
#include "game_share/mirror.h"
#include "game_share/singleton_registry.h"

// local
#include "service_main.h"
#include "gus_module_factory.h"
#include "gus_net.h"
#include "gus_mirror.h"


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace GUS
{

	class CGusMirrorImp 
	:	public CRefCount,
		public CGusMirror,
		public IServiceSingleton
	{
	public:
		static CGusMirrorImp *getInstance();

	private:
		// private constructor
		CGusMirrorImp();

	public:
		void registerModuleCallback(IMirrorModuleCallback *mirrorCallback);

		void unregisterModuleCallback(IMirrorModuleCallback *mirrorCallback);

		CMirroredDataSet *getDataSet(const std::string &dataSetName);

		// Service singleton methods
		void init();
		void release();

	private:
		typedef vector<IMirrorModuleCallback* >	TMirrorModuleCallbackCont;

		// mirror callbacks (static methods)
		static void mirrorUpdateFunc();
		static void cbServiceUp(const std::string &serviceName, NLNET::TServiceId serviceId, void *arg);
		static void cbServiceDown(const std::string &serviceName, NLNET::TServiceId serviceId, void *arg);
		static void cbMirrorIsReady( CMirror *mirror );
		static void cbScanMirrorChanges();

		TMirrorModuleCallbackCont	_ModuleCallbacks;
		CMirror						_Mirror;
	};


	CGusMirrorImp *CGusMirrorImp::getInstance()
	{
		static NLMISC::CSmartPtr<CGusMirrorImp> ptr=NULL;
		if (ptr==NULL)
		{
			ptr= new CGusMirrorImp;
		}
		return ptr;
	}

	CGusMirrorImp::CGusMirrorImp()
	{
	};

	void CGusMirrorImp::registerModuleCallback(IMirrorModuleCallback *mirrorCallback)
	{
		// first, check that the module is not already referenced
	//		if (find(_ModuleCallbacks.begin(), _ModuleCallbacks.end(), CSmartPtr<IMirrorModuleCallback>(mirrorCallback)) != _ModuleCallbacks.end())
		if (find(_ModuleCallbacks.begin(), _ModuleCallbacks.end(), mirrorCallback) != _ModuleCallbacks.end())
		{
			nlwarning("Mirror module : callback already registered");
			return;
		}

		_ModuleCallbacks.push_back(mirrorCallback);
	}

	void CGusMirrorImp::unregisterModuleCallback(IMirrorModuleCallback *mirrorCallback)
	{
		// find the module
	//		TMirrorModuleCallbackCont::iterator it = find(_ModuleCallbacks.begin(), _ModuleCallbacks.end(), CSmartPtr<IMirrorModuleCallback>(mirrorCallback));
		TMirrorModuleCallbackCont::iterator it = find(_ModuleCallbacks.begin(), _ModuleCallbacks.end(), mirrorCallback);
		if (it == _ModuleCallbacks.end())
		{
			nlwarning("Mirror module : callback not registered");
			return;
		}
		_ModuleCallbacks.erase(it);
	}

	void CGusMirrorImp::init()
	{
		// Retrieve configuration file
		CConfigFile &cf = IService::getInstance()->ConfigFile;

		// Check if mirror support is required
		CConfigFile::CVar *useMirror = cf.getVarPtr("UseMirror");
		if (useMirror == NULL || useMirror->asBool() == false)
		{
			//We don't use mirror
			return;
		}

		// Init the mirror system
		vector<string> datasetNames;
		CConfigFile::CVar *var = cf.getVarPtr("GUSMirrorConfig");
		if (var == NULL)
		{
			nlwarning("Missing mirror module configuration var 'GUSMirrorConfig'");
			return;
		}

		for (uint32 i = 0; i<var->size(); ++i)
		{
			CSString line = var->asString(i);
			CSString cmd = line.firstWord(true);
			if (cmd == "dataset")
			{
				CSString dsname = line.firstWord(true);
				nldebug("Mirror module : adding dataset '%s'", dsname.c_str());
				datasetNames.push_back(dsname);
			}
		}

		_Mirror.init( datasetNames, cbMirrorIsReady, mirrorUpdateFunc);
		_Mirror.setServiceUpCallback("*", cbServiceUp, NULL);
		_Mirror.setServiceDownCallback("*", cbServiceDown, NULL);
	}

	void CGusMirrorImp::release()
	{
		_Mirror.release();
	}

	CMirroredDataSet *CGusMirrorImp::getDataSet(const std::string &dataSetName)
	{
		if (_Mirror.isDataSetExist(dataSetName))
			return &(_Mirror.getDataSet(dataSetName));

		return NULL;
	}

	// this method is in fact a tickUpdate()
	void CGusMirrorImp::mirrorUpdateFunc()
	{
		// call the service singleton's tickUpdate()
		CServiceClass::tickUpdate();


		CGusMirrorImp *mm = CGusMirrorImp::getInstance();
		// callback registered modules
		TMirrorModuleCallbackCont::iterator first(mm->_ModuleCallbacks.begin()), last(mm->_ModuleCallbacks.end());
		for (; first != last; ++first)
		{
			IMirrorModuleCallback *mmc = *first;

			mmc->mirrorTickUpdate(mm);
		}
	}

	void CGusMirrorImp::cbServiceUp(const std::string &serviceName, NLNET::TServiceId serviceId, void *arg)
	{
		CGusMirrorImp *mm = CGusMirrorImp::getInstance();
		// callback registered modules
		TMirrorModuleCallbackCont::iterator first(mm->_ModuleCallbacks.begin()), last(mm->_ModuleCallbacks.end());
		for (; first != last; ++first)
		{
			IMirrorModuleCallback *mmc = *first;

			mmc->serviceMirrorUp(mm, serviceName, serviceId);
		}
	}

	void CGusMirrorImp::cbServiceDown(const std::string &serviceName, NLNET::TServiceId serviceId, void *arg)
	{
		CGusMirrorImp *mm = CGusMirrorImp::getInstance();
		// callback registered modules
		TMirrorModuleCallbackCont::iterator first(mm->_ModuleCallbacks.begin()), last(mm->_ModuleCallbacks.end());
		for (; first != last; ++first)
		{
			IMirrorModuleCallback *mmc = *first;

			mmc->serviceMirrorDown(mm, serviceName, serviceId);
		}
	}

	void CGusMirrorImp::cbMirrorIsReady( CMirror *mirror )
	{
		CGusMirrorImp *mm = CGusMirrorImp::getInstance();

//		mm->_State = "mirror ready";

		// Retrieve configuration file
		CConfigFile &cf = IService::getInstance()->ConfigFile;

		CConfigFile::CVar *var = cf.getVarPtr("GUSMirrorConfig");
		if (var == NULL)
		{
			nlwarning("Missing mirror module configuration var '%s'", "GUSMirrorConfig");
//			mm->_State = "mirror ready, bad configuration";
			return;
		}


		CSString currentDataset;
		for (uint32 i=0; i<var->size(); ++i)
		{
			CSString line = var->asString(i);
			CSString cmd = line.firstWord(true);
			if (cmd == "dataset")
			{
				currentDataset = line.firstWord(true);
			}
			else if (cmd == "property")
			{
				CSString propName = line.firstWord(true);
				if (currentDataset.empty())
					nlwarning("Mirror module : property '%s' declared without dataset !", propName.c_str());
				else if (propName.empty())
					nlwarning("Mirror module : empty property name in dataset '%s' !", currentDataset.c_str());
				else if (!mm->_Mirror.isDataSetExist(currentDataset))
					nlwarning("Mirror module : dataSet '%s' not in mirror !", currentDataset.c_str());
				else
				{
					// ok, all sanity check done
					CMirroredDataSet &ds = mm->_Mirror.getDataSet(currentDataset);

					// read the access mode
					CSString access = line.firstWord(true);

					bool readOnly = false;
					bool writeOnly = false;
					bool notify = false;

					if (access.find("r") != string::npos)
						readOnly = true;
					if (access.find("w") != string::npos)
						writeOnly = true;
					if (access.find("n") != string::npos)
						notify = true;

					if (!readOnly && !writeOnly)
					{
						nlwarning("Mirror module : invalid access for property '%s', need at least read ('r') or write ('w')", propName.c_str());
						continue;
					}

					// Flags are for read or write ONLY, if both are specified, then default acces (read and write) should be given
					if (readOnly && writeOnly)
						readOnly = writeOnly = false;

					TPropSubscribingOptions options;

					if (!writeOnly)
						options |= PSOReadOnly;
					if (!readOnly)
						options |= PSOWriteOnly;
					if (notify)
						options |= PSONotifyChanges;

					// read the notification group (if any)
					string notifyGroup = line.firstWord(true);

					nldebug("Mirror module : declaring property '%s' in dataSet '%s' with access '%s'",
						propName.c_str(),
						currentDataset.c_str(),
						access.c_str());
					ds.declareProperty( propName, options, notifyGroup);
				}
			}
		}

		mm->_Mirror.setNotificationCallback( cbScanMirrorChanges );

		// callback registered modules
		TMirrorModuleCallbackCont::iterator first(mm->_ModuleCallbacks.begin()), last(mm->_ModuleCallbacks.end());
		for (; first != last; ++first)
		{
			IMirrorModuleCallback *mmc = *first;

			mmc->mirrorIsReady(mm);
		}

	}

	void CGusMirrorImp::cbScanMirrorChanges()
	{
		CGusMirrorImp *mm = CGusMirrorImp::getInstance();

		// for each dataSet
		TNDataSets::iterator first(mm->_Mirror.dsBegin()), last(mm->_Mirror.dsEnd());
		for (; first != last; ++first)
		{
			CMirroredDataSet *ds = first->second;
			// scan additions
			ds->beginAddedEntities();
			TDataSetRow entityIndex = ds->getNextAddedEntity();
			while ( entityIndex != LAST_CHANGED )
			{
				TMirrorModuleCallbackCont::iterator first(mm->_ModuleCallbacks.begin()), last(mm->_ModuleCallbacks.end());
				for (; first != last; ++first)
				{
					IMirrorModuleCallback *mmc = *first;

					mmc->entityAdded(mm, ds, entityIndex);
				}

				entityIndex = ds->getNextAddedEntity();
			}
			ds->endAddedEntities();

			// scan removal
			ds->beginRemovedEntities();
			CEntityId	*entityId = NULL;
			entityIndex = ds->getNextRemovedEntity(&entityId);
			while (entityIndex != LAST_CHANGED)
			{
				TMirrorModuleCallbackCont::iterator first(mm->_ModuleCallbacks.begin()), last(mm->_ModuleCallbacks.end());
				for (; first != last; ++first)
				{
					IMirrorModuleCallback *mmc = *first;

					mmc->entityRemoved(mm, ds, entityIndex, entityId);
				}

				entityIndex = ds->getNextRemovedEntity(&entityId);
			}
			ds->endRemovedEntities();

			// scan notification
			ds->beginChangedValues();
			TPropertyIndex	propIndex;
			ds->getNextChangedValue(entityIndex, propIndex);
			while (entityIndex != LAST_CHANGED)
			{
				TMirrorModuleCallbackCont::iterator first(mm->_ModuleCallbacks.begin()), last(mm->_ModuleCallbacks.end());
				for (; first != last; ++first)
				{
					IMirrorModuleCallback *mmc = *first;

					mmc->propertyChanged(mm, ds, entityIndex, propIndex);
				}

				ds->getNextChangedValue(entityIndex, propIndex);
			}
			ds->endChangedValues();
		}
	}



	CGusMirror::IMirrorModuleCallback::IMirrorModuleCallback()
	{
	}

	CGusMirror::IMirrorModuleCallback::~IMirrorModuleCallback()
	{
		CGusMirror::getInstance()->unregisterModuleCallback(this);
	}


	CGusMirror *CGusMirror::getInstance()
	{
		return (CGusMirrorImp*)CGusMirrorImp::getInstance();
	}

	//-----------------------------------------------------------------------------
	// CGusMirrorImp instantiator
	//-----------------------------------------------------------------------------

	class CGusMirrorImpInstantiator
	{
	public:
		CGusMirrorImpInstantiator()
		{
			CGusMirrorImp::getInstance();
		}
	};
	static CGusMirrorImpInstantiator GusMirrorInstantiator;
	
} // namespace GUS
