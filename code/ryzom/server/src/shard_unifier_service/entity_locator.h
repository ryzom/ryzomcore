
#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"
#include "nel/misc/speaker_listener.h"

#ifndef ENTITY_LOCATOR_H
#define	ENTITY_LOCATOR_H

namespace ENTITYLOC
{
	class IEntityLocator;

	/** Callback interface to implement and register in the
	 *	entity locator to receive character connect/disconnect events.
	 */
	class ICharacterEventCb : public NLMISC::CListener<IEntityLocator>
	{
	public:
		virtual void onUserConnection(NLNET::IModuleProxy *locatorHost, uint32 userId) =0;
		virtual void onUserDisconnection(NLNET::IModuleProxy *locatorHost, uint32 userId) =0;

		virtual void onCharacterConnection(NLNET::IModuleProxy *locatorHost, uint32 charId, uint32 lastDisconnectionDate) =0;
		virtual void onCharacterDisconnection(NLNET::IModuleProxy *locatorHost, uint32 charId) =0;

	};


	/** Local interface for the entity locator */
	class IEntityLocator 
		:	public NLMISC::CManualSingleton<IEntityLocator>,
			public NLMISC::CSpeaker<ICharacterEventCb>
	{
	public:

		/// Check if a user is online somewhere
		virtual bool isUserOnline(uint32 userId) = 0;
		
		/** Return the proxy on the locator module that claims to host the character
		 *	Return NULL if the character is not currently online.
		 */
		virtual NLNET::IModuleProxy *getLocatorModuleForChar(uint32 charId) =0;

		/** Return the proxy on the locator module that claims to host the character
		 *	Return NULL if the character is not currently online.
		 */
		virtual NLNET::IModuleProxy *getLocatorModuleForChar(const ucstring &charName) =0;

		/** Return the shard Id of the shard hosting the character (or 0 if not online)
		 */
		virtual uint32 getShardIdForChar(const ucstring &charName) =0;

		/** Return the module for a given shard id
		 *	return NULL if no module available for the specified shard id
		*/
		virtual NLNET::IModuleProxy *getLocatorModuleForShard(uint32 shardId)=0;

	};	

} // namespace ENTITYLOC

#endif // ENTITY_LOCATOR_H
