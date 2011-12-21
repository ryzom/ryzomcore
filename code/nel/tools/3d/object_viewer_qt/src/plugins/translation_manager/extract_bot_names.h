// Translation Manager Plugin - OVQT Plugin <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Emanuel Costea <cemycc@gmail.com>
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

#ifndef EXTRACT_BOT_NAMES_H
#define	EXTRACT_BOT_NAMES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/config_file.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/path.h"
#include "nel/misc/diff_tool.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/primitive_utils.h"

namespace TranslationManager
{

struct TCreatureInfo
{
	NLMISC::CSheetId SheetId;
	bool ForceSheetName;
	bool DisplayName;

	void readGeorges(const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
	{
		const NLGEORGES::UFormElm &item=form->getRootNode();

		SheetId=sheetId;
		item.getValueByName(ForceSheetName, "3d data.ForceDisplayCreatureName");
		item.getValueByName(DisplayName, "3d data.DisplayName");
	}

	void serial(NLMISC::IStream &f)
	{
		f.serial(SheetId);
		f.serial(ForceSheetName);
		f.serial(DisplayName);
	}

	static uint getVersion ()
	{
		return 1;
	}

	void removed()
	{
	}
};

struct TEntryInfo
{
	std::string SheetName;
};

struct ExtractBotNames
{
private:
	std::vector<std::string> Filters;
	std::map<NLMISC::CSheetId, TCreatureInfo> Creatures;
	std::set<std::string> GenericNames;
	std::map<std::string, TEntryInfo> SimpleNames;
	std::set<std::string> Functions;
private:
	TCreatureInfo *getCreature(const std::string &sheetName);
	ucstring makeGroupName(const ucstring &translationName);
	std::string removeAndStoreFunction(const std::string &fullName);
	void addGenericName(const std::string &name, const std::string &sheetName);
	void addSimpleName(const std::string &name, const std::string &sheetName);
public:
	void extractBotNamesFromPrimitives(NLLIGO::CLigoConfig ligoConfig);
	void setRequiredSettings(std::list<std::string> filters, std::string level_design_path);
	std::set<std::string> getGenericNames();
	std::map<std::string, TEntryInfo> getSimpleNames();
	std::string cleanupName(const std::string &name);
	ucstring cleanupUcName(const ucstring &name);
	void cleanSimpleNames();
	void cleanGenericNames();
};
}

#endif	/* EXTRACT_BOT_NAMES_H */