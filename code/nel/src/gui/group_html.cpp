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

//#include <crtdbg.h>

#include "stdpch.h"
#include "nel/gui/group_html.h"

#include <string>
#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/algo.h"
#include "nel/gui/libwww.h"
#include "nel/gui/group_html.h"
#include "nel/gui/group_list.h"
#include "nel/gui/group_container.h"
#include "nel/gui/view_link.h"
#include "nel/gui/ctrl_scroll.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/ctrl_text_button.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/group_paragraph.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/lua_manager.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/dbgroup_combo_box.h"
#include "nel/gui/lua_ihm.h"
#include "nel/misc/i18n.h"
#include "nel/misc/md5.h"
#include "nel/3d/texture_file.h"
#include "nel/misc/big_file.h"
#include "nel/gui/url_parser.h"

using namespace std;
using namespace NLMISC;


// Default timeout to connect a server
#define DEFAULT_RYZOM_CONNECTION_TIMEOUT (30.0)
// Allow up to 10 redirects, then give up
#define DEFAULT_RYZOM_REDIRECT_LIMIT (10)

namespace NLGUI
{

	// Uncomment to see the log about image download
	//#define LOG_DL 1

	CGroupHTML::SWebOptions CGroupHTML::options;


	// Active cURL www transfer
	class CCurlWWWData
	{
		public:
			CCurlWWWData(CURL *curl, const std::string &url)
				: Request(curl), Url(url), Content(""), HeadersSent(NULL)
			{
			}
			~CCurlWWWData()
			{
				if (Request)
					curl_easy_cleanup(Request);

				if (HeadersSent)
					curl_slist_free_all(HeadersSent);
			}

			void setRecvHeader(const std::string &header)
			{
				size_t pos = header.find(": ");
				if (pos == std::string::npos)
					return;

				std::string key = toLower(header.substr(0, pos));
				if (pos != std::string::npos)
				{
					HeadersRecv[key] = header.substr(pos + 2);
					//nlinfo(">> received header '%s' = '%s'", key.c_str(), HeadersRecv[key].c_str());
				}
			}

			// return last received "Location: <url>" header or empty string if no header set
			const std::string getLocationHeader()
			{
				if (HeadersRecv.count("location") > 0)
					return HeadersRecv["location"];

				return "";
			}

		public:
			CURL *Request;

			std::string Url;
			std::string Content;

			// headers sent with curl request, must be released after transfer
			curl_slist * HeadersSent;

			// headers received from curl transfer
			std::map<std::string, std::string> HeadersRecv;
	};

	// Check if domain is on TrustedDomain
	bool CGroupHTML::isTrustedDomain(const string &domain)
	{
		vector<string>::iterator it;
		it = find ( options.trustedDomains.begin(), options.trustedDomains.end(), domain);
		return it != options.trustedDomains.end();
	}

	void CGroupHTML::setImage(CViewBase * view, const string &file)
	{
		CCtrlButton *btn = dynamic_cast<CCtrlButton*>(view);
		if(btn)
		{
			btn->setTexture (file);
			btn->setTexturePushed(file);
			btn->invalidateCoords();
			btn->invalidateContent();
			btn->resetInvalidCoords();
			btn->updateCoords();
			paragraphChange();
		}
		else
		{
			CViewBitmap *btm = dynamic_cast<CViewBitmap*>(view);
			if(btm)
			{
				btm->setTexture (file);
				btm->invalidateCoords();
				btm->invalidateContent();
				btm->resetInvalidCoords();
				btm->updateCoords();
				paragraphChange();
			}
			else
			{
				CGroupCell *btgc = dynamic_cast<CGroupCell*>(view);
				if(btgc)
				{
					btgc->setTexture (file);
					btgc->invalidateCoords();
					btgc->invalidateContent();
					btgc->resetInvalidCoords();
					btgc->updateCoords();
					paragraphChange();
				}
			}
		}
	}

	// Get an url and return the local filename with the path where the url image should be
	string CGroupHTML::localImageName(const string &url)
	{
		string dest = "cache/";
		dest += getMD5((uint8 *)url.c_str(), (uint32)url.size()).toString();
		dest += ".cache";
		return dest;
	}

	// Add a image download request in the multi_curl
	void CGroupHTML::addImageDownload(const string &url, CViewBase *img)
	{
		string finalUrl = getAbsoluteUrl(url);

		// Search if we are not already downloading this url.
		for(uint i = 0; i < Curls.size(); i++)
		{
			if(Curls[i].url == finalUrl)
			{
	#ifdef LOG_DL
				nlwarning("already downloading '%s' img %p", finalUrl.c_str(), img);
	#endif
				Curls[i].imgs.push_back(img);
				return;
			}
		}

		// use requested url for local name
		string dest = localImageName(url);
		string tmpdest = localImageName(url)+".tmp";
	#ifdef LOG_DL
		nlwarning("add to download '%s' dest '%s' img %p", finalUrl.c_str(), dest.c_str(), img);
	#endif

		// erase the tmp file if exists
		if (NLMISC::CFile::fileExists(tmpdest))
			NLMISC::CFile::deleteFile(tmpdest);

		if (!NLMISC::CFile::fileExists(dest))
		{
			if (!MultiCurl)
			{
				nlwarning("Invalid MultiCurl handle, unable to download '%s'", finalUrl.c_str());
				return;
			}

			CURL *curl = curl_easy_init();
			if (!curl)
			{
				nlwarning("Creating cURL handle failed, unable to download '%s'", finalUrl.c_str());
				return;
			}

			FILE *fp = fopen (tmpdest.c_str(), "wb");
			if (fp == NULL)
			{
				curl_easy_cleanup(curl);

				nlwarning("Can't open file '%s' for writing: code=%d '%s'", tmpdest.c_str (), errno, strerror(errno));
				return;
			}
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, true);
			curl_easy_setopt(curl, CURLOPT_URL, finalUrl.c_str());

			curl_easy_setopt(curl, CURLOPT_FILE, fp);

			curl_multi_add_handle(MultiCurl, curl);
			Curls.push_back(CDataDownload(curl, finalUrl, dest, fp, ImgType, img, "", ""));
		#ifdef LOG_DL
			nlwarning("adding handle %x, %d curls", curl, Curls.size());
		#endif
			RunningCurls++;
		}
		else
		{
			setImage(img, dest);
		}
	}

	void CGroupHTML::initImageDownload()
	{
	#ifdef LOG_DL
		nlwarning("Init Image Download");
	#endif
	/*
	// Get current flag
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	// Turn on leak-checking bit
	tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;
	// Set flag to the new value
	_CrtSetDbgFlag( tmpFlag );
	*/
		string pathName = "cache";
		if ( ! CFile::isExists( pathName ) )
			CFile::createDirectory( pathName );
	}


	// Get an url and return the local filename with the path where the bnp should be
	string CGroupHTML::localBnpName(const string &url)
	{
		size_t lastIndex = url.find_last_of("/");
		string dest = "user/"+url.substr(lastIndex+1);
		return dest;
	}

	// Add a bnp download request in the multi_curl, return true if already downloaded
	bool CGroupHTML::addBnpDownload(const string &url, const string &action, const string &script, const string &md5sum)
	{
		// Search if we are not already downloading this url.
		for(uint i = 0; i < Curls.size(); i++)
		{
			if(Curls[i].url == url)
			{
	#ifdef LOG_DL
				nlwarning("already downloading '%s'", url.c_str());
	#endif
				return false;
			}
		}

		string dest = localBnpName(url);
		string tmpdest = localBnpName(url)+".tmp";
	#ifdef LOG_DL
		nlwarning("add to download '%s' dest '%s'", url.c_str(), dest.c_str());
	#endif
		
		// erase the tmp file if exists
		if (NLMISC::CFile::fileExists(tmpdest))
			NLMISC::CFile::deleteFile(tmpdest);

		// create/delete the local file
		if (NLMISC::CFile::fileExists(dest))
		{
			if (action == "override" || action == "delete")
			{
				CFile::setRWAccess(dest);
				NLMISC::CFile::deleteFile(dest);
			}
			else
			{
				return true;
			}
		}
		if (action != "delete")
		{
			if (!MultiCurl)
			{
				nlwarning("Invalid MultiCurl handle, unable to download '%s'", url.c_str());
				return false;
			}

			CURL *curl = curl_easy_init();
			if (!curl)
			{
				nlwarning("Creating cURL handle failed, unable to download '%s'", url.c_str());
				return false;
			}

			FILE *fp = fopen (tmpdest.c_str(), "wb");
			if (fp == NULL)
			{
				curl_easy_cleanup(curl);
				nlwarning("Can't open file '%s' for writing: code=%d '%s'", tmpdest.c_str (), errno, strerror(errno));
				return false;
			}

			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, true);
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

			curl_easy_setopt(curl, CURLOPT_FILE, fp);

			curl_multi_add_handle(MultiCurl, curl);
			Curls.push_back(CDataDownload(curl, url, dest, fp, BnpType, NULL, script, md5sum));
	#ifdef LOG_DL
			nlwarning("adding handle %x, %d curls", curl, Curls.size());
	#endif
			RunningCurls++;
		}
		else
			return true;

		return false;
	}

	void CGroupHTML::initBnpDownload()
	{
		if (!_TrustedDomain)
			return;

	#ifdef LOG_DL
		nlwarning("Init Bnp Download");
	#endif
		string pathName = "user";
		if ( ! CFile::isExists( pathName ) )
			CFile::createDirectory( pathName );
	}

	// Call this evenly to check if an element is downloaded and then manage it
	void CGroupHTML::checkDownloads()
	{
		//nlassert(_CrtCheckMemory());

		if(Curls.empty() && _CurlWWW == NULL)
			return;

		int NewRunningCurls = 0;
		while(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(MultiCurl, &NewRunningCurls))
		{
	#ifdef LOG_DL
			nlwarning("more to do now %d - %d curls", NewRunningCurls, Curls.size());
	#endif
		}
		if(NewRunningCurls < RunningCurls)
		{
			// some download are done, callback them
	#ifdef LOG_DL
			nlwarning ("new %d old %d", NewRunningCurls, RunningCurls);
	#endif
			// check msg
			CURLMsg *msg;
			int msgs_left;
			while ((msg = curl_multi_info_read(MultiCurl, &msgs_left)))
			{
	#ifdef LOG_DL
				nlwarning("> (%s) msgs_left %d", _Id.c_str(), msgs_left);
	#endif
				if (msg->msg == CURLMSG_DONE)
				{
					if (_CurlWWW && _CurlWWW->Request && _CurlWWW->Request == msg->easy_handle)
					{
						CURLcode res = msg->data.result;
						long code;
						curl_easy_getinfo(_CurlWWW->Request, CURLINFO_RESPONSE_CODE, &code);
	#ifdef LOG_DL
						nlwarning("(%s) web transfer '%p' completed with status %d, http %d, url (len %d) '%s'", _Id.c_str(), _CurlWWW->Request, res, code, _CurlWWW->Url.size(), _CurlWWW->Url.c_str());
	#endif

						if (res != CURLE_OK)
						{
							browseError(string("Connection failed with curl error " + string(curl_easy_strerror(res))).c_str());
						}
						else
						if ((code >= 301 && code <= 303) || code == 307 || code == 308)
						{
							if (_RedirectsRemaining < 0)
							{
								browseError(string("Redirect limit reached : " + _URL).c_str());
							}
							else
							{
								receiveCookies(_CurlWWW->Request, HTTPCurrentDomain, _TrustedDomain);

								// redirect, get the location and try browse again
								// we cant use curl redirection because 'addHTTPGetParams()' must be called on new destination
								std::string location(_CurlWWW->getLocationHeader());
								if (location.size() > 0)
								{
	#ifdef LOG_DL
									nlwarning("(%s) request (%d) redirected to (len %d) '%s'", _Id.c_str(), _RedirectsRemaining, location.size(), location.c_str());
	#endif
									location = getAbsoluteUrl(location);
									// throw away this handle and start with new one (easier than reusing)
									requestTerminated();

									_PostNextTime = false;
									_RedirectsRemaining--;

									doBrowse(location.c_str());
								}
								else
								{
									browseError(string("Request was redirected, but location was not set : "+_URL).c_str());
								}
							}
						}
						else
						{
							receiveCookies(_CurlWWW->Request, HTTPCurrentDomain, _TrustedDomain);

							_RedirectsRemaining = DEFAULT_RYZOM_REDIRECT_LIMIT;

							if ( (code < 200 || code >= 300) )
							{
								browseError(string("Connection failed (curl code " + toString((sint32)res) + "), http code " + toString((sint32)code) + ") : " + _CurlWWW->Url).c_str());
							}
							else
							{
								char *ch;
								std::string contentType;
								res = curl_easy_getinfo(_CurlWWW->Request, CURLINFO_CONTENT_TYPE, &ch);
								if (res == CURLE_OK)
								{
									contentType = ch;
								}

								htmlDownloadFinished(_CurlWWW->Content, contentType, code);
							}
							requestTerminated();
						}
					}

					for (vector<CDataDownload>::iterator it=Curls.begin(); it<Curls.end(); it++)
					{
						if(msg->easy_handle == it->curl)
						{
							CURLcode res = msg->data.result;
							long r;
							curl_easy_getinfo(it->curl, CURLINFO_RESPONSE_CODE, &r);
							fclose(it->fp);

	#ifdef LOG_DL
							nlwarning("(%s) transfer '%p' completed with status %d, http %d, url (len %d) '%s'", _Id.c_str(), it->curl, res, r, it->url.size(), it->url.c_str());
	#endif
							curl_multi_remove_handle(MultiCurl, it->curl);
							curl_easy_cleanup(it->curl);

							string tmpfile = it->dest + ".tmp";
							if(res != CURLE_OK || r < 200 || r >= 300 || ((it->md5sum != "") && (it->md5sum != getMD5(tmpfile).toString())))
							{
								NLMISC::CFile::deleteFile(tmpfile.c_str());
							}
							else
							{
								string finalUrl;
								if (it->type == ImgType)
								{
									CFile::moveFile(it->dest.c_str(), tmpfile.c_str());
									//if (lookupLocalFile (finalUrl, file.c_str(), false))
									{
										for(uint i = 0; i < it->imgs.size(); i++)
										{
											setImage(it->imgs[i], it->dest);
										}
									}
								}
								else
								{
									CFile::moveFile(it->dest.c_str(), tmpfile.c_str());
									//if (lookupLocalFile (finalUrl, file.c_str(), false))
									{
										CLuaManager::getInstance().executeLuaScript( it->luaScript, true );
									}
								}
							}

							Curls.erase(it);
							break;
						}
					}
				}
			}
		}
		RunningCurls = NewRunningCurls;
	#ifdef LOG_DL
		if (RunningCurls > 0 || Curls.size() > 0)
			nlwarning("(%s) RunningCurls %d, _Curls %d", _Id.c_str(), RunningCurls, Curls.size());
	#endif
	}


	void CGroupHTML::releaseDownloads()
	{
	#ifdef LOG_DL
		nlwarning("Release Downloads");
	#endif
		if(MultiCurl)
			curl_multi_cleanup(MultiCurl);
	}

	/*
	void dolibcurltest()
	{
		nlwarning("start libcurl test");

		initImageDownload();

		addImageDownload("http://www.ryzom.com/en/");
		addImageDownload("http://www.ryzom.com/fr/");
		addImageDownload("http://www.ryzom.com/de/");

		do
		{
			checkImageDownload();
			nlwarning("continue to sleep");
			nlSleep(300);
		}
		while(RunningCurls);

		releaseImageDownload();

		nlwarning("end libcurl test");
	}
	*/


	class CGroupListAdaptor : public CInterfaceGroup
	{
	public:
		CGroupListAdaptor(const TCtorParam &param)
			: CInterfaceGroup(param)
		{}

	private:
		void updateCoords()
		{
			if (_Parent)
			{
				// Get the W max from the parent
				_W = std::min(_Parent->getMaxWReal(), _Parent->getWReal());
				_WReal = _W;
			}
			CInterfaceGroup::updateCoords();
		}
	};

	// ***************************************************************************

	template<class A> void popIfNotEmpty(A &vect) { if(!vect.empty()) vect.pop_back(); }

	// ***************************************************************************

	void CGroupHTML::beginBuild ()
	{
		if (_Browsing)
		{
			_Connecting = false;

			removeContent ();
		}
		else
			nlwarning("_Browsing = FALSE");
	}


	TStyle CGroupHTML::parseStyle (const string &str_styles)
	{
		TStyle	styles;
		vector<string> elements;
		NLMISC::splitString(str_styles, ";", elements);

		for(uint i = 0; i < elements.size(); ++i)
		{
			vector<string> style;
			NLMISC::splitString(elements[i], ":", style);
			if (style.size() >= 2)
			{
				string fullstyle = style[1];
				for (uint j=2; j < style.size(); j++)
					fullstyle += ":"+style[j];
				styles[trim(style[0])] = trim(fullstyle);
			}
		}

		return styles;
	}

	// ***************************************************************************

	void CGroupHTML::addText (const char * buf, int len)
	{
		if (_Browsing)
		{
			if (_IgnoreText)
				return;

			// Build a UTF8 string
			string inputString(buf, buf+len);
	//		inputString.resize (len);
	//		uint i;
	//		for (i=0; i<(uint)len; i++)
	//			inputString[i] = buf[i];

			if (_ParsingLua && _TrustedDomain)
			{
				// we are parsing a lua script
				_LuaScript += inputString;
				// no more to do
				return;
			}

			// Build a unicode string
			ucstring inputUCString;
			inputUCString.fromUtf8(inputString);

			// Build the final unicode string
			ucstring tmp;
			tmp.reserve(len);
			uint ucLen = (uint)inputUCString.size();
			//uint ucLenWithoutSpace = 0;
			for (uint i=0; i<ucLen; i++)
			{
				ucchar output;
				bool keep;
				// special treatment for 'nbsp' (which is returned as a discreet space)
				if (inputString.size() == 1 && inputString[0] == 32)
				{
					// this is a nbsp entity
					output = inputUCString[i];
					keep = true;
				}
				else
				{
					// not nbsp, use normal white space removal routine
					keep = translateChar (output, inputUCString[i], (tmp.empty())?0:tmp[tmp.size()-1]);
				}

				if (keep)
				{
					tmp.push_back(output);
	/*
					// Break if the string is more than 50 chars long without space
					if (output != ucchar(' '))
					{
						ucLenWithoutSpace++;
						if (ucLenWithoutSpace == 50)
						{
							tmp.push_back(ucchar(' '));
							ucLenWithoutSpace = 0;
						}
					}
					else
					{
						ucLenWithoutSpace = 0;
					}
	*/			}
			}

			if (!tmp.empty())
				addString(tmp);
		}
	}

	// ***************************************************************************

	void CGroupHTML::addLink (uint element_number, const std::vector<bool> &present, const std::vector<const char *> &value)
	{
		if (_Browsing)
		{
			if (element_number == HTML_A)
			{
				if (present[MY_HTML_A_HREF] && value[MY_HTML_A_HREF])
				{
					string suri = value[MY_HTML_A_HREF];
					if(_TrustedDomain && suri.find("ah:") == 0)
					{
						// in ah: command we don't respect the uri standard so the HTAnchor_address doesn't work correctly
						_Link.push_back (suri);
					}
					else if (_TrustedDomain && suri[0] == '#')
					{
						// Direct url (hack for lua beginElement)
						_Link.push_back (suri.substr(1));
					}
					else
					{
						// convert href from "?key=val" into "http://domain.com/?key=val"
						_Link.push_back(getAbsoluteUrl(suri));
					}

					for(uint8 i = MY_HTML_A_ACCESSKEY; i < MY_HTML_A_Z_ACTION_SHORTCUT; i++)
					{
						if (present[i] && value[i])
						{
							string title = value[i];
						//	nlinfo("key %d = %s", i, title.c_str());
						}
					}
					//nlinfo("key of TITLE is : %d", MY_HTML_A_Z_ACTION_PARAMS);
					if (present[MY_HTML_A_Z_ACTION_PARAMS] && value[MY_HTML_A_Z_ACTION_PARAMS])
					{
						string title = value[MY_HTML_A_Z_ACTION_PARAMS];
						_LinkTitle.push_back(title);
					}
					else
						_LinkTitle.push_back("");
				}
				else
				{
					_Link.push_back("");
					_LinkTitle.push_back("");
				}

				
			}
		}
	}

	// ***************************************************************************

	#define getCellsParameters(prefix,inherit) \
	{\
		CGroupHTML::CCellParams cellParams; \
		if (!_CellParams.empty() && inherit) \
		{ \
			cellParams = _CellParams.back(); \
		} \
		if (present[prefix##_BGCOLOR] && value[prefix##_BGCOLOR]) \
			cellParams.BgColor = getColor (value[prefix##_BGCOLOR]); \
		if (present[prefix##_L_MARGIN] && value[prefix##_L_MARGIN]) \
			fromString(value[prefix##_L_MARGIN], cellParams.LeftMargin); \
		if (present[prefix##_NOWRAP]) \
			cellParams.NoWrap = true; \
		if (present[prefix##_ALIGN] && value[prefix##_ALIGN]) \
		{ \
			string align = value[prefix##_ALIGN]; \
			align = strlwr(align); \
			if (align == "left") \
				cellParams.Align = CGroupCell::Left; \
			if (align == "center") \
				cellParams.Align = CGroupCell::Center; \
			if (align == "right") \
				cellParams.Align = CGroupCell::Right; \
		} \
		if (present[prefix##_VALIGN] && value[prefix##_VALIGN]) \
		{ \
			string align = value[prefix##_VALIGN]; \
			align = strlwr(align); \
			if (align == "top") \
				cellParams.VAlign = CGroupCell::Top; \
			if (align == "middle") \
				cellParams.VAlign = CGroupCell::Middle; \
			if (align == "bottom") \
				cellParams.VAlign = CGroupCell::Bottom; \
		} \
		_CellParams.push_back (cellParams); \
	}


	static bool isHexa(char c)
	{
		return isdigit(c) || (tolower(c) >= 'a' && tolower(c) <= 'f');
	}

	static uint8 convertHexa(char c)
	{
		return (uint8) (tolower(c) - (isdigit(c) ? '0' : ('a' - 10)));
	}

	// scan a color component, and return pointer to next position
	static const char *scanColorComponent(const char *src, uint8 &intensity)
	{
		if (!src) return NULL;
		if (!isHexa(*src)) return NULL;
		uint8 value = convertHexa(*src++) << 4;
		if (!isHexa(*src)) return NULL;
		value += convertHexa(*src++);
		intensity = value;
		return src;
	}

	class CNameToCol
	{
	public:
		const char *Name;
		CRGBA Color;
		CNameToCol(const char *name, CRGBA color) : Name(name), Color(color) {}
	};

	static CNameToCol htmlColorNameToRGBA[] =
	{
		CNameToCol("AliceBlue", CRGBA(0xF0, 0xF8, 0xFF)),
		CNameToCol("AntiqueWhite", CRGBA(0xFA, 0xEB, 0xD7)),
		CNameToCol("Aqua", CRGBA(0x00, 0xFF, 0xFF)),
		CNameToCol("Aquamarine", CRGBA(0x7F, 0xFF, 0xD4)),
		CNameToCol("Azure", CRGBA(0xF0, 0xFF, 0xFF)),
		CNameToCol("Beige", CRGBA(0xF5, 0xF5, 0xDC)),
		CNameToCol("Bisque", CRGBA(0xFF, 0xE4, 0xC4)),
		CNameToCol("Black", CRGBA(0x00, 0x00, 0x00)),
		CNameToCol("BlanchedAlmond", CRGBA(0xFF, 0xEB, 0xCD)),
		CNameToCol("Blue", CRGBA(0x00, 0x00, 0xFF)),
		CNameToCol("BlueViolet", CRGBA(0x8A, 0x2B, 0xE2)),
		CNameToCol("Brown", CRGBA(0xA5, 0x2A, 0x2A)),
		CNameToCol("BurlyWood", CRGBA(0xDE, 0xB8, 0x87)),
		CNameToCol("CadetBlue", CRGBA(0x5F, 0x9E, 0xA0)),
		CNameToCol("Chartreuse", CRGBA(0x7F, 0xFF, 0x00)),
		CNameToCol("Chocolate", CRGBA(0xD2, 0x69, 0x1E)),
		CNameToCol("Coral", CRGBA(0xFF, 0x7F, 0x50)),
		CNameToCol("CornflowerBlue", CRGBA(0x64, 0x95, 0xED)),
		CNameToCol("Cornsilk", CRGBA(0xFF, 0xF8, 0xDC)),
		CNameToCol("Crimson", CRGBA(0xDC, 0x14, 0x3C)),
		CNameToCol("Cyan", CRGBA(0x00, 0xFF, 0xFF)),
		CNameToCol("DarkBlue", CRGBA(0x00, 0x00, 0x8B)),
		CNameToCol("DarkCyan", CRGBA(0x00, 0x8B, 0x8B)),
		CNameToCol("DarkGoldenRod", CRGBA(0xB8, 0x86, 0x0B)),
		CNameToCol("DarkGray", CRGBA(0xA9, 0xA9, 0xA9)),
		CNameToCol("DarkGreen", CRGBA(0x00, 0x64, 0x00)),
		CNameToCol("DarkKhaki", CRGBA(0xBD, 0xB7, 0x6B)),
		CNameToCol("DarkMagenta", CRGBA(0x8B, 0x00, 0x8B)),
		CNameToCol("DarkOliveGreen", CRGBA(0x55, 0x6B, 0x2F)),
		CNameToCol("Darkorange", CRGBA(0xFF, 0x8C, 0x00)),
		CNameToCol("DarkOrchid", CRGBA(0x99, 0x32, 0xCC)),
		CNameToCol("DarkRed", CRGBA(0x8B, 0x00, 0x00)),
		CNameToCol("DarkSalmon", CRGBA(0xE9, 0x96, 0x7A)),
		CNameToCol("DarkSeaGreen", CRGBA(0x8F, 0xBC, 0x8F)),
		CNameToCol("DarkSlateBlue", CRGBA(0x48, 0x3D, 0x8B)),
		CNameToCol("DarkSlateGray", CRGBA(0x2F, 0x4F, 0x4F)),
		CNameToCol("DarkTurquoise", CRGBA(0x00, 0xCE, 0xD1)),
		CNameToCol("DarkViolet", CRGBA(0x94, 0x00, 0xD3)),
		CNameToCol("DeepPink", CRGBA(0xFF, 0x14, 0x93)),
		CNameToCol("DeepSkyBlue", CRGBA(0x00, 0xBF, 0xFF)),
		CNameToCol("DimGray", CRGBA(0x69, 0x69, 0x69)),
		CNameToCol("DodgerBlue", CRGBA(0x1E, 0x90, 0xFF)),
		CNameToCol("Feldspar", CRGBA(0xD1, 0x92, 0x75)),
		CNameToCol("FireBrick", CRGBA(0xB2, 0x22, 0x22)),
		CNameToCol("FloralWhite", CRGBA(0xFF, 0xFA, 0xF0)),
		CNameToCol("ForestGreen", CRGBA(0x22, 0x8B, 0x22)),
		CNameToCol("Fuchsia", CRGBA(0xFF, 0x00, 0xFF)),
		CNameToCol("Gainsboro", CRGBA(0xDC, 0xDC, 0xDC)),
		CNameToCol("GhostWhite", CRGBA(0xF8, 0xF8, 0xFF)),
		CNameToCol("Gold", CRGBA(0xFF, 0xD7, 0x00)),
		CNameToCol("GoldenRod", CRGBA(0xDA, 0xA5, 0x20)),
		CNameToCol("Gray", CRGBA(0x80, 0x80, 0x80)),
		CNameToCol("Green", CRGBA(0x00, 0x80, 0x00)),
		CNameToCol("GreenYellow", CRGBA(0xAD, 0xFF, 0x2F)),
		CNameToCol("HoneyDew", CRGBA(0xF0, 0xFF, 0xF0)),
		CNameToCol("HotPink", CRGBA(0xFF, 0x69, 0xB4)),
		CNameToCol("IndianRed ", CRGBA(0xCD, 0x5C, 0x5C)),
		CNameToCol("Indigo  ", CRGBA(0x4B, 0x00, 0x82)),
		CNameToCol("Ivory", CRGBA(0xFF, 0xFF, 0xF0)),
		CNameToCol("Khaki", CRGBA(0xF0, 0xE6, 0x8C)),
		CNameToCol("Lavender", CRGBA(0xE6, 0xE6, 0xFA)),
		CNameToCol("LavenderBlush", CRGBA(0xFF, 0xF0, 0xF5)),
		CNameToCol("LawnGreen", CRGBA(0x7C, 0xFC, 0x00)),
		CNameToCol("LemonChiffon", CRGBA(0xFF, 0xFA, 0xCD)),
		CNameToCol("LightBlue", CRGBA(0xAD, 0xD8, 0xE6)),
		CNameToCol("LightCoral", CRGBA(0xF0, 0x80, 0x80)),
		CNameToCol("LightCyan", CRGBA(0xE0, 0xFF, 0xFF)),
		CNameToCol("LightGoldenRodYellow", CRGBA(0xFA, 0xFA, 0xD2)),
		CNameToCol("LightGrey", CRGBA(0xD3, 0xD3, 0xD3)),
		CNameToCol("LightGreen", CRGBA(0x90, 0xEE, 0x90)),
		CNameToCol("LightPink", CRGBA(0xFF, 0xB6, 0xC1)),
		CNameToCol("LightSalmon", CRGBA(0xFF, 0xA0, 0x7A)),
		CNameToCol("LightSeaGreen", CRGBA(0x20, 0xB2, 0xAA)),
		CNameToCol("LightSkyBlue", CRGBA(0x87, 0xCE, 0xFA)),
		CNameToCol("LightSlateBlue", CRGBA(0x84, 0x70, 0xFF)),
		CNameToCol("LightSlateGray", CRGBA(0x77, 0x88, 0x99)),
		CNameToCol("LightSteelBlue", CRGBA(0xB0, 0xC4, 0xDE)),
		CNameToCol("LightYellow", CRGBA(0xFF, 0xFF, 0xE0)),
		CNameToCol("Lime", CRGBA(0x00, 0xFF, 0x00)),
		CNameToCol("LimeGreen", CRGBA(0x32, 0xCD, 0x32)),
		CNameToCol("Linen", CRGBA(0xFA, 0xF0, 0xE6)),
		CNameToCol("Magenta", CRGBA(0xFF, 0x00, 0xFF)),
		CNameToCol("Maroon", CRGBA(0x80, 0x00, 0x00)),
		CNameToCol("MediumAquaMarine", CRGBA(0x66, 0xCD, 0xAA)),
		CNameToCol("MediumBlue", CRGBA(0x00, 0x00, 0xCD)),
		CNameToCol("MediumOrchid", CRGBA(0xBA, 0x55, 0xD3)),
		CNameToCol("MediumPurple", CRGBA(0x93, 0x70, 0xD8)),
		CNameToCol("MediumSeaGreen", CRGBA(0x3C, 0xB3, 0x71)),
		CNameToCol("MediumSlateBlue", CRGBA(0x7B, 0x68, 0xEE)),
		CNameToCol("MediumSpringGreen", CRGBA(0x00, 0xFA, 0x9A)),
		CNameToCol("MediumTurquoise", CRGBA(0x48, 0xD1, 0xCC)),
		CNameToCol("MediumVioletRed", CRGBA(0xC7, 0x15, 0x85)),
		CNameToCol("MidnightBlue", CRGBA(0x19, 0x19, 0x70)),
		CNameToCol("MintCream", CRGBA(0xF5, 0xFF, 0xFA)),
		CNameToCol("MistyRose", CRGBA(0xFF, 0xE4, 0xE1)),
		CNameToCol("Moccasin", CRGBA(0xFF, 0xE4, 0xB5)),
		CNameToCol("NavajoWhite", CRGBA(0xFF, 0xDE, 0xAD)),
		CNameToCol("Navy", CRGBA(0x00, 0x00, 0x80)),
		CNameToCol("OldLace", CRGBA(0xFD, 0xF5, 0xE6)),
		CNameToCol("Olive", CRGBA(0x80, 0x80, 0x00)),
		CNameToCol("OliveDrab", CRGBA(0x6B, 0x8E, 0x23)),
		CNameToCol("Orange", CRGBA(0xFF, 0xA5, 0x00)),
		CNameToCol("OrangeRed", CRGBA(0xFF, 0x45, 0x00)),
		CNameToCol("Orchid", CRGBA(0xDA, 0x70, 0xD6)),
		CNameToCol("PaleGoldenRod", CRGBA(0xEE, 0xE8, 0xAA)),
		CNameToCol("PaleGreen", CRGBA(0x98, 0xFB, 0x98)),
		CNameToCol("PaleTurquoise", CRGBA(0xAF, 0xEE, 0xEE)),
		CNameToCol("PaleVioletRed", CRGBA(0xD8, 0x70, 0x93)),
		CNameToCol("PapayaWhip", CRGBA(0xFF, 0xEF, 0xD5)),
		CNameToCol("PeachPuff", CRGBA(0xFF, 0xDA, 0xB9)),
		CNameToCol("Peru", CRGBA(0xCD, 0x85, 0x3F)),
		CNameToCol("Pink", CRGBA(0xFF, 0xC0, 0xCB)),
		CNameToCol("Plum", CRGBA(0xDD, 0xA0, 0xDD)),
		CNameToCol("PowderBlue", CRGBA(0xB0, 0xE0, 0xE6)),
		CNameToCol("Purple", CRGBA(0x80, 0x00, 0x80)),
		CNameToCol("Red", CRGBA(0xFF, 0x00, 0x00)),
		CNameToCol("RosyBrown", CRGBA(0xBC, 0x8F, 0x8F)),
		CNameToCol("RoyalBlue", CRGBA(0x41, 0x69, 0xE1)),
		CNameToCol("SaddleBrown", CRGBA(0x8B, 0x45, 0x13)),
		CNameToCol("Salmon", CRGBA(0xFA, 0x80, 0x72)),
		CNameToCol("SandyBrown", CRGBA(0xF4, 0xA4, 0x60)),
		CNameToCol("SeaGreen", CRGBA(0x2E, 0x8B, 0x57)),
		CNameToCol("SeaShell", CRGBA(0xFF, 0xF5, 0xEE)),
		CNameToCol("Sienna", CRGBA(0xA0, 0x52, 0x2D)),
		CNameToCol("Silver", CRGBA(0xC0, 0xC0, 0xC0)),
		CNameToCol("SkyBlue", CRGBA(0x87, 0xCE, 0xEB)),
		CNameToCol("SlateBlue", CRGBA(0x6A, 0x5A, 0xCD)),
		CNameToCol("SlateGray", CRGBA(0x70, 0x80, 0x90)),
		CNameToCol("Snow", CRGBA(0xFF, 0xFA, 0xFA)),
		CNameToCol("SpringGreen", CRGBA(0x00, 0xFF, 0x7F)),
		CNameToCol("SteelBlue", CRGBA(0x46, 0x82, 0xB4)),
		CNameToCol("Tan", CRGBA(0xD2, 0xB4, 0x8C)),
		CNameToCol("Teal", CRGBA(0x00, 0x80, 0x80)),
		CNameToCol("Thistle", CRGBA(0xD8, 0xBF, 0xD8)),
		CNameToCol("Tomato", CRGBA(0xFF, 0x63, 0x47)),
		CNameToCol("Turquoise", CRGBA(0x40, 0xE0, 0xD0)),
		CNameToCol("Violet", CRGBA(0xEE, 0x82, 0xEE)),
		CNameToCol("VioletRed", CRGBA(0xD0, 0x20, 0x90)),
		CNameToCol("Wheat", CRGBA(0xF5, 0xDE, 0xB3)),
		CNameToCol("White", CRGBA(0xFF, 0xFF, 0xFF)),
		CNameToCol("WhiteSmoke", CRGBA(0xF5, 0xF5, 0xF5)),
		CNameToCol("Yellow", CRGBA(0xFF, 0xFF, 0x00)),
		CNameToCol("YellowGreen", CRGBA(0x9A, 0xCD, 0x32))
	};

	// scan a color from a HTML form (#rrggbb format)
	bool scanHTMLColor(const char *src, CRGBA &dest)
	{
		if (!src || *src == '\0') return false;
		if (*src == '#')
		{
			++src;
			CRGBA result;
			src = scanColorComponent(src, result.R); if (!src) return false;
			src = scanColorComponent(src, result.G); if (!src) return false;
			src = scanColorComponent(src, result.B); if (!src) return false;
			src = scanColorComponent(src, result.A);
			if (!src)
			{
				// Alpha is optional
				result.A = 255;
			}
			dest = result;
			return true;
		}
		else
		{
			// slow but should suffice for now
			for(uint k = 0; k < sizeofarray(htmlColorNameToRGBA); ++k)
			{
				if (nlstricmp(src, htmlColorNameToRGBA[k].Name) == 0)
				{
					dest = htmlColorNameToRGBA[k].Color;
					return true;
				}
			}
			return false;
		}
	}

	// ***************************************************************************

	void CGroupHTML::beginElement (uint element_number, const std::vector<bool> &present, const std::vector<const char *> &value)
	{
		if (_Browsing)
		{
			// Paragraph ?
			switch(element_number)
			{
			case HTML_A:
			{
				CStyleParams style;
				style.FontSize = getFontSize();
				style.TextColor = LinkColor;
				style.Underlined = true;
				style.StrikeThrough = getFontStrikeThrough();

				if (present[HTML_A_STYLE] && value[HTML_A_STYLE])
					getStyleParams(value[HTML_A_STYLE], style);

				_FontSize.push_back(style.FontSize);
				_TextColor.push_back(style.TextColor);
				_FontUnderlined.push_back(style.Underlined);
				_FontStrikeThrough.push_back(style.StrikeThrough);
				_GlobalColor.push_back(LinkColorGlobalColor);
				_A.push_back(true);

				if (present[MY_HTML_A_TITLE] && value[MY_HTML_A_TITLE])
					_LinkTitle.push_back(value[MY_HTML_A_TITLE]);
				if (present[MY_HTML_A_CLASS] && value[MY_HTML_A_CLASS])
					_LinkClass.push_back(value[MY_HTML_A_CLASS]);

			}
				break;

			case HTML_DIV:
			{
				if (present[MY_HTML_DIV_NAME] && value[MY_HTML_DIV_NAME])
					_DivName = value[MY_HTML_DIV_NAME];

				string instClass;
				if (present[MY_HTML_DIV_CLASS] && value[MY_HTML_DIV_CLASS])
					instClass = value[MY_HTML_DIV_CLASS];

				// use generic template system
				if (_TrustedDomain && !instClass.empty() && instClass == "ryzom-ui-grouptemplate")
				{
					string id;
					if (present[MY_HTML_DIV_ID] && value[MY_HTML_DIV_ID])
						id = value[MY_HTML_DIV_ID];

					string style;
					if (present[MY_HTML_DIV_STYLE] && value[MY_HTML_DIV_STYLE])
						style = value[MY_HTML_DIV_STYLE];

					typedef pair<string, string> TTmplParam;
					vector<TTmplParam> tmplParams;
					
					string templateName;
					if (!style.empty())
					{
						TStyle styles = parseStyle(style);
						TStyle::iterator	it;
						for (it=styles.begin(); it != styles.end(); it++)
						{
							if ((*it).first == "template")
								templateName = (*it).second;
							else
								tmplParams.push_back(TTmplParam((*it).first, (*it).second));
						}
					}

					if (!templateName.empty())
					{
						string parentId;
						bool haveParentDiv = getDiv() != NULL;
						if (haveParentDiv)
							parentId = getDiv()->getId();
						else
							parentId = _Paragraph->getId();

						CInterfaceGroup *inst = CWidgetManager::getInstance()->getParser()->createGroupInstance(templateName, parentId+":"+id, tmplParams);
						if (inst)
						{
							inst->setId(parentId+":"+id);
							inst->updateCoords();
							if (haveParentDiv)
							{
									inst->setParent(getDiv());
									inst->setParentSize(getDiv());
									inst->setParentPos(getDiv());
									inst->setPosRef(Hotspot_TL);
									inst->setParentPosRef(Hotspot_TL);
									getDiv()->addGroup(inst);
							}
							else
							{
								if (!_Paragraph)
								{
									newParagraph (0);
									paragraphChange ();
								}

								getParagraph()->addChild(inst);
								paragraphChange();
							}
							_Divs.push_back(inst);
						}
					}
				}
			}
				break;

			case HTML_FONT:
			{
				bool found = false;
				if (present[HTML_FONT_COLOR] && value[HTML_FONT_COLOR])
				{
					CRGBA color;
					if (scanHTMLColor(value[HTML_FONT_COLOR], color))
					{
						_TextColor.push_back(color);
						found = true;
					}
				}
				if (!found)
				{
					_TextColor.push_back(_TextColor.empty() ? CRGBA::White : _TextColor.back());
				}

				if (present[HTML_FONT_SIZE] && value[HTML_FONT_SIZE])
				{
					uint fontsize;
					fromString(value[HTML_FONT_SIZE], fontsize);
					_FontSize.push_back(fontsize);
				}
				else
				{
					_FontSize.push_back(_FontSize.empty() ? TextFontSize : _FontSize.back());
				}
			}
				break;
			case HTML_BR:
				addString(ucstring ("\n"));
				break;
			case HTML_BODY:
				{
					if (present[HTML_BODY_BGCOLOR] && value[HTML_BODY_BGCOLOR])
					{
						CRGBA bgColor = getColor (value[HTML_BODY_BGCOLOR]);
						setBackgroundColor (bgColor);
					}
					
					string style;
					if (present[HTML_BODY_STYLE] && value[HTML_BODY_STYLE])
						style = value[HTML_BODY_STYLE];
					
					
					if (!style.empty())
					{
						TStyle styles = parseStyle(style);
						TStyle::iterator	it;

						it = styles.find("background-repeat");
						bool repeat = (it != styles.end() && it->second == "1");
						
						// Webig only
						it = styles.find("background-scale");
						bool scale = (it != styles.end() && it->second == "1");

						it = styles.find("background-image");
						if (it != styles.end())
						{
							string image = it->second;
							string::size_type texExt = strlwr(image).find("url(");
							// Url image
							if (texExt != string::npos)
								// Remove url()
								image = image.substr(4, image.size()-5);
							setBackground (image, scale, repeat);
						}
					}
				}
				break;
			case HTML_FORM:
				{
					// Build the form
					CGroupHTML::CForm form;

					// Get the action name
					if (present[HTML_FORM_ACTION] && value[HTML_FORM_ACTION])
					{
						form.Action = getAbsoluteUrl(string(value[HTML_FORM_ACTION]));
					}
					else
					{
						form.Action = _URL;
					}
					_Forms.push_back(form);
				}
				break;
			case HTML_H1:
				newParagraph(PBeginSpace);
				_FontSize.push_back(H1FontSize);
				_TextColor.push_back(H1Color);
				_GlobalColor.push_back(H1ColorGlobalColor);
				break;
			case HTML_H2:
				newParagraph(PBeginSpace);
				_FontSize.push_back(H2FontSize);
				_TextColor.push_back(H2Color);
				_GlobalColor.push_back(H2ColorGlobalColor);
				break;
			case HTML_H3:
				newParagraph(PBeginSpace);
				_FontSize.push_back(H3FontSize);
				_TextColor.push_back(H3Color);
				_GlobalColor.push_back(H3ColorGlobalColor);
				break;
			case HTML_H4:
				newParagraph(PBeginSpace);
				_FontSize.push_back(H4FontSize);
				_TextColor.push_back(H4Color);
				_GlobalColor.push_back(H4ColorGlobalColor);
				break;
			case HTML_H5:
				newParagraph(PBeginSpace);
				_FontSize.push_back(H5FontSize);
				_TextColor.push_back(H5Color);
				_GlobalColor.push_back(H5ColorGlobalColor);
				break;
			case HTML_H6:
				newParagraph(PBeginSpace);
				_FontSize.push_back(H6FontSize);
				_TextColor.push_back(H6Color);
				_GlobalColor.push_back(H6ColorGlobalColor);
				break;
			case HTML_IMG:
				{
					// Get the string name
					if (present[MY_HTML_IMG_SRC] && value[MY_HTML_IMG_SRC])
					{
						// Get the global color name
						bool globalColor = false;
						if (present[MY_HTML_IMG_GLOBAL_COLOR])
							globalColor = true;

						if (getA() && getParent () && getParent ()->getParent())
						{
							// Tooltip
							const char *tooltip = NULL;
							if (present[MY_HTML_IMG_ALT] && value[MY_HTML_IMG_ALT])
								tooltip = value[MY_HTML_IMG_ALT];

							string params = "name=" + getId() + "|url=" + getLink ();
							addButton(CCtrlButton::PushButton, value[MY_HTML_IMG_SRC], value[MY_HTML_IMG_SRC], value[MY_HTML_IMG_SRC],
								"", globalColor, "browse", params.c_str(), tooltip);
						}
						else
						{
							// Get the option to reload (class==reload)
							bool reloadImg = false;

							string style;
							if (present[MY_HTML_IMG_STYLE] && value[MY_HTML_IMG_STYLE])
								style = value[MY_HTML_IMG_STYLE];

							if (!style.empty())
							{
								TStyle styles = parseStyle(style);
								TStyle::iterator	it;

								it = styles.find("reload");
								if (it != styles.end() && (*it).second == "1")
									reloadImg = true;
							}
							
							addImage (value[MY_HTML_IMG_SRC], globalColor, reloadImg);
						}
					}
				}
				break;
			case HTML_INPUT:
				// Got one form ?
				if (!(_Forms.empty()))
				{
					// read general property
					string templateName;
					string minWidth;

					// Widget template name
					if (present[MY_HTML_INPUT_Z_BTN_TMPL] && value[MY_HTML_INPUT_Z_BTN_TMPL])
						templateName = value[MY_HTML_INPUT_Z_BTN_TMPL];
					// Input name is the new
					if (present[MY_HTML_INPUT_Z_INPUT_TMPL] && value[MY_HTML_INPUT_Z_INPUT_TMPL])
						templateName = value[MY_HTML_INPUT_Z_INPUT_TMPL];
					// Widget minimal width
					if (present[MY_HTML_INPUT_Z_INPUT_WIDTH] && value[MY_HTML_INPUT_Z_INPUT_WIDTH])
						minWidth = value[MY_HTML_INPUT_Z_INPUT_WIDTH];

					// Get the type
					if (present[MY_HTML_INPUT_TYPE] && value[MY_HTML_INPUT_TYPE])
					{
						// Global color flag
						bool globalColor = false;
						if (present[MY_HTML_INPUT_GLOBAL_COLOR])
							globalColor = true;

						// Tooltip
						const char *tooltip = NULL;
						if (present[MY_HTML_INPUT_ALT] && value[MY_HTML_INPUT_ALT])
							tooltip = value[MY_HTML_INPUT_ALT];

						string type = value[MY_HTML_INPUT_TYPE];
						type = strlwr (type);
						if (type == "image")
						{
							// The submit button
							string name;
							string normal;
							string pushed;
							string over;
							if (present[MY_HTML_INPUT_NAME] && value[MY_HTML_INPUT_NAME])
								name = value[MY_HTML_INPUT_NAME];
							if (present[MY_HTML_INPUT_SRC] && value[MY_HTML_INPUT_SRC])
								normal = value[MY_HTML_INPUT_SRC];

							// Action handler parameters : "name=group_html_id|form=id_of_the_form|submit_button=button_name"
							string param = "name=" + getId() + "|form=" + toString (_Forms.size()-1) + "|submit_button=" + name + "|submit_button_type=image";

							// Add the ctrl button
							addButton (CCtrlButton::PushButton, name, normal, pushed.empty()?normal:pushed, over,
								globalColor, "html_submit_form", param.c_str(), tooltip);
						}
						if (type == "button" || type == "submit")
						{
							// The submit button
							string name;
							string text;
							string normal;
							string pushed;
							string over;

							string buttonTemplate(!templateName.empty() ? templateName : DefaultButtonGroup );
							if (present[MY_HTML_INPUT_NAME] && value[MY_HTML_INPUT_NAME])
								name = value[MY_HTML_INPUT_NAME];
							if (present[MY_HTML_INPUT_SRC] && value[MY_HTML_INPUT_SRC])
								normal = value[MY_HTML_INPUT_SRC];
							if (present[MY_HTML_INPUT_VALUE] && value[MY_HTML_INPUT_VALUE])
								text = value[MY_HTML_INPUT_VALUE];

							// Action handler parameters : "name=group_html_id|form=id_of_the_form|submit_button=button_name"
							string param = "name=" + getId() + "|form=" + toString (_Forms.size()-1) + "|submit_button=" + name + "|submit_button_type=submit";
							if (text.size() > 0)
							{
								// escape AH param separator
								string tmp = text;
								while(NLMISC::strFindReplace(tmp, "|", "&#124;"))
									;
								param = param + "|submit_button_value=" + tmp;
							}

							// Add the ctrl button
							if (!_Paragraph)
							{
								newParagraph (0);
								paragraphChange ();
							}

							typedef pair<string, string> TTmplParam;
							vector<TTmplParam> tmplParams;
							tmplParams.push_back(TTmplParam("id", name));
							tmplParams.push_back(TTmplParam("onclick", "html_submit_form"));
							tmplParams.push_back(TTmplParam("onclick_param", param));
							//tmplParams.push_back(TTmplParam("text", text));
							tmplParams.push_back(TTmplParam("active", "true"));
							if (!minWidth.empty())
								tmplParams.push_back(TTmplParam("wmin", minWidth));
							CInterfaceGroup *buttonGroup = CWidgetManager::getInstance()->getParser()->createGroupInstance(buttonTemplate, _Paragraph->getId(), tmplParams);
							if (buttonGroup)
							{

								// Add the ctrl button
								CCtrlTextButton *ctrlButton = dynamic_cast<CCtrlTextButton*>(buttonGroup->getCtrl("button"));
								if (!ctrlButton) ctrlButton = dynamic_cast<CCtrlTextButton*>(buttonGroup->getCtrl("b"));
								if (ctrlButton)
								{
									ctrlButton->setModulateGlobalColorAll (globalColor);

									// Translate the tooltip
									if (tooltip)
									{
										if (CI18N::hasTranslation(tooltip))
										{
											ctrlButton->setDefaultContextHelp(CI18N::get(tooltip));
										}
										else
										{
											ctrlButton->setDefaultContextHelp(ucstring(tooltip));
										}
									}

									ctrlButton->setText(ucstring::makeFromUtf8(text));
								}
								getParagraph()->addChild (buttonGroup);
								paragraphChange ();
							}

	//						addButton (CCtrlTextButton::PushButton, name, normal, pushed.empty()?normal:pushed, over,
	//							globalColor, "html_submit_form", param.c_str(), tooltip);
						}
						else if (type == "text")
						{
							// Get the string name
							string name;
							ucstring ucValue;
							uint size = 120;
							uint maxlength = 1024;
							if (present[MY_HTML_INPUT_NAME] && value[MY_HTML_INPUT_NAME])
								name = value[MY_HTML_INPUT_NAME];
							if (present[MY_HTML_INPUT_SIZE] && value[MY_HTML_INPUT_SIZE])
								fromString(value[MY_HTML_INPUT_SIZE], size);
							if (present[MY_HTML_INPUT_VALUE] && value[MY_HTML_INPUT_VALUE])
								ucValue.fromUtf8(value[MY_HTML_INPUT_VALUE]);
							if (present[MY_HTML_INPUT_MAXLENGTH] && value[MY_HTML_INPUT_MAXLENGTH])
								fromString(value[MY_HTML_INPUT_MAXLENGTH], maxlength);

							string textTemplate(!templateName.empty() ? templateName : DefaultFormTextGroup);
							// Add the editbox
							CInterfaceGroup *textArea = addTextArea (textTemplate, name.c_str (), 1, size/12, false, ucValue, maxlength);
							if (textArea)
							{
								// Add the text area to the form
								CGroupHTML::CForm::CEntry entry;
								entry.Name = name;
								entry.TextArea = textArea;
								_Forms.back().Entries.push_back (entry);
							}
						}
						else if (type == "checkbox")
						{
							// The submit button
							string name;
							string normal = DefaultCheckBoxBitmapNormal;
							string pushed = DefaultCheckBoxBitmapPushed;
							string over = DefaultCheckBoxBitmapOver;
							bool checked = false;
							if (present[MY_HTML_INPUT_NAME] && value[MY_HTML_INPUT_NAME])
								name = value[MY_HTML_INPUT_NAME];
							if (present[MY_HTML_INPUT_SRC] && value[MY_HTML_INPUT_SRC])
								normal = value[MY_HTML_INPUT_SRC];
							checked = (present[MY_HTML_INPUT_CHECKED] && value[MY_HTML_INPUT_CHECKED]);

							// Add the ctrl button
							CCtrlButton *checkbox = addButton (CCtrlButton::ToggleButton, name, normal, pushed, over,
								globalColor, "", "", tooltip);
							if (checkbox)
							{
								checkbox->setPushed (checked);

								// Add the text area to the form
								CGroupHTML::CForm::CEntry entry;
								entry.Name = name;
								entry.Checkbox = checkbox;
								_Forms.back().Entries.push_back (entry);
							}
						}
						else if (type == "hidden")
						{
							if (present[MY_HTML_INPUT_NAME] && value[MY_HTML_INPUT_NAME])
							{
								// Get the name
								string name = value[MY_HTML_INPUT_NAME];

								// Get the value
								ucstring ucValue;
								if (present[MY_HTML_INPUT_VALUE] && value[MY_HTML_INPUT_VALUE])
									ucValue.fromUtf8(value[MY_HTML_INPUT_VALUE]);

								// Add an entry
								CGroupHTML::CForm::CEntry entry;
								entry.Name = name;
								entry.Value = decodeHTMLEntities(ucValue);
								_Forms.back().Entries.push_back (entry);
							}
						}
					}
				}
				break;
			case HTML_SELECT:
				if (!(_Forms.empty()))
				{
					// A select box
					string name;
					if (present[HTML_SELECT_NAME] && value[HTML_SELECT_NAME])
						name = value[HTML_SELECT_NAME];

					CDBGroupComboBox *cb = addComboBox(DefaultFormSelectGroup, name.c_str());
					CGroupHTML::CForm::CEntry entry;
					entry.Name = name;
					entry.ComboBox = cb;
					_Forms.back().Entries.push_back (entry);
				}
			break;
			case HTML_OPTION:
				// Got one form ?
				if (!(_Forms.empty()))
				{
					if (!_Forms.back().Entries.empty())
					{
						// clear the option string
						_SelectOptionStr.clear();

						std::string optionValue;
						bool	 selected = false;
						if (present[HTML_OPTION_VALUE] && value[HTML_OPTION_VALUE])
							optionValue = value[HTML_OPTION_VALUE];
						if (present[HTML_OPTION_SELECTED] && value[HTML_OPTION_SELECTED])
							selected = nlstricmp(value[HTML_OPTION_SELECTED], "selected") == 0;
						_Forms.back().Entries.back().SelectValues.push_back(optionValue);
						if (selected)
						{
							_Forms.back().Entries.back().InitialSelection = (sint)_Forms.back().Entries.back().SelectValues.size() - 1;
						}

					}
				}
				_SelectOption = true;
			break;
			case HTML_LI:
				if (getUL())
				{
					// First LI ?
					if (!_LI)
					{
						_LI = true;
						newParagraph(ULBeginSpace);
					}
					else
					{
						newParagraph(LIBeginSpace);
					}
					ucstring str;
					str += (ucchar)0x2219;
					str += (ucchar)' ';
					addString (str);
					flushString ();
					getParagraph()->setFirstViewIndent(LIIndent);
				}
				break;
			case HTML_P:
				newParagraph(PBeginSpace);
				break;
			case HTML_PRE:
				_PRE.push_back(true);
				break;
			case HTML_TABLE:
				{
					// Get cells parameters
					getCellsParameters (MY_HTML_TABLE, false);

					CGroupTable *table = new CGroupTable(TCtorParam());
					table->BgColor = _CellParams.back().BgColor;

					if (present[MY_HTML_TABLE_WIDTH] && value[MY_HTML_TABLE_WIDTH])
						getPercentage (table->ForceWidthMin, table->TableRatio, value[MY_HTML_TABLE_WIDTH]);
					if (present[MY_HTML_TABLE_BORDER] && value[MY_HTML_TABLE_BORDER])
						fromString(value[MY_HTML_TABLE_BORDER], table->Border);
					if (present[MY_HTML_TABLE_BORDERCOLOR] && value[MY_HTML_TABLE_BORDERCOLOR])
						table->BorderColor = getColor (value[MY_HTML_TABLE_BORDERCOLOR]);
					if (present[MY_HTML_TABLE_CELLSPACING] && value[MY_HTML_TABLE_CELLSPACING])
						fromString(value[MY_HTML_TABLE_CELLSPACING], table->CellSpacing);
					if (present[MY_HTML_TABLE_CELLPADDING] && value[MY_HTML_TABLE_CELLPADDING])
						fromString(value[MY_HTML_TABLE_CELLPADDING], table->CellPadding);

					// Table must fit the container size

					addGroup (table, 0);

					_Tables.push_back(table);

					// Add a cell pointer
					_Cells.push_back(NULL);
					_TR.push_back(false);
				}
				break;
			case HTML_TD:
				{
					// Get cells parameters
					getCellsParameters (MY_HTML_TD, true);

					CGroupTable *table = getTable();
					if (table)
					{
						if (!_Cells.empty())
						{
							_Cells.back() = new CGroupCell(CViewBase::TCtorParam());
							string style;
							if (present[MY_HTML_TD_STYLE] && value[MY_HTML_TD_STYLE])
								style = value[MY_HTML_TD_STYLE];

							// Set the cell parameters
							if (!style.empty())
							{
								TStyle styles = parseStyle(style);
								TStyle::iterator	it;

								it = styles.find("background-repeat");
								_Cells.back()->setTextureTile(it != styles.end());

								// Webig only
								it = styles.find("background-scale");
								_Cells.back()->setTextureScale(it != styles.end());

								it = styles.find("background-image");
								if (it != styles.end())
								{
									nlinfo("found background-image %s", it->second.c_str());
									string image = (*it).second;
									string::size_type texExt = strlwr(image).find("url(");
									// Url image
									if (texExt != string::npos)
									{
										// Remove url()
										image = image.substr(4, image.size()-5);
										addImageDownload(image, _Cells.back());
									// Image in BNP
									}
									else
									{
										_Cells.back()->setTexture(image);
									}
								}
							}

							if (present[MY_HTML_TD_COLSPAN] && value[MY_HTML_TD_COLSPAN])
								fromString(value[MY_HTML_TD_COLSPAN], _Cells.back()->ColSpan);
							if (present[MY_HTML_TD_ROWSPAN] && value[MY_HTML_TD_ROWSPAN])
								fromString(value[MY_HTML_TD_ROWSPAN], _Cells.back()->RowSpan);

							_Cells.back()->BgColor = _CellParams.back().BgColor;
							_Cells.back()->Align = _CellParams.back().Align;
							_Cells.back()->VAlign = _CellParams.back().VAlign;
							_Cells.back()->LeftMargin = _CellParams.back().LeftMargin;
							_Cells.back()->NoWrap = _CellParams.back().NoWrap;
							_Cells.back()->ColSpan = std::max(1, _Cells.back()->ColSpan);
							_Cells.back()->RowSpan = std::max(1, _Cells.back()->RowSpan);

							float temp;
							if (present[MY_HTML_TD_WIDTH] && value[MY_HTML_TD_WIDTH])
								getPercentage (_Cells.back()->WidthWanted, _Cells.back()->TableRatio, value[MY_HTML_TD_WIDTH]);
							if (present[MY_HTML_TD_HEIGHT] && value[MY_HTML_TD_HEIGHT])
								getPercentage (_Cells.back()->Height, temp, value[MY_HTML_TD_HEIGHT]);

							_Cells.back()->NewLine = getTR();
							table->addChild (_Cells.back());
							newParagraph(TDBeginSpace);

							// Reset TR flag
							if (!_TR.empty())
								_TR.back() = false;
						}
					}
				}
				break;
			case HTML_TEXTAREA:
				// Got one form ?
				if (!(_Forms.empty()))
				{
					// read general property
					string templateName;

					// Widget template name
					if (present[MY_HTML_TEXTAREA_Z_INPUT_TMPL] && value[MY_HTML_TEXTAREA_Z_INPUT_TMPL])
						templateName = value[MY_HTML_TEXTAREA_Z_INPUT_TMPL];

					// Get the string name
					_TextAreaName = "";
					_TextAreaRow = 1;
					_TextAreaCols = 10;
					_TextAreaContent = "";
					_TextAreaMaxLength = 1024;
					if (present[MY_HTML_TEXTAREA_NAME] && value[MY_HTML_TEXTAREA_NAME])
						_TextAreaName = value[MY_HTML_TEXTAREA_NAME];
					if (present[MY_HTML_TEXTAREA_ROWS] && value[MY_HTML_TEXTAREA_ROWS])
						fromString(value[MY_HTML_TEXTAREA_ROWS], _TextAreaRow);
					if (present[MY_HTML_TEXTAREA_COLS] && value[MY_HTML_TEXTAREA_COLS])
						fromString(value[MY_HTML_TEXTAREA_COLS], _TextAreaCols);
					if (present[MY_HTML_TEXTAREA_MAXLENGTH] && value[MY_HTML_TEXTAREA_MAXLENGTH])
						fromString(value[MY_HTML_TEXTAREA_MAXLENGTH], _TextAreaMaxLength);

					_TextAreaTemplate = !templateName.empty() ? templateName : DefaultFormTextAreaGroup;
					_TextArea = true;
				}
				break;
			case HTML_TITLE:
				{
					if(!_TitlePrefix.empty())
						_TitleString = _TitlePrefix + " - ";
					else
						_TitleString = "";
					_Title = true;
				}
				break;
			case HTML_I:
				{
					_Localize = true;
				}
				break;
			case HTML_TR:
				{
					// Get cells parameters
					getCellsParameters (MY_HTML_TR, true);

					// Set TR flag
					if (!_TR.empty())
						_TR.back() = true;
				}
				break;
			case HTML_UL:
				_Indent += ULIndent;
				_LI = false;
				endParagraph();
				_UL.push_back(true);
				break;
			case HTML_OBJECT:
				_ObjectType = "";
				_ObjectData = "";
				_ObjectMD5Sum = "";
				_ObjectAction = "";
				if (present[HTML_OBJECT_TYPE] && value[HTML_OBJECT_TYPE])
					_ObjectType = value[HTML_OBJECT_TYPE];
				if (present[HTML_OBJECT_DATA] && value[HTML_OBJECT_DATA])
					_ObjectData = value[HTML_OBJECT_DATA];
				if (present[HTML_OBJECT_ID] && value[HTML_OBJECT_ID])
					_ObjectMD5Sum = value[HTML_OBJECT_ID];
				if (present[HTML_OBJECT_STANDBY] && value[HTML_OBJECT_STANDBY])
					_ObjectAction = value[HTML_OBJECT_STANDBY];
				_Object = true;

				break;
			case HTML_SPAN:
				{
					CStyleParams style;
					style.TextColor = getTextColor();
					style.FontSize = getFontSize();
					style.FontWeight = getFontWeight();
					style.FontOblique = getFontOblique();
					style.Underlined = getFontUnderlined();
					style.StrikeThrough = getFontStrikeThrough();

					if (present[MY_HTML_SPAN_STYLE] && value[MY_HTML_SPAN_STYLE])
						getStyleParams(value[MY_HTML_SPAN_STYLE], style);

					_TextColor.push_back(style.TextColor);
					_FontSize.push_back(style.FontSize);
					_FontWeight.push_back(style.FontWeight);
					_FontOblique.push_back(style.FontOblique);
					_FontUnderlined.push_back(style.Underlined);
					_FontStrikeThrough.push_back(style.StrikeThrough);
				}
				break;

			case HTML_STYLE:
				_IgnoreText = true;
				break;
			}
		}
	}

	// ***************************************************************************

	void CGroupHTML::endElement (uint element_number)
	{
		if (_Browsing)
		{
			// Paragraph ?
			switch(element_number)
			{
			case HTML_FONT:
				popIfNotEmpty (_TextColor);
				popIfNotEmpty (_FontSize);
			break;
			case HTML_A:
				popIfNotEmpty (_FontSize);
				popIfNotEmpty (_TextColor);
				popIfNotEmpty (_FontUnderlined);
				popIfNotEmpty (_FontStrikeThrough);
				popIfNotEmpty (_GlobalColor);
				popIfNotEmpty (_A);
				popIfNotEmpty (_Link);
				popIfNotEmpty (_LinkTitle);
				popIfNotEmpty (_LinkClass);
				break;
			case HTML_H1:
			case HTML_H2:
			case HTML_H3:
			case HTML_H4:
			case HTML_H5:
			case HTML_H6:
				popIfNotEmpty (_FontSize);
				popIfNotEmpty (_TextColor);
				popIfNotEmpty (_GlobalColor);
				endParagraph();
				break;
			case HTML_PRE:
				popIfNotEmpty (_PRE);
				break;
			case HTML_DIV:
				_DivName = "";
				popIfNotEmpty (_Divs);
				break;

			case HTML_TABLE:
				popIfNotEmpty (_CellParams);
				popIfNotEmpty (_TR);
				popIfNotEmpty (_Cells);
				popIfNotEmpty (_Tables);
				endParagraph();
				// Add a cell
				break;
			case HTML_TD:
				popIfNotEmpty (_CellParams);
				if (!_Cells.empty())
					_Cells.back() = NULL;
				break;
			case HTML_TR:
				popIfNotEmpty (_CellParams);
				break;
			case HTML_TEXTAREA:
				{
					_TextArea = false;
					if (!(_Forms.empty()))
					{
						CInterfaceGroup *textArea = addTextArea (_TextAreaTemplate, _TextAreaName.c_str (), _TextAreaRow, _TextAreaCols, true, _TextAreaContent, _TextAreaMaxLength);
						if (textArea)
						{
							// Add the text area to the form
							CGroupHTML::CForm::CEntry entry;
							entry.Name = _TextAreaName;
							entry.TextArea = textArea;
							_Forms.back().Entries.push_back (entry);
						}
					}
				}
				break;
			case HTML_TITLE:
				{
					_Title = false;

					// Get the parent container
					setTitle (_TitleString);
				}
				break;
			case HTML_SELECT:
				{
					_SelectOption = false;
					if (!(_Forms.empty()))
					{
						if (!_Forms.back().Entries.empty())
						{
							CDBGroupComboBox *cb = _Forms.back().Entries.back().ComboBox;
							if (cb)
							{
								cb->setSelectionNoTrigger(_Forms.back().Entries.back().InitialSelection);
								cb->setW(cb->evalContentWidth() + 16);
							}
						}
					}
				}
				break;
			case HTML_OPTION:
				{
					// insert the parsed text into the select control
					CDBGroupComboBox *cb = _Forms.back().Entries.back().ComboBox;
					if (cb)
					{
						cb->addText(_SelectOptionStr);
					}
				}
				break;
			case HTML_I:
				{
					_Localize = false;
				}
				break;
			case HTML_UL:
				if (getUL())
				{
					_Indent -= ULIndent;
					_Indent = std::max(_Indent, (uint)0);
					endParagraph();
					popIfNotEmpty (_UL);
				}
				break;
			case HTML_SPAN:
				popIfNotEmpty (_FontSize);
				popIfNotEmpty (_FontWeight);
				popIfNotEmpty (_FontOblique);
				popIfNotEmpty (_TextColor);
				popIfNotEmpty (_FontUnderlined);
				popIfNotEmpty (_FontStrikeThrough);
				break;
			case HTML_STYLE:
				_IgnoreText = false;
				break;
			case HTML_OBJECT:
				if (_TrustedDomain)
				{
					if (_ObjectType=="application/ryzom-data")
					{
						if (!_ObjectData.empty())
						{
							if (addBnpDownload(_ObjectData, _ObjectAction, _ObjectScript, _ObjectMD5Sum))
							{
								CLuaManager::getInstance().executeLuaScript("\nlocal __ALLREADYDL__=true\n"+_ObjectScript, true);
							}
							_ObjectScript = "";
						}
					}
					_Object = false;
				}
				break;
			}
		}
	}

	// ***************************************************************************
	void CGroupHTML::beginUnparsedElement(const char *buffer, int length)
	{
		string str(buffer, buffer+length);
		if (stricmp(str.c_str(), "lua") == 0)
		{
			// we receive an embeded lua script
			_ParsingLua = _TrustedDomain; // Only parse lua if TrustedDomain
			_LuaScript = "";
		}
	}

	// ***************************************************************************
	void CGroupHTML::endUnparsedElement(const char *buffer, int length)
	{
		string str(buffer, buffer+length);
		if (stricmp(str.c_str(), "lua") == 0)
		{
			if (_ParsingLua && _TrustedDomain)
			{
				_ParsingLua = false;
				// execute the embeded lua script
				_LuaScript = "\nlocal __CURRENT_WINDOW__=\""+this->_Id+"\" \n"+_LuaScript;
				CLuaManager::getInstance().executeLuaScript(_LuaScript, true);
			}
		}
	}


	// ***************************************************************************
	NLMISC_REGISTER_OBJECT(CViewBase, CGroupHTML, std::string, "html");


	// ***************************************************************************
	uint32							CGroupHTML::_GroupHtmlUIDPool= 0;
	CGroupHTML::TGroupHtmlByUIDMap	CGroupHTML::_GroupHtmlByUID;


	// ***************************************************************************
	CGroupHTML::CGroupHTML(const TCtorParam &param)
	:	CGroupScrollText(param),
		_TimeoutValue(DEFAULT_RYZOM_CONNECTION_TIMEOUT),
		_RedirectsRemaining(DEFAULT_RYZOM_REDIRECT_LIMIT)
	{
		// add it to map of group html created
		_GroupHtmlUID= ++_GroupHtmlUIDPool; // valid assigned Id begin to 1!
		_GroupHtmlByUID[_GroupHtmlUID]= this;

		// init
		_TrustedDomain = false;
		_ParsingLua = false;
		_IgnoreText = false;
		_BrowseNextTime = false;
		_PostNextTime = false;
		_Browsing = false;
		_Connecting = false;
		_CurrentViewLink = NULL;
		_CurrentViewImage = NULL;
		_Indent = 0;
		_LI = false;
		_SelectOption = false;
		_GroupListAdaptor = NULL;

		// Register
		CWidgetManager::getInstance()->registerClockMsgTarget(this);

		// HTML parameters
		BgColor = CRGBA::Black;
		ErrorColor = CRGBA(255, 0, 0);
		LinkColor = CRGBA(0, 0, 255);
		TextColor = CRGBA(255, 255, 255);
		H1Color = CRGBA(255, 255, 255);
		H2Color = CRGBA(255, 255, 255);
		H3Color = CRGBA(255, 255, 255);
		H4Color = CRGBA(255, 255, 255);
		H5Color = CRGBA(255, 255, 255);
		H6Color = CRGBA(255, 255, 255);
		ErrorColorGlobalColor = false;
		LinkColorGlobalColor = false;
		TextColorGlobalColor = false;
		H1ColorGlobalColor = false;
		H2ColorGlobalColor = false;
		H3ColorGlobalColor = false;
		H4ColorGlobalColor = false;
		H5ColorGlobalColor = false;
		H6ColorGlobalColor = false;
		TextFontSize = 9;
		H1FontSize = 18;
		H2FontSize = 15;
		H3FontSize = 12;
		H4FontSize = 9;
		H5FontSize = 9;
		H6FontSize = 9;
		LIBeginSpace = 4;
		ULBeginSpace = 12;
		PBeginSpace	 = 12;
		TDBeginSpace = 0;
		LIIndent = -10;
		ULIndent = 30;
		LineSpaceFontFactor = 0.5f;
		DefaultButtonGroup =			"html_text_button";
		DefaultFormTextGroup =			"edit_box_widget";
		DefaultFormTextAreaGroup =		"edit_box_widget_multiline";
		DefaultFormSelectGroup =		"html_form_select_widget";
		DefaultCheckBoxBitmapNormal =	"checkbox_normal.tga";
		DefaultCheckBoxBitmapPushed =	"checkbox_pushed.tga";
		DefaultCheckBoxBitmapOver =		"checkbox_over.tga";
		DefaultBackgroundBitmapView =	"bg";
		clearContext();

		MultiCurl = curl_multi_init();
		RunningCurls = 0;
		_CurlWWW = NULL;

		initImageDownload();
		initBnpDownload();
		initLibWWW();
	}

	// ***************************************************************************

	CGroupHTML::~CGroupHTML()
	{
		//releaseImageDownload();

		// TestYoyo
		//nlinfo("** CGroupHTML Destroy: %x, %s, uid%d", this, _Id.c_str(), _GroupHtmlUID);

		/*	Erase from map of Group HTML (thus requestTerminated() callback won't be called)
			Do it first, just because don't want requestTerminated() to be called while I'm destroying
			(useless and may be dangerous)
		*/
		_GroupHtmlByUID.erase(_GroupHtmlUID);

		// stop browsing
		stopBrowse (); // NB : we don't call updateRefreshButton here, because :
					   // 1) it is useless,
					   // 2) it crashed before when it called getElementFromId (that didn't work when a master group was being removed...). Btw it should work now
					   //     this is why the call to 'updateRefreshButton' has been removed from stopBrowse

		clearContext();
		if (_CurlWWW)
			delete _CurlWWW;
	}

	std::string CGroupHTML::getProperty( const std::string &name ) const
	{
		if( name == "url" )
		{
			return _URL;
		}
		else
		if( name == "title_prefix" )
		{
			return _TitlePrefix.toString();
		}
		else
		if( name == "background_color" )
		{
			return toString( BgColor );
		}
		else
		if( name == "error_color" )
		{
			return toString( ErrorColor );
		}
		else
		if( name == "link_color" )
		{
			return toString( LinkColor );
		}
		else
		if( name == "h1_color" )
		{
			return toString( H1Color );
		}
		else
		if( name == "h2_color" )
		{
			return toString( H2Color );
		}
		else
		if( name == "h3_color" )
		{
			return toString( H3Color );
		}
		else
		if( name == "h4_color" )
		{
			return toString( H4Color );
		}
		else
		if( name == "h5_color" )
		{
			return toString( H5Color );
		}
		else
		if( name == "h6_color" )
		{
			return toString( H6Color );
		}
		else
		if( name == "error_color_global_color" )
		{			
			return toString( ErrorColorGlobalColor );
		}
		else
		if( name == "link_color_global_color" )
		{			
			return toString( LinkColorGlobalColor );
		}
		else
		if( name == "text_color_global_color" )
		{
			return toString( TextColorGlobalColor );
		}
		else
		if( name == "h1_color_global_color" )
		{			
			return toString( H1ColorGlobalColor );
		}
		else
		if( name == "h2_color_global_color" )
		{			
			return toString( H2ColorGlobalColor );
		}
		else
		if( name == "h3_color_global_color" )
		{			
			return toString( H3ColorGlobalColor );
		}
		else
		if( name == "h4_color_global_color" )
		{			
			return toString( H4ColorGlobalColor );
		}
		else
		if( name == "h5_color_global_color" )
		{			
			return toString( H5ColorGlobalColor );
		}
		else
		if( name == "h6_color_global_color" )
		{			
			return toString( H6ColorGlobalColor );
		}
		else
		if( name == "text_font_size" )
		{			
			return toString( TextFontSize );
		}
		else
		if( name == "h1_font_size" )
		{			
			return toString( H1FontSize );
		}
		else
		if( name == "h2_font_size" )
		{			
			return toString( H2FontSize );
		}
		else
		if( name == "h3_font_size" )
		{			
			return toString( H3FontSize );
		}
		else
		if( name == "h4_font_size" )
		{			
			return toString( H4FontSize );
		}
		else
		if( name == "h5_font_size" )
		{			
			return toString( H5FontSize );
		}
		else
		if( name == "h6_font_size" )
		{			
			return toString( H6FontSize );
		}
		else
		if( name == "td_begin_space" )
		{
			return toString( TDBeginSpace );
		}
		else
		if( name == "paragraph_begin_space" )
		{
			return toString( PBeginSpace );
		}
		else
		if( name == "li_begin_space" )
		{
			return toString( LIBeginSpace );
		}
		else
		if( name == "ul_begin_space" )
		{
			return toString( ULBeginSpace );
		}
		else
		if( name == "li_indent" )
		{
			return toString( LIIndent );
		}
		else
		if( name == "ul_indent" )
		{
			return toString( ULIndent );
		}
		else
		if( name == "multi_line_space_factor" )
		{
			return toString( LineSpaceFontFactor );
		}
		else
		if( name == "form_text_area_group" )
		{
			return DefaultFormTextGroup;
		}
		else
		if( name == "form_select_group" )
		{
			return DefaultFormSelectGroup;
		}
		else
		if( name == "checkbox_bitmap_normal" )
		{
			return DefaultCheckBoxBitmapNormal;
		}
		else
		if( name == "checkbox_bitmap_pushed" )
		{
			return DefaultCheckBoxBitmapPushed;
		}
		else
		if( name == "checkbox_bitmap_over" )
		{
			return DefaultCheckBoxBitmapOver;
		}
		else
		if( name == "background_bitmap_view" )
		{
			return DefaultBackgroundBitmapView;
		}
		else
		if( name == "home" )
		{
			return Home;
		}
		else
		if( name == "browse_next_time" )
		{
			return toString( _BrowseNextTime );
		}
		else
		if( name == "browse_tree" )
		{
			return _BrowseTree;
		}
		else
		if( name == "browse_undo" )
		{
			return _BrowseUndoButton;
		}
		else
		if( name == "browse_redo" )
		{
			return _BrowseRedoButton;
		}
		else
		if( name == "browse_refresh" )
		{
			return _BrowseRefreshButton;
		}
		else
		if( name == "timeout" )
		{
			return toString( _TimeoutValue );
		}
		else
			return CGroupScrollText::getProperty( name );
	}

	void CGroupHTML::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "url" )
		{
			_URL = value;
			return;
		}
		else
		if( name == "title_prefix" )
		{
			_TitlePrefix = value;
			return;
		}
		else
		if( name == "background_color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				BgColor = c;
			return;
		}
		else
		if( name == "error_color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				ErrorColor = c;
			return;
		}
		else
		if( name == "link_color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				LinkColor = c;
			return;
		}
		else
		if( name == "h1_color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				H1Color = c;
			return;
		}
		else
		if( name == "h2_color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				H2Color = c;
			return;
		}
		else
		if( name == "h3_color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				H3Color = c;
			return;
		}
		else
		if( name == "h4_color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				H4Color = c;
			return;
		}
		else
		if( name == "h5_color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				H5Color = c;
			return;
		}
		else
		if( name == "h6_color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				H6Color = c;
			return;
		}
		else
		if( name == "error_color_global_color" )
		{			
			bool b;
			if( fromString( value, b ) )
				ErrorColorGlobalColor = b;
			return;
		}
		else
		if( name == "link_color_global_color" )
		{			
			bool b;
			if( fromString( value, b ) )
				LinkColorGlobalColor = b;
			return;
		}
		else
		if( name == "text_color_global_color" )
		{
			bool b;
			if( fromString( value, b ) )
				TextColorGlobalColor = b;
			return;
		}
		else
		if( name == "h1_color_global_color" )
		{			
			bool b;
			if( fromString( value, b ) )
				H1ColorGlobalColor = b;
			return;
		}
		else
		if( name == "h2_color_global_color" )
		{			
			bool b;
			if( fromString( value, b ) )
				H2ColorGlobalColor = b;
			return;
		}
		else
		if( name == "h3_color_global_color" )
		{			
			bool b;
			if( fromString( value, b ) )
				H3ColorGlobalColor = b;
			return;
		}
		else
		if( name == "h4_color_global_color" )
		{			
			bool b;
			if( fromString( value, b ) )
				H4ColorGlobalColor = b;
			return;
		}
		else
		if( name == "h5_color_global_color" )
		{			
			bool b;
			if( fromString( value, b ) )
				H5ColorGlobalColor = b;
			return;
		}
		else
		if( name == "h6_color_global_color" )
		{			
			bool b;
			if( fromString( value, b ) )
				H6ColorGlobalColor = b;
			return;
		}
		else
		if( name == "text_font_size" )
		{			
			uint i;
			if( fromString( value, i ) )
				TextFontSize = i;
			return;
		}
		else
		if( name == "h1_font_size" )
		{			
			uint i;
			if( fromString( value, i ) )
				H1FontSize = i;
			return;
		}
		else
		if( name == "h2_font_size" )
		{			
			uint i;
			if( fromString( value, i ) )
				H2FontSize = i;
			return;
		}
		else
		if( name == "h3_font_size" )
		{			
			uint i;
			if( fromString( value, i ) )
				H3FontSize = i;
			return;
		}
		else
		if( name == "h4_font_size" )
		{			
			uint i;
			if( fromString( value, i ) )
				H4FontSize = i;
			return;
		}
		else
		if( name == "h5_font_size" )
		{			
			uint i;
			if( fromString( value, i ) )
				H5FontSize = i;
			return;
		}
		else
		if( name == "h6_font_size" )
		{			
			uint i;
			if( fromString( value, i ) )
				H6FontSize = i;
			return;
		}
		else
		if( name == "td_begin_space" )
		{
			uint i;
			if( fromString( value, i ) )
				TDBeginSpace = i;
			return;
		}
		else
		if( name == "paragraph_begin_space" )
		{
			uint i;
			if( fromString( value, i ) )
				PBeginSpace = i;
			return;
		}
		else
		if( name == "li_begin_space" )
		{
			uint i;
			if( fromString( value, i ) )
				LIBeginSpace = i;
			return;
		}
		else
		if( name == "ul_begin_space" )
		{
			uint i;
			if( fromString( value, i ) )
				ULBeginSpace = i;
			return;
		}
		else
		if( name == "li_indent" )
		{
			uint i;
			if( fromString( value, i ) )
				LIIndent = i;
			return;
		}
		else
		if( name == "ul_indent" )
		{
			uint i;
			if( fromString( value, i ) )
				ULIndent = i;
			return;
		}
		else
		if( name == "multi_line_space_factor" )
		{
			float f;
			if( fromString( value, f ) )
				LineSpaceFontFactor = f;
			return;
		}
		else
		if( name == "form_text_area_group" )
		{
			DefaultFormTextGroup = value;
			return;
		}
		else
		if( name == "form_select_group" )
		{
			DefaultFormSelectGroup = value;
			return;
		}
		else
		if( name == "checkbox_bitmap_normal" )
		{
			DefaultCheckBoxBitmapNormal = value;
			return;
		}
		else
		if( name == "checkbox_bitmap_pushed" )
		{
			DefaultCheckBoxBitmapPushed = value;
			return;
		}
		else
		if( name == "checkbox_bitmap_over" )
		{
			DefaultCheckBoxBitmapOver = value;
			return;
		}
		else
		if( name == "background_bitmap_view" )
		{
			DefaultBackgroundBitmapView = value;
			return;
		}
		else
		if( name == "home" )
		{
			Home = value;
			return;
		}
		else
		if( name == "browse_next_time" )
		{
			bool b;
			if( fromString( value, b ) )
				_BrowseNextTime = b;
			return;
		}
		else
		if( name == "browse_tree" )
		{
			_BrowseTree = value;
			return;
		}
		else
		if( name == "browse_undo" )
		{
			_BrowseUndoButton = value;
			return;
		}
		else
		if( name == "browse_redo" )
		{
			_BrowseRedoButton = value;
			return;
		}
		else
		if( name == "browse_refresh" )
		{
			_BrowseRefreshButton = value;
			return;
		}
		else
		if( name == "timeout" )
		{
			double d;
			if( fromString( value, d ) )
				_TimeoutValue = d;
			return;
		}
		else
			CGroupScrollText::setProperty( name, value );
	}

	xmlNodePtr CGroupHTML::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CGroupScrollText::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "html" );
		xmlSetProp( node, BAD_CAST "url", BAD_CAST _URL.c_str() );
		xmlSetProp( node, BAD_CAST "title_prefix", BAD_CAST _TitlePrefix.toString().c_str() );
		xmlSetProp( node, BAD_CAST "background_color", BAD_CAST toString( BgColor ).c_str() );
		xmlSetProp( node, BAD_CAST "error_color", BAD_CAST toString( ErrorColor ).c_str() );
		xmlSetProp( node, BAD_CAST "link_color", BAD_CAST toString( LinkColor ).c_str() );
		xmlSetProp( node, BAD_CAST "background_color", BAD_CAST toString( BgColor ).c_str() );
		xmlSetProp( node, BAD_CAST "h1_color", BAD_CAST toString( H1Color ).c_str() );
		xmlSetProp( node, BAD_CAST "h2_color", BAD_CAST toString( H2Color ).c_str() );
		xmlSetProp( node, BAD_CAST "h3_color", BAD_CAST toString( H3Color ).c_str() );
		xmlSetProp( node, BAD_CAST "h4_color", BAD_CAST toString( H4Color ).c_str() );
		xmlSetProp( node, BAD_CAST "h5_color", BAD_CAST toString( H5Color ).c_str() );
		xmlSetProp( node, BAD_CAST "h6_color", BAD_CAST toString( H6Color ).c_str() );
		
		xmlSetProp( node, BAD_CAST "error_color_global_color",
			BAD_CAST toString( ErrorColorGlobalColor ).c_str() );
		xmlSetProp( node, BAD_CAST "link_color_global_color",
			BAD_CAST toString( LinkColorGlobalColor ).c_str() );
		xmlSetProp( node, BAD_CAST "text_color_global_color",
			BAD_CAST toString( TextColorGlobalColor ).c_str() );
		xmlSetProp( node, BAD_CAST "h1_color_global_color",
			BAD_CAST toString( H1ColorGlobalColor ).c_str() );
		xmlSetProp( node, BAD_CAST "h2_color_global_color",
			BAD_CAST toString( H2ColorGlobalColor ).c_str() );
		xmlSetProp( node, BAD_CAST "h3_color_global_color",
			BAD_CAST toString( H3ColorGlobalColor ).c_str() );
		xmlSetProp( node, BAD_CAST "h4_color_global_color",
			BAD_CAST toString( H4ColorGlobalColor ).c_str() );
		xmlSetProp( node, BAD_CAST "h5_color_global_color",
			BAD_CAST toString( H5ColorGlobalColor ).c_str() );
		xmlSetProp( node, BAD_CAST "h6_color_global_color",
			BAD_CAST toString( H6ColorGlobalColor ).c_str() );

		xmlSetProp( node, BAD_CAST "text_font_size", BAD_CAST toString( TextFontSize ).c_str() );
		xmlSetProp( node, BAD_CAST "h1_font_size", BAD_CAST toString( H1FontSize ).c_str() );
		xmlSetProp( node, BAD_CAST "h2_font_size", BAD_CAST toString( H2FontSize ).c_str() );
		xmlSetProp( node, BAD_CAST "h3_font_size", BAD_CAST toString( H3FontSize ).c_str() );
		xmlSetProp( node, BAD_CAST "h4_font_size", BAD_CAST toString( H4FontSize ).c_str() );
		xmlSetProp( node, BAD_CAST "h5_font_size", BAD_CAST toString( H5FontSize ).c_str() );
		xmlSetProp( node, BAD_CAST "h6_font_size", BAD_CAST toString( H6FontSize ).c_str() );
		xmlSetProp( node, BAD_CAST "td_begin_space", BAD_CAST toString( TDBeginSpace ).c_str() );
		xmlSetProp( node, BAD_CAST "paragraph_begin_space", BAD_CAST toString( PBeginSpace ).c_str() );
		xmlSetProp( node, BAD_CAST "li_begin_space", BAD_CAST toString( LIBeginSpace ).c_str() );
		xmlSetProp( node, BAD_CAST "ul_begin_space", BAD_CAST toString( ULBeginSpace ).c_str() );
		xmlSetProp( node, BAD_CAST "li_indent", BAD_CAST toString( LIIndent ).c_str() );
		xmlSetProp( node, BAD_CAST "ul_indent", BAD_CAST toString( ULIndent ).c_str() );
		xmlSetProp( node, BAD_CAST "multi_line_space_factor", BAD_CAST toString( LineSpaceFontFactor ).c_str() );
		xmlSetProp( node, BAD_CAST "form_text_area_group", BAD_CAST DefaultFormTextGroup.c_str() );
		xmlSetProp( node, BAD_CAST "form_select_group", BAD_CAST DefaultFormSelectGroup.c_str() );
		xmlSetProp( node, BAD_CAST "checkbox_bitmap_normal", BAD_CAST DefaultCheckBoxBitmapNormal.c_str() );
		xmlSetProp( node, BAD_CAST "checkbox_bitmap_pushed", BAD_CAST DefaultCheckBoxBitmapPushed.c_str() );
		xmlSetProp( node, BAD_CAST "checkbox_bitmap_over", BAD_CAST DefaultCheckBoxBitmapOver.c_str() );
		xmlSetProp( node, BAD_CAST "background_bitmap_view", BAD_CAST DefaultBackgroundBitmapView.c_str() );
		xmlSetProp( node, BAD_CAST "home", BAD_CAST Home.c_str() );
		xmlSetProp( node, BAD_CAST "browse_next_time", BAD_CAST toString( _BrowseNextTime ).c_str() );
		xmlSetProp( node, BAD_CAST "browse_tree", BAD_CAST _BrowseTree.c_str() );
		xmlSetProp( node, BAD_CAST "browse_undo", BAD_CAST _BrowseUndoButton.c_str() );
		xmlSetProp( node, BAD_CAST "browse_redo", BAD_CAST _BrowseRedoButton.c_str() );
		xmlSetProp( node, BAD_CAST "browse_refresh", BAD_CAST _BrowseRefreshButton.c_str() );
		xmlSetProp( node, BAD_CAST "timeout", BAD_CAST toString( _TimeoutValue ).c_str() );

		return node;
	}

	// ***************************************************************************

	bool CGroupHTML::parse(xmlNodePtr cur,CInterfaceGroup *parentGroup)
	{
		nlassert( CWidgetManager::getInstance()->isClockMsgTarget(this));


		if(!CGroupScrollText::parse(cur, parentGroup))
			return false;

		// TestYoyo
		//nlinfo("** CGroupHTML parsed Ok: %x, %s, %s, uid%d", this, _Id.c_str(), typeid(this).name(), _GroupHtmlUID);

		CXMLAutoPtr ptr;

		// Get the url
		ptr = xmlGetProp (cur, (xmlChar*)"url");
		if (ptr)
			_URL = (const char*)ptr;

		// Bkup default for undo/redo
		_AskedUrl= _URL;

		ptr = xmlGetProp (cur, (xmlChar*)"title_prefix");
		if (ptr)
			_TitlePrefix = CI18N::get((const char*)ptr);

		// Parameters
		ptr = xmlGetProp (cur, (xmlChar*)"background_color");
		if (ptr)
			BgColor = convertColor(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"error_color");
		if (ptr)
			ErrorColor = convertColor(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"link_color");
		if (ptr)
			LinkColor = convertColor(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"text_color");
		if (ptr)
			TextColor = convertColor(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"h1_color");
		if (ptr)
			H1Color = convertColor(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"h2_color");
		if (ptr)
			H2Color = convertColor(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"h3_color");
		if (ptr)
			H3Color = convertColor(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"h4_color");
		if (ptr)
			H4Color = convertColor(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"h5_color");
		if (ptr)
			H5Color = convertColor(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"h6_color");
		if (ptr)
			H6Color = convertColor(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"error_color_global_color");
		if (ptr)
			ErrorColorGlobalColor = convertBool(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"link_color_global_color");
		if (ptr)
			LinkColorGlobalColor = convertBool(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"text_color_global_color");
		if (ptr)
			TextColorGlobalColor = convertBool(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"h1_color_global_color");
		if (ptr)
			H1ColorGlobalColor = convertBool(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"h2_color_global_color");
		if (ptr)
			H2ColorGlobalColor = convertBool(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"h3_color_global_color");
		if (ptr)
			H3ColorGlobalColor = convertBool(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"h4_color_global_color");
		if (ptr)
			H4ColorGlobalColor = convertBool(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"h5_color_global_color");
		if (ptr)
			H5ColorGlobalColor = convertBool(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"h6_color_global_color");
		if (ptr)
			H6ColorGlobalColor = convertBool(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"text_font_size");
		if (ptr)
			fromString((const char*)ptr, TextFontSize);
		ptr = xmlGetProp (cur, (xmlChar*)"h1_font_size");
		if (ptr)
			fromString((const char*)ptr, H1FontSize);
		ptr = xmlGetProp (cur, (xmlChar*)"h2_font_size");
		if (ptr)
			fromString((const char*)ptr, H2FontSize);
		ptr = xmlGetProp (cur, (xmlChar*)"h3_font_size");
		if (ptr)
			fromString((const char*)ptr, H3FontSize);
		ptr = xmlGetProp (cur, (xmlChar*)"h4_font_size");
		if (ptr)
			fromString((const char*)ptr, H4FontSize);
		ptr = xmlGetProp (cur, (xmlChar*)"h5_font_size");
		if (ptr)
			fromString((const char*)ptr, H5FontSize);
		ptr = xmlGetProp (cur, (xmlChar*)"h6_font_size");
		if (ptr)
			fromString((const char*)ptr, H6FontSize);
		ptr = xmlGetProp (cur, (xmlChar*)"td_begin_space");
		if (ptr)
			fromString((const char*)ptr, TDBeginSpace);
		ptr = xmlGetProp (cur, (xmlChar*)"paragraph_begin_space");
		if (ptr)
			fromString((const char*)ptr, PBeginSpace);
		ptr = xmlGetProp (cur, (xmlChar*)"li_begin_space");
		if (ptr)
			fromString((const char*)ptr, LIBeginSpace);
		ptr = xmlGetProp (cur, (xmlChar*)"ul_begin_space");
		if (ptr)
			fromString((const char*)ptr, ULBeginSpace);
		ptr = xmlGetProp (cur, (xmlChar*)"li_indent");
		if (ptr)
			fromString((const char*)ptr, LIIndent);
		ptr = xmlGetProp (cur, (xmlChar*)"ul_indent");
		if (ptr)
			fromString((const char*)ptr, ULIndent);
		ptr = xmlGetProp (cur, (xmlChar*)"multi_line_space_factor");
		if (ptr)
			fromString((const char*)ptr, LineSpaceFontFactor);
		ptr = xmlGetProp (cur, (xmlChar*)"form_text_group");
		if (ptr)
			DefaultFormTextGroup = (const char*)(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"form_text_area_group");
		if (ptr)
			DefaultFormTextAreaGroup = (const char*)(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"form_select_group");
		if (ptr)
			DefaultFormSelectGroup = (const char*)(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"checkbox_bitmap_normal");
		if (ptr)
			DefaultCheckBoxBitmapNormal = (const char*)(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"checkbox_bitmap_pushed");
		if (ptr)
			DefaultCheckBoxBitmapPushed = (const char*)(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"checkbox_bitmap_over");
		if (ptr)
			DefaultCheckBoxBitmapOver = (const char*)(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"background_bitmap_view");
		if (ptr)
			DefaultBackgroundBitmapView = (const char*)(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"home");
		if (ptr)
			Home = (const char*)(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"browse_next_time");
		if (ptr)
			_BrowseNextTime = convertBool(ptr);
		ptr = xmlGetProp (cur, (xmlChar*)"browse_tree");
		if(ptr)
			_BrowseTree = (const char*)ptr;
		ptr = xmlGetProp (cur, (xmlChar*)"browse_undo");
		if(ptr)
			_BrowseUndoButton= (const char*)ptr;
		ptr = xmlGetProp (cur, (xmlChar*)"browse_redo");
		if(ptr)
			_BrowseRedoButton = (const char*)ptr;
		ptr = xmlGetProp (cur, (xmlChar*)"browse_refresh");
		if(ptr)
			_BrowseRefreshButton = (const char*)ptr;
		ptr = xmlGetProp (cur, (xmlChar*)"timeout");
		if(ptr)
			fromString((const char*)ptr, _TimeoutValue);

		return true;
	}

	// ***************************************************************************

	bool CGroupHTML::handleEvent (const NLGUI::CEventDescriptor& eventDesc)
	{
		bool traited = CGroupScrollText::handleEvent (eventDesc);

		if (eventDesc.getType() == NLGUI::CEventDescriptor::system)
		{
			const NLGUI::CEventDescriptorSystem &systemEvent = (const NLGUI::CEventDescriptorSystem &) eventDesc;
			if (systemEvent.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::clocktick)
			{
				// Handle now
				handle ();
			}
		}

		return traited;
	}

	// ***************************************************************************

	void CGroupHTML::endParagraph()
	{
		// Remove previous paragraph if empty
		if (_Paragraph && (_Paragraph->getNumChildren() == 0))
		{
			_Paragraph->getParent ()->delGroup(_Paragraph);
			_Paragraph = NULL;
		}

		_Paragraph = NULL;

		paragraphChange ();
	}

	// ***************************************************************************

	void CGroupHTML::newParagraph(uint beginSpace)
	{
		// Remove previous paragraph if empty
		if (_Paragraph && (_Paragraph->getNumChildren() == 0))
		{
			_Paragraph->getParent ()->delGroup(_Paragraph);
			_Paragraph = NULL;
		}

		// Add a new paragraph
		CGroupParagraph *newParagraph = new CGroupParagraph(CViewBase::TCtorParam());
		newParagraph->setResizeFromChildH(true);

		newParagraph->setBrowseGroup (this);
		newParagraph->setIndent(_Indent);

		// Add to the group
		addGroup (newParagraph, beginSpace);
		_Paragraph = newParagraph;

		paragraphChange ();
	}

	// ***************************************************************************

	void CGroupHTML::browse(const char *url)
	{
		// modify undo/redo
		pushUrlUndoRedo(url);

		// do the browse, with no undo/redo
		doBrowse(url);
	}

	// ***************************************************************************
	void CGroupHTML::refresh()
	{
		if (!_URL.empty())
			doBrowse(_URL.c_str());
	}

	// ***************************************************************************
	void CGroupHTML::doBrowse(const char *url)
	{
		// Stop previous browse
		if (_Browsing)
		{
			// Clear all the context
			clearContext();

			_Browsing = false;
			updateRefreshButton();

	#ifdef LOG_DL
			nlwarning("(%s) *** ALREADY BROWSING, break first", _Id.c_str());
	#endif
		}

	#ifdef LOG_DL
		nlwarning("(%s) Browsing URL : '%s'", _Id.c_str(), url);
	#endif

		// go
		_URL = url;
		_Connecting = false;
		_BrowseNextTime = true;

		// if a BrowseTree is bound to us, try to select the node that opens this URL (auto-locate)
		if(!_BrowseTree.empty())
		{
			CGroupTree	*groupTree=dynamic_cast<CGroupTree*>(CWidgetManager::getInstance()->getElementFromId(_BrowseTree));
			if(groupTree)
			{
				string	nodeId= selectTreeNodeRecurs(groupTree->getRootNode(), url);
				// select the node
				if(!nodeId.empty())
				{
					groupTree->selectNodeById(nodeId);
				}
			}
		}
	}

	// ***************************************************************************

	void CGroupHTML::browseError (const char *msg)
	{
		// Get the list group from CGroupScrollText
		removeContent();
		newParagraph(0);
		CViewText *viewText = new CViewText ("", (string("Error : ")+msg).c_str());
		viewText->setColor (ErrorColor);
		viewText->setModulateGlobalColor(ErrorColorGlobalColor);
		getParagraph()->addChild (viewText);
		if(!_TitlePrefix.empty())
			setTitle (_TitlePrefix);

		stopBrowse ();
		updateRefreshButton();
	}

	// ***************************************************************************

	bool CGroupHTML::isBrowsing()
	{
		return _Browsing;
	}


	void CGroupHTML::stopBrowse ()
	{
	#ifdef LOG_DL
		nlwarning("*** STOP BROWSE (%s)", _Id.c_str());
	#endif

		// Clear all the context
		clearContext();

		_Browsing = false;

		requestTerminated();
	}

	// ***************************************************************************

	void CGroupHTML::updateCoords()
	{
		CGroupScrollText::updateCoords();
	}

	// ***************************************************************************

	bool CGroupHTML::translateChar(ucchar &output, ucchar input, ucchar lastCharParam) const
	{
		// Keep this char ?
		bool keep = true;

		switch (input)
		{
			// Return / tab only in <PRE> mode
		case '\t':
		case '\n':
			{
				// Get the last char
				ucchar lastChar = lastCharParam;
				if (lastChar == 0)
					lastChar = getLastChar();
				keep = ((lastChar != (ucchar)' ') &&
						(lastChar != 0)) || getPRE() || (_CurrentViewImage && (lastChar == 0));
				if(!getPRE())
					input = ' ';
			}
			break;
		case ' ':
			{
				// Get the last char
				ucchar lastChar = lastCharParam;
				if (lastChar == 0)
					lastChar = getLastChar();
				keep = ((lastChar != (ucchar)' ') &&
						(lastChar != (ucchar)'\n') &&
						(lastChar != 0)) || getPRE() || (_CurrentViewImage && (lastChar == 0));
			}
			break;
		case 0xd:
			keep = false;
			break;
		}

		if (keep)
		{
			output = input;
		}

		return keep;
	}

	// ***************************************************************************

	void CGroupHTML::addString(const ucstring &str)
	{
		ucstring tmpStr = str;

		if (_Localize)
		{
			string	_str = tmpStr.toString();
			string::size_type	p = _str.find('#');
			if (p == string::npos)
			{
				tmpStr = CI18N::get(_str);
			}
			else
			{
				string	cmd = _str.substr(0, p);
				string	arg = _str.substr(p+1);

				if (cmd == "date")
				{
					uint	year, month, day;
					sscanf(arg.c_str(), "%d/%d/%d", &year, &month, &day);
					tmpStr = CI18N::get( "uiMFIDate");

					year += (year > 70 ? 1900 : 2000);

					strFindReplace(tmpStr, "%year", toString("%d", year) );
					strFindReplace(tmpStr, "%month", CI18N::get(toString("uiMonth%02d", month)) );
					strFindReplace(tmpStr, "%day", toString("%d", day) );
				}
				else
				{
					tmpStr = arg;
				}
			}
		}

		// In title ?
		if (_Title)
		{
			_TitleString += tmpStr;
		}
		else if (_TextArea)
		{
			_TextAreaContent += tmpStr;
		}
		else if (_Object)
		{
			_ObjectScript += tmpStr.toString();
		}
		else if (_SelectOption)
		{
			if (!(_Forms.empty()))
			{
				if (!_Forms.back().Entries.empty())
				{
					_SelectOptionStr += tmpStr;
				}
			}
		}
		else
		{
			// In a paragraph ?
			if (!_Paragraph)
			{
				newParagraph (0);
				paragraphChange ();
			}

			// Text added ?
			bool added = false;
			bool embolden = getFontWeight() >= 700;

			// Number of child in this paragraph
			if (_CurrentViewLink)
			{
				bool skipLine = !_CurrentViewLink->getText().empty() && *(_CurrentViewLink->getText().rbegin()) == (ucchar) '\n';
				// Compatible with current parameters ?
				if (!skipLine &&
					(getTextColor() == _CurrentViewLink->getColor()) &&
					(getFontSize() == (uint)_CurrentViewLink->getFontSize()) &&
					(getFontUnderlined() == _CurrentViewLink->getUnderlined()) &&
					(getFontStrikeThrough() == _CurrentViewLink->getStrikeThrough()) &&
					(embolden == _CurrentViewLink->getEmbolden()) &&
					(getFontOblique() == _CurrentViewLink->getOblique()) &&
					(getLink() == _CurrentViewLink->Link) &&
					(getGlobalColor() == _CurrentViewLink->getModulateGlobalColor()))
				{
					// Concat the text
					_CurrentViewLink->setText(_CurrentViewLink->getText()+tmpStr);
					_CurrentViewLink->invalidateContent();
					added = true;
				}
			}

			// Not added ?
			if (!added)
			{
				if (getA() && string(getLinkClass()) == "ryzom-ui-button")
				{
					string buttonTemplate = DefaultButtonGroup;
					// Action handler parameters : "name=group_html_id|form=id_of_the_form|submit_button=button_name"
					string param = "name=" + this->_Id + "|url=" + getLink();

					typedef pair<string, string> TTmplParam;
					vector<TTmplParam> tmplParams;
					tmplParams.push_back(TTmplParam("id", ""));
					tmplParams.push_back(TTmplParam("onclick", "browse"));
					tmplParams.push_back(TTmplParam("onclick_param", param));
					tmplParams.push_back(TTmplParam("active", "true"));
					CInterfaceGroup *buttonGroup = CWidgetManager::getInstance()->getParser()->createGroupInstance(buttonTemplate, _Paragraph->getId(), tmplParams);
					if (buttonGroup)
					{

						// Add the ctrl button
						CCtrlTextButton *ctrlButton = dynamic_cast<CCtrlTextButton*>(buttonGroup->getCtrl("button"));
						if (!ctrlButton) ctrlButton = dynamic_cast<CCtrlTextButton*>(buttonGroup->getCtrl("b"));
						if (ctrlButton)
						{
							ctrlButton->setModulateGlobalColorAll (false);

							// Translate the tooltip
							ctrlButton->setDefaultContextHelp(ucstring::makeFromUtf8(getLinkTitle()));
							ctrlButton->setText(tmpStr);
						}
						getParagraph()->addChild (buttonGroup);
						paragraphChange ();
					}
		
				}
				else
				{
					CViewLink *newLink = new CViewLink(CViewBase::TCtorParam());
					if (getA())
					{
						newLink->Link = getLink();
						newLink->LinkTitle = getLinkTitle();
						if (!newLink->Link.empty())
						{
							newLink->setHTMLView (this);
						}
					}
					newLink->setText(tmpStr);
					newLink->setColor(getTextColor());
					newLink->setFontSize(getFontSize());
					newLink->setEmbolden(embolden);
					newLink->setOblique(getFontOblique());
					newLink->setUnderlined(getFontUnderlined());
					newLink->setStrikeThrough(getFontStrikeThrough());
					newLink->setMultiLineSpace((uint)((float)getFontSize()*LineSpaceFontFactor));
					newLink->setMultiLine(true);
					newLink->setModulateGlobalColor(getGlobalColor());
					// newLink->setLineAtBottom (true);

					if (getA() && !newLink->Link.empty())
					{
						getParagraph()->addChildLink(newLink);
					}
					else
					{
						getParagraph()->addChild(newLink);
					}
					paragraphChange ();
				}
			}
		}
	}

	// ***************************************************************************

	void CGroupHTML::addImage(const char *img, bool globalColor, bool reloadImg)
	{
		// In a paragraph ?
		if (!_Paragraph)
		{
			newParagraph (0);
			paragraphChange ();
		}

		string finalUrl;

		// No more text in this text view
		_CurrentViewLink = NULL;

		// Not added ?
		CViewBitmap *newImage = new CViewBitmap (TCtorParam());

		//
		// 1/ try to load the image with the old system (local files in bnp)
		//
		string image = CFile::getPath(img) + CFile::getFilenameWithoutExtension(img) + ".tga";
		if (lookupLocalFile (finalUrl, image.c_str(), false))
		{
			newImage->setRenderLayer(getRenderLayer()+1);
			image = finalUrl;
		}
		else
		{
			//
			// 2/ if it doesn't work, try to load the image in cache
			//
			image = localImageName(img);
			if (!reloadImg && lookupLocalFile (finalUrl, image.c_str(), false))
			{
				// don't display image that are not power of 2
				uint32 w, h;
				CBitmap::loadSize (image, w, h);
				if (w == 0 || h == 0 || ((!NLMISC::isPowerOf2(w) || !NLMISC::isPowerOf2(h)) && !NL3D::CTextureFile::supportNonPowerOfTwoTextures()))
					image.clear();
			}
			else
			{
				//
				// 3/ if it doesn't work, display a placeholder and ask to dl the image into the cache
				//
				image = "web_del.tga";
				addImageDownload(img, newImage);
			}
		}
		newImage->setTexture (image);
		newImage->setModulateGlobalColor(globalColor);

		getParagraph()->addChild(newImage);
		paragraphChange ();
	}

	// ***************************************************************************

	CInterfaceGroup *CGroupHTML::addTextArea(const std::string &templateName, const char *name, uint /* rows */, uint cols, bool multiLine, const ucstring &content, uint maxlength)
	{
		// In a paragraph ?
		if (!_Paragraph)
		{
			newParagraph (0);
			paragraphChange ();
		}

		// No more text in this text view
		_CurrentViewLink = NULL;

		{
			// Not added ?
			std::vector<std::pair<std::string,std::string> > templateParams;
			templateParams.push_back (std::pair<std::string,std::string> ("w", toString (cols*12)));
			//templateParams.push_back (std::pair<std::string,std::string> ("h", toString (rows*12)));
			templateParams.push_back (std::pair<std::string,std::string> ("id", name));
			templateParams.push_back (std::pair<std::string,std::string> ("prompt", ""));
			templateParams.push_back (std::pair<std::string,std::string> ("multiline", multiLine?"true":"false"));
			templateParams.push_back (std::pair<std::string,std::string> ("want_return", multiLine?"true":"false"));
			templateParams.push_back (std::pair<std::string,std::string> ("enter_recover_focus", "false"));
			if (maxlength > 0)
				templateParams.push_back (std::pair<std::string,std::string> ("max_num_chars", toString(maxlength)));
			CInterfaceGroup *textArea = CWidgetManager::getInstance()->getParser()->createGroupInstance (templateName.c_str(),
				getParagraph()->getId(), templateParams.empty()?NULL:&(templateParams[0]), (uint)templateParams.size());

			// Group created ?
			if (textArea)
			{
				// Set the content
				CGroupEditBox *eb = dynamic_cast<CGroupEditBox*>(textArea->getGroup("eb"));
				if (eb)
					eb->setInputString(decodeHTMLEntities(content));

				textArea->invalidateCoords();
				getParagraph()->addChild (textArea);
				paragraphChange ();

				return textArea;
			}
		}

		// Not group created
		return NULL;
	}

	// ***************************************************************************
	CDBGroupComboBox *CGroupHTML::addComboBox(const std::string &templateName, const char *name)
	{
		// In a paragraph ?
		if (!_Paragraph)
		{
			newParagraph (0);
			paragraphChange ();
		}


		{
			// Not added ?
			std::vector<std::pair<std::string,std::string> > templateParams;
			templateParams.push_back (std::pair<std::string,std::string> ("id", name));
			CInterfaceGroup *group = CWidgetManager::getInstance()->getParser()->createGroupInstance (templateName.c_str(),
				getParagraph()->getId(), templateParams.empty()?NULL:&(templateParams[0]), (uint)templateParams.size());

			// Group created ?
			if (group)
			{
				// Set the content
				CDBGroupComboBox *cb = dynamic_cast<CDBGroupComboBox *>(group);
				if (!cb)
				{
					nlwarning("'%s' template has bad type, combo box expected", templateName.c_str());
					delete cb;
					return NULL;
				}
				else
				{
					getParagraph()->addChild (cb);
					paragraphChange ();
					return cb;
				}
			}
		}

		// Not group created
		return NULL;
	}

	// ***************************************************************************

	CCtrlButton *CGroupHTML::addButton(CCtrlButton::EType type, const std::string &/* name */, const std::string &normalBitmap, const std::string &pushedBitmap,
									  const std::string &overBitmap, bool useGlobalColor, const char *actionHandler, const char *actionHandlerParams,
									  const char *tooltip)
	{
		// In a paragraph ?
		if (!_Paragraph)
		{
			newParagraph (0);
			paragraphChange ();
		}

		// Add the ctrl button
		CCtrlButton *ctrlButton = new CCtrlButton(TCtorParam());

		// Load only tga files.. (conversion in dds filename is done in the lookup procedure)
		string normal = normalBitmap.empty()?"":CFile::getPath(normalBitmap) + CFile::getFilenameWithoutExtension(normalBitmap) + ".tga";

		// if the image doesn't exist on local, we check in the cache
	//	if(!CFile::fileExists(normal))
		if(!CPath::exists(normal))
		{
			// search in the compressed texture
			CViewRenderer &rVR = *CViewRenderer::getInstance();
			sint32 id = rVR.getTextureIdFromName(normal);
			if(id == -1)
			{
				normal = localImageName(normalBitmap);
				if(!CFile::fileExists(normal))
				{
					normal = "web_del.tga";
					addImageDownload(normalBitmap, ctrlButton);
				}
			}
		}

		string pushed = pushedBitmap.empty()?"":CFile::getPath(pushedBitmap) + CFile::getFilenameWithoutExtension(pushedBitmap) + ".tga";
		// if the image doesn't exist on local, we check in the cache, don't download it because the "normal" will already setuped it
	//	if(!CFile::fileExists(pushed))
		if(!CPath::exists(pushed))
		{
			// search in the compressed texture
			CViewRenderer &rVR = *CViewRenderer::getInstance();
			sint32 id = rVR.getTextureIdFromName(pushed);
			if(id == -1)
			{
				pushed = localImageName(pushedBitmap);
			}
		}

		string over = overBitmap.empty()?"":CFile::getPath(overBitmap) + CFile::getFilenameWithoutExtension(overBitmap) + ".tga";

		ctrlButton->setType (type);
		if (!normal.empty())
			ctrlButton->setTexture (normal);
		if (!pushed.empty())
			ctrlButton->setTexturePushed (pushed);
		if (!over.empty())
			ctrlButton->setTextureOver (over);
		ctrlButton->setModulateGlobalColorAll (useGlobalColor);
		ctrlButton->setActionOnLeftClick (actionHandler);
		ctrlButton->setParamsOnLeftClick (actionHandlerParams);

		// Translate the tooltip or display raw text (tooltip from webig)
		if (tooltip)
		{
			if (CI18N::hasTranslation(tooltip))
			{
				ctrlButton->setDefaultContextHelp(CI18N::get(tooltip));
				//ctrlButton->setOnContextHelp(CI18N::get(tooltip).toString());
			}
			else
			{
				ctrlButton->setDefaultContextHelp(ucstring(tooltip));
				//ctrlButton->setOnContextHelp(string(tooltip));
			}

			ctrlButton->setInstantContextHelp(true);
			ctrlButton->setToolTipParent(TTMouse);
			ctrlButton->setToolTipParentPosRef(Hotspot_TTAuto);
			ctrlButton->setToolTipPosRef(Hotspot_TTAuto);
			ctrlButton->setActionOnLeftClickParams(tooltip);
		}

		getParagraph()->addChild (ctrlButton);
		paragraphChange ();

		return ctrlButton;
	}

	// ***************************************************************************

	void CGroupHTML::flushString()
	{
		_CurrentViewLink = NULL;
	}

	// ***************************************************************************

	void CGroupHTML::clearContext()
	{
		_Paragraph = NULL;
		_PRE.clear();
		_TextColor.clear();
		_GlobalColor.clear();
		_FontSize.clear();
		_FontWeight.clear();
		_FontOblique.clear();
		_FontUnderlined.clear();
		_FontStrikeThrough.clear();
		_Indent = 0;
		_LI = false;
		_UL.clear();
		_A.clear();
		_Link.clear();
		_LinkTitle.clear();
		_Tables.clear();
		_Cells.clear();
		_TR.clear();
		_Forms.clear();
		_Groups.clear();
		_CellParams.clear();
		_Title = false;
		_TextArea = false;
		_Object = false;
		_Localize = false;

		// TR

		paragraphChange ();

		// clear the pointer to the current image download since all the button are deleted
	#ifdef LOG_DL
		nlwarning("Clear pointers to %d curls", Curls.size());
	#endif
		for(uint i = 0; i < Curls.size(); i++)
		{
			Curls[i].imgs.clear();
		}

	}

	// ***************************************************************************

	ucchar CGroupHTML::getLastChar() const
	{
		if (_CurrentViewLink)
		{
			const ucstring &str = _CurrentViewLink->getText();
			if (!str.empty())
				return str[str.length()-1];
		}
		return 0;
	}

	// ***************************************************************************

	void CGroupHTML::paragraphChange ()
	{
		_CurrentViewLink = NULL;
		_CurrentViewImage = NULL;
		CGroupParagraph *paragraph = getParagraph();
		if (paragraph)
		{
			// Number of child in this paragraph
			uint numChild = paragraph->getNumChildren();
			if (numChild)
			{
				// Get the last child
				CViewBase *child = paragraph->getChild(numChild-1);

				// Is this a string view ?
				_CurrentViewLink = dynamic_cast<CViewLink*>(child);
				_CurrentViewImage = dynamic_cast<CViewBitmap*>(child);
			}
		}
	}

	// ***************************************************************************

	CInterfaceGroup *CGroupHTML::getCurrentGroup()
	{
		if (!_Cells.empty() && _Cells.back())
			return _Cells.back()->Group;
		else
			return _GroupListAdaptor;
	}

	// ***************************************************************************

	void CGroupHTML::addGroup (CInterfaceGroup *group, uint beginSpace)
	{
		if (!group)
			return;

		// Remove previous paragraph if empty
		if (_Paragraph && (_Paragraph->getNumChildren() == 0))
		{
			_Paragraph->getParent ()->delGroup(_Paragraph);
			_Paragraph = NULL;
		}

		if (!_DivName.empty())
		{
			group->setName(_DivName);
			_Groups.push_back(group);
		}

		group->setSizeRef(CInterfaceElement::width);

		// Compute begin space between paragraph and tables

		// * If first in group, no begin space
		// * If behind a paragraph, take the biggest begin space between the previous paragraph and current one.

		// Pointer on the current paragraph (can be a table too)
		CGroupParagraph *p = dynamic_cast<CGroupParagraph*>(group);

		CInterfaceGroup *parentGroup = CGroupHTML::getCurrentGroup();
		const std::vector<CInterfaceGroup*> &groups = parentGroup->getGroups ();
		group->setParent(parentGroup);
		group->setParentSize(parentGroup);
		if (groups.empty())
		{
			group->setParentPos(parentGroup);
			group->setPosRef(Hotspot_TL);
			group->setParentPosRef(Hotspot_TL);
			beginSpace = 0;
		}
		else
		{
			// Last is a paragraph ?
			group->setParentPos(groups.back());
			group->setPosRef(Hotspot_TL);
			group->setParentPosRef(Hotspot_BL);

			// Begin space for previous paragraph
			CGroupParagraph *previous = dynamic_cast<CGroupParagraph*>(groups.back());
			if (previous)
				beginSpace = std::max(beginSpace, previous->getTopSpace());
		}

		// Set the begin space
		if (p)
			p->setTopSpace(beginSpace);
		else
			group->setY(-(sint32)beginSpace);
		parentGroup->addGroup (group);
	}

	// ***************************************************************************

	void CGroupHTML::setTitle (const ucstring &title)
	{
		CInterfaceElement *parent = getParent();
		if (parent)
		{
			if ((parent = parent->getParent()))
			{
				CGroupContainer *container = dynamic_cast<CGroupContainer*>(parent);
				if (container)
				{
					container->setUCTitle (title);
				}
			}
		}
	}

	// ***************************************************************************

	bool CGroupHTML::lookupLocalFile (string &result, const char *url, bool isUrl)
	{
		result = url;
		string tmp;

		// folder used for images cache
		static const string cacheDir = "cache";

		string::size_type protocolPos = strlwr(result).find("://");

		if (protocolPos != string::npos)
		{
			// protocol present, it's an url so file must be searched in cache folder
			// TODO: case of special characters & and ?
			result = cacheDir + result.substr(protocolPos+2);

			// if the file is already cached, use it
			if (CFile::fileExists(result)) tmp = result;
		}
		else
		{
			// Url is a file ?
			if (strlwr(result).find("file:") == 0)
			{
				result = result.substr(5, result.size()-5);
			}

			tmp = CPath::lookup (CFile::getFilename(result), false, false, false);
			if (tmp.empty())
			{
				// try to find in local directory
				tmp = CPath::lookup (result, false, false, true);
			}
		}

		if (!tmp.empty())
		{
			// Normalize the path
			if (isUrl)
				//result = "file:"+strlwr(CPath::standardizePath (CPath::getFullPath (CFile::getPath(result)))+CFile::getFilename(result));*/
				result = "file:/"+tmp;
			else
				result = tmp;
			return true;
		}
		else
		{
			// Is it a texture in the big texture ?
			if (CViewRenderer::getInstance()->getTextureIdFromName (result) >= 0)
			{
				return true;
			}
			else
			{
				// This is not a file in the CPath, let libwww open this URL
				result = url;
				return false;
			}
		}
	}

	// ***************************************************************************

	void CGroupHTML::submitForm (uint formId, const char *submitButtonType, const char *submitButtonName, const char *submitButtonValue, sint32 x, sint32 y)
	{
		// Form id valid ?
		if (formId < _Forms.size())
		{
			_PostNextTime = true;
			_PostFormId = formId;
			_PostFormSubmitType = submitButtonType;
			_PostFormSubmitButton = submitButtonName;
			_PostFormSubmitValue = submitButtonValue;
			_PostFormSubmitX = x;
			_PostFormSubmitY = y;
		}
	}

	// ***************************************************************************

	void CGroupHTML::setBackgroundColor (const CRGBA &bgcolor)
	{
		// Should have a child named bg
		CViewBase *view = getView (DefaultBackgroundBitmapView);
		if (view)
		{
			CViewBitmap *bitmap = dynamic_cast<CViewBitmap*> (view);
			if (bitmap)
			{
				// Change the background color
				bitmap->setColor (bgcolor);
				bitmap->setModulateGlobalColor(false);
			}
		}
	}

	// ***************************************************************************

	void CGroupHTML::setBackground (const string &bgtex, bool scale, bool tile)
	{
		// Should have a child named bg
		CViewBase *view = getView (DefaultBackgroundBitmapView);
		if (view)
		{
			CViewBitmap *bitmap = dynamic_cast<CViewBitmap*> (view);
			if (bitmap)
			{
				bitmap->setParentPosRef(Hotspot_TL);
				bitmap->setPosRef(Hotspot_TL);
				bitmap->setX(0);
				bitmap->setY(0);
				bitmap->setRenderLayer(-2);
				bitmap->setScale(scale);
				bitmap->setTile(tile);
				addImageDownload(bgtex, view);
			}
		}
	}


	struct CButtonFreezer : public CInterfaceElementVisitor
	{
		virtual void visitCtrl(CCtrlBase *ctrl)
		{
			CCtrlBaseButton		*textButt = dynamic_cast<CCtrlTextButton *>(ctrl);
			if (textButt)
			{
				textButt->setFrozen(true);
			}
		}
	};

	// ***************************************************************************

	void CGroupHTML::handle ()
	{
		H_AUTO(RZ_Interface_Html_handle)

		const CWidgetManager::SInterfaceTimes &times = CWidgetManager::getInstance()->getInterfaceTimes();

		// handle curl downloads
		checkDownloads();

		if (_Connecting)
		{
			// Check timeout if needed
			if (_TimeoutValue != 0 && _ConnectingTimeout <= ( times.thisFrameMs / 1000.0f ) )
			{
				browseError(("Connection timeout : "+_URL).c_str());

				_Connecting = false;
			}
		}
		else
		if (_BrowseNextTime || _PostNextTime)
		{
			// Set timeout
			_Connecting = true;
			_ConnectingTimeout = ( times.thisFrameMs / 1000.0f ) + _TimeoutValue;

			// freeze form buttons
			CButtonFreezer freezer;
			this->visit(&freezer);

			// Home ?
			if (_URL == "home")
				_URL = home();

			string finalUrl;
			bool isLocal = lookupLocalFile (finalUrl, _URL.c_str(), true);

			// Save new url
			_URL = finalUrl;

			// file is probably from bnp (ingame help)
			if (isLocal)
			{
				doBrowseLocalFile(finalUrl);
			}
			else
			{
				_TrustedDomain = isTrustedDomain(setCurrentDomain(finalUrl));

				SFormFields formfields;
				if (_PostNextTime)
				{
					buildHTTPPostParams(formfields);
					// _URL is set from form.Action
					finalUrl = _URL;
				}
				else
				{
					// Add custom get params from child classes
					addHTTPGetParams (finalUrl, _TrustedDomain);
				}

				doBrowseRemoteUrl(finalUrl, "", _PostNextTime, formfields);
			}

			_BrowseNextTime = false;
			_PostNextTime = false;
		}
	}

	// ***************************************************************************
	void CGroupHTML::buildHTTPPostParams (SFormFields &formfields)
	{
		// Add text area text
		uint i;

		if (_PostFormId >= _Forms.size())
		{
			nlwarning("(%s) invalid form index %d, _Forms %d", _Id.c_str(), _PostFormId, _Forms.size());
			return;
		}
		// Ref the form
		CForm &form = _Forms[_PostFormId];

		// Save new url
		_URL = form.Action;
		_TrustedDomain = isTrustedDomain(setCurrentDomain(_URL));

		for (i=0; i<form.Entries.size(); i++)
		{
			// Text area ?
			bool addEntry = false;
			ucstring entryData;
			if (form.Entries[i].TextArea)
			{
				// Get the edit box view
				CInterfaceGroup *group = form.Entries[i].TextArea->getGroup ("eb");
				if (group)
				{
					// Should be a CGroupEditBox
					CGroupEditBox *editBox = dynamic_cast<CGroupEditBox*>(group);
					if (editBox)
					{
						entryData = editBox->getViewText()->getText();
						addEntry = true;
					}
				}
			}
			else if (form.Entries[i].Checkbox)
			{
				// todo handle unicode POST here
				if (form.Entries[i].Checkbox->getPushed ())
				{
					entryData = ucstring ("on");
					addEntry = true;
				}
			}
			else if (form.Entries[i].ComboBox)
			{
				CDBGroupComboBox *cb = form.Entries[i].ComboBox;
				entryData.fromUtf8(form.Entries[i].SelectValues[cb->getSelection()]);
				addEntry = true;
			}
			// This is a hidden value
			else
			{
				entryData = form.Entries[i].Value;
				addEntry = true;
			}

			// Add this entry
			if (addEntry)
			{
				formfields.add(form.Entries[i].Name, CI18N::encodeUTF8(entryData));
			}
		}

		if (_PostFormSubmitType == "image")
		{
			// Add the button coordinates
			if (_PostFormSubmitButton.find_first_of("[") == string::npos)
			{
				formfields.add(_PostFormSubmitButton + "_x", NLMISC::toString(_PostFormSubmitX));
				formfields.add(_PostFormSubmitButton + "_y", NLMISC::toString(_PostFormSubmitY));
			}
			else
			{
				formfields.add(_PostFormSubmitButton, NLMISC::toString(_PostFormSubmitX));
				formfields.add(_PostFormSubmitButton, NLMISC::toString(_PostFormSubmitY));
			}
		}
		else
			formfields.add(_PostFormSubmitButton, _PostFormSubmitValue);

		// Add custom params from child classes
		addHTTPPostParams(formfields, _TrustedDomain);
	}

	// ***************************************************************************
	void CGroupHTML::doBrowseLocalFile(const std::string &uri)
	{
		std::string filename;
		if (strlwr(uri).find("file:/") == 0)
		{
			filename = uri.substr(6, uri.size() - 6);
		}
		else
		{
			filename = uri;
		}

	#if LOG_DL
		nlwarning("(%s) browse local file '%s'", filename.c_str());
	#endif

		_TrustedDomain = true;

		// Stop previous browse, remove content
		stopBrowse ();

		_Browsing = true;
		updateRefreshButton();

		CIFile in;
		if (in.open(filename))
		{
			std::string html;
			while(!in.eof())
			{
				char buf[1024];
				in.getline(buf, 1024);
				html += std::string(buf) + "\n";
			}
			in.close();

			if (!renderHtmlString(html))
			{
				browseError((string("Failed to parse html from file : ")+filename).c_str());
			}
		}
		else
		{
			browseError((string("The page address is malformed : ")+filename).c_str());
		}
	}

	// ***************************************************************************
	void CGroupHTML::doBrowseRemoteUrl(const std::string &url, const std::string &referer, bool doPost, const SFormFields &formfields)
	{
		// Stop previous request and remove content
		stopBrowse ();

		_Browsing = true;
		updateRefreshButton();

		// Reset the title
		if(_TitlePrefix.empty())
			setTitle (CI18N::get("uiPleaseWait"));
		else
			setTitle (_TitlePrefix + " - " + CI18N::get("uiPleaseWait"));

	#if LOG_DL
		nlwarning("(%s) browse url (trusted=%s) '%s', referer='%s', post='%s', nb form values %d",
				_Id.c_str(), (_TrustedDomain ? "true" :"false"), url.c_str(), referer.c_str(), (doPost ? "true" : "false"), formfields.Values.size());
	#endif

		if (!MultiCurl)
		{
			browseError(string("Invalid MultCurl handle, loading url failed : "+url).c_str());
			return;
		}

		CURL *curl = curl_easy_init();
		if (!curl)
		{
			nlwarning("(%s) failed to create curl handle", _Id.c_str());
			browseError(string("Failed to create cURL handle : " + url).c_str());
			return;
		}

		// do not follow redirects, we have own handler
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0);
		// after redirect
		curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1);

		// tell curl to use compression if possible (gzip, deflate)
		// leaving this empty allows all encodings that curl supports
		//curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

		// limit curl to HTTP and HTTPS protocols only
		curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
		curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);

		// Destination
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// User-Agent:
		std::string userAgent = options.appName + "/" + options.appVersion;
		curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());

		// Cookies
		sendCookies(curl, HTTPCurrentDomain, _TrustedDomain);

		// Referer
		if (!referer.empty())
		{
			curl_easy_setopt(curl, CURLOPT_REFERER, referer.c_str());
	#ifdef LOG_DL
			nlwarning("(%s) set referer '%s'", _Id.c_str(), referer.c_str());
	#endif
		}

		if (doPost)
		{
			// serialize form data and add it to curl
			std::string data;
			for(uint i=0; i<formfields.Values.size(); ++i)
			{
				char * escapedName = curl_easy_escape(curl, formfields.Values[i].name.c_str(), formfields.Values[i].name.size());
				char * escapedValue = curl_easy_escape(curl, formfields.Values[i].value.c_str(), formfields.Values[i].value.size());

				if (i>0)
					data += "&";

				data += std::string(escapedName) + "=" + escapedValue;

				curl_free(escapedName);
				curl_free(escapedValue);
			}
			curl_easy_setopt(curl, CURLOPT_POST, 1);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
			curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, data.c_str());
		}
		else
		{
			curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
		}

		// transfer handle
		_CurlWWW = new CCurlWWWData(curl, url);

		// set the language code used by the client
		std::vector<std::string> headers;
		headers.push_back("Accept-Language: "+options.languageCode);
		headers.push_back("Accept-Charset: utf-8");
		for(uint i=0; i< headers.size(); ++i)
		{
			_CurlWWW->HeadersSent = curl_slist_append(_CurlWWW->HeadersSent, headers[i].c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _CurlWWW->HeadersSent);

		// catch headers for redirect
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curlHeaderCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEHEADER, _CurlWWW);

		// catch body
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlDataCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, _CurlWWW);

	#if LOG_DL
		// progress callback
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, curlProgressCallback);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, _CurlWWW);
	#else
		// progress off
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	#endif

		//
		curl_multi_add_handle(MultiCurl, curl);

		// start the transfer
		int NewRunningCurls = 0;
		curl_multi_perform(MultiCurl, &NewRunningCurls);
		RunningCurls++;
	}

	// ***************************************************************************
	void CGroupHTML::htmlDownloadFinished(const std::string &content, const std::string &type, long code)
	{
	#ifdef LOG_DL
		nlwarning("(%s) HTML download finished, content length %d, type '%s', code %d", _Id.c_str(), content.size(), type.c_str(), code);
	#endif

		// set trusted domain for parsing
		_TrustedDomain = isTrustedDomain(setCurrentDomain(_URL));

		// create <html> markup for image downloads
		if (type.find("image/") == 0 && content.size() > 0)
		{
			try
			{
				std::string dest = localImageName(_URL);
				COFile out;
				out.open(dest);
				out.serialBuffer((uint8 *)(content.c_str()), content.size());
				out.close();
	#ifdef LOG_DL
				nlwarning("(%s) image saved to '%s', url '%s'", _Id.c_str(), dest.c_str(), _URL.c_str());
	#endif
			}
			catch(...) { }

			// create html code with image url inside and do the request again
			renderHtmlString("<html><head><title>"+_URL+"</title></head><body><img src=\"" + _URL + "\"></body></html>");
		}
		else
		{
			renderHtmlString(content);
		}
	}

	// ***************************************************************************

	bool CGroupHTML::renderHtmlString(const std::string &html)
	{
		bool success;

		//
		_Browsing = true;

		// clear content
		beginBuild();

		success = parseHtml(html);

		// invalidate coords
		endBuild();

		// set the browser as complete
		_Browsing = false;
		updateRefreshButton();

		// check that the title is set, or reset it (in the case the page
		// does not provide a title)
		if (_TitleString.empty())
		{
			setTitle(_TitlePrefix);
		}

		return success;
	}

	// ***************************************************************************

	void CGroupHTML::draw ()
	{
		CGroupScrollText::draw ();
	}

	// ***************************************************************************

	void CGroupHTML::endBuild ()
	{
		invalidateCoords();
	}

	// ***************************************************************************

	void CGroupHTML::addHTTPGetParams (string &/* url */, bool /*trustedDomain*/)
	{
	}

	// ***************************************************************************

	void CGroupHTML::addHTTPPostParams (SFormFields &/* formfields */, bool /*trustedDomain*/)
	{
	}

	// ***************************************************************************
	void CGroupHTML::requestTerminated()
	{
		if (_CurlWWW)
		{
	#if LOG_DL
			nlwarning("(%s) stop curl, url '%s'", _Id.c_str(), _CurlWWW->Url.c_str());
	#endif
			if (MultiCurl)
				curl_multi_remove_handle(MultiCurl, _CurlWWW->Request);

			delete _CurlWWW;

			_CurlWWW = NULL;
			_Connecting = false;
		}
	}

	// ***************************************************************************

	string	CGroupHTML::home ()
	{
		return Home;
	}

	// ***************************************************************************

	void CGroupHTML::removeContent ()
	{
		// Remove old document
		if (!_GroupListAdaptor)
		{
			_GroupListAdaptor = new CGroupListAdaptor(CViewBase::TCtorParam()); // deleted by the list
			_GroupListAdaptor->setResizeFromChildH(true);
			getList()->addChild (_GroupListAdaptor, true);
		}

		// Group list adaptor not exist ?
		_GroupListAdaptor->clearGroups();
		_GroupListAdaptor->clearControls();
		_GroupListAdaptor->clearViews();
		CWidgetManager::getInstance()->clearViewUnders();
		CWidgetManager::getInstance()->clearCtrlsUnders();

		// Clear all the context
		clearContext();

		// Reset default background color
		setBackgroundColor (BgColor);

		paragraphChange ();
	}

	// ***************************************************************************
	const std::string &CGroupHTML::selectTreeNodeRecurs(CGroupTree::SNode *node, const std::string &url)
	{
		static std::string	emptyString;
		if(!node)
		{
			return emptyString;
		}

		// if this node match
		if(actionLaunchUrlRecurs(node->AHName, node->AHParams, url))
		{
			return node->Id;
		}
		// fails => look into children
		else
		{
			for(uint i=0;i<node->Children.size();i++)
			{
				const string &childRes= selectTreeNodeRecurs(node->Children[i], url);
				if(!childRes.empty())
					return childRes;
			}

			// none match...
			return emptyString;
		}
	}

	// ***************************************************************************
	bool	CGroupHTML::actionLaunchUrlRecurs(const std::string &ah, const std::string &params, const std::string &url)
	{
		// check if this action match
		if( (ah=="launch_help" || ah=="browse") && IActionHandler::getParam (params, "url") == url)
		{
			return true;
		}
		// can be a proc that contains launch_help/browse => look recurs
		else if(ah=="proc")
		{
			const std::string &procName= params;
			// look into this proc
			uint	numActions= CWidgetManager::getInstance()->getParser()->getProcedureNumActions(procName);
			for(uint i=0;i<numActions;i++)
			{
				string	procAh, procParams;
				if( CWidgetManager::getInstance()->getParser()->getProcedureAction(procName, i, procAh, procParams))
				{
					// recurs proc if needed!
					if (actionLaunchUrlRecurs(procAh, procParams, url))
						return true;
				}
			}
		}

		return false;
	}

	// ***************************************************************************
	void	CGroupHTML::clearUndoRedo()
	{
		// erase any undo/redo
		_BrowseUndo.clear();
		_BrowseRedo.clear();

		// update buttons validation
		updateUndoRedoButtons();
	}

	// ***************************************************************************
	void	CGroupHTML::pushUrlUndoRedo(const std::string &url)
	{
		// if same url, no op
		if(url==_AskedUrl)
			return;

		// erase any redo, push undo, set current
		_BrowseRedo.clear();
		if(!_AskedUrl.empty())
			_BrowseUndo.push_back(_AskedUrl);
		_AskedUrl= url;

		// limit undo
		while(_BrowseUndo.size()>MaxUrlUndoRedo)
			_BrowseUndo.pop_front();

		// update buttons validation
		updateUndoRedoButtons();
	}

	// ***************************************************************************
	void	CGroupHTML::browseUndo()
	{
		if(_BrowseUndo.empty())
			return;

		// push to redo, pop undo, and set current
		_BrowseRedo.push_front(_AskedUrl);
		_AskedUrl= _BrowseUndo.back();
		_BrowseUndo.pop_back();

		// update buttons validation
		updateUndoRedoButtons();

		// and then browse the undoed url, with no undo/redo
		doBrowse(_AskedUrl.c_str());
	}

	// ***************************************************************************
	void	CGroupHTML::browseRedo()
	{
		if(_BrowseRedo.empty())
			return;

		// push to undo, pop redo, and set current
		_BrowseUndo.push_back(_AskedUrl);
		_AskedUrl= _BrowseRedo.front();
		_BrowseRedo.pop_front();

		// update buttons validation
		updateUndoRedoButtons();

		// and then browse the redoed url, with no undo/redo
		doBrowse(_AskedUrl.c_str());
	}

	// ***************************************************************************
	void	CGroupHTML::updateUndoRedoButtons()
	{
		CCtrlBaseButton		*butUndo= dynamic_cast<CCtrlBaseButton *>(CWidgetManager::getInstance()->getElementFromId(_BrowseUndoButton));
		CCtrlBaseButton		*butRedo= dynamic_cast<CCtrlBaseButton *>(CWidgetManager::getInstance()->getElementFromId(_BrowseRedoButton));

		// gray according to list size
		if(butUndo)
			butUndo->setFrozen(_BrowseUndo.empty());
		if(butRedo)
			butRedo->setFrozen(_BrowseRedo.empty());
	}

	// ***************************************************************************
	void	CGroupHTML::updateRefreshButton()
	{
		CCtrlBaseButton		*butRefresh = dynamic_cast<CCtrlBaseButton *>(CWidgetManager::getInstance()->getElementFromId(_BrowseRefreshButton));

		bool enabled = !_Browsing && !_Connecting;
		if(butRefresh)
			butRefresh->setFrozen(!enabled);
	}

	// ***************************************************************************

	NLMISC_REGISTER_OBJECT(CViewBase, CGroupHTMLInputOffset, std::string, "html_input_offset");

	CGroupHTMLInputOffset::CGroupHTMLInputOffset(const TCtorParam &param)
		: CInterfaceGroup(param),
		Offset(0)
	{
	}

	xmlNodePtr CGroupHTMLInputOffset::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CInterfaceGroup::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "html_input_offset" );
		xmlSetProp( node, BAD_CAST "y_offset", BAD_CAST toString( Offset ).c_str() );

		return node;
	}

	// ***************************************************************************
	bool CGroupHTMLInputOffset::parse(xmlNodePtr cur, CInterfaceGroup *parentGroup)
	{
		if (!CInterfaceGroup::parse(cur, parentGroup)) return false;
		CXMLAutoPtr ptr;
		// Get the url
		ptr = xmlGetProp (cur, (xmlChar*)"y_offset");
		if (ptr)
			fromString((const char*)ptr, Offset);
		return true;
	}

	// ***************************************************************************
	int CGroupHTML::luaBrowse(CLuaState &ls)
	{
		const char *funcName = "browse";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		browse(ls.toString(1));
		return 0;
	}

	// ***************************************************************************
	int CGroupHTML::luaRefresh(CLuaState &ls)
	{
		const char *funcName = "refresh";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		refresh();
		return 0;
	}

	// ***************************************************************************
	int CGroupHTML::luaRemoveContent(CLuaState &ls)
	{
		const char *funcName = "removeContent";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		removeContent();
		return 0;
	}

	// ***************************************************************************
	int CGroupHTML::luaInsertText(CLuaState &ls)	
	{
		const char *funcName = "insertText";
		CLuaIHM::checkArgCount(ls, funcName, 3);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
		CLuaIHM::checkArgType(ls, funcName, 3, LUA_TBOOLEAN);
		
		string name = ls.toString(1);

		ucstring text;
		text.fromUtf8(ls.toString(2));

		if (!_Forms.empty())
		{
			for (uint i=0; i<_Forms.back().Entries.size(); i++)
			{
				if (_Forms.back().Entries[i].TextArea && _Forms.back().Entries[i].Name == name)
				{
					// Get the edit box view
					CInterfaceGroup *group = _Forms.back().Entries[i].TextArea->getGroup ("eb");
					if (group)
					{
						// Should be a CGroupEditBox
						CGroupEditBox *editBox = dynamic_cast<CGroupEditBox*>(group);
						if (editBox)
							editBox->writeString(text, false, ls.toBoolean(3));
					}
				}
			}
		}

		return 0;
	}

	// ***************************************************************************
	int CGroupHTML::luaAddString(CLuaState &ls)
	{
		const char *funcName = "addString";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		addString(ucstring(ls.toString(1)));
		return 0;
	}

	// ***************************************************************************
	int CGroupHTML::luaAddImage(CLuaState &ls)
	{
		const char *funcName = "addImage";
		CLuaIHM::checkArgCount(ls, funcName, 2);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		CLuaIHM::checkArgType(ls, funcName, 2, LUA_TBOOLEAN);
		if (!_Paragraph)
		{
			newParagraph(0);
			paragraphChange();
		}
		string url = getLink();
		if (!url.empty())
		{
			string params = "name=" + getId() + "|url=" + getLink ();
			addButton(CCtrlButton::PushButton, ls.toString(1), ls.toString(1), ls.toString(1),
								"", ls.toBoolean(2), "browse", params.c_str(), "");
		}
		else
		{
			addImage(ls.toString(1), ls.toBoolean(2));
		}


		return 0;
	}

	// ***************************************************************************
	int CGroupHTML::luaBeginElement(CLuaState &ls)
	{
		const char *funcName = "beginElement";
		CLuaIHM::checkArgCount(ls, funcName, 2);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		CLuaIHM::checkArgType(ls, funcName, 2, LUA_TTABLE);

		uint element_number = (uint)ls.toNumber(1);
		std::vector<bool> present;
		std::vector<const char *> value;
		present.resize(30, false);
		value.resize(30);

		CLuaObject params;
		params.pop(ls);
		uint max_idx = 0;


		ENUM_LUA_TABLE(params, it)
		{
			if (!it.nextKey().isNumber())
			{
				nlwarning("%s : bad key encountered with type %s, number expected.", funcName, it.nextKey().getTypename());
				continue;
			}
			if (!it.nextValue().isString())
			{
				nlwarning("%s : bad value encountered with type %s for key %s, string expected.", funcName, it.nextValue().getTypename(), it.nextKey().toString().c_str());
				continue;
			}
			uint idx = (uint)it.nextKey().toNumber();

			present.insert(present.begin() + (uint)it.nextKey().toNumber(), true);

			string str = it.nextValue().toString();
			size_t size = str.size() + 1;
			char * buffer = new char[ size ];
			strncpy(buffer, str.c_str(), size );

			value.insert(value.begin() + (uint)it.nextKey().toNumber(), buffer);
		}

		beginElement(element_number, present, value);
		if (element_number == HTML_A)
			addLink(element_number, present, value);

		return 0;
	}


	// ***************************************************************************
	int CGroupHTML::luaEndElement(CLuaState &ls)
	{
		const char *funcName = "endElement";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);

		uint element_number = (uint)ls.toNumber(1);
		endElement(element_number);

		return 0;
	}


	// ***************************************************************************
	int CGroupHTML::luaShowDiv(CLuaState &ls)
	{
		const char *funcName = "showDiv";
		CLuaIHM::checkArgCount(ls, funcName, 2);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		CLuaIHM::checkArgType(ls, funcName, 2, LUA_TBOOLEAN);

		if (!_Groups.empty())
		{
			for (uint i=0; i<_Groups.size(); i++)
			{
				CInterfaceGroup *group = _Groups[i];
				if (group->getName() == ls.toString(1))
				{
					group->setActive(ls.toBoolean(2));
				}
			}
		}
		return 0;
	}

	// ***************************************************************************
	void CGroupHTML::setURL(const std::string &url)
	{
		browse(url.c_str());
	}

	// ***************************************************************************
	inline bool isDigit(ucchar c, uint base = 16)
	{
		if (c>='0' && c<='9') return true;
		if (base != 16) return false;
		if (c>='A' && c<='F') return true;
		if (c>='a' && c<='f') return true;
		return false;
	}

	// ***************************************************************************
	inline ucchar convertHexDigit(ucchar c)
	{
		if (c>='0' && c<='9') return c-'0';
		if (c>='A' && c<='F') return c-'A'+10;
		if (c>='a' && c<='f') return c-'a'+10;
		return 0;
	}

	// ***************************************************************************
	ucstring CGroupHTML::decodeHTMLEntities(const ucstring &str)
	{
		ucstring result;
		uint last, pos;

		for (uint i=0; i<str.length(); ++i)
		{
			// HTML entity
			if (str[i] == '&' && (str.length()-i) >= 4)
			{
				pos = i+1;

				// unicode character
				if (str[pos] == '#')
				{
					++pos;

					// using decimal by default
					uint base = 10;

					// using hexadecimal if &#x
					if (str[pos] == 'x')
					{
						base = 16;
						++pos;
					}

					// setup "last" to point at the first character following "&#x?[0-9a-f]+"
					for (last = pos; last < str.length(); ++last) if (!isDigit(str[last], base)) break;

					// make sure that at least 1 digit was found
					// and have the terminating ';' to complete the token: "&#x?[0-9a-f]+;"
					if (last == pos || str[last] != ';')
					{
						result += str[i];
						continue;
					}

					ucchar c = 0;

					// convert digits to unicode character
					while (pos<last) c = convertHexDigit(str[pos++]) + c*ucchar(base);

					// append our new character to the result string
					result += c;

					// move 'i' forward to point at the ';' .. the for(...) will increment i to point to next char
					i = last;

					continue;
				}

				// special xml characters
				if (str.substr(i+1,5)==ucstring("quot;"))	{ i+=5; result+='\"'; continue; }
				if (str.substr(i+1,4)==ucstring("amp;"))	{ i+=4; result+='&'; continue; }
				if (str.substr(i+1,3)==ucstring("lt;"))	{ i+=3; result+='<'; continue; }
				if (str.substr(i+1,3)==ucstring("gt;"))	{ i+=3; result+='>'; continue; }
			}

			// all the special cases are catered for... treat this as a normal character
			result += str[i];
		}

		return result;
	}

	// ***************************************************************************
	std::string CGroupHTML::getAbsoluteUrl(const std::string &url)
	{
		CUrlParser uri(url);
		if (uri.isAbsolute())
			return url;

		uri.inherit(_URL);

		return uri.toString();
	}

	// ***************************************************************************
	// CGroupHTML::CStyleParams style;
	// style.FontSize;    // font-size: 10px;
	// style.TextColor;   // color: #ABCDEF;
	// style.Underlined;  // text-decoration: underline;     text-decoration-line: underline;
	// style.StrikeThrough; // text-decoration: line-through;  text-decoration-line: line-through;
	void CGroupHTML::getStyleParams(const std::string &styleString, CStyleParams &style, bool inherit)
	{
		TStyle styles = parseStyle(styleString);
		TStyle::iterator it;
		for (it=styles.begin(); it != styles.end(); ++it)
		{
			if (it->first == "font-size")
			{
				float tmp;
				sint size = 0;
				getPercentage (size, tmp, it->second.c_str());
				if (size > 0)
					style.FontSize = size;
			}
			else
			if (it->first == "font-style")
			{
				if (it->second == "italic" || it->second == "oblique")
					style.FontOblique = true;
			}
			else
			if (it->first == "font-weight")
			{
				// https://developer.mozilla.org/en-US/docs/Web/CSS/font-weight
				uint weight = 400;
				if (it->second == "normal")
					weight = 400;
				else
				if (it->second == "bold")
					weight = 700;
				else
				if (it->second == "lighter")
				{
					const uint lighter[] = {100, 100, 100, 100, 100, 400, 400, 700, 700};
					int index = getFontWeight() / 100 - 1;
					clamp(index, 1, 9);
					weight = lighter[index-1];
				}
				else
				if (it->second == "bolder")
				{
					const uint bolder[] =  {400, 400, 400, 700, 700, 900, 900, 900, 900};
					uint index = getFontWeight() / 100 + 1;
					clamp(index, 1, 9);
					weight = bolder[index-1];
				}
				else
				if (fromString(it->second, weight))
				{
					weight = (weight / 100);
					clamp(weight, 1, 9);
					weight *= 100;
				}
				style.FontWeight = weight;
			}
			else
			if (it->first == "color")
				scanHTMLColor(it->second.c_str(), style.TextColor);
			else
			if (it->first == "text-decoration" || it->first == "text-decoration-line")
			{
				std::string prop(strlwr(it->second));
				style.Underlined = (prop.find("underline") != std::string::npos);
				style.StrikeThrough = (prop.find("line-through") != std::string::npos);
			}
		}
		if (inherit)
		{
			style.Underlined = getFontUnderlined() || style.Underlined;
			style.StrikeThrough = getFontStrikeThrough() || style.StrikeThrough;
		}
	}

	// ***************************************************************************
	size_t CGroupHTML::curlHeaderCallback(char *buffer, size_t size, size_t nmemb, void *pCCurlWWWData)
	{
		CCurlWWWData * me = static_cast<CCurlWWWData *>(pCCurlWWWData);
		if (me)
		{
			std::string header;
			header.append(buffer, size * nmemb);
			me->setRecvHeader(header.substr(0, header.find_first_of("\n\r")));
		}

		return size * nmemb;
	}

	// ***************************************************************************
	size_t CGroupHTML::curlDataCallback(char *buffer, size_t size, size_t nmemb, void *pCCurlWWWData)
	{
		CCurlWWWData * me = static_cast<CCurlWWWData *>(pCCurlWWWData);
		if (me)
			me->Content.append(buffer, size * nmemb);

		return size * nmemb;
	}

	// ***************************************************************************
	size_t CGroupHTML::curlProgressCallback(void *pCCurlWWWData, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
	{
		CCurlWWWData * me = static_cast<CCurlWWWData *>(pCCurlWWWData);
		if (me)
		{
			if (dltotal > 0 || dlnow > 0 || ultotal > 0 || ulnow > 0)
			{
				nlwarning("> dltotal %d, dlnow %d, ultotal %d, ulnow %d, url '%s'", dltotal, dlnow, ultotal, ulnow, me->Url.c_str());
			}
		}

		// return 1 to cancel download
		return 0;
	}

}

