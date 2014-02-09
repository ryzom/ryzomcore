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
#include "string_manager_client.h"
#include "nel/misc/file.h"
#include "client_cfg.h"
#include "net_manager.h"
#include "connection.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/algo.h"
#include "misc.h"
#include "entity_cl.h"

using namespace std;
using namespace NLMISC;

namespace STRING_MANAGER
{

	// ***************************************************************************
	map<string, CStringManagerClient::CItem> CStringManagerClient::_SpecItem_TempMap;
	map<ucstring, ucstring> CStringManagerClient::_DynStrings;
	vector<ucstring> CStringManagerClient::_TitleWords;
	bool CStringManagerClient::_SpecItem_MemoryCompressed = false;
	char *CStringManagerClient::_SpecItem_Labels = NULL;
	ucchar *CStringManagerClient::_SpecItem_NameDesc = NULL;
	vector<CStringManagerClient::CItemLight> CStringManagerClient::_SpecItems;
	bool MustReleaseStaticArrays = true;


	CStringManagerClient *CStringManagerClient::_Instance= NULL;
	ucstring CStringManagerClient::_WaitString("???");


	CStringManagerClient::CStringManagerClient()
	{
		_CacheInited = false;
		_CacheLoaded = false;
		// insert the empty string.
		_ReceivedStrings.insert(make_pair((uint)EmptyStringId, ucstring()));
		// reserve some place to avoid reallocation as possible
		_CacheStringToSave.reserve(1024);
	}
	// destructor.
	CStringManagerClient::~CStringManagerClient()
	{
		if (MustReleaseStaticArrays)
		{
			if (_SpecItem_MemoryCompressed)
			{
				delete [] _SpecItem_Labels;
				_SpecItem_Labels = NULL;
				delete [] _SpecItem_NameDesc;
				_SpecItem_NameDesc = NULL;
				_SpecItems.clear();
				_SpecItem_MemoryCompressed = false;
			}
		}
	}


	CStringManagerClient *CStringManagerClient::instance()
	{
		if (_Instance == 0)
			_Instance = new CStringManagerClient();

		return _Instance;
	}

	void CStringManagerClient::release(bool mustReleaseStaticArrays)
	{
		if (_Instance != 0)
		{
			bool prev = MustReleaseStaticArrays;
			MustReleaseStaticArrays = mustReleaseStaticArrays;
			delete _Instance;
			MustReleaseStaticArrays = prev;
			_Instance  =0;
		}
	}


	void CStringManagerClient::initCache(const std::string &shardId, const std::string &languageCode)
	{
		H_AUTO( CStringManagerClient_initCache )

		_ShardId = shardId;
		_LanguageCode = languageCode;

		// to be inited, shard id and language code must be filled
		if (!_ShardId.empty() && !_LanguageCode.empty())
			_CacheInited = true;
		else
			_CacheInited = false;
	}

	void CStringManagerClient::loadCache(uint32 timestamp)
	{
		H_AUTO( CStringManagerClient_loadCache )

		if (_CacheInited)
		{
			try
			{
				_CacheFilename = std::string("save/") + _ShardId.substr(0, _ShardId.find(":")) + ".string_cache";

				nlinfo("SM : Try to open the string cache : %s", _CacheFilename.c_str());

				if (CFile::fileExists(_CacheFilename))
				{
					// there is a cache file, check date reset it if needed
					{
						NLMISC::CIFile file(_CacheFilename);
						file.serial(_Timestamp);
					}

					if (_Timestamp != timestamp)
					{
						nlinfo("SM: Clearing string cache : outofdate");
						// the cache is not sync, reset it
						NLMISC::COFile file(_CacheFilename);
						file.serial(timestamp);
					}
					else
					{
						nlinfo("SM : string cache in sync. cool");
					}
				}
				else
				{
					nlinfo("SM: Creating string cache");
					// cache file don't exist, create it with the timestamp
					NLMISC::COFile file(_CacheFilename);
					file.serial(timestamp);
				}

				// clear all current data.
				_ReceivedStrings.clear();
				_ReceivedDynStrings.clear();
				// NB : we keep the waiting strings and dyn strings

				// insert the empty string.
				_ReceivedStrings.insert(make_pair((uint)EmptyStringId, ucstring()));

				// load the cache file
				NLMISC::CIFile file(_CacheFilename);
				file.serial(_Timestamp);
				nlassert(_Timestamp == timestamp);

				while (!file.eof())
				{
					uint32			id;
					ucstring		str;

					file.serial(id);
					file.serial(str);

					//nldebug("SM : loading string [%6u] as [%s] in cache", id, str.toString().c_str());

					_ReceivedStrings.insert(std::make_pair(id, str));
				}

				_CacheLoaded = true;
			}
			catch(const NLMISC::Exception &e)
			{
				nlinfo("SM : loadCache failed, exception : %s", e.what());
				nlinfo("SM : cache deactivated");
				// unactivated cache.
				_CacheFilename.erase();
			}
		}
	}



	void CStringManagerClient::waitString(uint32 stringId, const IStringWaiterRemover *premover, ucstring *result)
	{
		H_AUTO( CStringManagerClient_waitString )

		nlassert(premover && result);
		ucstring value;
		if (getString(stringId, value))
			*result = value;
		else
		{
			// wait for the string
			TStringWaiter sw;
			sw.Result = result;
			sw.Remover = premover;
			_StringsWaiters.insert(std::make_pair(stringId, sw));
		}
	}

	void CStringManagerClient::waitString(uint32 stringId, IStringWaitCallback *pcallback)
	{
		H_AUTO( CStringManagerClient_waitString2 )

		nlassert(pcallback != 0);
		ucstring value;
		if (getString(stringId, value))
		{
			pcallback->onStringAvailable(stringId, value);
		}
		else
		{
			// wait for the string
			_StringsCallbacks.insert(std::make_pair(stringId, pcallback));
		}
	}


	void CStringManagerClient::waitDynString(uint32 stringId, const IStringWaiterRemover *premover, ucstring *result)
	{
		H_AUTO( CStringManagerClient_waitDynString )

		nlassert(premover && result);
		ucstring value;
		if (getDynString(stringId, value))
			*result = value;
		else
		{
			// wait for the string
			TStringWaiter sw;
			sw.Result = result;
			sw.Remover = premover;
			_DynStringsWaiters.insert(std::make_pair(stringId, sw));
		}
	}

	void CStringManagerClient::waitDynString(uint32 stringId, IStringWaitCallback *pcallback)
	{
		H_AUTO( CStringManagerClient_waitDynString2 )

		nlassert(pcallback != 0);
		ucstring value;
		if (getDynString(stringId, value))
		{
			pcallback->onDynStringAvailable(stringId, value);
		}
		else
		{
			_DynStringsCallbacks.insert(std::make_pair(stringId, pcallback));
		}
	}

	void CStringManagerClient::removeStringWaiter(const IStringWaiterRemover *remover)
	{
		H_AUTO( CStringManagerClient_removeStringWaiter )

		// search in waiting string
		{
restartLoop1:
			TStringWaitersContainer::iterator first(_StringsWaiters.begin()), last(_StringsWaiters.end());
			for (; first != last; ++first)
			{
				if (first->second.Remover == remover)
				{
					_StringsWaiters.erase(first);
					goto restartLoop1;
				}
			}
		}
		// search in waiting dyn string
		{
restartLoop2:
			TStringWaitersContainer::iterator first(_DynStringsWaiters.begin()), last(_DynStringsWaiters.end());
			for (; first != last; ++first)
			{
				if (first->second.Remover == remover)
				{
					_DynStringsWaiters.erase(first);
					goto restartLoop2;
				}
			}
		}
	}

	void CStringManagerClient::removeStringWaiter(const IStringWaitCallback *callback)
	{
//		H_AUTO( CStringManagerClient_removeStringWaiter2 )

		// search in waiting string
		{
restartLoop3:
			TStringCallbacksContainer::iterator first(_StringsCallbacks.begin()), last(_StringsCallbacks.end());
			for (; first != last; ++first)
			{
				if (first->second == callback)
				{
					_StringsCallbacks.erase(first);
					goto restartLoop3;
				}
			}
		}
		// search in waiting dyn string
		{
restartLoop4:
			TStringCallbacksContainer::iterator first(_DynStringsCallbacks.begin()), last(_DynStringsCallbacks.end());
			for (; first != last; ++first)
			{
				if (first->second == callback)
				{
					_DynStringsCallbacks.erase(first);
					goto restartLoop4;
				}
			}
		}
	}



	bool CStringManagerClient::getString(uint32 stringId, ucstring &result)
	{
		H_AUTO( CStringManagerClient_getString )

		if (ClientCfg.Local)
		{
			TStringsContainer::iterator it(_ReceivedStrings.find(stringId));
			if (it != _ReceivedStrings.end())
			{
				result = it->second;
			}
			else
			{
				result = "StringID = " + toString(stringId);
			}
		}
		else
		{
			TStringsContainer::iterator it(_ReceivedStrings.find(stringId));
			if (it == _ReceivedStrings.end())
			{
				CHashSet<uint>::iterator it(_WaitingStrings.find(stringId));
				if (it == _WaitingStrings.end())
				{
					_WaitingStrings.insert(stringId);
					// need to ask for this string.
					NLMISC::CBitMemStream bms;
					const std::string msgType = "STRING_MANAGER:STRING_RQ";
					if( GenericMsgHeaderMngr.pushNameToStream(msgType,bms) )
					{
						bms.serial( stringId );
						NetMngr.push( bms );
						//nldebug("<CStringManagerClient::getString> sending 'STRING_MANAGER:STRING_RQ' message to server");
					}
					else
					{
						nldebug("<CStringManagerClient::getString> unknown message name 'STRING_MANAGER:STRING_RQ'");
					}
				}

				if (ClientCfg.DebugStringManager)
				{
					char tmp[1024];
					sprintf(tmp, "<WAIT STR %u>", stringId);
					result = ucstring(tmp);
				}
				else
					result.erase(); // = _WaitString;
				return false;
			}

			if (ClientCfg.DebugStringManager)
			{
				char tmp[1024];
				sprintf(tmp, "<STR %u>", stringId);
				result = ucstring(tmp) + it->second;
			}
			else
			{
				result = it->second;
				if (result.size() > 9 && result.substr(0, 9) == ucstring("<missing:")) 
				{
					map<ucstring, ucstring>::iterator itds = _DynStrings.find(result.substr(9, result.size()-10));
					if (itds != _DynStrings.end())
						result = itds->second;
				}
			}
		}

		return true;
	}

	void CStringManagerClient::receiveString(uint32 stringId, const ucstring &str)
	{
		H_AUTO( CStringManagerClient_receiveString )

		//nlinfo("String %u available : [%s]", stringId, str.toString().c_str());

		CHashSet<uint>::iterator	it(_WaitingStrings.find(stringId));
		if (it != _WaitingStrings.end())
		{
			_WaitingStrings.erase(it);
		}
		bool updateCache = true;
		if (_ReceivedStrings.find(stringId) != _ReceivedStrings.end())
		{
			TStringsContainer::iterator it(_ReceivedStrings.find(stringId));
			nlwarning("Receiving stringID %u (%s), already in received string (%s), replacing with new value.",
				stringId,
				str.toString().c_str(),
				it->second.toString().c_str());

			if (it->second != str)
				it->second = str;
			else
				updateCache = false;
		}
		else
		{
			_ReceivedStrings.insert(std::make_pair(stringId, str));
		}

		if (updateCache)
		{
			// update the string cache. DON'T SAVE now cause
			if (_CacheInited && !_CacheFilename.empty())
			{
				CCacheString	cs;
				cs.StringId= stringId;
				cs.String= str;
				_CacheStringToSave.push_back(cs);
			}
		}

		// update the waiting strings
		{
			std::pair<TStringWaitersContainer::iterator, TStringWaitersContainer::iterator> range =
				_StringsWaiters.equal_range(stringId);

			if (range.first != range.second)
			{
				for (; range.first != range.second; ++range.first)
				{
					TStringWaiter &sw = range.first->second;
					*(sw.Result) = str;
				}
				_StringsWaiters.erase(stringId);
			}
		}

		// callback the waiter
		{
			std::pair<TStringCallbacksContainer::iterator, TStringCallbacksContainer::iterator> range =
				_StringsCallbacks.equal_range(stringId);

			if (range.first != range.second)
			{
				for (; range.first != range.second; ++range.first)
				{
					range.first->second->onStringAvailable(stringId, str);
				}
				_StringsCallbacks.erase(stringId);
			}
		}


		// try to complete any pending dyn string
		{
			TDynStringsContainer::iterator first, last;
restartLoop:
			first = _WaitingDynStrings.begin();
			last = _WaitingDynStrings.end();
			for (; first != last; ++first)
			{
				ucstring value;
				uint number = first->first;
				/// Warning: if getDynString() return true, 'first' is erased => don't use it after in this loop
				if (getDynString(number, value))
				{
					//nlinfo("DynString %u available : [%s]", number, value.toString().c_str());
					// this dyn string is now complete !
					// update the waiting dyn strings
					{
						std::pair<TStringWaitersContainer::iterator, TStringWaitersContainer::iterator> range =
							_DynStringsWaiters.equal_range(number);

						if (range.first != range.second)
						{
							for (; range.first != range.second; ++range.first)
							{
								TStringWaiter &sw = range.first->second;
								*(sw.Result) = str;
							}
							_DynStringsWaiters.erase(number);
						}
					}
					// callback the waiting dyn strings
					{
						std::pair<TStringCallbacksContainer::iterator, TStringCallbacksContainer::iterator> range =
							_DynStringsCallbacks.equal_range(number);

						if (range.first != range.second)
						{
							for (; range.first != range.second; ++range.first)
							{
								range.first->second->onDynStringAvailable(number, value);
							}
							_DynStringsCallbacks.erase(number);
						}
					}
					goto restartLoop;
				}
			}
		}
	}

	void CStringManagerClient::flushStringCache()
	{
		if(!_CacheStringToSave.empty())
		{
			NLMISC::COFile file(_CacheFilename, true);
			for(uint i=0;i<_CacheStringToSave.size();i++)
			{
				file.serial(_CacheStringToSave[i].StringId);
				file.serial(_CacheStringToSave[i].String);
			}

			_CacheStringToSave.clear();
		}
	}

	void CStringManagerClient::receiveDynString(NLMISC::CBitMemStream &bms)
	{
		H_AUTO( CStringManagerClient_receiveDynString )

		TDynStringInfo dynInfo;
		dynInfo.Status = TDynStringInfo::received;
		// read the dynamic string Id
		uint32	dynId;
		bms.serial(dynId);

		/// read the base string Id
		bms.serial(dynInfo.StringId);

		// try to build the string
		dynInfo.Message = bms;
		buildDynString(dynInfo);

		if (dynInfo.Status == TDynStringInfo::complete)
		{
			if (!ClientCfg.Light)
			{
				//nlinfo("DynString %u available : [%s]", dynId, dynInfo.String.toString().c_str());
			}

			_ReceivedDynStrings.insert(std::make_pair(dynId, dynInfo));
			// security, if dynstring Message received twice, it is possible that the dynstring is still in waiting list
			_WaitingDynStrings.erase(dynId);

			// update the waiting dyn strings
			{
				std::pair<TStringWaitersContainer::iterator, TStringWaitersContainer::iterator> range =
					_DynStringsWaiters.equal_range(dynId);

				if (range.first != range.second)
				{
					for (; range.first != range.second; ++range.first)
					{
						TStringWaiter &sw = range.first->second;
						*(sw.Result) = dynInfo.String;
					}
					_DynStringsWaiters.erase(dynId);
				}
			}
			// callback the waiting dyn strings
			{
				std::pair<TStringCallbacksContainer::iterator, TStringCallbacksContainer::iterator> range =
					_DynStringsCallbacks.equal_range(dynId);

				if (range.first != range.second)
				{
					for (; range.first != range.second; ++range.first)
					{
						range.first->second->onDynStringAvailable(dynId, dynInfo.String);
					}
					_DynStringsCallbacks.erase(dynId);
				}
			}
		}
		else
			_WaitingDynStrings.insert(std::make_pair(dynId, dynInfo));
	}


	bool	CStringManagerClient::buildDynString(TDynStringInfo &dynInfo)
	{
		H_AUTO( CStringManagerClient_buildDynString )

		if (dynInfo.Status == TDynStringInfo::received)
		{
			if (!getString(dynInfo.StringId, dynInfo.String))
			{
				// can't continue now, need the base string.
				return false;
			}
			// ok, we have the base string, we can serial the parameters
			ucstring::iterator first(dynInfo.String.begin()), last(dynInfo.String.end());
			for (; first != last; ++first)
			{
				if (*first == '%')
				{
					first ++;
					if (first != last && *first != '%')
					{
						// we have a replacement point.
						TParamValue param;
						param.ReplacementPoint = (first-1) - dynInfo.String.begin();
						switch(*first)
						{
						case 's':
							param.Type = string_id;
							try
							{
								dynInfo.Message.serial(param.StringId);
							}
							catch(const Exception &)
							{
								param.StringId = EmptyStringId;
							}
							break;
						case 'i':
							param.Type = integer;
							try
							{
								dynInfo.Message.serial(param.Integer);
							}
							catch(const Exception &)
							{
								param.Integer= 0;
							}
							break;
						case 't':
							param.Type = time;
							try
							{
								dynInfo.Message.serial(param.Time);
							}
							catch(const Exception &)
							{
								param.Time= 0;
							}
							break;
						case '$':
							param.Type = money;
							try
							{
								dynInfo.Message.serial(param.Money);
							}
							catch(const Exception &)
							{
								param.Money= 0;
							}
							break;
						case 'm':
							param.Type = dyn_string_id;
							try
							{
								dynInfo.Message.serial(param.DynStringId);
							}
							catch(const Exception &)
							{
								param.DynStringId= EmptyDynStringId;
							}
							break;
						default:
							nlwarning("Error: unknown replacement tag %%%c", (char)*first);
							return false;
						}

						dynInfo.Params.push_back(param);
					}
				}
			}
			dynInfo.Status = TDynStringInfo::serialized;
		}

		if (dynInfo.Status == TDynStringInfo::serialized)
		{
			// try to retreive all string parameter to build the string.
			ucstring temp;
			temp.reserve(dynInfo.String.size() * 2);
			ucstring::iterator src(dynInfo.String.begin());
			ucstring::iterator move = src;

			std::vector<TParamValue>::iterator first(dynInfo.Params.begin()), last(dynInfo.Params.end());
			for (; first != last; ++first)
			{
				TParamValue &param = *first;
				switch(param.Type)
				{
				case string_id:
					{
						ucstring str;
						if (!getString(param.StringId, str))
							return false;

						// If the string is a player name, we may have to remove the shard name (if the string looks like a player name)
						if(!str.empty() && !PlayerSelectedHomeShardNameWithParenthesis.empty())
						{
							// fast pre-test
							if( str[str.size()-1]==')' )
							{
								// the player name must be at least bigger than the string with ()
								if(str.size()>PlayerSelectedHomeShardNameWithParenthesis.size())
								{
									// If the shard name is the same as the player home shard name, remove it
									uint	len= (uint)PlayerSelectedHomeShardNameWithParenthesis.size();
									uint	start= (uint)str.size()-len;
									if(ucstrnicmp(str, start, len, PlayerSelectedHomeShardNameWithParenthesis)==0)
										str.resize(start);
								}
							}
						}

						// If the string contains a title, then remove it
						ucstring::size_type pos = str.find('$');
						if ( ! str.empty() && pos != ucstring::npos)
						{
							str = CEntityCL::removeTitleFromName(str);
						}

						// append this string
						temp.append(move, src+param.ReplacementPoint);
						temp += str;
						move = dynInfo.String.begin()+param.ReplacementPoint+2;
					}

					break;
				case integer:
					{
						char value[1024];
						sprintf(value, "%d", param.Integer);
						temp.append(move, src+param.ReplacementPoint);
						temp+=ucstring(value);
						move = dynInfo.String.begin()+param.ReplacementPoint+2;
					}
					break;
				case time:
					{
						ucstring value;
						uint32 time = (uint32)param.Time;
						if( time >= (10*60*60) )
						{
							uint32 nbHours = time / (10*60*60);
							time -= nbHours * 10 * 60 * 60;
							value = toString("%d ", nbHours) + CI18N::get("uiMissionTimerHour") + " ";

							uint32 nbMinutes = time / (10*60);
							time -= nbMinutes * 10 * 60;
							value = value + toString("%d ", nbMinutes) + CI18N::get("uiMissionTimerMinute") + " ";
						}
						else if( time >= (10*60) )
						{
							uint32 nbMinutes = time / (10*60);
							time -= nbMinutes * 10 * 60;
							value = value + toString("%d ", nbMinutes) + CI18N::get("uiMissionTimerMinute") + " ";
						}
						uint32 nbSeconds = time / 10;
						value = value + toString("%d", nbSeconds) + CI18N::get("uiMissionTimerSecond");
						temp.append(move, src+param.ReplacementPoint);
						temp+=value;
						move = dynInfo.String.begin()+param.ReplacementPoint+2;
					}
					break;
				case money:
					///\todo nicoB/Boris : this is a temp patch that display money as integers
					{
						char value[1024];
						sprintf(value, "%u", (uint32)param.Money);
						temp.append(move, src+param.ReplacementPoint);
						temp+=ucstring(value);
						move = dynInfo.String.begin()+param.ReplacementPoint+2;
					}
					// TODO
//					temp.append(move, src+param.ReplacementPoint);
//					move = dynInfo.String.begin()+param.ReplacementPoint+2;
					break;
				case dyn_string_id:
					{
						ucstring dynStr;
						if (!getDynString(param.DynStringId, dynStr))
							return false;
						temp.append(move, src+param.ReplacementPoint);
						temp += dynStr;
						move = dynInfo.String.begin()+param.ReplacementPoint+2;
					}
					break;
				 default:
					nlwarning("Unknown parameter type.");
					break;
				}
			}
			// append the rest of the string
			temp.append(move, dynInfo.String.end());

			// apply any 'delete' character in the string and replace double '%'
			{
				uint i =0;
				while (i < temp.size())
				{
					if (temp[i] == 8)
					{
						// remove the 'delete' char AND the next char
						temp.erase(i, 2);
					}
					else if (temp[i] == '%' && i < temp.size()-1 && temp[i+1] == '%')
					{
						temp.erase(i, 1);
					}
					else
						++i;
				}
			}

			dynInfo.Status = TDynStringInfo::complete;
			dynInfo.Message.clear();
			dynInfo.String = temp;
			return true;
		}
		if (dynInfo.Status == TDynStringInfo::complete)
			return true;

		nlwarning("Inconsistent dyn string status : %u", dynInfo.Status);
		return false;
	}


	bool CStringManagerClient::getDynString(uint32 dynStringId, ucstring &result)
	{
		H_AUTO( CStringManagerClient_getDynString )

		if (dynStringId == EmptyDynStringId)
			return true;
		if (ClientCfg.Local)
		{
			TDynStringsContainer::iterator it(_ReceivedDynStrings.find(dynStringId));
			if (it != _ReceivedDynStrings.end())
			{
				result = it->second.String;
			}
			else
			{
				result = "DYNSTR = " + toString(dynStringId);
			}
			return true;
		}
		else
		{
			TDynStringsContainer::iterator it(_ReceivedDynStrings.find(dynStringId));
			if (it != _ReceivedDynStrings.end())
			{
				// ok, we have the string with all the parts.
				if (ClientCfg.DebugStringManager)
				{
					char tmp[1024];
					sprintf(tmp, "<DYNSTR %u>", dynStringId);
					result = ucstring(tmp) + it->second.String;
				}
				else
					result = it->second.String;

				// security/antiloop checking
				it = _WaitingDynStrings.find(dynStringId);
				if (it != _WaitingDynStrings.end())
				{
					nlwarning("CStringManager::getDynString : the string %u is received but still in _WaintingDynStrings !", dynStringId);
					_WaitingDynStrings.erase(it);
				}

				return true;
			}
			else
			{
				// check to see if the string is available now.
				it = _WaitingDynStrings.find(dynStringId);
				if (it == _WaitingDynStrings.end())
				{
					if (ClientCfg.DebugStringManager)
					{
						nlwarning("DynStringID %u is unknown !", dynStringId);
						char tmp[1024];
						sprintf(tmp, "<UNKNOWN DYNSTR %u>", dynStringId);
						result = ucstring(tmp);
					}
					else
						result.erase(); //_WaitString;
					return false;
				}
				if (buildDynString(it->second))
				{
					if (ClientCfg.DebugStringManager)
					{
						char tmp[1024];
						sprintf(tmp, "<DYNSTR %u>", dynStringId);
						result = ucstring(tmp) + it->second.String;
					}
					else
						result = it->second.String;
					_ReceivedDynStrings.insert(std::make_pair(dynStringId, it->second));
					_WaitingDynStrings.erase(it);

					return true;
				}
				if (ClientCfg.DebugStringManager)
				{
					char tmp[1024];
					sprintf(tmp, "<WAIT DYNSTR %u>", dynStringId);
					result = ucstring(tmp);
				}
				else
					result.erase(); // = _WaitString;
				return false;
			}
		}
	}

	// Tool fct to lookup a reference file
	static string	lookupReferenceFile(const std::string &fileName)
	{
		string	referenceFile;
		// special location for the "wk" language
		if(ClientCfg.LanguageCode=="wk")
		{
			referenceFile = "./translation/translated/"+CFile::getFilename(fileName);
			if (!CFile::fileExists(referenceFile))
			{
				nlwarning("Translation file: %s not found", referenceFile.c_str());
				referenceFile.clear();
			}
		}
		else
		{
			referenceFile = CPath::lookup(fileName, false);
		}

		return referenceFile;
	}

	void CLoadProxy::loadStringFile(const std::string &filename, ucstring &text)
	{
		vector<TStringInfo>	reference;
		vector<TStringInfo> addition;
		vector<TStringInfo> diff;

		// get the correct path name of the ref file
		std::string referenceFile= lookupReferenceFile(filename);

		// load the reference file
		if (!referenceFile.empty())
		{
			STRING_MANAGER::loadStringFile(referenceFile, reference, false);
		}

		// try to find the working file
		string workingFile("./translation/work/"+CFile::getFilename(filename));
		if (CFile::fileExists(workingFile))
		{
			STRING_MANAGER::loadStringFile(workingFile, addition, false);
		}
		else
		{
			// no addition file ? just copy the reference into addition
			addition = reference;
		}

		TStringDiffContext context(addition, reference, diff);
		TStringDiff			differ;
		differ.makeDiff(this, context);

		text.erase();
		text = prepareStringFile(context.Diff, true);
	}

	void CLoadProxy::onEquivalent(uint addIndex, uint /* refIndex */, TStringDiffContext &context)
	{
		context.Diff.push_back(context.Addition[addIndex]);
	}
	void CLoadProxy::onAdd(uint addIndex, uint /* refIndex */, TStringDiffContext &context)
	{
		context.Diff.push_back(context.Addition[addIndex]);
		//nldebug("Adding new string '%s' in CI18N", context.Addition[addIndex].Identifier.c_str());
		if (ClientCfg.DebugStringManager)
			context.Diff.back().Text = ucstring("<NEW>")+context.Diff.back().Text;
	}
	void CLoadProxy::onRemove(uint /* addIndex */, uint /* refIndex */, TStringDiffContext &/* context */)
	{
		// nothing to do because we don't insert bad value
	}
	void CLoadProxy::onChanged(uint addIndex, uint /* refIndex */, TStringDiffContext &context)
	{
		// we use the addition value in this case
		context.Diff.push_back(context.Addition[addIndex]);
		//nldebug("Using changed string '%s' in CI18N", context.Addition[addIndex].Identifier.c_str());
		if (ClientCfg.DebugStringManager)
			context.Diff.back().Text = ucstring("<CHG>")+context.Diff.back().Text;
	}
	void CLoadProxy::onSwap(uint /* newIndex */, uint /* refIndex */, TStringDiffContext &/* context */)
	{
		// don't swap.
	}

// ***************************************************************************
/*
	The readed for skill_word_en.txt for example. Do all the job of Diffs with work and translated dirs.
*/
class CReadWorkSheetFile : public TWorkSheetDiff::IDiffCallback
{
public:
	void readWorkSheetFile(const string &filename, ucstring &text)
	{
		TWorksheet	addition;
		TWorksheet	reference;
		TWorksheet	diff;

		// get the correct path name of the ref file
		std::string referenceFile= lookupReferenceFile(filename);

		// load the reference file
		if (!referenceFile.empty())
		{
			STRING_MANAGER::loadExcelSheet(referenceFile, reference);
			STRING_MANAGER::makeHashCode(reference, false);
		}

		// try to find the working file
		string workingFile("./translation/work/"+CFile::getFilename(filename));
		if (CFile::fileExists(workingFile))
		{
			STRING_MANAGER::loadExcelSheet(workingFile, addition);
			STRING_MANAGER::makeHashCode(addition, true);
		}
		else
		{
			text = prepareExcelSheet(reference);
			return;
		}

		if (addition.size() == 0)
			return;

		// check column consistency.
		bool doDiff = true;
		uint i;
		if (reference.ColCount != addition.ColCount)
		{
			nlwarning("Can't check for difference for file %s, column number is not the same!", filename.c_str());
			doDiff = false;
		}
		if (doDiff)
		{
			// check eachg column name
			for (i=0; i<addition.ColCount; ++i)
			{
				if (addition.getData(0, i) != reference.getData(0, i))
				{
					nlwarning("Can't check difference for file %s, collumn name differ !", filename.c_str());
					doDiff = false;
					break;
				}
			}
			if (doDiff)
			{
				// create the column header.
				while (diff.ColCount < addition.ColCount)
					diff.insertColumn(0);
				diff.push_back(*addition.begin());
				TWordsDiffContext	context(addition, reference, diff);
				TWorkSheetDiff	differ;
				differ.makeDiff(this, context, true);
			}
		}

		if (!doDiff)
		{
			diff = reference;
		}

		text = prepareExcelSheet(diff);
	}

	void onEquivalent(uint /* addIndex */, uint refIndex, TWordsDiffContext &context)
	{
		context.Diff.push_back(context.Reference[refIndex]);
	}
	void onAdd(uint addIndex, uint /* refIndex */, TWordsDiffContext &context)
	{
		context.Diff.push_back(context.Addition[addIndex]);
		nlinfo("Using newly sheet row %s", context.Diff.getData(context.Diff.size()-1, 1).toString().c_str());
	}
	void onRemove(uint /* addIndex */, uint /* refIndex */, TWordsDiffContext &/* context */)
	{
		// nothing to do because we don't insert bad value
	}
	void onChanged(uint addIndex, uint /* refIndex */, TWordsDiffContext &context)
	{
		context.Diff.push_back(context.Addition[addIndex]);
		nlinfo("Using changed sheet row %s", context.Diff.getData(context.Diff.size()-1, 1).toString().c_str());
	}
	void onSwap(uint /* newIndex */, uint /* refIndex */, TWordsDiffContext &/* context */)
	{
		// don't swap.
	}
};

// ***************************************************************************
const string	StringClientPackedFileName= "./save/string_client.pack";
// Must Increment this number if change are made to the code (else change not taken into account)
const uint		StringClientPackedVersion= 0;
bool CStringManagerClient::checkWordFileDates(vector<CFileCheck> &fileChecks, const std::vector<std::string> &fileNames, const std::string &languageCode)
{
	fileChecks.resize(fileNames.size());

	// **** First get the date of all the .txt files.
	for(uint i=0;i<fileChecks.size();i++)
	{
		// get the correct path name of the ref file
		std::string referenceFile= lookupReferenceFile(fileNames[i]);
		fileChecks[i].ReferenceDate= referenceFile.empty()?0:CFile::getFileModificationDate(referenceFile);

		// get then one of the working File (NB: 0 is a valid reponse for Final Client: no working file)
		string workingFile("./translation/work/"+CFile::getFilename(fileNames[i]));
		fileChecks[i].AdditionDate= CPath::exists(workingFile)?CFile::getFileModificationDate(workingFile):0;
	}


	// **** Compare with the packed file
	CIFile	packFile;
	if(packFile.open(StringClientPackedFileName))
	{
		CPackHeader	packHeader;
		packFile.serial(packHeader);
		// Compare!
		return fileChecks == packHeader.FileChecks &&
			   languageCode == packHeader.LanguageCode &&
			   StringClientPackedVersion == packHeader.PackedVersion;
	}
	else
		// must rebuild!
		return false;
}


// ***************************************************************************
void CStringManagerClient::initI18NSpecialWords(const std::string &languageCode)
{
	ucstring		womenNameColIdent= string("women_name");
	ucstring		descColIdent= string("description");
	ucstring		descColIdent2= string("description2");

	// List of words to append to the local CI18N system.
	static const char	*specialWords[]=
	{
		// The first is the name of the file. Second is the Key column identifier.
		// 3rd is the extenstion to add to the key for correct matchup (sheet cases)
		"skill",	"skill ID",		"",
		"faction",	"faction",		"",
		"place",	"placeId",		"",
		"item",		"item ID",		".sitem",
		"creature",	"creature ID",	".creature",
		"sbrick",	"sbrick ID",	".sbrick",
		"sphrase",	"sphrase ID",	".sphrase",
		"title",	"title_id",	"",
		"classificationtype",	"classification_type",	"",
		"outpost",	"outpost ID",	"",
	};
	uint	numSpecialWords= sizeof(specialWords) / (3*sizeof(specialWords[0]));

	// Build the file names
	vector<string>		fileNames;
	fileNames.resize(numSpecialWords);
	for(uint i=0;i<numSpecialWords;i++)
	{
		fileNames[i]= string(specialWords[i*3+0]) + "_words_" + languageCode + ".txt";
	}

	// **** Check the file modification dates.
	vector<CFileCheck>	fileChecks;
	bool mustRebuild= !checkWordFileDates(fileChecks, fileNames, languageCode);

	// **** rebuild or load?
	if(mustRebuild)
	{
		for(uint i=0;i<numSpecialWords;i++)
		{
			uint32	profile0= (uint32)ryzomGetLocalTime();

			ucstring ucs;
			std::string fileName = fileNames[i];
			std::string keyExtenstion = specialWords[i*3+2];

			// read the ucstring and make diffs with data in ./translation/work.
			CReadWorkSheetFile	rwsf;
			rwsf.readWorkSheetFile(fileName, ucs);
			if(ucs.empty())
				continue;

			// transform the text into a WorkSheet.
			TWorksheet	ws;
			STRING_MANAGER::readExcelSheet(ucs, ws);

			// Get the Key and Data ColIndex.
			uint	nameColIndex = 0, keyColIndex = 0;
			if( !ws.findCol(ucstring("name"), nameColIndex) )
				continue;
			if( !ws.findCol(ucstring(specialWords[i*3+1]), keyColIndex) )
				continue;

			// Get the women name index if possible.
			uint	womenNameColIndex = std::numeric_limits<uint>::max();
			if( !ws.findCol(womenNameColIdent, womenNameColIndex) )
				womenNameColIndex= std::numeric_limits<uint>::max();

			// Get the description index if possible.
			uint	descColIndex = std::numeric_limits<uint>::max();
			if( !ws.findCol(descColIdent, descColIndex) )
				descColIndex= std::numeric_limits<uint>::max();
			uint	descColIndex2 = std::numeric_limits<uint>::max();
			if( !ws.findCol(descColIdent2, descColIndex2) )
				descColIndex2= std::numeric_limits<uint>::max();

			// For all rows minus the first header one.
			for(uint j=1;j<ws.size();j++)
			{
				// Get the key and name string.
				const ucstring &key=  ws.getData(j, keyColIndex);
				const ucstring &name= ws.getData(j, nameColIndex);
				// Append to the I18N.
				// avoid case problems
				string	keyStr= NLMISC::toLower(key.toString());

				// append the special key extension.
				keyStr+= keyExtenstion;

				// insert in map.
				std::map<std::string, CItem>::iterator	it;
				it= _SpecItem_TempMap.find( keyStr );
				if ( it!=_SpecItem_TempMap.end() )
				{
					nlwarning("Error in initI18NSpecialWords(), %s already exist (not replaced)!", keyStr.c_str());
				}
				else
				{
					_SpecItem_TempMap[keyStr].Name= name;
					// replace all \n in the desc with true \n
					while(strFindReplace(_SpecItem_TempMap[keyStr].Name, "\\n", "\n"));

					// insert in map of Women Name if OK.
					if(womenNameColIndex!=std::numeric_limits<uint>::max())
					{
						const ucstring &womenName= ws.getData(j, womenNameColIndex);
						_SpecItem_TempMap[keyStr].WomenName= womenName;
						// replace all \n in the women name with true \n
						while(strFindReplace(_SpecItem_TempMap[keyStr].WomenName, "\\n", "\n"));
					}

					// insert in map of Description if OK.
					if(descColIndex!=std::numeric_limits<uint>::max())
					{
						const ucstring &desc= ws.getData(j, descColIndex);
						_SpecItem_TempMap[keyStr].Desc= desc;
						// replace all \n in the desc with true \n
						while(strFindReplace(_SpecItem_TempMap[keyStr].Desc, "\\n", "\n"));
					}

					// insert in map of Description2 if OK.
					if(descColIndex2!=std::numeric_limits<uint>::max())
					{
						const ucstring &desc= ws.getData(j, descColIndex2);
						_SpecItem_TempMap[keyStr].Desc2= desc;
						// replace all \n in the desc with true \n
						while(strFindReplace(_SpecItem_TempMap[keyStr].Desc2, "\\n", "\n"));
					}
				}
			}

			nlinfo ("%d seconds for I18N words: Total: %s",  (uint32)(ryzomGetLocalTime()-profile0+500)/1000, fileName.c_str());
		}
	}
	else
	{
		CIFile	packFile;
		nlverify(packFile.open(StringClientPackedFileName));

		// Serial the header
		CPackHeader	packHeader;
		packFile.serial(packHeader);

		// Serial the map
		packFile.serialCont(_SpecItem_TempMap);
	}

	// **** Save if rebuilt
	if(mustRebuild)
	{
		COFile	packFile;
		if(packFile.open(StringClientPackedFileName))
		{
			// Write the header
			CPackHeader	packHeader;
			packHeader.LanguageCode= languageCode;
			packHeader.FileChecks= fileChecks;
			packHeader.PackedVersion= StringClientPackedVersion;
			packFile.serial(packHeader);

			// Serial the map
			packFile.serialCont(_SpecItem_TempMap);
		}
	}
}

// ***************************************************************************
void CStringManagerClient::specialWordsMemoryCompress()
{
	// Convert map to the light structure

	// Count
	uint32 nNbEntries = 0, nLabelSize = 0, nNameDescSize = 0;
	map<string, CItem>::iterator it = _SpecItem_TempMap.begin();
	while (it != _SpecItem_TempMap.end())
	{
		nNbEntries++;
		nLabelSize += (uint32)it->first.size() + 1;
		nNameDescSize += (uint32)it->second.Name.size() + 1;
		nNameDescSize += (uint32)it->second.WomenName.size() + 1;
		nNameDescSize += (uint32)it->second.Desc.size() + 1;
		nNameDescSize += (uint32)it->second.Desc2.size() + 1;
		it++;
	}

	// Make big strings
	_SpecItems.resize(nNbEntries);
	_SpecItem_Labels = new char[nLabelSize];
	_SpecItem_NameDesc = new ucchar[nNameDescSize];

	nNbEntries = 0;
	nLabelSize = 0;
	nNameDescSize = 0;
	it = _SpecItem_TempMap.begin();
	while (it != _SpecItem_TempMap.end())
	{

		if (strnicmp(it->first.c_str(), "bf", 2) == 0)
		{
			uint nDbg = 0;
			nDbg++;
		}

		_SpecItems[nNbEntries].Label = _SpecItem_Labels+nLabelSize;
		strcpy(_SpecItems[nNbEntries].Label, it->first.c_str());
		nLabelSize += (uint32)it->first.size() + 1;

		_SpecItems[nNbEntries].Name = _SpecItem_NameDesc+nNameDescSize;
		memcpy(_SpecItems[nNbEntries].Name, it->second.Name.c_str(), 2*(it->second.Name.size()+1));
		_SpecItems[nNbEntries].Name[it->second.Name.size()] = 0;
		nNameDescSize += (uint32)it->second.Name.size() + 1;

		_SpecItems[nNbEntries].WomenName = _SpecItem_NameDesc+nNameDescSize;
		memcpy(_SpecItems[nNbEntries].WomenName, it->second.WomenName.c_str(), 2*(it->second.WomenName.size()+1));
		_SpecItems[nNbEntries].WomenName[it->second.WomenName.size()] = 0;
		nNameDescSize += (uint32)it->second.WomenName.size() + 1;

		_SpecItems[nNbEntries].Desc = _SpecItem_NameDesc+nNameDescSize;
		memcpy(_SpecItems[nNbEntries].Desc, it->second.Desc.c_str(), 2*(it->second.Desc.size()+1));
		_SpecItems[nNbEntries].Desc[it->second.Desc.size()] = 0;
		nNameDescSize += (uint32)it->second.Desc.size() + 1;

		_SpecItems[nNbEntries].Desc2 = _SpecItem_NameDesc+nNameDescSize;
		memcpy(_SpecItems[nNbEntries].Desc2, it->second.Desc2.c_str(), 2*(it->second.Desc2.size()+1));
		_SpecItems[nNbEntries].Desc2[it->second.Desc2.size()] = 0;
		nNameDescSize += (uint32)it->second.Desc2.size() + 1;

		nNbEntries++;
		it++;
	}
	// No need to sort the vector because the map was already sorted in the good order
	contReset(_SpecItem_TempMap);
	_SpecItem_MemoryCompressed = true;
}

// ***************************************************************************
const ucchar * CStringManagerClient::getSpecialWord(const std::string &label, bool women)
{
	if (label.empty())
	{
		static ucstring	emptyString;
		return emptyString.c_str();
	}

	if (label[0] == '#')
	{
		static ucstring	rawString;
		rawString = label.substr(1, label.size()-1);
		return rawString.c_str();
	}

	// avoid case problems
	static std::string lwrLabel;
	lwrLabel = label;
	strlwr(lwrLabel);

	if (_SpecItem_MemoryCompressed)
	{
		CItemLight tmp;
		tmp.Label = (char*)lwrLabel.c_str();
		vector<CItemLight>::iterator it = lower_bound(_SpecItems.begin(), _SpecItems.end(), tmp, CItemLightComp());

		if (it != _SpecItems.end())
		{
			if (strcmp(it->Label, lwrLabel.c_str()) == 0)
			{
				if( UseFemaleTitles && women )
				{
					ucstring ustr(it->WomenName);
					if( !ustr.empty() )
						return it->WomenName;
				}
				return it->Name;
			}
		}
	}
	else
	{
		map<string,CItem>::iterator it = _SpecItem_TempMap.find(lwrLabel);
		if (it != _SpecItem_TempMap.end())
		{
			if( UseFemaleTitles && women )
				if (!it->second.WomenName.empty())
				return it->second.WomenName.c_str();
			return it->second.Name.c_str();
		}
	}

	static ucstring	badString;

	badString = ucstring(std::string("<NotExist:")+lwrLabel+">");

	return badString.c_str();
}

// ***************************************************************************
const ucchar * CStringManagerClient::getSpecialDesc(const std::string &label)
{
	static ucstring	emptyString;
	if (label.empty())
		return emptyString.c_str();

	// avoid case problems
	static std::string	lwrLabel;
	lwrLabel= label;
	strlwr(lwrLabel);

	if (_SpecItem_MemoryCompressed)
	{
		CItemLight tmp;
		tmp.Label = (char*)lwrLabel.c_str();
		vector<CItemLight>::iterator it = lower_bound(_SpecItems.begin(), _SpecItems.end(), tmp, CItemLightComp());

		if (it != _SpecItems.end())
		{
			if (strcmp(it->Label, lwrLabel.c_str()) == 0)
				return it->Desc;
		}
	}
	else
	{
		map<string,CItem>::iterator it = _SpecItem_TempMap.find(lwrLabel);
		if (it != _SpecItem_TempMap.end())
			return it->second.Desc.c_str();
	}

	return emptyString.c_str();
}

// ***************************************************************************
const ucchar * CStringManagerClient::getSpecialDesc2(const std::string &label)
{
	static ucstring	emptyString;
	if (label.empty())
		return emptyString.c_str();

	// avoid case problems
	static std::string	lwrLabel;
	lwrLabel= label;
	strlwr(lwrLabel);

	if (_SpecItem_MemoryCompressed)
	{
		CItemLight tmp;
		tmp.Label = (char*)lwrLabel.c_str();
		vector<CItemLight>::iterator it = lower_bound(_SpecItems.begin(), _SpecItems.end(), tmp, CItemLightComp());

		if (it != _SpecItems.end())
		{
			if (strcmp(it->Label, lwrLabel.c_str()) == 0)
				return it->Desc2;
		}
	}
	else
	{
		map<string,CItem>::iterator it = _SpecItem_TempMap.find(lwrLabel);
		if (it != _SpecItem_TempMap.end())
		{
			return it->second.Desc2.c_str();
		}
	}

	return emptyString.c_str();
}

// ***************************************************************************
/*const ucchar *CStringManagerClient::getBrickLocalizedName(BRICK_FAMILIES::TBrickFamily e)
{
	return getSpecialWord(BRICK_FAMILIES::toString(e));
}
*/

// ***************************************************************************
const ucchar *CStringManagerClient::getPlaceLocalizedName(const string &placeNameID)
{
	return getSpecialWord(placeNameID);
}

// ***************************************************************************
const ucchar *CStringManagerClient::getFactionLocalizedName(const string &factionNameID)
{
	return getSpecialWord(factionNameID);
}

// ***************************************************************************
const ucchar *CStringManagerClient::getSkillLocalizedName(SKILLS::ESkills e)
{
	return getSpecialWord(SKILLS::toString(e));
}

// ***************************************************************************
const ucchar *CStringManagerClient::getItemLocalizedName(CSheetId id)
{
	return getSpecialWord(id.toString());
}

// ***************************************************************************
const ucchar *CStringManagerClient::getCreatureLocalizedName(NLMISC::CSheetId id)
{
	return getSpecialWord(id.toString());
}

// ***************************************************************************
const ucchar *CStringManagerClient::getSBrickLocalizedName(NLMISC::CSheetId id)
{
	return getSpecialWord(id.toString());
}

// ***************************************************************************
const ucchar *CStringManagerClient::getSPhraseLocalizedName(NLMISC::CSheetId id)
{
	return getSpecialWord(id.toString());
}

// ***************************************************************************
/*const ucchar *CStringManagerClient::getBrickLocalizedDescription(BRICK_FAMILIES::TBrickFamily e)
{
	return getSpecialDesc(BRICK_FAMILIES::toString(e));
}
*/
// ***************************************************************************
const ucchar *CStringManagerClient::getSkillLocalizedDescription(SKILLS::ESkills e)
{
	return getSpecialDesc(SKILLS::toString(e));
}

// ***************************************************************************
const ucchar *CStringManagerClient::getItemLocalizedDescription(CSheetId id)
{
	return getSpecialDesc(id.toString());
}

// ***************************************************************************
const ucchar *CStringManagerClient::getSBrickLocalizedDescription(NLMISC::CSheetId id)
{
	return getSpecialDesc(id.toString());
}

// ***************************************************************************
const ucchar *CStringManagerClient::getSBrickLocalizedCompositionDescription(NLMISC::CSheetId id)
{
	return getSpecialDesc2(id.toString());
}

// ***************************************************************************
const ucchar *CStringManagerClient::getSPhraseLocalizedDescription(NLMISC::CSheetId id)
{
	return getSpecialDesc(id.toString());
}

// ***************************************************************************
const ucchar *CStringManagerClient::getTitleLocalizedName(const ucstring &titleId, bool women)
{
	vector<ucstring> listInfos = getTitleInfos(titleId, women);

	if (listInfos.size() > 0)
	{
		_TitleWords.push_back(listInfos[0]);
		return _TitleWords.back().c_str();
	}
	
	return titleId.c_str();
}

// ***************************************************************************
vector<ucstring> CStringManagerClient::getTitleInfos(const ucstring &titleId, bool women)
{
	//ucstring infosUC;
	//infosUC.fromUtf8(titleId);
	vector<ucstring> listInfos;
	splitUCString(titleId, ucstring("#"), listInfos);

	if (listInfos.size() > 0)
	{
		if (titleId[0] != '#')
		{
			listInfos[0] = getSpecialWord(listInfos[0].toUtf8(), women);
		}
	}

	return listInfos;
}

// ***************************************************************************
const ucchar *CStringManagerClient::getClassificationTypeLocalizedName(EGSPD::CClassificationType::TClassificationType type)
{
	return getSpecialDesc(EGSPD::CClassificationType::toString(type));
}

// ***************************************************************************
const ucchar *CStringManagerClient::getOutpostLocalizedName(NLMISC::CSheetId id)
{
	return getSpecialWord(id.toString());
}

// ***************************************************************************
const ucchar *CStringManagerClient::getOutpostLocalizedDescription(NLMISC::CSheetId id)
{
	return getSpecialDesc(id.toString());
}

// ***************************************************************************
const ucchar *CStringManagerClient::getOutpostBuildingLocalizedName(NLMISC::CSheetId id)
{
	return getSpecialWord(id.toString());
}

// ***************************************************************************
const ucchar *CStringManagerClient::getOutpostBuildingLocalizedDescription(NLMISC::CSheetId id)
{
	return getSpecialDesc(id.toString());
}

// ***************************************************************************
const ucchar *CStringManagerClient::getSquadLocalizedName(NLMISC::CSheetId id)
{
	return getSpecialWord(id.toString());
}

// ***************************************************************************
const ucchar *CStringManagerClient::getSquadLocalizedDescription(NLMISC::CSheetId id)
{
	return getSpecialDesc(id.toString());
}

// ***************************************************************************
void CStringManagerClient::replaceDynString(const ucstring &name, const ucstring &text)
{
	_DynStrings[name] = text;
}


// ***************************************************************************
void CStringManagerClient::replaceSBrickName(NLMISC::CSheetId id, const ucstring &name, const ucstring &desc, const ucstring &desc2)
{
	std::string	label= id.toString();
	if (label.empty())
	{
		return;
	}

	// avoid case problems
	static std::string	lwrLabel;
	lwrLabel= label;
	strlwr(lwrLabel);

	if (_SpecItem_MemoryCompressed)
	{
		ucchar *strName = (ucchar *)name.c_str();
		ucchar *strDesc = (ucchar *)desc.c_str();
		ucchar *strDesc2 = (ucchar *)desc2.c_str();
		CItemLight tmp;
		tmp.Label = (char*)lwrLabel.c_str();
		vector<CItemLight>::iterator it = lower_bound(_SpecItems.begin(), _SpecItems.end(), tmp, CItemLightComp());

		if (it != _SpecItems.end())
		{
			if (strcmp(it->Label, lwrLabel.c_str()) == 0)
			{
				it->Name = strName;
				it->Desc = strDesc;
				it->Desc2 = strDesc2;
			}
			else
			{
				it->Label = tmp.Label;
				it->Name = strName;
				it->Desc = strDesc;
				it->Desc2 = strDesc2;
			}
		}
		else
		{
			tmp.Name = strName;
			tmp.Desc = strDesc;
			tmp.Desc2 = strDesc2;
			_SpecItems.push_back(tmp);
		}
	}
	else
	{
		map<string, CItem>::iterator it(_SpecItem_TempMap.find(lwrLabel));
		if (it != _SpecItem_TempMap.end())
		{
			it->second.Name= name;
			it->second.Desc= desc;
			it->second.Desc2= desc2;
		}
		else
		{
			CItem newItem;
			newItem.Name = name;
			newItem.Desc = desc;
			newItem.Desc2 = desc2;
			_SpecItem_TempMap.insert(pair<string,CItem>(lwrLabel,newItem));
		}
	}
}


} // namespace STRING_MANAGER
