/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef PARTICLE_SOUND_PAGE_H
#define PARTICLE_SOUND_PAGE_H

#include <nel/misc/types_nl.h>
#include "ui_particle_sound_form.h"

// STL includes

// NeL includes
#include "nel/3d/ps_sound.h"

// Project includes

#include "ps_wrapper.h"
#include "particle_node.h"

namespace NLSOUND
{
class UAudioMixer;
}

namespace NLQT
{

/**
@class CSoundPage
@brief Page for QStackWidget, to edit sounds in a particle system
*/
class CSoundPage: public QWidget
{
	Q_OBJECT

public:
	CSoundPage(QWidget *parent = 0);
	~CSoundPage();

	/// Set the sounds to edit.
	void setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocatedBindable *locatedBindable);

private Q_SLOTS:
	void browse();
	void play();
	void setSpawn(bool state);
	void setMute(bool state);
	void setKeepPitch(bool state);
	void setSoundName(const QString &text);
	void setEmissionPercent(float value);

private:

	/// wrapper to set the gain of sounds
	struct CGainWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSSound *S;
		float get(void) const
		{
			return S->getGain();
		}
		void set(const float &v)
		{
			S->setGain(v);
		}
		scheme_type *getScheme(void) const
		{
			return S->getGainScheme();
		}
		void setScheme(scheme_type *s)
		{
			S->setGainScheme(s);
		}
	} _GainWrapper;

	/// wrapper to set the pitch of sounds
	struct CPitchWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSSound *S;
		float get(void) const
		{
			return S->getPitch();
		}
		void set(const float &v)
		{
			S->setPitch(v);
		}
		scheme_type *getScheme(void) const
		{
			return S->getPitchScheme();
		}
		void setScheme(scheme_type *s)
		{
			S->setPitchScheme(s);
		}
	} _PitchWrapper;

	void updateModifiedFlag()
	{
		if (_Node) _Node->setModified(true);
	}

	/// the sound being edited
	NL3D::CPSSound *_Sound;

	CWorkspaceNode *_Node;

	Ui::CSoundPage _ui;

}; /* class CSoundPage */

} /* namespace NLQT */

#endif // PARTICLE_SOUND_PAGE_H
