// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "stdsound.h"
#include "nel/sound/sound_animation.h"
#include "nel/sound/sound_anim_marker.h"
#include "nel/misc/stream.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/file.h"

using namespace std;
using namespace NLSOUND;
using namespace NLMISC;

namespace NLSOUND {


// ********************************************************

void CSoundAnimation::addMarker(CSoundAnimMarker* marker)
{
	if (marker == NULL)
		return;

	_Dirty = true;
	_Markers.push_back(marker);
	sort();
}

// ********************************************************

void CSoundAnimation::removeMarker(CSoundAnimMarker* marker)
{
	if (marker == NULL)
		return;

	_Dirty = true;
	vector<CSoundAnimMarker*>::iterator iter;
	for (iter = _Markers.begin(); iter != _Markers.end(); iter++)
	{
		if (*iter == marker)
		{
			_Markers.erase(iter);
			break;
		}
	}
}

// ********************************************************

void CSoundAnimation::sort()
{
	//std::sort<CSoundAnimMarker*, eqmarker>(_Markers.begin(), _Markers.end(), eqmarker());
}

// ********************************************************

void CSoundAnimation::save()
{
	// File stream
	COFile file;
	vector<NLMISC::CSheetId>	sounds;

	// Open the file
	if (!file.open(_Filename.c_str()))
	{
		throw NLMISC::Exception("Can't open the file for writing");
	}

	// Create the XML stream
	COXml output;

	// Init
	if (output.init (&file, "1.0"))
	{
		xmlDocPtr xmlDoc = output.getDocument();

		// Create the first node
		xmlNodePtr root = xmlNewDocNode (xmlDoc, NULL, (const xmlChar*)"SOUNDANIMATION", NULL);
		xmlDocSetRootElement (xmlDoc, root);

		vector<CSoundAnimMarker*>::iterator iter;
		for (iter = _Markers.begin(); iter != _Markers.end(); iter++)
		{
			CSoundAnimMarker* marker = (*iter);

			set<string>::iterator iter;

			char s[64];
			smprintf(s, 64, "%f", marker->getTime());

			xmlNodePtr markerNode = xmlNewChild (root, NULL, (const xmlChar*)"MARKER", NULL);
			xmlSetProp (markerNode, (const xmlChar*) "time", (const xmlChar*) s);

			marker->getSounds(sounds);

			vector<NLMISC::CSheetId>::iterator iter2;
			for (iter2 = sounds.begin(); iter2 != sounds.end(); iter2++)
			{
				xmlNodePtr soundNode = xmlNewChild ( markerNode, NULL, (const xmlChar*)"SOUND", NULL );
				xmlSetProp (soundNode, (const xmlChar*)"name", (const xmlChar*)iter2->toString().c_str() /*CStringMapper::unmap(*iter2).c_str()*/);
			}

			sounds.clear();
		}

		// Flush the stream, write all the output file
		output.flush ();
	}

	// Close the file
	file.close ();

	_Dirty = false;
}

// ********************************************************

void CSoundAnimation::play(UAudioMixer* mixer, float lastTime, float curTime, NL3D::CCluster *cluster, CSoundContext &context)
{
	vector<CSoundAnimMarker*>::iterator iter;
	for (iter = _Markers.begin(); iter != _Markers.end(); iter++)
	{
		CSoundAnimMarker* marker = *iter;
		nlassert(marker);

		if ((lastTime <= marker->getTime()) && (marker->getTime() < curTime))
		{
			marker->play(mixer, cluster, context);
		}
	}
}

// ********************************************************

void CSoundAnimation::load()
{
	CIFile file;

	// Open the file
	if (!file.open(_Filename.c_str()))
	{
		throw NLMISC::Exception("Can't open the file for reading");
	}

	// Create the XML stream
	CIXml input;

	// Init
	if (input.init (file))
	{
		xmlNodePtr animNode = input.getRootNode ();
		xmlNodePtr markerNode = input.getFirstChildNode(animNode, "MARKER");

		while (markerNode != 0)
		{
			CSoundAnimMarker* marker = new CSoundAnimMarker();

			const char *time = (const char*) xmlGetProp(markerNode, (xmlChar*) "time");
			if (time == 0)
			{
				throw NLMISC::Exception("Invalid sound animation marker");
			}

			float val;
			NLMISC::fromString(time, val);

			marker->setTime(val);
			xmlFree ((void*)time);


			xmlNodePtr soundNode = input.getFirstChildNode(markerNode, "SOUND");

			while (soundNode != 0)
			{
				char *name = (char*) xmlGetProp(soundNode, (xmlChar*) "name");
				if (name == 0)
				{
					throw NLMISC::Exception("Invalid sound animation marker");
				}

				marker->addSound(NLMISC::CSheetId(string(name), "sound"));

				xmlFree ((void*)name);

				soundNode = input.getNextChildNode(soundNode, "SOUND");
			}

			addMarker(marker);

			markerNode = input.getNextChildNode(markerNode, "MARKER");
		}
	}

	// Close the file
	file.close ();
	_Dirty = false;
}

} //namespace NLSOUND
