
#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"
#include "nel/misc/speaker_listener.h"

#ifndef CHARACTER_SYNCH_H
#define CHARACTER_SYNCH_H

namespace CHARSYNC
{



	class ICharacterSync;

	/** Callback interface to implement and register in the
	 *	entity character synchronizer to receive character update events.
	 */
	class ICharacterSyncCb : public NLMISC::CListener<ICharacterSync>
	{
	public:

		/** Callback called when the name of a character have been changed */
		virtual void onCharacterNameUpdated(uint32 charId, const std::string &oldName, const std::string &newName) =0;

		/** Callback called when a character is deleted/removed */
		virtual void onBeforeCharacterDelete(uint32 charId) =0;

	};


	/** Local interface for the character synchronizer */
	class ICharacterSync 
		:	public NLMISC::CManualSingleton<ICharacterSync>,
			public NLMISC::CSpeaker<ICharacterSyncCb>
	{
	public:
		/** Get the name of a user */
		virtual std::string getUserName(uint32 userId) =0;
		/** Get the name of a character */
		virtual ucstring getCharacterName(uint32 charId) =0;
		/// Try to find a shard id from a name and session id. Return 0 if not found
		virtual uint32 findCharId(const std::string &charName, uint32 homeSessionId) =0;
		/// Try to find a shard id from a name without session id, return 0 if 0 or more than one match.
		virtual uint32 findCharId(const std::string &charName) =0;

	};	

} // namespace ENTITYLOC

#endif // CHARACTER_SYNCH_H
