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
#include "view_text_formated.h"
#include "../user_entity.h"
#include "../entities.h"
#include "../string_manager_client.h"
#include "action_handler_misc.h"
//
#include "game_share/xml_auto_ptr.h"
//
#include "nel/misc/i18n.h"

////////////
// EXTERN //
////////////
using namespace STRING_MANAGER;

NLMISC_REGISTER_OBJECT(CViewBase, CViewTextFormated, std::string, "text_formated");

// ****************************************************************************
bool CViewTextFormated::parse(xmlNodePtr cur,CInterfaceGroup * parentGroup)
{
	if (!CViewText::parse(cur, parentGroup)) return false;
	CXMLAutoPtr prop((const char*) xmlGetProp( cur, (xmlChar*)"format" ));
	if (prop)
		setFormatString(ucstring((const char *) prop));
	else
		setFormatString(ucstring("$t"));
	return true;
}

// ****************************************************************************
void CViewTextFormated::checkCoords()
{
	if (!getActive()) return;
	ucstring formatedResult;
	formatedResult = formatString(_FormatString, ucstring(""));

	//
	setText (formatedResult);
	CViewText::checkCoords();
}

// ****************************************************************************
void CViewTextFormated::setFormatString(const ucstring &format)
{
	_FormatString = format;
	if ( (_FormatString.size()>2) && (_FormatString[0] == 'u') && (_FormatString[1] == 'i'))
		_FormatString = NLMISC::CI18N::get (format.toString());
}

// ****************************************************************************
ucstring CViewTextFormated::formatString(const ucstring &inputString, const ucstring &paramString)
{
	ucstring formatedResult;
	// Apply the format
	for(ucstring::const_iterator it = inputString.begin(); it != inputString.end();)
	{
		if (*it == '$')
		{
			++it;
			if (it == inputString.end()) break;
			switch(*it)
			{
				case 't': // add text ID
					formatedResult += paramString;
				break;

				case 'P':
				case 'p':  // add player name
					if (ClientCfg.Local)
					{
						formatedResult += ucstring("player");
					}
					else
					{
						if(UserEntity)
						{
							ucstring name = UserEntity->getEntityName();
							if (*it == 'P') setCase(name, CaseUpper);
							formatedResult += name;
						}
					}
				break;
				//
				case 's':
				case 'b': // add bot name
				{
					ucstring botName;
					bool womanTitle = false;
					if (ClientCfg.Local)
					{
						botName = ucstring("NPC");
					}
					else
					{
						CLFECOMMON::TCLEntityId trader = CLFECOMMON::INVALID_SLOT;
						if(UserEntity)
							trader = UserEntity->trader();
						if (trader != CLFECOMMON::INVALID_SLOT)
						{
							CEntityCL *entity = EntitiesMngr.entity(trader);
							if (entity != NULL)
							{
								uint32 nDBid = entity->getNameId();

								if (nDBid != 0)
								{
									CStringManagerClient *pSMC = CStringManagerClient::instance();
									pSMC->getString(nDBid, botName);
								}
								else
								{
									botName = entity->getDisplayName();
								}

								CCharacterCL *pChar = dynamic_cast<CCharacterCL*>(entity);
								if (pChar != NULL)
									womanTitle = pChar->getGender() == GSGENDER::female;
							}
						}
					}

					// get the title translated
					ucstring sTitleTranslated = botName;
					CStringPostProcessRemoveName spprn;
					spprn.Woman = womanTitle;
					spprn.cbIDStringReceived(sTitleTranslated);

					botName = CEntityCL::removeTitleAndShardFromName(botName);

					// short name (with no title such as 'guard', 'merchant' ...)
					if (*it == 's')
					{
						// But if there is no name, display only the title
						if (botName.empty())
							botName = sTitleTranslated;
					}
					else
					{
						// Else we want the title !
						if (!botName.empty())
							botName += " ";
						botName += sTitleTranslated;
					}

					formatedResult += botName;
				}
				break;
				default:
					formatedResult += (ucchar) '$';
				break;
			}
			++it;
		}
		else
		{
			formatedResult += (ucchar) *it;
			++it;
		}
	}

	return formatedResult;
}
