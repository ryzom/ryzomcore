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

#include "stdpch.h"
#include "particle_editor.h"

// NeL includes
#include <nel/3d/driver.h>
#include <nel/3d/text_context_user.h>
#include <nel/3d/scene.h>
#include <nel/3d/particle_system_model.h>
#include <nel/3d/particle_system_shape.h>

// Project includes
#include "scheme_manager.h"
#include "modules.h"

namespace NLQT
{

CParticleEditor::CParticleEditor(void)
	: _ActiveNode(0),
	  _State(State::Stopped),
	  _Speed(1.0f),
	  _AutoRepeat(false),
	  _DisplayBBox(false),
	  _DisplayHelpers(false),
	  _AutoUpdateBBox(false),
	  _EmptyBBox(true),
	  _PW(0),
	  _Driver(0),
	  _Scene(0),
	  _FontManager(0),
	  _FontGen(0),
	  _SchemeManager(0)
{
}

CParticleEditor::~CParticleEditor(void)
{
}

void CParticleEditor::init()
{
	_Driver = Modules::objView().getIDriver();
	_Scene = Modules::objView().getCScene();

	NL3D::CTextContextUser *textContext = dynamic_cast<NL3D::CTextContextUser *>(Modules::objView().getTextContext());
	_FontManager = textContext->getTextContext().getFontManager();
	_FontGen = textContext->getTextContext().getFontGenerator();
	NL3D::CParticleSystem::setSerializeIdentifierFlag(true);

	_SchemeManager = new CSchemeManager();
}

void CParticleEditor::release()
{
	stop();
	closeWorkspace();
	delete _SchemeManager;
}

void CParticleEditor::setActiveNode(CWorkspaceNode *node)
{
	if (node == _ActiveNode) return;
	if (node == 0)
		_ActiveNode->getPSModel()->hide();
	_ActiveNode = node;

	bool wasRunning = _State == State::RunningSingle;
	if (wasRunning)
	{
		stop();
	}
	if (wasRunning && _ActiveNode)
	{
		start();
	}
}

NL3D::CParticleSystemModel *CParticleEditor::getModelFromPS(NL3D::CParticleSystem *ps) const
{
	if (!ps) return	0;
	CWorkspaceNode *node = _PW->getNodeFromPS(ps);
	if (!node) return 0;
	return node->getPSModel();
}

void CParticleEditor::loadWorkspace(const std::string &fullPath)
{
	// Add to the path
	std::auto_ptr<CParticleWorkspace> newPW(new CParticleWorkspace);
	newPW->init(fullPath);

	// save empty workspace
	try
	{
		newPW->load();
	}
	catch(NLMISC::EStream &e)
	{
		nlerror(e.what());
		return;
	}

	// try to load each ps
	CWorkspaceNode *firstLoadedNode = 0;
	TPWNodeItr itr = newPW->getNodeList().begin();
	while(itr != newPW->getNodeList().end())
	{
		CWorkspaceNode *node = (*itr);
		try
		{
			node->loadPS();
		}
		catch(NLMISC::EStream &e)
		{
			nlwarning(e.what());
		}
		if (node->isLoaded() && !firstLoadedNode)
			firstLoadedNode = node;
		++itr;
	}
	closeWorkspace();
	_PW = newPW.release();
	setActiveNode(firstLoadedNode);
}

void CParticleEditor::createNewWorkspace(const std::string &fullPath)
{
	CParticleWorkspace *newPW = new CParticleWorkspace;
	newPW->init(fullPath);
	// save empty workspace
	try
	{
		newPW->save();
	}
	catch(NLMISC::EStream &e)
	{
		nlerror(e.what());
	}
	closeWorkspace();
	_PW = newPW;
}

void CParticleEditor::saveWorkspaceStructure()
{
	nlassert(_PW);
	try
	{
		_PW->save();
	}
	catch(NLMISC::EStream &e)
	{
		nlerror(e.what());
	}
}


void CParticleEditor::saveWorkspaceContent()
{
	TPWNodeItr itr = _PW->getNodeList().begin();
	while(itr != _PW->getNodeList().end())
	{
		CWorkspaceNode *node = (*itr);
		if (node->isModified())
			node->savePS();
		node->setModified(false);
		++itr;
	}
}

void CParticleEditor::closeWorkspace()
{
	setActiveNode(0);
	delete _PW;
	_PW = 0;
}

void CParticleEditor::start()
{
	switch(_State)
	{
	case State::Stopped:
		if (_ActiveNode)
		{
			if (checkHasLoop(*_ActiveNode)) return;
			play(*_ActiveNode);
			nlassert(_PlayingNodes.empty());
			_PlayingNodes.push_back(_ActiveNode);
		}
		break;
	case State::RunningSingle:
		// no-op
		return;
		break;
	case State::RunningMultiple:
		stop();
		start();
		break;
	case State::PausedSingle:
		if (_ActiveNode)
		{
			unpause(*_ActiveNode);
		}
		break;
	case State::PausedMultiple:
		for(uint k = 0; k < _PlayingNodes.size(); ++k)
		{
			if (_PlayingNodes[k])
			{
				unpause(*_PlayingNodes[k]);
			}
		}
		stop();
		start();
		break;
	default:
		nlassert(0);
		break;
	}
	_State = State::RunningSingle;
}

void CParticleEditor::startMultiple()
{
	switch(_State)
	{
	case State::Stopped:
	{
		if (!_PW) return;
		nlassert(_PlayingNodes.empty());
		TPWNodeItr itr = _PW->getNodeList().begin();
		while(itr != _PW->getNodeList().end())
		{
			CWorkspaceNode *node = (*itr);
			if (node->isLoaded())
				if (checkHasLoop(*node)) return;
			++itr;
		}

		itr = _PW->getNodeList().begin();
		while(itr != _PW->getNodeList().end())
		{
			CWorkspaceNode *node = (*itr);
			if (node->isLoaded())
			{
				// really start the node only if there's no trigger anim
				if (node->getTriggerAnim().empty())
					play(*node);

				_PlayingNodes.push_back(node);
			}
			++itr;
		}
	}
	break;
	case State::PausedSingle:
	case State::RunningSingle:
		stop();
		startMultiple();
		break;
	case State::RunningMultiple:
		// no-op
		return;
		break;
	case State::PausedMultiple:
		for(uint k = 0; k < _PlayingNodes.size(); ++k)
		{
			if (_PlayingNodes[k])
			{
				unpause(*_PlayingNodes[k]);
			}
		}
		break;
	default:
		nlassert(0);
		break;
	}
	_State = State::RunningMultiple;
}

void CParticleEditor::pause()
{
	switch(_State)
	{
	case State::Stopped:
		// no-op
		return;
	case State::RunningSingle:
		if (_ActiveNode)
		{
			pause(*_ActiveNode);
		}
		_State = State::PausedSingle;
		break;
	case State::RunningMultiple:
		for(uint k = 0; k < _PlayingNodes.size(); ++k)
		{
			if (_PlayingNodes[k])
			{
				pause(*_PlayingNodes[k]);
			}
		}
		_State = State::PausedMultiple;
		break;
	case State::PausedSingle:
	case State::PausedMultiple:
		// no-op
		return;
	default:
		nlassert(0);
		break;
	}
}

void CParticleEditor::stop()
{
	switch(_State)
	{
	case State::Stopped:
		// no-op
		return;
	case State::RunningSingle:
	case State::RunningMultiple:
	case State::PausedSingle:
	case State::PausedMultiple:
		for(uint k = 0; k < _PlayingNodes.size(); ++k)
		{
			if (_PlayingNodes[k])
			{
				stop(*_PlayingNodes[k]);
			}
		}
		_PlayingNodes.clear();
		break;
	default:
		nlassert(0);
		break;
	}
	_State = State::Stopped;
}

void CParticleEditor::update()
{
	if (!_ActiveNode) return;
	if (_PW == 0) return;

	NL3D::CParticleSystem *currPS = _ActiveNode->getPSPointer();

	// compute BBox
	if (_DisplayBBox)
	{
		if (_AutoUpdateBBox)
		{
			NLMISC::CAABBox currBBox;
			currPS->forceComputeBBox(currBBox);
			if (_EmptyBBox)
			{
				_EmptyBBox = false;
				_CurrBBox = currBBox;
			}
			else
			{
				_CurrBBox = NLMISC::CAABBox::computeAABBoxUnion(currBBox, _CurrBBox);
			}
			currPS->setPrecomputedBBox(_CurrBBox);
		}
	}

	// auto repeat feature
	if (_AutoRepeat)
	{
		if (isRunning())
		{
			bool allFXFinished = true;
			bool fxStarted = false;
			TPWNodeItr itr = _PlayingNodes.begin();
			while(itr != _PlayingNodes.end())
			{
				CWorkspaceNode *node = (*itr);
				if (node->isLoaded())
				{
					if (isRunning(node))
					{
						fxStarted = true;
						if (node->getPSPointer()->getSystemDate() <= node->getPSPointer()->evalDuration())
						{
							allFXFinished = false;
							break;
						}
						else
						{
							if (node->getPSPointer()->getCurrNumParticles() != 0)
							{
								allFXFinished = false;
								break;
							}
						}
					}
				}
				++itr;
			}
			if (fxStarted && allFXFinished)
				restartAllFX();
		}
	}

	// draw PSmodel
	TPWNodeItr itr = _PW->getNodeList().begin();
	while(itr != _PW->getNodeList().end())
	{
		CWorkspaceNode *node = (*itr);
		if (node->isLoaded())
		{
			if (node  == _ActiveNode)
				node->getPSModel()->enableDisplayTools(!isRunning(node) || _DisplayHelpers);
			else
				node->getPSModel()->enableDisplayTools(false);

			// hide / show the node
			if (_State == State::RunningMultiple || _State == State::PausedMultiple)
			{
				if (isRunning(node))
					node->getPSModel()->show();
				else
					node->getPSModel()->hide();
			}
			else
			{
				if (node == _ActiveNode)
					node->getPSModel()->show();
				else
					node->getPSModel()->hide();
			}
		}
		++itr;
	}
}

void CParticleEditor::restartAllFX()
{
	if (_State == State::RunningMultiple || _State == State::PausedMultiple)
	{
		for(uint k = 0; k < _PlayingNodes.size(); ++k)
		{
			if (_PlayingNodes[k])
			{
				stop(*_PlayingNodes[k]);
			}
		}
	}
	else
	{
		for(uint k = 0; k < _PlayingNodes.size(); ++k)
		{
			if (_PlayingNodes[k])
			{
				stop(*_PlayingNodes[k]);
				play(*_PlayingNodes[k]);
			}
		}
	}
}

void CParticleEditor::setSpeed(float value)
{
	_Speed = value;

	if (!isRunning()) return;

	TPWNodeItr itr = _PW->getNodeList().begin();
	while(itr != _PW->getNodeList().end())
	{
		CWorkspaceNode *node = (*itr);
		if (node->isLoaded())
			node->getPSModel()->setEllapsedTimeRatio(value);
		++itr;
	}
}

void CParticleEditor::setDisplayBBox(bool enable)
{
	_DisplayBBox = enable;
	NL3D::CParticleSystem::forceDisplayBBox(enable);
}

void CParticleEditor::enableAutoCount(bool enable)
{
	if (!_ActiveNode) return;
	if (enable == _ActiveNode->getPSPointer()->getAutoCountFlag()) return;
	_ActiveNode->getPSPointer()->setAutoCountFlag(enable);
	_ActiveNode->setModified(true);
}


void CParticleEditor::resetAutoCount(CWorkspaceNode *node, bool reset /* = true */)
{
	if (!node) return;
	if (node->getResetAutoCountFlag() == reset) return;
	node->setResetAutoCountFlag(reset);
	node->setModified(true);
}

bool CParticleEditor::isRunning(CWorkspaceNode *node)
{
	nlassert(node);
	return node->isStateMemorized();
}

bool CParticleEditor::checkHasLoop(CWorkspaceNode &node)
{
	nlassert(node.isLoaded());
	if (!node.getPSPointer()->hasLoop()) return false;
	return true;
}

void CParticleEditor::play(CWorkspaceNode &node)
{
	if (isRunning(&node)) return;
	// NB : node must be stopped, no check is done
	nlassert(node.isLoaded());
	// if node not started, start it
	node.memorizeState();
	// enable the system to take the right date from the scene
	node.getPSModel()->enableAutoGetEllapsedTime(true);
	node.getPSPointer()->setSystemDate(0.f);
	node.getPSPointer()->reactivateSound();
	node.getPSModel()->activateEmitters(true);
	if (node.getPSPointer()->getAutoCountFlag())
	{
		if (node.getResetAutoCountFlag())
		{
			// reset particle size arrays
			node.getPSPointer()->matchArraySize();
		}
		resetAutoCount(&node, false);
	}

	// Set speed playback particle system
	node.getPSModel()->setEllapsedTimeRatio(_Speed);
}

void CParticleEditor::unpause(CWorkspaceNode &node)
{
	if (!isRunning(&node)) return;
	nlassert(node.isLoaded());
	node.getPSModel()->enableAutoGetEllapsedTime(true);
}

void CParticleEditor::pause(CWorkspaceNode &node)
{
	if (!isRunning(&node)) return;
	nlassert(node.isLoaded());
	node.getPSModel()->enableAutoGetEllapsedTime(false);
	node.getPSModel()->setEllapsedTime(0.f);
}

void CParticleEditor::stop(CWorkspaceNode &node)
{
	if (!isRunning(&node)) return;
	nlassert(node.isLoaded());
	node.restoreState();
	node.getPSModel()->enableAutoGetEllapsedTime(false);
	node.getPSModel()->setEllapsedTime(0.f);
	node.getPSModel()->activateEmitters(true);
	node.getPSPointer()->stopSound();
}

} /* namespace NLQT */