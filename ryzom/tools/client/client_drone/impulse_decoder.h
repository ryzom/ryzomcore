/** \file impulse_decoder.h
 * <File description>
 *
 * $Id: impulse_decoder.h,v 1.1 2005/07/11 15:22:33 cado Exp $
 */



#ifndef NL_IMPULSE_DECODER_H
#define NL_IMPULSE_DECODER_H

#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/bit_mem_stream.h"

#include "game_share/action.h"
#include "game_share/entity_types.h"


/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CImpulseDecoder
{
private:
	CLFECOMMON::TPacketNumber	_LastAck0[1];
	CLFECOMMON::TPacketNumber	_LastAck1[2];
	CLFECOMMON::TPacketNumber	_LastAck2[4];

public:

	/// Constructor
	CImpulseDecoder();


	///
	void		decode(NLMISC::CBitMemStream &inbox, CLFECOMMON::TPacketNumber receivedPacket, CLFECOMMON::TPacketNumber receivedAck, CLFECOMMON::TPacketNumber nextSentPacket, std::vector<CLFECOMMON::CAction *> &actions);

	///
	void		reset();
};


#endif // NL_IMPULSE_DECODER_H

/* End of impulse_decoder.h */
