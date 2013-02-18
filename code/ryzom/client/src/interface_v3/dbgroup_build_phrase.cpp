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



#include "stdpch.h"
#include "dbgroup_build_phrase.h"
#include "sbrick_manager.h"
#include "sphrase_manager.h"
#include "interface_manager.h"
#include "dbctrl_sheet.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/group_editbox.h"
#include "../client_cfg.h"
#include "nel/gui/view_text.h"
#include "skill_manager.h"
#include "../string_manager_client.h"


using namespace std;
using namespace NLMISC;


// ***************************************************************************
const std::string	CDBGroupBuildPhrase::BrickSelectionModal= "ui:interface:build_phrase_select_brick";
const std::string	CDBGroupBuildPhrase::BrickSelectionDB= "UI:PHRASE:SELECT";
const std::string	CDBGroupBuildPhrase::BrickBuildDB= "UI:PHRASE:BUILD";


// ***************************************************************************

NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupBuildPhrase, std::string, "build_phrase");

CDBGroupBuildPhrase::CDBGroupBuildPhrase(const TCtorParam &param)
:CInterfaceGroup(param)
{
	_GroupValid= false;
	_ValidateButton= NULL;
	_NumMandatories= 0;
	_NumOptionals= 0;
	_NumCredits= 0;

	_Setuped= false;
	_TextureIdSlotDisabled= 0;

	// Name of the magic sentence
	_UserSentenceName= NULL;
	_SpellView= NULL;

	_NewSpellNumber= 0;

	_TextPhraseDesc= NULL;

	nlctassert(MaxRootBrickTypeFilter>0);
	for(uint i=0;i<MaxRootBrickTypeFilter;i++)
	{
		_RootBrickTypeFilter[i]= BRICK_TYPE::UNKNOWN;
	}
}

// ***************************************************************************
CDBGroupBuildPhrase::~CDBGroupBuildPhrase()
{
}


// ***************************************************************************
// ***************************************************************************
// Widget Setup
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool		CDBGroupBuildPhrase::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	if(!CInterfaceGroup::parse(cur, parentGroup))
		return false;

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// Init the disabled texture id
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	_TextureIdSlotDisabled= rVR.getTextureIdFromName ("w_slot_brick_disabled.tga");

	// Create now (before sons ctrl sheet parsing) the variables
	uint i;
	// Bricks and their Params
	for(i=0;i<MaxBricks;i++)
	{
		NLGUI::CDBManager::getInstance()->getDbProp(BrickBuildDB + ":MAIN:" + toString(i)+":SHEET");
		for(uint j=0;j<MaxParam;j++)
			NLGUI::CDBManager::getInstance()->getDbProp(BrickBuildDB + ":PARAM:" + toString(i) + ":" + toString(j) + ":SHEET");
	}

	// spellView: to update the icon, use a special phrase manager entry
	NLGUI::CDBManager::getInstance()->getDbProp(BrickBuildDB + ":EDITION_PHRASE:PHRASE")->setValue32(CSPhraseManager::EditionSlot);

	return true;
}

// ***************************************************************************
void		CDBGroupBuildPhrase::updateCoords ()
{
	if(!_Setuped)
		setupBuildSentence();

	CInterfaceGroup::updateCoords();
}

// ***************************************************************************
void		CDBGroupBuildPhrase::setupBuildSentence()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	_Setuped= true;

	// Get the widget controls
	_ValidateButton= dynamic_cast<CCtrlBaseButton*>(CInterfaceGroup::getCtrl("ok_cancel:ok"));
	if(_ValidateButton)
		_ValidateButton->setFrozen(true);

	_TextPhraseDesc= dynamic_cast<CViewText*>(CInterfaceGroup::getView("infos:phrase_desc"));

	// retrieved brick view/ctrls
	string	idCtrl, idBack, idCost, idCredit, idInfo;
	CDBCtrlSheet	*ctrl;
	CViewBitmap		*back;
	CViewText		*cost;
	CViewText		*credit;
	CViewText		*info;


	// **** get all bricks (including root)
	sint	index= 0;
	for(;;)
	{
		// retrieve
		idCtrl= string("bricks:main_brick")+toString(index);
		idBack= string("bricks:main_back")+toString(index);
		idCost= string("bricks:main_cost")+toString(index);
		idCredit= string("bricks:main_credit")+toString(index);
		idInfo= string("bricks:main_info")+toString(index);
		ctrl= dynamic_cast<CDBCtrlSheet*>(CInterfaceGroup::getCtrl( idCtrl ));
		back= dynamic_cast<CViewBitmap*>(CInterfaceGroup::getView( idBack ));
		cost= dynamic_cast<CViewText*>(CInterfaceGroup::getView( idCost ));
		credit= dynamic_cast<CViewText*>(CInterfaceGroup::getView( idCredit ));
		info= dynamic_cast<CViewText*>(CInterfaceGroup::getView( idInfo ));
		// A brick Slot is valid only if all Ctrl/View are ok, same for its param.
		bool	ok= true;
		CWord	newWord;
		if(ctrl && back && cost && info)
		{
			newWord.Slot.Brick= ctrl;
			newWord.Slot.Back= back;
			newWord.CostView= cost;
			newWord.CreditView= credit;
			newWord.InfoView= info;
			// Must have all params
			for(uint i=0;i<MaxParam;i++)
			{
				idCtrl= string("bricks:param_brick") + toString(index) + "_" + toString(i);
				idBack= string("bricks:param_back") + toString(index) + "_" + toString(i);
				ctrl= dynamic_cast<CDBCtrlSheet*>(CInterfaceGroup::getCtrl( idCtrl ));
				back= dynamic_cast<CViewBitmap*>(CInterfaceGroup::getView( idBack ));
				if(ctrl && back)
				{
					newWord.ParamSlot[i].Brick= ctrl;
					newWord.ParamSlot[i].Back= back;
				}
				else
				{
					// fails to find all params!
					ok= false;
					break;
				}
			}
		}
		else
			ok= false;
		// ok with this ctrl?
		if(ok)
		{
			// unactive alls.
			newWord.reset();
			// append
			_MainWords.push_back(newWord);
			// next
			index++;
		}
		else
		{
			// stop to search
			break;
		}
	}

	// If not at least find the root, error!
	if(index==0)
	{
		nlwarning("ERROR: RootBrick not found!");
		_GroupValid= false;
	}
	else
	{
		_GroupValid= true;
		// Active at least the root brick.
		_MainWords[0].setBrick(0);
	}

	// Setup special for Root
	if(_GroupValid)
	{
		sint	rootTextMaxw;
		fromString(CWidgetManager::getInstance()->getParser()->getDefine("phrase_build_root_info_maxw"), rootTextMaxw);
		_MainWords[0].InfoView->setLineMaxW(rootTextMaxw);
	}

	// Get the sentence texts
	_UserSentenceName= dynamic_cast<CGroupEditBox*>(CInterfaceGroup::getGroup("eb_spell_name:eb"));

	// Get the SpellView
	_SpellView= dynamic_cast<CDBCtrlSheet*>(CInterfaceGroup::getCtrl( "spell_view" ));
}


// ***************************************************************************
void		CDBGroupBuildPhrase::draw ()
{
	CInterfaceGroup::draw();
}


// ***************************************************************************
const CSBrickSheet	*CDBGroupBuildPhrase::getRootBrick()
{
	if(!_GroupValid)
		return NULL;

	return _MainWords[0].Slot.Brick->asSBrickSheet();
}


// ***************************************************************************
// ***************************************************************************
// Setup operations
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CDBGroupBuildPhrase::clearBuildingPhrase()
{
	if(!_GroupValid)
		return;

	// update display
	_MainWords[0].reset();
	_MainWords[0].setBrick(0);
	updateDisplayFromRootBrick();

	// Empty the name
	if(_UserSentenceName)
	{
		_UserSentenceName->setInputString(ucstring());
	}

	// update Display
	updateAllDisplay();
}


// ***************************************************************************
void			CDBGroupBuildPhrase::startComposition(const CSPhraseCom &phrase)
{
	if(!_GroupValid)
		return;
	CSBrickManager		*pBM= CSBrickManager::getInstance();


	ucstring	name;

	// if phrase empty (new phrase), invent a new name
	if(phrase.empty())
	{
		// build a default name
		name= CI18N::get("uimPhraseNew");
		// Append a dummy number
		_NewSpellNumber++;
		name+= " " + toString(_NewSpellNumber);
	}
	else
	{
		// copy name
		name= phrase.Name;

		// get the root Brick. Must exist.
		CSBrickSheet	*rootBrick= pBM->getBrick(phrase.Bricks[0]);
		if(rootBrick)
		{
			// Phrase to Ctrls: simulate clicks!
			uint	curBrickIndex= 1;
			uint	brickIndexForParam= 0;
			uint	curParam= 0;
			uint	numParam= 0;

			// simulate a root selection
			validateRoot(rootBrick);
			// setup params of the root
			numParam= (uint)rootBrick->ParameterFamilies.size();
			brickIndexForParam= 0;

			// For all brick not the root
			for(uint i=1;i<phrase.Bricks.size();i++)
			{
				CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);

				/* NB: the ORDER is important for grammar checkup
					Must come first Root / Mandatory, then credit or optionals.
					Each params must follow its related brick
					Parameters May also have their own parameters
					This is recursive, param sons may have params sons.
				*/

				// a param of brick?
				if(curParam<numParam)
				{
					validateParam(brickIndexForParam, curParam, brick);
					curParam++;
					// If this parameter has additonal parameter they follow
					if( brick && !brick->ParameterFamilies.empty() )
						numParam+= (uint)brick->ParameterFamilies.size();
				}
				// a mandatory/optional/credit?
				else
				{
					// must be a mandatory?
					if(curBrickIndex<1+_NumMandatories)
						validateMain(curBrickIndex, brick);
					// create a new optional/credit
					else
						validateNewOpCredit(brick);

					// bkup index for param, and increment
					brickIndexForParam= curBrickIndex;
					curBrickIndex++;
					// following bricks are its param!
					curParam= 0;
					numParam= 0;
					if(brick)
						numParam= (uint)brick->ParameterFamilies.size();
				}
			}
		}
	}

	// set the editable name.
	if(_UserSentenceName)
		_UserSentenceName->setInputString(name);
}


// ***************************************************************************
// ***************************************************************************
// Start Selection of Bricks
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CDBGroupBuildPhrase::fillSelectionRoot()
{
	// the root must be OK.
	if(!_GroupValid)		return;
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	// fillSelection with all root
	std::vector<CSheetId>		bricks;
	bricks= pBM->getRootBricks();
	// get only ones known
	filterKnownBricks(bricks);
	// get only ones that match The BrickType filter
	filterRootBrickType(bricks);
	// some additional root filter
	filterRootPossibles(bricks);
	// fill db
	fillSelection(bricks);
}
// ***************************************************************************
void			CDBGroupBuildPhrase::fillSelectionMain(uint index)
{
	nlassert(index>0);
	// the root must be OK.
	if(!_GroupValid)		return;
	const CSBrickSheet		*rootBrick= getRootBrick();
	if(!rootBrick)		return;
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	// error?
	if(index>=_MainWords.size())
	{
		resetSelection();
		return;
	}

	// get the current Brick.
	std::vector<CSheetId>		bricks;
	const CSBrickSheet	*brick= _MainWords[index].Slot.Brick->asSBrickSheet();
	if(!brick)
	{
		// It is possible for Mandatory that this brick is still not validated
		if(index<1+_NumMandatories)
		{
			// get the related family and bricks associated to it
			bricks= pBM->getFamilyBricks(rootBrick->MandatoryFamilies[index-1]);
		}
		else
		{
			resetSelection();
			return;
		}
	}
	else
	{
		// fill selection with all bricks of the same family (whatever the main brick)
		bricks= pBM->getFamilyBricks(brick->BrickFamily);
	}

	// get only ones known
	filterKnownBricks(bricks);

	// For mandatories, filter effects
	CSPhraseCom		currentPhrase;
	buildCurrentPhrase(currentPhrase);
	pBM->getSabrinaCom().filterMandatoryComposition(currentPhrase.Bricks, bricks);

	// For Combat Optional, must filter by exclusion and combat exclusion
	if(brick && brick->isCombat() && brick->isOptional() )
	{
		// Ensure not same bricks are setuped. Also don't insert me since I am already here...
		filterBrickSetuped(bricks);
		// Ensure only optional of compatible Skill are inserted. Don't test with me, since i may be removed!
		filterSkillSetuped(bricks, true, false, index);
	}
	// for power optional, must filter by brick
	else if(brick && brick->isSpecialPower() && (brick->isOptional()||brick->isMandatory()) )
	{
		// Ensure not same bricks are setuped. Also don't insert me since I am already here...
		filterBrickSetuped(bricks);
	}

	// For optional or credit, filter by BrickExclusion.
	if(index>=1/*+_NumMandatories*/)
		filterBrickExclusion(bricks, index);

	// For Optional/Credits, must append first the special "Remove Brick" choice
	if(index>=1+_NumMandatories)
	{
		bricks.insert(bricks.begin(), pBM->getInterfaceRemoveBrick());
	}

	// fill db
	fillSelection(bricks);
}

// ***************************************************************************
void			CDBGroupBuildPhrase::fillSelectionParam(uint index, uint paramIndex)
{
	// the root must be OK.
	if(!_GroupValid)		return;
	const CSBrickSheet		*rootBrick= getRootBrick();
	if(!rootBrick)		return;
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	// error?
	if( index>=_MainWords.size() || paramIndex>=MaxParam)
	{
		resetSelection();
		return;
	}

	// get the current Brick.
	const CSBrickSheet	*brick;
	brick= _MainWords[index].ParamSlot[paramIndex].Brick->asSBrickSheet();
	if(!brick)
	{
		resetSelection();
		return;
	}
	// fill selection with all bricks of the same family (whatever the root)
	std::vector<CSheetId>		bricks;
	bricks= pBM->getFamilyBricks(brick->BrickFamily);
	// get only ones known
	filterKnownBricks(bricks);
	// fill db
	fillSelection(bricks);
}

// ***************************************************************************
void			CDBGroupBuildPhrase::fillSelectionNewOp()
{
	// the root must be OK.
	if(!_GroupValid)		return;
	const CSBrickSheet		*rootBrick= getRootBrick();
	if(!rootBrick)		return;

	// Build a brick array of possible new bricks.
	std::vector<NLMISC::CSheetId>	bricks;
	fillNewOptionalBricks(bricks);

	// fill db
	fillSelection(bricks);
}


// ***************************************************************************
void			CDBGroupBuildPhrase::fillSelectionNewCredit()
{
	// the root must be OK.
	if(!_GroupValid)		return;
	const CSBrickSheet		*rootBrick= getRootBrick();
	if(!rootBrick)		return;

	// Build a brick array of possible new bricks.
	std::vector<NLMISC::CSheetId>	bricks;
	fillNewCreditBricks(bricks);

	// fill db
	fillSelection(bricks);
}


// ***************************************************************************
// ***************************************************************************
// Validate Selection of Bricks
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CDBGroupBuildPhrase::validateRoot(const CSBrickSheet *sheet)
{
	// the root must be OK.
	if(!_GroupValid)		return;

	// If same brick no op
	if(getRootBrick() == sheet)
		return;

	// reset the whole sentence with default values
	resetSentence(sheet->Id.asInt());

	// update the display
	updateAllDisplay();
}


// ***************************************************************************
void			CDBGroupBuildPhrase::validateMain(uint index, const CSBrickSheet *sheet)
{
	nlassert(index>0);
	// the root must be OK.
	if(!_GroupValid)		return;
	const CSBrickSheet		*rootBrick= getRootBrick();
	if(!rootBrick)		return;
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	// If the brick chosen is actually the "Remove Brick", then call the correct method
	if(sheet->Id == pBM->getInterfaceRemoveBrick())
	{
		deleteOpCredit(index);
		return;
	}

	// set the brick!
	if(sheet)
		_MainWords[index].setBrick(sheet->Id.asInt());

	// must update params if change!
	updateParams(index);

	// update the display
	updateAllDisplay();
}

// ***************************************************************************
void			CDBGroupBuildPhrase::validateParam(uint index, uint paramIndex, const CSBrickSheet *sheet)
{
	// the root must be OK.
	if(!_GroupValid)		return;
	const CSBrickSheet		*rootBrick= getRootBrick();
	if(!rootBrick)			return;
	if(paramIndex>=MaxParam)	return;

	// The setuped parameter may have also parameter sons
	bool	mustResetParamHrc= false;
	vector<uint16>		newParamSons;
	if(sheet)			newParamSons= sheet->ParameterFamilies;
	mustResetParamHrc= newParamSons != _MainWords[index].ParamSlot[paramIndex].ViewParamFamilies;

	// set the brick!
	if(sheet)
	{
		_MainWords[index].setParamBrick(paramIndex, sheet->Id.asInt());
	}

	// update the param sons
	if(mustResetParamHrc)
		updateParamHrc(index);

	// update the display
	updateAllDisplay();
}

// ***************************************************************************
void			CDBGroupBuildPhrase::validateNewOpCredit(const CSBrickSheet *sheet)
{
	// the root must be OK.
	if(!_GroupValid)		return;
	const CSBrickSheet		*rootBrick= getRootBrick();
	if(!rootBrick)		return;

	if(!sheet)
		return;

	// we should not have to do this test...
	if(getNumMainBricks()>=_MainWords.size())
		return;

	// get the dest index.
	uint	index;
	// credit brick?
	if(sheet->isCredit())
		index= 1+_NumMandatories+_NumOptionals+_NumCredits;
	// else optional brick
	else
		index= 1+_NumMandatories+_NumOptionals;

	// Shift Optional Row. Copy the Brick setup from index to index+1.
	for(uint i= getNumMainBricks();i>index;i--)
	{
		_MainWords[i].copySetup(_MainWords[i-1]);
	}

	// set the brick!
	_MainWords[index].setBrick(sheet->Id.asInt());

	// Increment the number of options/credits setuped!
	if(sheet->isCredit())
		_NumCredits++;
	else
		_NumOptionals++;

	// update the NewOp controler
	updateNewButtons();

	// must update params if change!
	updateParams(index);

	// update the display
	updateAllDisplay();
}

// ***************************************************************************
void			CDBGroupBuildPhrase::deleteOpCredit(uint index)
{
	nlassert(index>0);
	// the root must be OK.
	if(!_GroupValid)		return;
	const CSBrickSheet		*rootBrick= getRootBrick();
	if(!rootBrick)		return;

	// check index
	bool	isCredit;
	if(index>=1+_NumMandatories && index<1+_NumMandatories+_NumOptionals)
		isCredit= false;
	else if(index>=1+_NumMandatories+_NumOptionals && index<getNumMainBricks() )
		isCredit= true;
	else
		return;

	// Shift Optional Row. Copy the Brick setup from index+1 to index.
	for(uint i= index+1;i<getNumMainBricks();i++)
	{
		_MainWords[i-1].copySetup(_MainWords[i]);
	}

	// reset the last row display
	_MainWords[getNumMainBricks()-1].reset();

	// an optional/credit is removed!
	if(isCredit)
		_NumCredits--;
	else
		_NumOptionals--;

	// update the NewOp ctrl
	updateNewButtons();

	// update the display
	updateAllDisplay();
}


// ***************************************************************************
// ***************************************************************************
// System
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
static uint			getLowestBit(uint64 val)
{
	uint	ret= 0;
	while(val)
	{
		if(val&1)
			return ret;
		ret++;
		val>>=1;
	}
	return 0;
}


// ***************************************************************************
static void			getDefaultSheetForFamily(uint familyId, sint32 &sheet, bool &valid)
{
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	uint64	knownBF= pBM->getKnownBrickBitField(familyId);
	// Known?
	if(knownBF)
	{
		// default: get the lowest sheet
		sint	posInFamily= getLowestBit(knownBF);

		// get the sheet
		sheet= pBM->getBrickSheet(familyId, posInFamily).asInt();
		valid= true;
	}
	else
	{
		// setup the Sheet with the lowest brick
		sheet= pBM->getBrickSheet(familyId, 0).asInt();
		// invalidate the word
		valid= false;
	}
}


// ***************************************************************************
void			CDBGroupBuildPhrase::updateParams(uint index)
{
	// get the Word
	CWord	*word;
	if(index<_MainWords.size())
		word= &_MainWords[index];
	else
		return;

	// Get the Brick setuped. may be NULL for unsetuped mandatories
	const CSBrickSheet	*brick= word->Slot.Brick->asSBrickSheet();

	// retrieve brick paramFamilies
	vector<uint16>	paramFamilies;
	if(brick)
		paramFamilies= brick->ParameterFamilies;

	// if param families of the new brick different from what setuped in view, reset all param hierachy
	if(word->Slot.ViewParamFamilies != paramFamilies)
	{
		// must reset the view of parameters
		word->resetParams();

		// and compute all from setuped main brick
		updateParamHrc(index);
	}
}


// ***************************************************************************
/*	used for updateParamHrc()
 *
 */
class CParamTreeNode
{
public:
	const CSBrickSheet				*Brick;
	bool							Valid;
	std::vector<uint16>				ViewParamFamilies;
	CParamTreeNode					*Parent;
	std::vector<CParamTreeNode*>	Sons;

public:
	CParamTreeNode(CParamTreeNode *parent) : Brick(NULL), Valid(true), Parent(parent) {}
	~CParamTreeNode()
	{
		Parent= NULL;
		Brick=	NULL;
		deleteSons();
	}

	// delete param sons
	void	deleteSons()
	{
		for(uint i=0;i<Sons.size();i++)	delete Sons[i];
		Sons.clear();
	}

	// rebuild default param sons (recurs) if BrickParamFamilies and ViewParamFamilies differ.
	// return false if error
	bool	synchronizeParams(uint	maxDepth)
	{
		if(maxDepth==0)
			return false;

		// get the brick Parameter Families.
		vector<uint16>	brickParamFamilies;
		if(Brick)
			brickParamFamilies= Brick->ParameterFamilies;
		// compare with the current View one. if equals then OK! just recurs test sons
		if(brickParamFamilies == ViewParamFamilies)
		{
			for(uint i=0;i<Sons.size();i++)
			{
				if(!Sons[i]->synchronizeParams(maxDepth-1))
					return false;
			}
			return true;
		}
		// else must rebuild all this branch
		else
		{
			return buildSonsFromBrick(maxDepth);
		}
	}

	// build the son list from brick Setup, and recurs
	// return false if error
	bool	buildSonsFromBrick(uint maxDepth)
	{
		if(maxDepth==0)
			return false;

		CSBrickManager	*pBM= CSBrickManager::getInstance();

		// first delete my sons, and any old view setup
		deleteSons();
		ViewParamFamilies.clear();

		// then copy brick families to view families (=> view synchronized to brick)
		if(Brick)
			ViewParamFamilies= Brick->ParameterFamilies;

		// Add a default Son for each family
		uint	i;
		for(i=0;i<ViewParamFamilies.size();i++)
		{
			uint	familyId= ViewParamFamilies[i];
			sint32	sheet;
			bool	sonValid;
			// bkup the valid state in this node
			getDefaultSheetForFamily(familyId, sheet, sonValid);

			// init son
			CParamTreeNode	*sonNode= new CParamTreeNode(this);
			sonNode->Brick= pBM->getBrick(CSheetId(sheet));
			sonNode->Valid= sonValid;
			// add this son to its father
			Sons.push_back(sonNode);
		}

		// recurs.
		for(i=0;i<Sons.size();i++)
		{
			if(!Sons[i]->buildSonsFromBrick(maxDepth - 1))
				return false;
		}

		return true;
	}

	// build the word raw param list from hierarchy
	// return false if error
	bool		buildRawParamList(CDBGroupBuildPhrase::CWord &word, uint &rawParamIndex)
	{
		// If this is the root
		if(!Parent)
		{
			// just copy the ViewParamFamilies
			word.Slot.ViewParamFamilies= ViewParamFamilies;
		}
		// else, add this parameter to list
		else
		{
			// Check possible Brick Data error
			if(rawParamIndex >= CDBGroupBuildPhrase::MaxParam)
			{
				nlwarning("BRICK ERROR: Not enough param Space (%d) to add all son parameter", CDBGroupBuildPhrase::MaxParam);
				return false;
			}

			// Setup this brick in the param list
			CSheetId	sheet;
			if(Brick)
				sheet= Brick->Id;
			word.setParamBrick(rawParamIndex, sheet.asInt());
			word.ParamSlot[rawParamIndex].Valid= Valid;
			word.ParamSlot[rawParamIndex].Brick->setGrayed( !Valid );

			// bkup its ViewParameters
			word.ParamSlot[rawParamIndex].ViewParamFamilies= ViewParamFamilies;

			// next param index
			rawParamIndex++;
		}

		// for all sons, recurs
		for(uint i=0;i<Sons.size();i++)
		{
			// recurs
			if(!Sons[i]->buildRawParamList(word, rawParamIndex))
				return false;
		}

		return true;
	}

};

// ***************************************************************************
void			CDBGroupBuildPhrase::updateParamHrc(uint index)
{
	/*
		When we are here, we may have some Parameter Hierarchy inconsitency between the ViewParameterFamilies and
		the Brick parameter Families.
		we must fix them and rebuild all the raw param list
	*/
	// get the Word
	CWord	*word;
	if(index<_MainWords.size())
		word= &_MainWords[index];
	else
		return;

	// If a Param error has already been detected on this Slot, no-op
	if(word->ParamError)
		return;


	// **** From the current View setup of 'word', build the Parameter Hierarchy (in simple tree form)
	CParamTreeNode		rootNode(NULL);
	// NB: here rootNode represent the Main (ie mandatory, optional or credit) brick. Therefore, it is not a real parameter.
	rootNode.Brick= word->Slot.Brick->asSBrickSheet();
	rootNode.Valid= true;
	rootNode.ViewParamFamilies= word->Slot.ViewParamFamilies;
	uint	rawParamIndex= 0;
	CParamTreeNode		*curNode= &rootNode;
	while(curNode)
	{
		// if this node still need sons, then the next raw index param must be a son of curNode
		if(curNode->Sons.size() < curNode->ViewParamFamilies.size())
		{
			nlassert(rawParamIndex < word->NumTotalParams);
			// create a son node, and fill from rawList
			CParamTreeNode	*sonNode= new CParamTreeNode(curNode);
			sonNode->Brick= word->ParamSlot[rawParamIndex].Brick->asSBrickSheet();
			sonNode->Valid= word->ParamSlot[rawParamIndex].Valid;
			sonNode->ViewParamFamilies= word->ParamSlot[rawParamIndex].ViewParamFamilies;
			// append to the sons
			curNode->Sons.push_back(sonNode);
			// next in rawList
			rawParamIndex++;
			// recurs to son
			curNode= sonNode;
		}
		// else the next must be a brother: return to parent
		else
			curNode= curNode->Parent;
	}
	// we must have run all the raw list
	nlassert(rawParamIndex == word->NumTotalParams);
	nlassert(rootNode.Sons.size() == word->Slot.ViewParamFamilies.size());


	// **** Parse the Parameter tree, and invalidate each branch where View families and Brick families differs.
	// Avoid .sbrick data erros: allow only a max recurs level of 10
	if(!rootNode.synchronizeParams(10))
	{
		// in this case, ABORT, but don't crash: setup 0 parameters...
		word->resetParams();
		word->ParamError= true;
		return;
	}


	// **** rebuild completely the parameter list from the parameter tree.
	// clear first
	word->resetParams();
	// then rebuild
	rawParamIndex= 0;
	if(rootNode.buildRawParamList(*word, rawParamIndex))
	{
		// then the new NumTotalParams is....
		word->NumTotalParams= rawParamIndex;
	}
	// ERROR CASE
	else
	{
		// in this case, ABORT, but don't crash: setup 0 parameters...
		word->resetParams();
		word->ParamError= true;
		return;
	}

}


// ***************************************************************************
void			CDBGroupBuildPhrase::updateAllDisplay(const CSPhraseCom &phrase)
{
	// NB: phrase MUST COME FROM buildCurrentPhrase() else doesn't work.

	CSBrickManager		*pBM= CSBrickManager::getInstance();
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CSPhraseManager		*pPM= CSPhraseManager::getInstance();

	// **** update total cost
	// get the cost and credit
	uint32	totalCost, totalCredit;
	pBM->getSabrinaCom().getPhraseCost(phrase.Bricks, totalCost, totalCredit);

	// update database
	NLGUI::CDBManager::getInstance()->getDbProp("UI:PHRASE:BUILD:TOTAL_COST")->setValue32(totalCost);
	NLGUI::CDBManager::getInstance()->getDbProp("UI:PHRASE:BUILD:TOTAL_CREDIT")->setValue32(totalCredit);

	// **** Update the Cost of All Root/Mandat/ops/Credits.
	if(phrase.Bricks.size())
	{
		// Parse the phrase, and setup the related Cost.
		uint	curBrickIndex= 0;
		for(uint i=0;i<phrase.Bricks.size();)
		{
			// Get the brick.
			CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);
			// if not found, skip it (eg important for mandatories not setuped)
			if(!brick)
			{
				i++;
			}
			else
			{
				// get the cost for this brick and its params.
				sint32	cost;
				float relative_cost;
				cost= pBM->getSabrinaCom().getPhraseBrickAndParamCost(phrase.Bricks, i);
				relative_cost = pBM->getSabrinaCom().getPhraseBrickAndParamRelativeCost(phrase.Bricks, i);
				ucstring	costText;
				if( cost == 0 && relative_cost != 0.f )
				{
					cost = (sint32)(relative_cost * 100.f);
					costText= toString("%+d", cost) + string("%");
				}
				else
					costText= toString("%+d", cost);

				// set the MainWord cost
				if(cost>=0)
				{
					_MainWords[curBrickIndex].CostView->setActive(true);
					_MainWords[curBrickIndex].CreditView->setActive(false);
					_MainWords[curBrickIndex].CostView->setText(costText);
				}
				else
				{
					_MainWords[curBrickIndex].CreditView->setActive(true);
					_MainWords[curBrickIndex].CostView->setActive(false);
					_MainWords[curBrickIndex].CreditView->setText(costText);
				}

				// Next brick: skip me and my params
				i+= 1 + _MainWords[curBrickIndex].NumTotalParams;
			}

			// next slot to setup. if all slots setuped, break.
			curBrickIndex++;
			if(curBrickIndex>=getNumMainBricks())
				break;
		}
	}

	// **** Additionaly Update the Info Text
	for(uint i=0;i<1+_NumMandatories;i++)
	{
		CWord	&word= _MainWords[i];
		// If the brick is setuped, hide the info text, else display
		if( word.Slot.Brick->asSBrickSheet() )
			word.InfoView->setActive(false);
		else
		{
			word.InfoView->setActive(true);
			if(i==0)
				word.InfoView->setText( CI18N::get("uiTextHelpSelectRootBrick") );
			else
				// start effect index at 1 (human readable :) )
				word.InfoView->setText( CI18N::get("uiTextHelpSelectEffectBrick") + toString(i) );
		}
	}

	// **** Additionaly Update the New Buttons
	bool	mandatOk= true;
	// If only one of root effect is not setuped...
	for(uint i=0;i<1+_NumMandatories;i++)
	{
		CWord	&word= _MainWords[i];
		if( !word.Slot.Brick->asSBrickSheet() )
			mandatOk= false;
	}
	// set DB value accordeing to it.
	NLGUI::CDBManager::getInstance()->getDbProp("UI:PHRASE:BUILD:ROOT_EFFECT_VALID")->setValue32(mandatOk);

	// update valid button
	if(_ValidateButton)
	{
		bool	active= false;

		// valid only if all mandat active and cost ok
		active= totalCredit>=totalCost && mandatOk;

		// valid only if All bricks exist, and are known
		uint	i;
		for(i=0;i<phrase.Bricks.size();i++)
		{
			// Get the brick.
			CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);
			if(!brick || !pBM->isBrickKnown(brick->Id))
			{
				active= false;
				break;
			}
		}

		// valid only if no parameter error has been encountered
		for(i=0;i<1+_NumMandatories+_NumOptionals+_NumCredits;i++)
		{
			if(_MainWords[i].ParamError)
			{
				active= false;
				break;
			}
		}

		// If OK, still do some check
		if(active)
		{
			const CSBrickSheet	*brick= getRootBrick();
			if(!brick)
				active= false;
			// check req size
			else if( brick->MandatoryFamilies.size()+1>_MainWords.size() )
				active= false;
		}

		_ValidateButton->setFrozen(!active);
	}


	// **** Additionaly Update the Combat Restrict options
	// Get the rootBrick
	const CSBrickSheet	*rootBrick= getRootBrick();
	if(rootBrick && rootBrick->isCombat())
	{
		// show the weapon restriction interface
		NLGUI::CDBManager::getInstance()->getDbProp("UI:PHRASE:BUILD:RESTRICT_COMBAT:ENABLED")->setValue32(1);

		// If not already done, retrieve the weapon skills, and fill the sbricks SHEET
		if(_WeaponSkills.empty())
		{
			// get define, and verify data
			uint	numWeaponSkill;
			fromString(CWidgetManager::getInstance()->getParser()->getDefine("phrase_max_restrict_combat"), numWeaponSkill);
			string	strWeaponSkill= CWidgetManager::getInstance()->getParser()->getDefine("phrase_def_skill_restrict_combat");
			vector<string>	weaponSkillList;
			splitString(strWeaponSkill, " ", weaponSkillList);
			nlassert(weaponSkillList.size()==numWeaponSkill);

			// NOTE TO CODER WHO CHANGE SKILLS::ESkill. If you change combat skills, ask yoyo for modification
			// or search "phrase_def_skill_restrict_combat" and "phrase_max_restrict_combat" in XML.
			nlctassert( SKILLS::SFM1SSM && SKILLS::SFM1SAM && SKILLS::SFM1BMM && SKILLS::SFM1BSM &&
						SKILLS::SFM1PSM && SKILLS::SFM2SSM && SKILLS::SFM2SAM && SKILLS::SFM2BMM &&
						SKILLS::SFM2PPM && SKILLS::SFMCADM && SKILLS::SFMCAHM && SKILLS::SFR1APM &&
						SKILLS::SFR2AAM && SKILLS::SFR2ALM && SKILLS::SFR2ARM);
			nlctassert(SKILLS::SH - SKILLS::SF == 47);

			// backup the skill array, and fill the associated brick in interface
			_WeaponSkills.resize(numWeaponSkill);
			for(uint i=0;i<numWeaponSkill;i++)
			{
				_WeaponSkills[i]= SKILLS::toSkill(weaponSkillList[i]);

				// Get the associated brick
				uint32	viewBrickCombatSheetId= pBM->getVisualBrickForSkill(_WeaponSkills[i]).asInt();

				// And fill in DB
				NLGUI::CDBManager::getInstance()->getDbProp(toString("UI:PHRASE:BUILD:RESTRICT_COMBAT:%d:SHEET", i))->setValue32(viewBrickCombatSheetId);
			}
		}

		// For each weapon skill, test if match or not the current phrase
		for(uint i=0;i<_WeaponSkills.size();i++)
		{
			bool	ok= pPM->skillCompatibleWithCombatPhrase(_WeaponSkills[i], phrase.Bricks);
			NLGUI::CDBManager::getInstance()->getDbProp(toString("UI:PHRASE:BUILD:RESTRICT_COMBAT:%d:LOCKED", i))->setValue32(!ok);
		}
	}
	else
	{
		// hide the weapon restriction interface
		NLGUI::CDBManager::getInstance()->getDbProp("UI:PHRASE:BUILD:RESTRICT_COMBAT:ENABLED")->setValue32(0);
	}

	// **** Setup the phrase Desc
	if(_TextPhraseDesc)
	{
		ucstring	text;
		pPM->buildPhraseDesc(text, phrase, 0, false, "composition");
		_TextPhraseDesc->setTextFormatTaged(text);
	}


	// **** Since some bricks may have changed, update the spell view
	updateSpellView();

}


// ***************************************************************************
void			CDBGroupBuildPhrase::updateAllDisplay()
{
	// build the current phrase
	CSPhraseCom		newPhrase;
	buildCurrentPhrase(newPhrase);
	updateAllDisplay(newPhrase);
}


// ***************************************************************************
void			CDBGroupBuildPhrase::resetSelection()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	for(uint i=0;i<MaxSelection;i++)
	{
		NLGUI::CDBManager::getInstance()->getDbProp(BrickSelectionDB+ ":" + toString(i) + ":SHEET")->setValue32(0);
	}
}

// ***************************************************************************
void			CDBGroupBuildPhrase::fillSelection(const std::vector<CSheetId> &bricks)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	uint	num= min((uint)MaxSelection, (uint)bricks.size());
	for(uint i=0;i<MaxSelection;i++)
	{
		if(i<num)
			NLGUI::CDBManager::getInstance()->getDbProp(BrickSelectionDB+ ":" + toString(i) + ":SHEET")->setValue32(bricks[i].asInt());
		else
			NLGUI::CDBManager::getInstance()->getDbProp(BrickSelectionDB+ ":" + toString(i) + ":SHEET")->setValue32(0);
	}
}

// ***************************************************************************
void			CDBGroupBuildPhrase::filterKnownBricks(std::vector<CSheetId> &bricks)
{
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	pBM->filterKnownBricks(bricks);
}

// ***************************************************************************
void			CDBGroupBuildPhrase::filterBrickExclusion(std::vector<CSheetId> &bricks, uint16 indexToSkip)
{
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	std::vector<CSheetId>	res;
	res.reserve(bricks.size());

	static vector<string>	forbidWords(30);

	// For All optionals/credits setuped, build the Exclude Set
	set<string>		excludeSet;
	// test for all bricks
	for(uint j=1/*+_NumMandatories*/;j<1+_NumMandatories+_NumOptionals+_NumCredits;j++)
	{
		// skip brick at index indexToSkip (if replacing a brick for example, don't consider it's forbiden flags)
		if (j == indexToSkip)
			continue;

		const CSBrickSheet	*brick= _MainWords[j].Slot.Brick->asSBrickSheet();
		if(brick)
		{
			// get all words.
			forbidWords.clear();
			splitString(brick->getForbiddenExclude(), ":", forbidWords);
			// for all words, insert in the set.
			for(uint j=0;j<forbidWords.size();j++)
				excludeSet.insert(forbidWords[j]);
		}
	}

	// keep only unfiltered ones
	for(uint i=0;i<bricks.size();i++)
	{
		const CSBrickSheet	*brick= pBM->getBrick(bricks[i]);
		if(brick)
		{
			// For all define words, search if excluded from the current set of optional setup
			forbidWords.clear();
			splitString(brick->getForbiddenDef(), ":", forbidWords);
			bool	ok= true;
			for(uint j=0;j<forbidWords.size();j++)
			{
				if(excludeSet.find(forbidWords[j]) != excludeSet.end() )
				{
					ok= false;
					break;
				}
			}

			// ok , add it
			if(ok)
				res.push_back(bricks[i]);
		}
	}

	// replace with filtered one
	bricks= res;
}


// ***************************************************************************
void			CDBGroupBuildPhrase::filterFamilySetuped(std::vector<uint16> &families)
{
	std::vector<uint16>	res;
	res.reserve(families.size());

	// keep only unsetuped ones
	for(uint i=0;i<families.size();i++)
	{
		uint	family= families[i];
		bool	ok= true;

		// test for all opionals/Credits
		for(uint j=1+_NumMandatories;j<getNumMainBricks();j++)
		{
			const CSBrickSheet	*brick= _MainWords[j].Slot.Brick->asSBrickSheet();
			if(brick && brick->BrickFamily==(sint)family)
			{
				ok= false;
				break;
			}
		}

		// insert only if not found
		if(ok)
		{
			res.push_back(family);
		}
	}

	// replace with filtered one
	families= res;
}

// ***************************************************************************
void			CDBGroupBuildPhrase::filterBrickSetuped(std::vector<NLMISC::CSheetId> &bricks)
{
	std::vector<CSheetId>	res;
	res.reserve(bricks.size());

	// keep only unsetuped ones
	for(uint i=0;i<bricks.size();i++)
	{
		CSheetId	brick= bricks[i];
		bool	ok= true;

		// test for all bricks
		for(uint j=1/*+_NumMandatories*/;j<getNumMainBricks();j++)
		{
			if(brick == CSheetId(_MainWords[j].Slot.Brick->getSheetId()) )
			{
				ok= false;
				break;
			}
		}

		// insert only if not found
		if(ok)
		{
			res.push_back(brick);
		}
	}

	// replace with filtered one
	bricks= res;
}


// ***************************************************************************
void			CDBGroupBuildPhrase::filterSkillSetuped(std::vector<NLMISC::CSheetId> &bricks, bool checkOptional, bool checkCredit, sint avoidCheckIndex)
{
	// check nothing => return.
	if(!checkOptional && !checkCredit)
		return;

	CSBrickManager	*pBM= CSBrickManager::getInstance();


	// **** build the compatible skill formula for the bricks we want to check
	CReqSkillFormula	testFormula;
	uint	checkStart= checkOptional? 1+_NumMandatories : 1+_NumMandatories+_NumOptionals;
	uint	checkEnd= checkCredit? 1+_NumMandatories+_NumOptionals : getNumMainBricks();

	// test for all optionals and/or Credits
	uint	i;
	for(uint i= checkStart;i<checkEnd;i++)
	{
		// For Replacement of brick, don't test with the brick replaced (suppose will be removed!)
		if(avoidCheckIndex==(sint)i)
			continue;

		const CSBrickSheet	*brick= _MainWords[i].Slot.Brick->asSBrickSheet();
		// If bricks use a skill, and with the formula
		if(brick && brick->getSkill()!=SKILLS::unknown)
		{
			CReqSkillFormula	brickFormula;
			for(uint j=0;j<brick->UsedSkills.size();j++)
			{
				brickFormula.orV(CSkillValue(brick->UsedSkills[j]));
			}

			// and with the phraseFormula
			testFormula.andV(brickFormula);
		}
	}


	// **** check for each brick if compatible with this formula
	std::vector<CSheetId>	res;
	res.reserve(bricks.size());

	// keep only unsetuped ones
	for(i=0;i<bricks.size();i++)
	{
		const CSBrickSheet	*brick= pBM->getBrick(bricks[i]);
		if(brick && brick->getSkill()!=SKILLS::unknown)
		{
			// simulate a choose of this brick, ie AND its skill formula with the current phrase formula
			CReqSkillFormula	brickFormula;
			for(uint j=0;j<brick->UsedSkills.size();j++)
			{
				brickFormula.orV(CSkillValue(brick->UsedSkills[j]));
			}

			CReqSkillFormula	tempFormula= testFormula;
			tempFormula.andV(brickFormula);

			// Nb: the following test works if testFormula is empty(), because in this case
			// tempFormula.and(brickFormula)==brickFormula and hence is of form SFR | SFM | ....

			// if one of the ored skill has a size 1 (eg SFR&SFM fails), then it's ok!
			// else it's mean that there is no "skill on same branch solution".
			bool	ok= false;
			std::list<CReqSkillFormula::CSkillValueAnd>::iterator	it(tempFormula.OrSkills.begin()),
				end(tempFormula.OrSkills.end());
			for(;it!=end;it++)
			{
				CReqSkillFormula::CSkillValueAnd	skillAnd= *it;
				// ok, there is still one usable skill
				if(skillAnd.AndSkills.size()==1)
				{
					ok= true;
					break;
				}
			}

			// insert only if ok
			if(ok)
			{
				res.push_back(bricks[i]);
			}
		}
	}

	// replace with filtered one
	bricks= res;

}


// ***************************************************************************
void			CDBGroupBuildPhrase::filterRootBrickType(std::vector<NLMISC::CSheetId> &bricks)
{
	// no filter => ok.
	if(_RootBrickTypeFilter[0]==BRICK_TYPE::UNKNOWN)
		return;

	CSBrickManager	*pBM= CSBrickManager::getInstance();
	std::vector<CSheetId>	res;
	res.reserve(bricks.size());

	// keep only match ones
	for(uint i=0;i<bricks.size();i++)
	{
		const CSBrickSheet	*brick0= pBM->getBrick(bricks[i]);
		if(brick0)
		{
			// insert if one filter match (OR)
			for(uint j=0;j<MaxRootBrickTypeFilter;j++)
			{
				if( _RootBrickTypeFilter[j] != BRICK_TYPE::UNKNOWN &&
					_RootBrickTypeFilter[j] == BRICK_FAMILIES::brickType(brick0->BrickFamily) )
				{
					res.push_back(bricks[i]);
					break;
				}
			}
		}
	}

	// replace with filtered one
	bricks= res;
}


// ***************************************************************************
void			CDBGroupBuildPhrase::filterRootPossibles(std::vector<NLMISC::CSheetId> &bricks)
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	std::vector<CSheetId>	res;
	res.reserve(bricks.size());

	// keep only match ones
	for(uint i=0;i<bricks.size();i++)
	{
		const CSBrickSheet	*brick0= pBM->getBrick(bricks[i]);
		if(brick0)
		{
			// insert only if not a proc enchantment
			if(!brick0->isProcEnchantment())
				res.push_back(bricks[i]);
		}
	}

	// replace with filtered one
	bricks= res;
}


// ***************************************************************************
void			CDBGroupBuildPhrase::CSlot::reset()
{
	// Must do the test for credits not setuped.
	if(Brick && Back)
	{
		Brick->setActive(false);
		Brick->setSheetId(0);
		Back->setActive(false);
	}

	// Sheet 0
	ViewParamFamilies.clear();
}

// ***************************************************************************
void			CDBGroupBuildPhrase::CWord::resetParams()
{
	for(uint i=0;i<MaxParam;i++)
		ParamSlot[i].reset();
	// reset numParams to 0
	Slot.ViewParamFamilies.clear();
	NumTotalParams= 0;
	ParamError= false;
}

// ***************************************************************************
void			CDBGroupBuildPhrase::CWord::reset()
{
	Slot.reset();
	// reset also all params
	resetParams();
	// Hide the Cost View
	CostView->setActive(false);
	CreditView->setActive(false);
	InfoView->setActive(false);
}

// ***************************************************************************
void			CDBGroupBuildPhrase::CWord::setBrick(uint32 sheetId)
{
	Slot.Brick->setActive(true);
	Slot.Brick->setSheetId(sheetId);
	Slot.Back->setActive(true);
}

// ***************************************************************************
void			CDBGroupBuildPhrase::CWord::setParamBrick(uint param, uint32 sheetId)
{
	if(param>=MaxParam)
		return;

	ParamSlot[param].Brick->setActive(true);
	ParamSlot[param].Brick->setSheetId(sheetId);
	ParamSlot[param].Back->setActive(true);
}

// ***************************************************************************
void			CDBGroupBuildPhrase::CWord::copySetup(const CWord &w)
{
	// reset me first
	reset();

	// set the brick
	setBrick(w.Slot.Brick->getSheetId());
	Slot.Valid= w.Slot.Valid;
	Slot.ViewParamFamilies= w.Slot.ViewParamFamilies;

	// copy ParamBricks
	NumTotalParams= w.NumTotalParams;
	ParamError= w.ParamError;
	for(uint i=0;i<NumTotalParams;i++)
	{
		setParamBrick(i, w.ParamSlot[i].Brick->getSheetId());
		ParamSlot[i].Valid= w.ParamSlot[i].Valid;
		ParamSlot[i].ViewParamFamilies= w.ParamSlot[i].ViewParamFamilies;
	}

	// set the cost
	CostView->setText(w.CostView->getText());
	CostView->setActive(w.CostView->getActive());
	CreditView->setText(w.CreditView->getText());
	CreditView->setActive(w.CreditView->getActive());
	InfoView->setText(w.InfoView->getText());
	InfoView->setActive(w.InfoView->getActive());
}


// ***************************************************************************
void			CDBGroupBuildPhrase::resetSentence(sint32 rootSheetId)
{
	if(!_GroupValid)
		return;

	uint	i;

	// first update the root brick
	_MainWords[0].setBrick(rootSheetId);
	// then update display for others
	updateDisplayFromRootBrick();

	// get the root brick
	const CSBrickSheet	*rootBrick= getRootBrick();
	if(!rootBrick)
		return;

	// *** Init Root
	// update parameters of the root
	updateParams(0);

	// *** Init Mandatories
	for(i=1;i<1+_NumMandatories;i++)
	{
		// get the family Known BitField
		uint	familyId= rootBrick->MandatoryFamilies[i-1];
		sint32	sheet;
		bool	valid;
		getDefaultSheetForFamily(familyId, sheet, valid);

		// Setup the ctrl sheet of mandatory to 0 by default!
		_MainWords[i].setBrick(0);
		_MainWords[i].Slot.Valid= valid;

		// set the ctrl sheet display
		_MainWords[i].Slot.Brick->setGrayed( !valid );

		// update the parameters of this main brick
		updateParams(i);
	}

	// *** Init Optional/Credits
	// ungray all optional slots.
	for(i=1+_NumMandatories;i<_MainWords.size();i++)
	{
		_MainWords[i].Slot.Brick->setGrayed( false );
	}

	// update the NewOp controler
	updateNewButtons();

}

// ***************************************************************************
void		CDBGroupBuildPhrase::updateDisplayFromRootBrick()
{
	if(!_GroupValid)
		return;

	uint i;

	// get the root bricks
	const CSBrickSheet	*brick= getRootBrick();

	// Reset the Root params only (don't reset the sheetId setuped!)
	_MainWords[0].resetParams();

	// Hide all other slots by default
	for(i=1;i<_MainWords.size();i++)
	{
		_MainWords[i].reset();
	}
	_NumMandatories= 0;
	_NumOptionals= 0;
	_NumCredits= 0;

	// if brick not found, or if not enough ctrl, hide all
	if(!brick || brick->MandatoryFamilies.size()+1>_MainWords.size() )
	{
		// empty sheet
		_MainWords[0].Slot.Brick->setSheetId(0);
	}
	// else ok, setup the composition
	else
	{
		_NumMandatories= (uint32)brick->MandatoryFamilies.size();
		// Don't enable any optional/credit by default
		_NumCredits= 0;
		_NumOptionals= 0;
	}

	// update the group and sons coords
	invalidateCoords();
}

// ***************************************************************************
void			CDBGroupBuildPhrase::buildCurrentPhrase(CSPhraseCom &newPhrase)
{
	/* Word Order: Root/Mandatory/Optional/Credits (with all their params)
	*/

	// Reset.
	newPhrase.Name.clear();
	newPhrase.Bricks.clear();
	uint	i;
	const CSBrickSheet	*brick;

	// Add All bricks with their parameters
	for(i=0;i<getNumMainBricks();i++)
	{
		CWord	&word= _MainWords[i];
		brick= word.Slot.Brick->asSBrickSheet();
		newPhrase.Bricks.push_back(brick?brick->Id:CSheetId());
		// For all its params.
		for(uint j=0;j<word.NumTotalParams;j++)
		{
			brick= word.ParamSlot[j].Brick->asSBrickSheet();
			newPhrase.Bricks.push_back(brick?brick->Id:CSheetId());
		}
	}

	// Set the Name
	if(_UserSentenceName)
	{
		newPhrase.Name= _UserSentenceName->getInputString();
	}
}


// ***************************************************************************
void			CDBGroupBuildPhrase::updateNewButtons()
{
	// TODO_BRICK: have I to hide button and their text if not possible to add any op/credit?

	/*if(!_NewOpButton)
		return;

	uint	newOpIndex= _NumMandatories+_NumOptionals;

	// if some place for a new optional, show and place the NewOpButton
	if(newOpIndex<_MandOpWords.size())
	{
		// if comes from a delete, ensure the back+1 is hidden
		if(newOpIndex+1<_MandOpWords.size())
		{
			_MandOpWords[newOpIndex+1].Slot.Back->setActive(false);
		}

		// Show a new Optional possibilty ONLY if really possible!!
		std::vector<NLMISC::CSheetId>	bricks;
		fillNewOptionalBricks(bricks);
		// if no choice possible
		if(bricks.empty())
		{
			_NewOpButton->setActive(false);
		}
		else
		{
			// show the Back under the button
			_MandOpWords[newOpIndex].Slot.Back->setActive(true);
			// And move the button under it
			_NewOpButton->setParentPos(_MandOpWords[newOpIndex].Slot.Back);
			_NewOpButton->setActive(true);
		}
	}
	else
	{
		_NewOpButton->setActive(false);
	}

	// retrace all.
	invalidateCoords();*/
}

// ***************************************************************************
void			CDBGroupBuildPhrase::updateSpellView()
{
	if(_SpellView)
	{
		// Build the current phrase
		CSPhraseCom		newPhrase;
		buildCurrentPhrase(newPhrase);

		// Set the edited version to the phrase Manager => auto updated in icon
		CSPhraseManager	*pPM= CSPhraseManager::getInstance();

		// replace the content of the edited phrase.
		pPM->setPhrase(CSPhraseManager::EditionSlot, newPhrase);
	}
}


// ***************************************************************************
void			CDBGroupBuildPhrase::fillNewOptionalBricks(std::vector<NLMISC::CSheetId> &bricks)
{
	if(!_GroupValid)		return;
	const CSBrickSheet		*rootBrick= getRootBrick();
	if(!rootBrick)		return;
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	// get all possible newFamilies
	vector<uint16>	optionalFamilies;
	optionalFamilies= rootBrick->OptionalFamilies;
	// Filter ones that already exist in the composition
	// Don't filter Combat and Power optional families (Use Brick exclusion system instead)
	if( !rootBrick->isCombat() && !rootBrick->isSpecialPower())
		filterFamilySetuped(optionalFamilies);

	// Select all bricks for those families.
	for(uint i=0;i<optionalFamilies.size();i++)
	{
		const vector<CSheetId>	&famBricks= pBM->getFamilyBricks(optionalFamilies[i]);
		bricks.insert(bricks.end(), famBricks.begin(), famBricks.end());
	}

	// get only ones known
	filterKnownBricks(bricks);

	// Combat special
	if( rootBrick->isCombat() )
	{
		// Ensure not same bricks are setuped
		filterBrickSetuped(bricks);
		// Ensure only optional of compatible Skill are inserted
		filterSkillSetuped(bricks, true, false);
	}
	else if ( rootBrick->isSpecialPower() )
	{
		// Ensure not same bricks are setuped
		filterBrickSetuped(bricks);
	}
	// filter by BrickExclusion
	filterBrickExclusion(bricks);
}

// ***************************************************************************
void			CDBGroupBuildPhrase::fillNewCreditBricks(std::vector<NLMISC::CSheetId> &bricks)
{
	if(!_GroupValid)		return;
	const CSBrickSheet		*rootBrick= getRootBrick();
	if(!rootBrick)		return;
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	// get all possible newFamilies
	vector<uint16>	creditFamilies;
	creditFamilies= rootBrick->CreditFamilies;
	// Filter ones that already exist in the composition
	filterFamilySetuped(creditFamilies);

	// Select all bricks for those families.
	for(uint i=0;i<creditFamilies.size();i++)
	{
		const vector<CSheetId>	&famBricks= pBM->getFamilyBricks(creditFamilies[i]);
		bricks.insert(bricks.end(), famBricks.begin(), famBricks.end());
	}

	// get only ones known
	filterKnownBricks(bricks);

	// filter by BrickExclusion
	filterBrickExclusion(bricks);
}


// ***************************************************************************
void			CDBGroupBuildPhrase::setRootBrickTypeFilter(BRICK_TYPE::EBrickType	rootBtFilter,
   BRICK_TYPE::EBrickType rootBtFilter2, BRICK_TYPE::EBrickType rootBtFilter3, BRICK_TYPE::EBrickType rootBtFilter4)
{
	nlctassert(MaxRootBrickTypeFilter==4);
	_RootBrickTypeFilter[0]= rootBtFilter;
	_RootBrickTypeFilter[1]= rootBtFilter2;
	_RootBrickTypeFilter[2]= rootBtFilter3;
	_RootBrickTypeFilter[3]= rootBtFilter4;
}

