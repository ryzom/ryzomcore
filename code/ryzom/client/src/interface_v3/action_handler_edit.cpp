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

using namespace std;
using namespace NLMISC;

#include "nel/gui/action_handler.h"
#include "nel/gui/group_editbox.h"
#include "interface_manager.h"
#include "../client_chat_manager.h"
#include "people_interraction.h"
#include "../r2/editor.h"
/*#include "user_controls.h"
#include "view.h"
#include "misc.h"
#include "input.h"*/

////////////
// EXTERN //
////////////

extern CClientChatManager		ChatMngr;


////////////
// GLOBAL //
////////////

/**********************************************************************************************************
*																										  *
*										edit 		handlers  actions									  *
*																										  *
***********************************************************************************************************/

// ***************************************************************************

// used for character classifiction (when the user press Ctrl-arrow)
static inline uint getCharacterCategory(ucchar c)
{
	if (c == ' ') return 0;
	if (c > 127 || isalpha((char) c)) return 1; // alpha & other characters
	if (isdigit((char) c)) return 2;
	return 3; // other cases, including punctuation
}

// ***************************************************************************

/** skip a block of character in a string, (same behaviour than when Ctrl-arrow is pressed)
  * It returns the new index
  */
static uint skipUCCharsRight(uint startPos, const ucstring &str)
{
	uint pos = startPos;
	uint endIndex = (uint)str.length();
	uint ccat = getCharacterCategory(str[pos]);
	// skip characters of the same category
	while (pos != endIndex && getCharacterCategory(str[pos]) == ccat) ++pos;
	// skip spaces
	while (pos != endIndex && str[pos] == ' ') ++pos;
	return pos;
}

// ***************************************************************************

/** skip a block of character in a string, (same behaviour than when Ctrl-arrow is pressed)
  * It returns the new index
  */
static uint skipUCCharsLeft(uint startPos, const ucstring &str)
{
	uint pos = startPos;
	-- pos;
	while (pos != 0 && str[pos] == ' ') --pos;
	if (pos == 0) return pos;
	uint ccat = getCharacterCategory(str[pos]);
	// skip characters of the same category
	while (pos != 0 && getCharacterCategory(str[pos - 1]) == ccat) --pos;
	return pos;
}

// ***************************************************************************

class CAHEdit : public IActionHandler
{
protected:
	// Current edit box. Filled by init.
	CGroupEditBox		*_GroupEdit;
	bool				_LooseSelection;

	// Init the action manager
	void init ()
	{
		// Get the current edit box
		_GroupEdit = NULL;
		_LooseSelection = false;

		// Get the interface manager
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		if (pIM)
		{
			CCtrlBase *basectrl = CWidgetManager::getInstance()->getCaptureKeyboard();
			if (basectrl)
				_GroupEdit = dynamic_cast<CGroupEditBox*>(basectrl);
		}

		if (_GroupEdit)
			_GroupEdit->stopParentBlink();
	}

	// For action handler that modify selection
	void handleSelection ()
	{
		if (_GroupEdit)
		{
			// If selection not active
			if (CGroupEditBox::getCurrSelection() == NULL)
			{
				// then start selection at curPos
				CGroupEditBox::setSelectCursorPos(_GroupEdit->getCursorPos());
				CGroupEditBox::setCurrSelection (_GroupEdit);
			}
		}
	}

	// For action handler that unselect
	void handleUnselection()
	{
		if (_GroupEdit)
		{
			// If selection active
			CGroupEditBox *currSelection = dynamic_cast< CGroupEditBox* >( CGroupEditBox::getCurrSelection() );
			if (currSelection != NULL)
			{
				if (currSelection != _GroupEdit)
				{
					nlwarning("Selection can only be on focus");
				}

				// disable selection
				CGroupEditBox::setCurrSelection(NULL);
				_LooseSelection = true;
			}
		}
	}

	// Init part
	virtual void initPart ()
	{
		init();
		handleUnselection();
	}

	// Action part
	virtual void actionPart () = 0;

	// R2 editor part (no op by default)
	virtual void forwardToEditor() {}

	// Tha action handler
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		initPart ();
		if (_GroupEdit)
		{
			actionPart ();
		}
		else
		{
			forwardToEditor();
		}
	}
};

// ***************************************************************************

class CAHEditPreviousChar : public CAHEdit
{
	void actionPart ()
	{
		if (_LooseSelection)
		{
			sint32	minPos= min(_GroupEdit->getCursorPos(), _GroupEdit->getSelectCursorPos());
			sint32	maxPos= max(_GroupEdit->getCursorPos(), _GroupEdit->getSelectCursorPos());

			// Special Code if left or right: Set the cursor pos to bounds of selection.
			if(minPos!=maxPos)
			{
				_GroupEdit->setCursorPos(minPos);
				// don't call std Left
				return;
			}
		}

		if (_GroupEdit->getCursorPos() > 0)
		{
			_GroupEdit->setCursorPos(_GroupEdit->getCursorPos()-1);
			_GroupEdit->setCursorAtPreviousLineEnd(false);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHEditPreviousChar, "edit_previous_char");

// ***************************************************************************

class CAHEditNextChar : public CAHEdit
{
	void actionPart ()
	{
		if (_LooseSelection)
		{
			sint32	minPos = min(_GroupEdit->getCursorPos(), _GroupEdit->getSelectCursorPos());
			sint32	maxPos = max(_GroupEdit->getCursorPos(), _GroupEdit->getSelectCursorPos());

			if(minPos!=maxPos)
			{
				_GroupEdit->setCursorPos(maxPos);
				// don't call std Right
				return;
			}
		}

		if (_GroupEdit->getCursorPos() < (sint32) _GroupEdit->getInputStringRef().length())
		{
			_GroupEdit->setCursorPos(_GroupEdit->getCursorPos()+1);
			_GroupEdit->setCursorAtPreviousLineEnd(false);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHEditNextChar, "edit_next_char");

// ***************************************************************************

class CAHEditPreviousWord : public CAHEdit
{
	void actionPart ()
	{
		if (_GroupEdit->getCursorPos() > 0)
		{
			_GroupEdit->setCursorPos(skipUCCharsLeft(_GroupEdit->getCursorPos(), _GroupEdit->getInputStringRef()));
			_GroupEdit->setCursorAtPreviousLineEnd(false);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHEditPreviousWord, "edit_previous_word");

// ***************************************************************************

class CAHEditNextWord : public CAHEdit
{
	void actionPart ()
	{
		if (_GroupEdit->getCursorPos() < (sint32) _GroupEdit->getInputStringRef().length())
		{
			_GroupEdit->setCursorPos(skipUCCharsRight(_GroupEdit->getCursorPos(), _GroupEdit->getInputStringRef()));
			_GroupEdit->setCursorAtPreviousLineEnd(false);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHEditNextWord, "edit_next_word");

// ***************************************************************************

class CAHEditGotoLineBegin : public CAHEdit
{
	void actionPart ()
	{
		// go to the start of line
		if (_GroupEdit->getViewText())
		{
			sint line = _GroupEdit->getViewText()->getLineFromIndex(_GroupEdit->getCursorPos() + (uint)_GroupEdit->getPrompt().length());
			if (line == -1) return;
			sint newPos = std::max(_GroupEdit->getViewText()->getLineStartIndex(line), (sint) _GroupEdit->getPrompt().length());
			if (newPos == -1) return;
			_GroupEdit->setCursorPos(newPos - (sint32)_GroupEdit->getPrompt().length());
			_GroupEdit->setCursorAtPreviousLineEnd(false);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHEditGotoLineBegin, "edit_goto_line_begin");

// ***************************************************************************

class CAHEditGotoLineEnd : public CAHEdit
{
	void actionPart ()
	{
		// go to the end of line
		if (_GroupEdit->getViewText())
		{
			if (_GroupEdit->getViewText()->getMultiLine())
			{
				sint line = _GroupEdit->getViewText()->getLineFromIndex(_GroupEdit->getCursorPos() + (uint)_GroupEdit->getPrompt().length(), _GroupEdit->isCursorAtPreviousLineEnd());
				if (line == -1) return;
				sint newPos;
				bool endOfPreviousLine;
				_GroupEdit->getViewText()->getLineEndIndex(line, newPos, endOfPreviousLine);
				if (newPos != -1)
				{
					_GroupEdit->setCursorPos(newPos - (sint32)_GroupEdit->getPrompt().length());
					_GroupEdit->setCursorAtPreviousLineEnd(endOfPreviousLine);
				}
			}
			else
			{
				_GroupEdit->setCursorPos((sint32)_GroupEdit->getPrompt().length() + (sint32)_GroupEdit->getInputString().length());
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHEditGotoLineEnd, "edit_goto_line_end");

// ***************************************************************************

class CAHEditGotoBlockBegin : public CAHEdit
{
	void actionPart ()
	{
		_GroupEdit->setCursorPos(0);
		_GroupEdit->setCursorAtPreviousLineEnd(false);
	}
};
REGISTER_ACTION_HANDLER (CAHEditGotoBlockBegin, "edit_goto_block_begin");

// ***************************************************************************

class CAHEditGotoBlockEnd : public CAHEdit
{
	void actionPart ()
	{
		_GroupEdit->setCursorPos((sint32)_GroupEdit->getInputStringRef().length());
		_GroupEdit->setCursorAtPreviousLineEnd(false);
	}
};
REGISTER_ACTION_HANDLER (CAHEditGotoBlockEnd, "edit_goto_block_end");

// ***************************************************************************

class CAHEditPreviousLine : public CAHEdit
{
	void actionPart ()
	{
		if (_GroupEdit->getMaxHistoric() && (! _GroupEdit->getViewText()->getMultiLine()))
		{
			// Get the start of the string.
			ucstring	startStr= _GroupEdit->getInputStringRef().substr(0, _GroupEdit->getCursorPos());

			// Search all historic string that match startStr.
			for(sint i=_GroupEdit->getCurrentHistoricIndex()+1;i<(sint)_GroupEdit->getNumHistoric();i++)
			{
				if( _GroupEdit->getHistoric(i).compare(0, _GroupEdit->getCursorPos(), startStr)==0 )
				{
					_GroupEdit->setInputStringRef (_GroupEdit->getHistoric(i));
					_GroupEdit->setCurrentHistoricIndex(i);
					break;
				}
			}
		}
		else if (_GroupEdit->getViewText()->getMultiLine())
		{
			uint cursorPosInText = _GroupEdit->getCursorPos() + (uint)_GroupEdit->getPrompt().length();
			if (
				(_GroupEdit->getCursorPos() == (sint32) _GroupEdit->getInputStringRef().length() && _GroupEdit->getViewText()->getNumLine() == 1) ||
				_GroupEdit->getViewText()->getLineFromIndex(cursorPosInText, _GroupEdit->isCursorAtPreviousLineEnd()) == 0
				) // in the first line .. ?
			{
				// .. so do nothing
				return;
			}
			sint cx, cy;
			sint height;
			_GroupEdit->getViewText()->getCharacterPositionFromIndex(cursorPosInText, _GroupEdit->isCursorAtPreviousLineEnd(), cx, cy, height);
			cy += height + _GroupEdit->getViewText()->getMultiLineSpace();
			uint newCharIndex;
			bool newLineEnd;
			_GroupEdit->getViewText()->getCharacterIndexFromPosition(cx, cy, newCharIndex, newLineEnd);
			if (newLineEnd)
			{
				_GroupEdit->setCursorPos(newCharIndex - (sint32)_GroupEdit->getPrompt().length());
				_GroupEdit->setCursorAtPreviousLineEnd(true);
				sint32 newPos = _GroupEdit->getCursorPos();
				clamp(newPos, (sint32) 0, (sint32) _GroupEdit->getInputStringRef().size());
				_GroupEdit->setCursorPos(newPos);
				return;
			}
			_GroupEdit->setCursorAtPreviousLineEnd(false);
			// takes character whose X is closer to the current X
			sint cx0, cx1;
			sint cy0, cy1;
			_GroupEdit->getViewText()->getCharacterPositionFromIndex(newCharIndex, _GroupEdit->isCursorAtPreviousLineEnd(), cx0, cy0, height);
			_GroupEdit->getViewText()->getCharacterPositionFromIndex(newCharIndex + 1, _GroupEdit->isCursorAtPreviousLineEnd(), cx1, cy1, height);
			if (abs(cx0 - cx) < abs(cx1 - cx) || cy0 != cy1)
			{
				_GroupEdit->setCursorPos(newCharIndex);
			}
			else
			{
				_GroupEdit->setCursorPos(newCharIndex + 1);
			}
			_GroupEdit->setCursorPos(_GroupEdit->getCursorPos()-(sint32)_GroupEdit->getPrompt().length());
			sint32 newpos = _GroupEdit->getCursorPos();
			clamp(newpos, (sint32) 0, (sint32)_GroupEdit->getInputStringRef().size());
			_GroupEdit->setCursorPos(newpos);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHEditPreviousLine, "edit_previous_line");

// ***************************************************************************

class CAHEditNextLine : public CAHEdit
{
	void actionPart ()
	{
		if( (! _GroupEdit->getViewText()->getMultiLine()) && _GroupEdit->getMaxHistoric() && _GroupEdit->getCurrentHistoricIndex()>0)
		{
			// Get the start of the string.
			ucstring	startStr= _GroupEdit->getInputStringRef().substr(0, _GroupEdit->getCursorPos());

			// Search all historic string that match startStr.
			for(sint i=_GroupEdit->getCurrentHistoricIndex()-1;i>=0;i--)
			{
				if( _GroupEdit->getHistoric(i).compare(0, _GroupEdit->getCursorPos(), startStr)==0 )
				{
					_GroupEdit->setInputStringRef (_GroupEdit->getHistoric(i));
					_GroupEdit->setCurrentHistoricIndex(i);
					break;
				}
			}
		}
		else if (_GroupEdit->getViewText()->getMultiLine())
		{
			sint cx, cy;
			sint height;
			_GroupEdit->getViewText()->getCharacterPositionFromIndex(_GroupEdit->getCursorPos() + (sint)_GroupEdit->getPrompt().length(), _GroupEdit->isCursorAtPreviousLineEnd(), cx, cy, height);
			if (cy != 0)
			{
				cy -= height;
				uint newCharIndex;
				bool newLineEnd;
				_GroupEdit->getViewText()->getCharacterIndexFromPosition(cx, cy, newCharIndex, newLineEnd);
				if (newLineEnd)
				{
					_GroupEdit->setCursorPos(newCharIndex - (sint32)_GroupEdit->getPrompt().length());
					_GroupEdit->setCursorAtPreviousLineEnd(true);
					sint32 newPos = _GroupEdit->getCursorPos();
					clamp(newPos, (sint32) 0, (sint32) _GroupEdit->getInputStringRef().size());
					_GroupEdit->setCursorPos(newPos);
					return;
				}
				_GroupEdit->setCursorAtPreviousLineEnd(false);
				// takes character whose X is closer to the current X
				sint cx0, cx1;
				sint cy0, cy1;
				_GroupEdit->getViewText()->getCharacterPositionFromIndex(newCharIndex, _GroupEdit->isCursorAtPreviousLineEnd(), cx0, cy0, height);
				_GroupEdit->getViewText()->getCharacterPositionFromIndex(newCharIndex + 1, _GroupEdit->isCursorAtPreviousLineEnd(), cx1, cy1, height);
				if (abs(cx0 - cx) < abs(cx1 - cx) || cy0 != cy1)
				{
					_GroupEdit->setCursorPos(newCharIndex);
				}
				else
				{
					_GroupEdit->setCursorPos(min(sint32(newCharIndex + 1), sint32(_GroupEdit->getInputStringRef().length() + _GroupEdit->getPrompt().length())));
				}
				_GroupEdit->setCursorPos(_GroupEdit->getCursorPos()-(sint32)_GroupEdit->getPrompt().length());
				sint32 newPos = _GroupEdit->getCursorPos();
				clamp(newPos, (sint32) 0, (sint32) _GroupEdit->getInputStringRef().size());
				_GroupEdit->setCursorPos(newPos);
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHEditNextLine, "edit_next_line");

// ***************************************************************************

class CAHEditDeleteChar : public CAHEdit
{
protected:
	void initPart ()
	{
		init();
	}
	void actionPart ()
	{
		// if selection is activated and not same cursors pos, then cut the selection
		if(CGroupEditBox::getCurrSelection() != NULL && _GroupEdit->getCursorPos() != CGroupEditBox::getSelectCursorPos())
		{
			if (CGroupEditBox::getCurrSelection() != _GroupEdit)
			{
				nlwarning("Selection can only be on focus");
			}
			_GroupEdit->cutSelection();
			// Change Handler
			if (!_GroupEdit->getAHOnChange().empty())
			{
				CInterfaceManager *pIM = CInterfaceManager::getInstance();
				CAHManager::getInstance()->runActionHandler(_GroupEdit->getAHOnChange(), _GroupEdit, _GroupEdit->getParamsOnChange());
			}
		}
		// else cut forwards
		else if(_GroupEdit->getCursorPos() < (sint32) _GroupEdit->getInputStringRef().length())
		{
			ucstring inputString = _GroupEdit->getInputStringRef();
			ucstring::iterator it = inputString.begin() + _GroupEdit->getCursorPos();
			inputString.erase(it);
			_GroupEdit->setInputStringRef (inputString);
			if (!_GroupEdit->getAHOnChange().empty())
			{
				CInterfaceManager *pIM = CInterfaceManager::getInstance();
				CAHManager::getInstance()->runActionHandler(_GroupEdit->getAHOnChange(), _GroupEdit, _GroupEdit->getParamsOnChange());
			}
		}
		// must stop selection in all case
		CGroupEditBox::setCurrSelection(NULL);
		_GroupEdit->setCursorAtPreviousLineEnd(false);
	}
};
REGISTER_ACTION_HANDLER (CAHEditDeleteChar, "edit_delete_char");

// ***************************************************************************

class CAHEditSelectAll : public CAHEdit
{
	void initPart ()
	{
		init();
	}
	void actionPart ()
	{
		_GroupEdit->setSelectionAll();
	}
};
REGISTER_ACTION_HANDLER (CAHEditSelectAll, "edit_select_all");

// ***************************************************************************

class CAHEditCopy : public CAHEdit
{
	void initPart ()
	{
		init();
	}
	void actionPart ()
	{
		_GroupEdit->copy();
	}
	void forwardToEditor()
	{
		R2::getEditor().copy();
	}
};
REGISTER_ACTION_HANDLER (CAHEditCopy, "edit_copy");

// ***************************************************************************

class CAHEditPaste : public CAHEdit
{
	void initPart ()
	{
		init();
	}
	void actionPart ()
	{
		_GroupEdit->paste();
	}
	void forwardToEditor()
	{
		R2::getEditor().paste();
	}
};
REGISTER_ACTION_HANDLER (CAHEditPaste, "edit_paste");

// ***************************************************************************

class CAHEditCut : public CAHEditDeleteChar
{
	void initPart ()
	{
		init();
	}
	void actionPart ()
	{
		// if selection is activated and not same cursors pos, then cut the selection
		if(CGroupEditBox::getCurrSelection() != NULL && _GroupEdit->getCursorPos() != CGroupEditBox::getSelectCursorPos())
		{
			// Copy selection
			_GroupEdit->copy();

			// Cut selection
			CAHEditDeleteChar::actionPart();
		}
	}
};
REGISTER_ACTION_HANDLER (CAHEditCut, "edit_cut");

// ***************************************************************************

class CAHEditExpand : public CAHEdit
{
	void initPart ()
	{
		init();
	}
	void actionPart ()
	{
		_GroupEdit->expand();
	}
};
REGISTER_ACTION_HANDLER (CAHEditExpand, "edit_expand");

// ***************************************************************************
class CAHEditExpandOrCycleTell : public CAHEdit
{
	void initPart ()
	{
		init();
	}
	void actionPart ()
	{
		// If the line starts with '/tell ', do not try to expand
		static const ucstring TELL_STR("/tell ");
		if (_GroupEdit->getInputString().substr(0, TELL_STR.length()) != TELL_STR)
		{
			if (_GroupEdit->expand()) return;
		}
		CInterfaceManager *im = CInterfaceManager::getInstance();
		if (!im->isInGame()) return;
		// there was no / at the start of the line so try to cycle through the last people on which a tell was done
		const ucstring *lastTellPeople = ChatMngr.cycleLastTell();
		if (!lastTellPeople) return;
		// Get chat box from ist edit box
		// If it isn't a user chat or the main chat, just display 'tell' with the name. Otherwise, change the target of the window
		CChatWindow *cw = getChatWndMgr().getChatWindowFromCaller(_GroupEdit);
		if (!cw) return;
		CFilteredChat *fc = PeopleInterraction.getFilteredChatFromChatWindow(cw);
		if (fc)
		{
			fc->Filter.setTargetPlayer(*lastTellPeople);
		}
		else
		{
			// it is not a filtered chat, display 'tell' (must be ingame)
			_GroupEdit->setCommand(ucstring("tell ") + *lastTellPeople + (ucchar) ' ', false);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHEditExpandOrCycleTell, "edit_expand_or_cycle_tell");

// ***************************************************************************

class CAHEditBack: public CAHEdit
{
	void initPart ()
	{
		init();
	}
	void actionPart ()
	{
		_GroupEdit->back();
	}
};
REGISTER_ACTION_HANDLER (CAHEditBack, "edit_back");

// ***************************************************************************

class CAHEditSelectPreviousChar : public CAHEditPreviousChar
{
	void initPart ()
	{
		init();
		handleSelection();
	}
};
REGISTER_ACTION_HANDLER (CAHEditSelectPreviousChar, "edit_select_previous_char");

// ***************************************************************************

class CAHEditSelectNextChar : public CAHEditNextChar
{
	void initPart ()
	{
		init();
		handleSelection();
	}
};
REGISTER_ACTION_HANDLER (CAHEditSelectNextChar, "edit_select_next_char");

// ***************************************************************************

class CAHEditSelectPreviousWord : public CAHEditPreviousWord
{
	void initPart ()
	{
		init();
		handleSelection();
	}
};
REGISTER_ACTION_HANDLER (CAHEditSelectPreviousWord, "edit_select_previous_word");

// ***************************************************************************

class CAHEditSelectNextWord : public CAHEditNextWord
{
	void initPart ()
	{
		init();
		handleSelection();
	}
};
REGISTER_ACTION_HANDLER (CAHEditSelectNextWord, "edit_select_next_word");

// ***************************************************************************

class CAHEditSelectToLineBegin : public CAHEditGotoLineBegin
{
	void initPart ()
	{
		init();
		handleSelection();
	}
};
REGISTER_ACTION_HANDLER (CAHEditSelectToLineBegin, "edit_select_to_line_begin");

// ***************************************************************************

class CAHEditSelectToLineEnd : public CAHEditGotoLineEnd
{
	void initPart ()
	{
		init();
		handleSelection();
	}
};
REGISTER_ACTION_HANDLER (CAHEditSelectToLineEnd, "edit_select_to_line_end");

// ***************************************************************************

class CAHEditSelectToBlockBegin : public CAHEditGotoBlockBegin
{
	void initPart ()
	{
		init();
		handleSelection();
	}
};
REGISTER_ACTION_HANDLER (CAHEditSelectToBlockBegin, "edit_select_to_block_begin");

// ***************************************************************************

class CAHEditSelectToBlockEnd : public CAHEditGotoBlockEnd
{
	void initPart ()
	{
		init();
		handleSelection();
	}
};
REGISTER_ACTION_HANDLER (CAHEditSelectToBlockEnd, "edit_select_to_block_end");

// ***************************************************************************
