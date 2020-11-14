/** \file impulse_decoder.cpp
 * <File description>
 *
 * $Id: impulse_decoder.cpp,v 1.1 2005/07/11 15:22:33 cado Exp $
 */



//#include "stdpch.h"

#include "impulse_decoder.h"

#include "game_share/action_factory.h"

using namespace std;
using namespace NLMISC;
using namespace CLFECOMMON;

/*
 * Constructor
 */
CImpulseDecoder::CImpulseDecoder()
{
	reset();
}


void	CImpulseDecoder::decode(CBitMemStream &inbox, TPacketNumber receivedPacket, TPacketNumber receivedAck, TPacketNumber nextSentPacket, vector<CLFECOMMON::CAction *> &actions)
{
	uint				level;

	for (level=0; level<3; ++level)
	{
		TPacketNumber	*lAck;
		uint			channel;

		switch (level)
		{
		case 0: lAck = _LastAck0; channel = 0; break;
		case 1: lAck = _LastAck1; channel = receivedPacket&1; break;
		case 2: lAck = _LastAck2; channel = receivedPacket&3; break;
		}

		bool			keep = true;
		bool			checkOnce = false;
		uint			num = 0;

		TPacketNumber	lastAck = lAck[channel];

		while (true)
		{
			bool	next;
			inbox.serial(next);

			if (!next)
				break;

			if (!checkOnce)
			{
				checkOnce = true;
				keep = (receivedAck >= lAck[channel]);
				if (keep)
					lAck[channel] = nextSentPacket;
			}

			++num;
			CAction	*action = CActionFactory::getInstance()->unpack(inbox, false);

			if (keep)
			{
				actions.push_back(action);
				nlinfo("CLIMPD: received new impulsion %d (len=%u) at level %d (channel %d)", action->Code, CActionFactory::getInstance()->size(action), level, channel, num, (keep) ? "" : " (discarded)", lastAck, nextSentPacket);
			}
			else
			{
				nlinfo("CLIMPD: discarded action %d (len=%u) at level %d (channel %d)", action->Code, CActionFactory::getInstance()->size(action), level, channel, num, (keep) ? "" : " (discarded)", lastAck, nextSentPacket);
				CActionFactory::getInstance()->remove(action);
			}
		}

		if (checkOnce)
			nldebug("CLIMPD: at level %d (channel %d), %d actions%s (ReceivedAck=%d/lastAck=%d/nextSentPacket=%d)", level, channel, num, (keep) ? "" : " (discarded)", receivedAck, lastAck, nextSentPacket);
	}
}

void	CImpulseDecoder::reset()
{
	uint	i;
	for (i=0; i<1; ++i)		_LastAck0[i] = -1;
	for (i=0; i<2; ++i)		_LastAck1[i] = -1;
	for (i=0; i<4; ++i)		_LastAck2[i] = -1;
}
