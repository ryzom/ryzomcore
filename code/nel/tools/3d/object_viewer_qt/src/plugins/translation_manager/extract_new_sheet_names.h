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

#ifndef EXTRACT_NEW_SHEET_NAMES_H
#define	EXTRACT_NEW_SHEET_NAMES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/config_file.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/path.h"
#include "nel/misc/diff_tool.h"
#include "nel/misc/algo.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/primitive_utils.h"

namespace TranslationManager
{

// ***************************************************************************
/*
 *	Interface to build the whole list of words (key id) for a specific worksheet
 */
struct IWordListBuilder
{
	virtual bool buildWordList(std::vector<std::string> &allWords, std::string workSheetFileName) =0;
};

struct CSheetWordListBuilder : public IWordListBuilder
{
	std::string SheetExt;
	std::string SheetPath;

	virtual bool buildWordList(std::vector<std::string> &allWords, std::string workSheetFileName);
};

struct CRegionPrimWordListBuilder : public IWordListBuilder
{
	std::string PrimPath;
	std::vector<std::string> PrimFilter;
	NLLIGO::CLigoConfig LigoConfig;
	virtual bool buildWordList(std::vector<std::string> &allWords, std::string workSheetFileName);
};

}

#endif	/* EXTRACT_NEW_SHEET_NAMES_H */