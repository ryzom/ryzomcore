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

#ifndef PARTICLE_EDITOR_H
#define PARTICLE_EDITOR_H

#include <nel/misc/types_nl.h>

// STL includes
#include <map>
#include <string>
#include <vector>

// NeL includes
#include <nel/misc/smart_ptr.h>
#include <nel/3d/animation_time.h>

// Projects includes
#include "particle_node.h"

namespace NL3D
{
class CParticleSystem;
class CParticleSystemModel;
class CShapeBank;
class CScene;
class IDriver;
class CFontManager;
class CFontGenerator;
}

namespace NLQT
{
class CSchemeManager;

/**
@class CParticleEditor
@brief The main class of the particles editor.
@details - Provides access to a container containing all of the particles systems (getParticleWorkspace())
and also allows you to create an empty container (createNewWorkspace()) load / save from the file
and unload (loadWorkspace(), saveWorkspaceStructure(), saveWorkspaceContent())
- Has basic operations management system to control a particles systems (start(), stop(), pause(), setSpeed() / setAutoRepeat())
as in single mode, and in the multiple (start(), startMultiple()).
- Additional functions of particle systems: display / hide the assistanse elements (setDisplayHelpers()),
the calculation of bounding box (setAutoBBox(), getAutoBBox()), switching on the autoCount mode (setAutoBBox(), getAutoBBox()).
- Selection of the current system of particles for making various operations (setActiveNode(), getActiveNode()).
*/
class CParticleEditor
{
public:
	struct State
	{
		enum List
		{
			Stopped = 1,
			RunningSingle,
			RunningMultiple,
			PausedSingle,
			PausedMultiple
		};
	};
	CParticleEditor(void);
	~CParticleEditor(void);

	void init();

	void release();

	/// Active a new node of the workspace
	/// Current active node is ready for edition.
	/// Its bbox is displayed.
	void setActiveNode(CWorkspaceNode *node);

	/// Get the node of the workspace that is currently active
	CWorkspaceNode *getActiveNode() const
	{
		return _ActiveNode;
	}

	/// Get the particle system model that is currently active
	NL3D::CParticleSystemModel *getActivePSM() const
	{
		return _ActiveNode ? _ActiveNode->getPSModel() : NULL;
	}

	/// Get a model from a ps pointer. The ps must belong to the workspace
	NL3D::CParticleSystemModel *getModelFromPS(NL3D::CParticleSystem *ps) const;

	/// Load a new particle workspace (without asking if current workspace has been modified)
	void loadWorkspace(const std::string &fullPath);

	void createNewWorkspace(const std::string &fullPath);

	/// Save the workspace structure
	void saveWorkspaceStructure();

	/// Save the workspace content
	void saveWorkspaceContent();

	/// Cloase the workspace without saving or asking the user
	void closeWorkspace();

	/// Force the active system to start
	void start();

	/// Start all systems
	void startMultiple();

	/// Pause the playing systems
	void pause();

	/// Force the playing systems to stop
	void stop();

	/// Update display of number of particles
	void update();

	/// Sets the animation speed particles system
	void setSpeed(float value);

	void setDisplayBBox(bool enable);

	void setDisplayHelpers(bool enable)
	{
		_DisplayHelpers = enable;
	}

	void setAutoRepeat(bool enable)
	{
		_AutoRepeat = enable;
	}

	/// Auto bbox for fx
	void  setAutoBBox(bool enable)
	{
		_AutoUpdateBBox = enable;
	}

	bool  getAutoBBox() const
	{
		return _AutoUpdateBBox;
	}

	/// Enable / disbale auto-count
	void enableAutoCount(bool enable);

	/// Reset the autocount the next time the system will be started
	void resetAutoCount(CWorkspaceNode *node, bool reset = true);

	/// Reset the auto compute bbox
	void  resetAutoBBox()
	{
		_EmptyBBox = true;
	}

	/// Get current state
	int getState() const
	{
		return _State;
	}

	/// Return true if one or several system are being played
	bool isRunning() const
	{
		return _State == State::RunningSingle || _State == State::RunningMultiple;
	}

	/// Return true if a system is paused.
	/// Must call only if running
	bool isPaused() const
	{
		return _State == State::PausedSingle || _State == State::PausedMultiple;
	}

	CParticleWorkspace *getParticleWorkspace() const
	{
		return _PW;
	}

	NL3D::IDriver *getDriver() const
	{
		return _Driver;
	}

	NL3D::CScene *getScene() const
	{
		return _Scene;
	}

	/// Get the fontManager
	NL3D::CFontManager *getFontManager() const
	{
		return _FontManager;
	}

	/// Get the fontGenerator
	NL3D::CFontGenerator *getFontGenerator () const
	{
		return _FontGen;
	}

	CSchemeManager *getSchemeManager () const
	{
		return _SchemeManager;
	}

private:
	// Check if a node is inserted in the running list (it may be paused)
	bool isRunning(CWorkspaceNode *node);

	// Check is a node has loops
	bool checkHasLoop(CWorkspaceNode &node);

	void play(CWorkspaceNode &node);
	void unpause(CWorkspaceNode &node);
	void pause(CWorkspaceNode &node);
	void stop(CWorkspaceNode &node);
	void restartAllFX();

	// Currently active node of the workspace
	CWorkspaceNode	*_ActiveNode;

	// List of fxs that are currently playing
	TNodeVect _PlayingNodes;

	// Current state
	int	_State;
	float _Speed;
	bool _AutoRepeat;
	bool _DisplayBBox;
	bool _DisplayHelpers;

	// The system bbox must be updated automatically
	bool _AutoUpdateBBox;

	// The last computed bbox for the system
	bool _EmptyBBox;
	NLMISC::CAABBox _CurrBBox;


	CParticleWorkspace *_PW;
	NL3D::IDriver *_Driver;
	NL3D::CScene *_Scene;

	// Font manager
	NL3D::CFontManager *_FontManager;

	// Font generator
	NL3D::CFontGenerator *_FontGen;

	CSchemeManager *_SchemeManager;

}; /* class CParticleEditor */

} /* namespace NLQT */

#endif // PARTICLE_EDITOR_H
