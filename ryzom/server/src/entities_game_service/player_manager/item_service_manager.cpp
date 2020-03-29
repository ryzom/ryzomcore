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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"
#include "item_service_manager.h"

#include "nel/misc/hierarchical_timer.h"
#include "egs_sheets/egs_sheets.h"
#include "character.h"
#include "death_penalties.h"

#ifdef NL_DEBUG
// uncomment this if you want item service manager to be verbose
# define ITEM_SERVICE_DEBUG
#endif // NL_DEBUG

//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace NLMISC;
using namespace std;

//-----------------------------------------------------------------------------
// vars
//-----------------------------------------------------------------------------

CItemServiceManager * CItemServiceManager::_Instance = NULL;


/**
 * Base class for persistent item service providers (ISP)
 * WARNING: this class should never modify client->getPersistentItemServices() vector (only CItemServiceManager should do)
 * \author Sebastien Guignot
 * \author Nevrax France
 * \date 2005
 */
class IPersistentISP
{
public:
	/// dtor
	virtual ~IPersistentISP() {}

	/// set service data to work with, it MUST be called before you call any other method here
	void setServiceData(const IItemServiceData * serviceData) { _ServiceData = serviceData; }

	/// get service data
	const IItemServiceData * getServiceData() const { return _ServiceData; }

	/// returns true if this service replaces the given item service
	virtual bool replacesService(const CStaticItem * form) const =0;

	// here you can add new methods to manage dependencies between persistent services

private:
	const IItemServiceData *	_ServiceData;
};

/**
 * Base class for item service providers
 * WARNING: this class should never modify client->getPersistentItemServices() vector (only CItemServiceManager should do)
 * \author Sebastien Guignot
 * \author Nevrax France
 * \date 2005
 */
class IItemServiceProvider
{
public:
	/// dtor
	virtual ~IItemServiceProvider();

	/// set service data to work with, it MUST be called before you call any other method here
	void setServiceData(const IItemServiceData * serviceData);

	/// get service data
	const IItemServiceData * getServiceData() const { return _ServiceData; }

	/// returns persistent module, returns NULL if the service is not persistent
	IPersistentISP * persistent() { return _PersistentModule; }

	/// returns true if the service is available for the given client
	virtual bool serviceIsAvailable(CCharacter * client) =0;

	/// provide the service to the given client
	virtual void provideService(CCharacter * client) =0;

	/// compute the price of the service from a base price (from the sheet) for the given client
	/// by default it returns the base price
	virtual uint32 getServicePrice(uint32 basePrice, CCharacter * client) { return basePrice; }

	/// remove the service from the given client if possible, does nothing by default
	virtual void removeService(CCharacter * client) {}

protected:
	IItemServiceProvider(ITEM_SERVICE_TYPE::TItemServiceType serviceType, IPersistentISP * persistentModule = NULL);

private:
	const IItemServiceData *	_ServiceData;
	IPersistentISP *			_PersistentModule;
};

//-----------------------------------------------------------------------------
// methods IItemServiceProvider
//-----------------------------------------------------------------------------

IItemServiceProvider::IItemServiceProvider(ITEM_SERVICE_TYPE::TItemServiceType serviceType, IPersistentISP * persistentModule)
{
	nlassert(serviceType != ITEM_SERVICE_TYPE::Unknown);
	CItemServiceManager::getInstance()->registerServiceProvider(serviceType, this);
	_PersistentModule = persistentModule;
}

//-----------------------------------------------------------------------------

IItemServiceProvider::~IItemServiceProvider()
{
	if (_PersistentModule)
		delete _PersistentModule;
}

//-----------------------------------------------------------------------------

void IItemServiceProvider::setServiceData(const IItemServiceData * serviceData)
{
	_ServiceData = serviceData;
	if (_PersistentModule)
		_PersistentModule->setServiceData(serviceData);
}

//-----------------------------------------------------------------------------
// methods CItemServiceManager
//-----------------------------------------------------------------------------

CItemServiceManager::CItemServiceManager()
{
	_ServiceProviders.resize(ITEM_SERVICE_TYPE::NbItemServiceType, NULL);
}

//-----------------------------------------------------------------------------

void CItemServiceManager::registerServiceProvider(ITEM_SERVICE_TYPE::TItemServiceType serviceType, IItemServiceProvider * serviceProvider)
{
	nlassert(uint(serviceType) < _ServiceProviders.size());
	nlassert(_ServiceProviders[serviceType] == NULL);

	_ServiceProviders[serviceType] = serviceProvider;
}

//-----------------------------------------------------------------------------

bool CItemServiceManager::serviceIsAvailable(const CStaticItem * form, CCharacter * client)
{
	H_AUTO(CItemServiceManager_serviceIsAvailable);

	nlassert(form);
	nlassert(uint(form->ItemServiceType) < _ServiceProviders.size());
	nlassert(client);

	IItemServiceProvider * provider = _ServiceProviders[form->ItemServiceType];
	if (!provider)
		return false;

	// initialize provider with our service data
	provider->setServiceData(form->ItemServiceData);

	return provider->serviceIsAvailable(client);
}

//-----------------------------------------------------------------------------

uint32 CItemServiceManager::getServicePrice(const CStaticItem * form, CCharacter * client)
{
	H_AUTO(CItemServiceManager_getServicePrice);

	nlassert(form);
	nlassert(uint(form->ItemServiceType) < _ServiceProviders.size());
	nlassert(client);

	IItemServiceProvider * provider = _ServiceProviders[form->ItemServiceType];
	if (!provider)
		return form->ItemPrice;

	// initialize provider with our service data
	provider->setServiceData(form->ItemServiceData);

	return provider->getServicePrice(form->ItemPrice, client);
}

//-----------------------------------------------------------------------------

bool CItemServiceManager::provideService(const CStaticItem * form, CCharacter * client)
{
	H_AUTO(CItemServiceManager_provideService);

	nlassert(form);
	nlassert(uint(form->ItemServiceType) < _ServiceProviders.size());
	nlassert(client);

	IItemServiceProvider * provider = _ServiceProviders[form->ItemServiceType];
	if (!provider)
		return false;

	// initialize provider with our service data
	provider->setServiceData(form->ItemServiceData);

	if (!provider->serviceIsAvailable(client))
		return false;

	provider->provideService(client);

#ifdef ITEM_SERVICE_DEBUG
	nldebug("ITEM_SERVICE: item service '%s' of type '%s' has been provided to player '%s' [%s]",
		form->SheetId.toString().c_str(),
		ITEM_SERVICE_TYPE::toString(form->ItemServiceType).c_str(),
		client->getName().toString().c_str(),
		(provider->persistent() ? "persistent" : "non persistent")
		);
#endif // ITEM_SERVICE_DEBUG

	if (provider->persistent())
	{
		vector<CSheetId> & persistentServices = client->getPersistentItemServices();
		uint i = 0;
		while (i < persistentServices.size())
		{
			if (persistentServices[i] == form->SheetId)
				return true;

			const CStaticItem * otherForm = CSheets::getForm(persistentServices[i]);
			if (!otherForm)
				continue;

			// remove all services that are replaced by the new service
			if (provider->persistent()->replacesService(otherForm))
			{
				persistentServices[i] = persistentServices.back();
				persistentServices.pop_back();

#ifdef ITEM_SERVICE_DEBUG
				nldebug("ITEM_SERVICE: persistent item service '%s' of type '%s' has been removed [replaced by latest provided service]",
					otherForm->SheetId.toString().c_str(),
					ITEM_SERVICE_TYPE::toString(otherForm->ItemServiceType).c_str()
					);
#endif // ITEM_SERVICE_DEBUG
			}
			else
			{
				i++;
			}
		}

		// add the new persistent service on the client
		persistentServices.push_back(form->SheetId);
	}

	return true;
}

//-----------------------------------------------------------------------------

void CItemServiceManager::initPersistentServices(CCharacter * client)
{
	H_AUTO(CItemServiceManager_initPersistentServices);

	nlassert(client);

	for (uint i = 0; i < client->getPersistentItemServices().size(); i++)
	{
		const CSheetId & sheetId = client->getPersistentItemServices()[i];
		const CStaticItem * form = CSheets::getForm(sheetId);
		if (form)
		{
			if (form->Family == ITEMFAMILY::SERVICE)
			{
				if (!provideService(form, client))
				{
					nlwarning("Player %s: persistent item service '%s' is not available anymore",
						client->getId().toString().c_str(), sheetId.toString().c_str());
					DEBUG_STOP;
				}
			}
			else
			{
				nlwarning("Player %s: item '%s' is not a service", client->getId().toString().c_str(), sheetId.toString().c_str());
				DEBUG_STOP;
			}
		}
		else
		{
			nlwarning("Player %s: unknown persistent item service '%s'", client->getId().toString().c_str(), sheetId.toString().c_str());
			DEBUG_STOP;
		}
	}
}

//-----------------------------------------------------------------------------

const CStaticItem * CItemServiceManager::removePersistentService(ITEM_SERVICE_TYPE::TItemServiceType serviceType, CCharacter * client)
{
	H_AUTO(CItemServiceManager_removePersistentService);

	nlassert(uint(serviceType) < _ServiceProviders.size());
	nlassert(client);

	IItemServiceProvider * provider = _ServiceProviders[serviceType];
	if (!provider)
		return NULL;

	vector<CSheetId> & persistentServices = client->getPersistentItemServices();
	uint i = 0;
	while (i < persistentServices.size())
	{
		const CStaticItem * form = CSheets::getForm(persistentServices[i]);
		if (form && form->ItemServiceType == serviceType)
		{
			// initialize provider with our service data
			provider->setServiceData(form->ItemServiceData);

			// remove the service
			provider->removeService(client);

			persistentServices[i] = persistentServices.back();
			persistentServices.pop_back();

#ifdef ITEM_SERVICE_DEBUG
			nldebug("ITEM_SERVICE: persistent item service '%s' of type '%s' has been removed from player %s [explicit request]",
				form->SheetId.toString().c_str(),
				ITEM_SERVICE_TYPE::toString(form->ItemServiceType).c_str(),
				client->getId().toString().c_str()
				);
#endif // ITEM_SERVICE_DEBUG

			return form;
		}
		else
		{
			i++;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// provider CISPFeedAnimal
//-----------------------------------------------------------------------------

class CISPFeedAnimal : public IItemServiceProvider
{
public:
	CISPFeedAnimal(ITEM_SERVICE_TYPE::TItemServiceType serviceType, uint32 animalClientPos)
		: IItemServiceProvider(serviceType), _AnimalClientPos(animalClientPos)
	{
		nlassert(animalClientPos >= 1 && animalClientPos <= MAX_INVENTORY_ANIMAL);
	}

	bool serviceIsAvailable(CCharacter * client);
	void provideService(CCharacter * client);

private:
	/// get animal index on the server for the given client, return false if animal is not present
	bool getAnimalIndex(CCharacter * client, uint32 & animalIndex) const;

private:
	/// animal position in the pet interface (from 1 to MAX_INVENTORY_ANIMAL)
	const uint32 _AnimalClientPos;
};

//-----------------------------------------------------------------------------

bool CISPFeedAnimal::serviceIsAvailable(CCharacter * client)
{
	uint32 animalIndex;
	if (!getAnimalIndex(client, animalIndex))
		return false;

	const CPetAnimal & petAnimal = client->getPlayerPets()[animalIndex];

	if (client->isAnimalInStable(animalIndex, client->getCurrentStable()) && petAnimal.Satiety < petAnimal.MaxSatiety)
		return true;

	return false;
}

//-----------------------------------------------------------------------------

void CISPFeedAnimal::provideService(CCharacter * client)
{
	uint32 animalIndex;
	if (getAnimalIndex(client, animalIndex))
	{
		client->setAnimalSatietyToMax(animalIndex);
	}
}

//-----------------------------------------------------------------------------

bool CISPFeedAnimal::getAnimalIndex(CCharacter * client, uint32 & animalIndex) const
{
	animalIndex = _AnimalClientPos - 1;
	if (client->getPlayerPets()[animalIndex].AnimalStatus != CPetAnimal::not_present)
		return true;

	return false;

/*	uint32 currentPos = 0;
	for (uint32 i = 0; i < MAX_INVENTORY_ANIMAL; i++)
	{
		const CPetAnimal & petAnimal = client->getPlayerPets()[i];

		if (petAnimal.AnimalStatus != CPetAnimal::not_present)
			currentPos++;

		if (currentPos == _AnimalClientPos)
		{
			animalIndex = i;
			return true;
		}
	}

	return false;
*/
}

//-----------------------------------------------------------------------------
// provider CISPFeedAllAnimals
//-----------------------------------------------------------------------------

class CISPFeedAllAnimals : public IItemServiceProvider
{
public:
	CISPFeedAllAnimals() : IItemServiceProvider(ITEM_SERVICE_TYPE::StableFeedAllAnimals) {}

	bool serviceIsAvailable(CCharacter * client);
	void provideService(CCharacter * client);
};

//-----------------------------------------------------------------------------

bool CISPFeedAllAnimals::serviceIsAvailable(CCharacter * client)
{
	uint count = 0;
	for (uint i = 0; i < MAX_INVENTORY_ANIMAL; i++)
	{
		const CPetAnimal & petAnimal = client->getPlayerPets()[i];

		if (client->isAnimalInStable(i, client->getCurrentStable()) && petAnimal.Satiety < petAnimal.MaxSatiety)
		{
			count++;
			if (count >= 2)
				return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------

void CISPFeedAllAnimals::provideService(CCharacter * client)
{
	for (uint i = 0; i < MAX_INVENTORY_ANIMAL; i++)
	{
		if (client->isAnimalInStable(i, client->getCurrentStable()))
			client->setAnimalSatietyToMax(i);
	}
}

//-----------------------------------------------------------------------------
// provider CISPSpeedUpDPLoss
//-----------------------------------------------------------------------------

class CISPSpeedUpDPLoss : public IItemServiceProvider
{
	class CPersistent : public IPersistentISP
	{
	public:
		bool replacesService(const CStaticItem * form) const;
	};

public:
	CISPSpeedUpDPLoss() : IItemServiceProvider(ITEM_SERVICE_TYPE::SpeedUpDPLoss, new CPersistent) {}

	bool serviceIsAvailable(CCharacter * client);
	uint32 getServicePrice(uint32 basePrice, CCharacter * client);
	void provideService(CCharacter * client);
	void removeService(CCharacter * client);
};

//-----------------------------------------------------------------------------

bool CISPSpeedUpDPLoss::CPersistent::replacesService(const CStaticItem * form) const
{
	if (form->ItemServiceType == ITEM_SERVICE_TYPE::SpeedUpDPLoss)
	{
		const CSpeedUpDPLossData * serviceData = dynamic_cast<const CSpeedUpDPLossData *>(getServiceData());
		nlassert(serviceData);

		const CSpeedUpDPLossData * otherServiceData = dynamic_cast<const CSpeedUpDPLossData *>(form->ItemServiceData);
		nlassert(otherServiceData);

		if (serviceData->DurationInDays < otherServiceData->DurationInDays)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------

bool CISPSpeedUpDPLoss::serviceIsAvailable(CCharacter * client)
{
	const CSpeedUpDPLossData * serviceData = dynamic_cast<const CSpeedUpDPLossData *>(getServiceData());
	nlassert(serviceData);

	if (client->getDeathPenalties().isNull())
		return false;

	if (client->getDPLossDuration() <= serviceData->DurationInDays)
		return false;

	return true;
}

//-----------------------------------------------------------------------------

uint32 CISPSpeedUpDPLoss::getServicePrice(uint32 basePrice, CCharacter * client)
{
	sint32 clientLevel = client->getSkillBaseValue(client->getBestSkill());
	if (clientLevel < 1)
		clientLevel = 1;

	return basePrice * clientLevel;
}

//-----------------------------------------------------------------------------

void CISPSpeedUpDPLoss::provideService(CCharacter * client)
{
	const CSpeedUpDPLossData * serviceData = dynamic_cast<const CSpeedUpDPLossData *>(getServiceData());
	nlassert(serviceData);

	client->setDPLossDuration(serviceData->DurationInDays);
}

//-----------------------------------------------------------------------------

void CISPSpeedUpDPLoss::removeService(CCharacter * client)
{
	client->setDPLossDuration(0.f);
}

//-----------------------------------------------------------------------------
// register providers
//-----------------------------------------------------------------------------

static CISPFeedAnimal *		ISPFeedAnimal1		= new CISPFeedAnimal(ITEM_SERVICE_TYPE::StableFeedAnimal1, 1);
static CISPFeedAnimal *		ISPFeedAnimal2		= new CISPFeedAnimal(ITEM_SERVICE_TYPE::StableFeedAnimal2, 2);
static CISPFeedAnimal *		ISPFeedAnimal3		= new CISPFeedAnimal(ITEM_SERVICE_TYPE::StableFeedAnimal3, 3);
static CISPFeedAnimal *		ISPFeedAnimal4		= new CISPFeedAnimal(ITEM_SERVICE_TYPE::StableFeedAnimal4, 4);
static CISPFeedAllAnimals *	ISPFeedAllAnimals	= new CISPFeedAllAnimals;
static CISPSpeedUpDPLoss *	ISPSpeedUpDPLoss	= new CISPSpeedUpDPLoss;
