// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_MIXING_TRACK_H
#define NL_MIXING_TRACK_H

#include "nel/misc/types_nl.h"
#include "driver/sound_driver.h"
#include "driver/source.h"


namespace NLSOUND {


class CSourceCommon;


/**
 * A source selected for playing
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CTrack
{
public:
	/// Constructor
	CTrack() : m_LogicalSource(NULL), m_PhysicalSource(NULL) { }
	/// Init
	inline void init(ISoundDriver *soundDriver) { m_PhysicalSource = soundDriver->createSource(); }
	/// Destructor
	virtual ~CTrack() { /* nlassert(m_LogicalSource != NULL); [TODO KAETEMI: Try this.] */ if (m_PhysicalSource != NULL) delete m_PhysicalSource; m_PhysicalSource = NULL; }
	
	/// Return if the track succeeded to create a physical source.
	inline bool						hasPhysicalSource() const { return m_PhysicalSource != NULL; }
	/// Return the physical source. Asserts when NULL.
	inline ISource *getPhysicalSource() { nlassert(m_PhysicalSource != NULL); return m_PhysicalSource; }
	
	/// Return availability for playback
	/// FIXME: SWAPTEST [TODO: KAETEMI: Figure out what FIXME: SWAPTEST means.]
	// bool isAvailable() const { return (_SimpleSource==NULL); }
	bool isAvailable() const { nlassert(m_PhysicalSource != NULL); return (m_LogicalSource == NULL) && m_PhysicalSource->isStopped(); }
	/// Returns true if the track is physically playing (different from getUserSource()->isPlaying())
	bool isPlaying() const { nlassert(m_PhysicalSource != NULL); return m_PhysicalSource->isPlaying(); }

	/// Set logical source (if NULL, the track becomes available)
	void setLogicalSource(CSourceCommon *logicalSource) { m_LogicalSource = logicalSource; }
	/// Return the logical source
	CSourceCommon *getLogicalSource() { return m_LogicalSource; }

private:
	/// The current logical source
	CSourceCommon *m_LogicalSource;

	/// Physical source played by the driver
	ISource *m_PhysicalSource;
};


} // NLSOUND


#endif // NL_MIXING_TRACK_H

/* End of mixing_track.h */
