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

#include "stdligo.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive_class.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"

using namespace NLMISC;
using namespace std;

const uint32 NLLIGO_PRIMITIVE_VERSION = 1;

namespace NLLIGO
{

CPrimitiveContext	*CPrimitiveContext::_Instance = NULL;


// ***************************************************************************
// XML helpers
// ***************************************************************************

void Error (const char *filename, const char *format, ...)
{
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	nlwarning ("In File (%s) %s", filename, buffer);
}

// ***************************************************************************

void XMLError (xmlNodePtr xmlNode, const char *filename, const char *format, ... )
{
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	Error (filename, "node (%s), line (%p) : %s", xmlNode->name, xmlNode->content, buffer);
}


// ***************************************************************************

xmlNodePtr GetFirstChildNode (xmlNodePtr xmlNode, const char *filename, const char *childName)
{
	// Call the CIXml version
	xmlNodePtr result = CIXml::getFirstChildNode (xmlNode, childName);
	if (result) return result;

	// Output a formated error
	XMLError (xmlNode, filename, "Can't find XML node named (%s)", childName);
	return NULL;
}

// ***************************************************************************

bool GetPropertyString (string &result, const char *filename, xmlNodePtr xmlNode, const char *propName)
{
	// Call the CIXml version
	if (!CIXml::getPropertyString (result, xmlNode, propName))
	{
		// Output a formated error
		XMLError (xmlNode, filename, "Can't find XML node property (%s)", propName);
		return false;
	}
	return true;
}

// ***************************************************************************

bool ReadInt (const char *propName, int &result, const char *filename, xmlNodePtr xmlNode)
{
	string value;
	if (GetPropertyString (value, filename, xmlNode, propName))
	{
		result = atoi (value.c_str ());
		return true;
	}
	return false;
}

// ***************************************************************************

void WriteInt (const char *propName, int value, xmlNodePtr xmlNode)
{
	// Set properties
	xmlSetProp (xmlNode, (const xmlChar*)propName, (const xmlChar*)(toString (value).c_str ()));
}

// ***************************************************************************

bool ReadUInt (const char *propName, uint &result, const char *filename, xmlNodePtr xmlNode)
{
	string value;
	if (GetPropertyString (value, filename, xmlNode, propName))
	{
		result = strtoul (value.c_str (), NULL, 10);
		return true;
	}
	return false;
}

// ***************************************************************************

void WriteUInt (const char *propName, uint value, xmlNodePtr xmlNode)
{
	// Set properties
	xmlSetProp (xmlNode, (const xmlChar*)propName, (const xmlChar*)(toString (value).c_str ()));
}

// ***************************************************************************

bool ReadFloat (const char *propName, float &result, const char *filename, xmlNodePtr xmlNode)
{
	string value;
	if (GetPropertyString (value, filename, xmlNode, propName))
	{
		NLMISC::fromString(value, result);
		return true;
	}
	return false;
}

// ***************************************************************************

void WriteFloat (const char *propName, float value, xmlNodePtr xmlNode)
{
	// Set properties
	xmlSetProp (xmlNode, (const xmlChar*)propName, (const xmlChar*)(toString (value).c_str ()));
}

// ***************************************************************************

bool ReadVector (CPrimVector &point, const char *filename, xmlNodePtr xmlNode)
{
	CPrimVector pos;
	if (ReadFloat ("X", pos.x, filename, xmlNode))
	{
		if (ReadFloat ("Y", pos.y, filename, xmlNode))
		{
			if (ReadFloat ("Z", pos.z, filename, xmlNode))
			{
				pos.Selected = false;
				string result;
				if (CIXml::getPropertyString (result, xmlNode, "SELECTED"))
				{
					if (result == "true")
						pos.Selected = true;
				}
				point = pos;
				return true;
			}
		}
	}
	return false;
}

// ***************************************************************************

void WriteVector (const CPrimVector &point, xmlNodePtr xmlNode)
{
	// Set properties
	xmlSetProp (xmlNode, (const xmlChar*)"X", (const xmlChar*)(toString (point.x).c_str ()));
	xmlSetProp (xmlNode, (const xmlChar*)"Y", (const xmlChar*)(toString (point.y).c_str ()));
	xmlSetProp (xmlNode, (const xmlChar*)"Z", (const xmlChar*)(toString (point.z).c_str ()));
	if (point.Selected)
		xmlSetProp (xmlNode, (const xmlChar*)"SELECTED", (const xmlChar*)"true");
}


// ***************************************************************************

bool GetNodeString (string &result, const char *filename, xmlNodePtr xmlNode, const char *nodeName)
{
	// Look for the node
	xmlNodePtr node = CIXml::getFirstChildNode (xmlNode, nodeName);
	if (!node)
	{
		XMLError (xmlNode, filename, "Can't find XML node named (%s)", nodeName);
		return false;
	}

	// Get the node string
	if (!CIXml::getContentString (result, node))
	{
		XMLError (xmlNode, filename, "Can't find any text in the node named (%s)", nodeName);
		return false;
	}

	return true;
}

// ***************************************************************************

bool GetContentString (string &result, const char *filename, xmlNodePtr xmlNode)
{
	// Get the node string
	if (!CIXml::getContentString (result, xmlNode))
	{
		XMLError (xmlNode, filename, "Can't find any text in the node");
		return false;
	}

	return true;
}

// ***************************************************************************
// CPropertyString
// ***************************************************************************

CPropertyString::CPropertyString (const char *str)
{
	String = str;
}

CPropertyString::CPropertyString (const std::string &str)
{
	String = str;
}

// ***************************************************************************

CPropertyString::CPropertyString (const char *str, bool _default)
{
	String = str;
	Default = _default;
}

// ***************************************************************************
// CPropertyStringArray
// ***************************************************************************

CPropertyStringArray::CPropertyStringArray (const std::vector<std::string> &stringArray)
{
	StringArray = stringArray;
}

// ***************************************************************************

CPropertyStringArray::CPropertyStringArray (const std::vector<std::string> &stringArray, bool _default)
{
	StringArray = stringArray;
	Default = _default;
}

// ***************************************************************************

void CPrimPoint::serial (NLMISC::IStream &f)
{
	// serial base info
	IPrimitive::serial(f);
	f.serial(Point);
	f.serial(Angle);
}

// ***************************************************************************
void CPrimPath::serial (NLMISC::IStream &f)
{
	IPrimitive::serial(f);
	f.serialCont(VPoints);
}

// ***************************************************************************

bool CPrimZone::contains (const NLMISC::CVector &v, const std::vector<CPrimVector> &points)
{
	uint32 i;
	CVector vMin, vMax;

	// Point or line can't contains !
	if (points.size() < 3)
		return false;

	// Get the bounding rectangle of the zone
	vMax = vMin = points[0];
	for (i = 0; i < points.size(); ++i)
	{
		if (vMin.x > points[i].x)
			vMin.x = points[i].x;
		if (vMin.y > points[i].y)
			vMin.y = points[i].y;

		if (vMax.x < points[i].x)
			vMax.x = points[i].x;
		if (vMax.y < points[i].y)
			vMax.y = points[i].y;
	}

	if ((v.x < vMin.x) || (v.y < vMin.y) || (v.x > vMax.x) || (v.y > vMax.y))
		return false;

	uint32 nNbIntersection = 0;
	for (i = 0; i < points.size(); ++i)
	{
		const CVector &p1 = points[i];
		const CVector &p2 = points[(i+1)%points.size()];

		if (((p1.y-v.y) <= 0.0)&&((p2.y-v.y) <= 0.0))
			continue;
		if (((p1.y-v.y) > 0.0)&&((p2.y-v.y) > 0.0))
			continue;
		float xinter = p1.x + (p2.x-p1.x) * ((v.y-p1.y)/(p2.y-p1.y));
		if (xinter > v.x)
			++nNbIntersection;
	}
	if ((nNbIntersection&1) == 1) // odd intersections so the vertex is inside
		return true;
	else
		return false;
}

// ***************************************************************************

bool CPrimZone::contains (const NLMISC::CVector &v, const std::vector<NLMISC::CVector> &points)
{
	uint32 i;
	CVector vMin, vMax;

	// Point or line can't contains !
	if (points.size() < 3)
		return false;

	// Get the bounding rectangle of the zone
	vMax = vMin = points[0];
	for (i = 0; i < points.size(); ++i)
	{
		if (vMin.x > points[i].x)
			vMin.x = points[i].x;
		if (vMin.y > points[i].y)
			vMin.y = points[i].y;

		if (vMax.x < points[i].x)
			vMax.x = points[i].x;
		if (vMax.y < points[i].y)
			vMax.y = points[i].y;
	}

	if ((v.x < vMin.x) || (v.y < vMin.y) || (v.x > vMax.x) || (v.y > vMax.y))
		return false;

	uint32 nNbIntersection = 0;
	for (i = 0; i < points.size(); ++i)
	{
		const CVector &p1 = points[i];
		const CVector &p2 = points[(i+1)%points.size()];

		if (((p1.y-v.y) <= 0.0)&&((p2.y-v.y) <= 0.0))
			continue;
		if (((p1.y-v.y) > 0.0)&&((p2.y-v.y) > 0.0))
			continue;
		float xinter = p1.x + (p2.x-p1.x) * ((v.y-p1.y)/(p2.y-p1.y));
		if (xinter > v.x)
			++nNbIntersection;
	}
	if ((nNbIntersection&1) == 1) // odd intersections so the vertex is inside
		return true;
	else
		return false;
}

// ***************************************************************************
// CPrimNode
// ***************************************************************************

bool CPrimNode::read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config)
{
	return IPrimitive::read (xmlNode, filename, version, config);
}

// ***************************************************************************

uint CPrimNode::getNumVector () const
{
	return 0;
}

// ***************************************************************************

const CPrimVector *CPrimNode::getPrimVector () const
{
	return NULL;
}

// ***************************************************************************

CPrimVector	*CPrimNode::getPrimVector ()
{
	return NULL;
}

// ***************************************************************************

NLLIGO::IPrimitive *CPrimNode::copy () const
{
	return new CPrimNode (*this);
}


// ***************************************************************************
// CPrimNode
// ***************************************************************************

/*void CPrimNode::operator= (const CPrimNode &node)
{
	// Copy the IPrimitive
	IPrimitive::operator= (node);
}

// ***************************************************************************

void CPrimPoint::operator= (const CPrimPoint &node)
{
	// Copy the IPrimitive
	IPrimitive::operator= (node);
}

// ***************************************************************************

void CPrimPath::operator= (const CPrimPath &node)
{
	// Copy the IPrimitive
}

// ***************************************************************************

void CPrimZone::operator= (const CPrimZone &node)
{
	// Copy the IPrimitive
	IPrimitive::operator= (node);
}
*/

// ***************************************************************************

uint CPrimPoint::getNumVector () const
{
	return 1;
}

// ***************************************************************************

const CPrimVector *CPrimPoint::getPrimVector () const
{
	return &Point;
}

// ***************************************************************************

CPrimVector	*CPrimPoint::getPrimVector ()
{
	return &Point;
}

// ***************************************************************************

NLLIGO::IPrimitive *CPrimPoint::copy () const
{
	return new CPrimPoint (*this);
}

// ***************************************************************************

bool CPrimPoint::read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config)
{
	// Read points
	xmlNodePtr ptNode = GetFirstChildNode (xmlNode, filename, "PT");
	if (ptNode)
	{
		// Read a vector
		if (!ReadVector (Point, filename, ptNode))
			return false;

		ptNode = CIXml::getFirstChildNode (xmlNode, "ANGLE");
		if (ptNode)
		{
			// Read a float
			if (!ReadFloat ("VALUE", Angle, filename, ptNode))
				return false;
		}
		else
			Angle = 0;
	}
	else
	{
		return false;
	}

	return IPrimitive::read (xmlNode, filename, version, config);
}

// ***************************************************************************

void CPrimPoint::write (xmlNodePtr xmlNode, const char *filename) const
{
	// Save the point
	xmlNodePtr ptNode = xmlNewChild ( xmlNode, NULL, (const xmlChar*)"PT", NULL);
	WriteVector (Point, ptNode);

	// Save the angle
	if (Angle != 0)
	{
		xmlNodePtr ptNode = xmlNewChild ( xmlNode, NULL, (const xmlChar*)"ANGLE", NULL);
		WriteFloat ("VALUE", Angle, ptNode);
	}

	IPrimitive::write (xmlNode, filename);
}

// ***************************************************************************
// CPrimPath
// ***************************************************************************

uint CPrimPath::getNumVector () const
{
	return (uint)VPoints.size ();
}

// ***************************************************************************

const CPrimVector *CPrimPath::getPrimVector () const
{
	if (VPoints.empty())
		return NULL;
	return &(VPoints[0]);
}

// ***************************************************************************

NLLIGO::IPrimitive *CPrimPath::copy () const
{
	return new CPrimPath (*this);
}

// ***************************************************************************

CPrimVector	*CPrimPath::getPrimVector ()
{
	if (VPoints.empty())
		return NULL;
	return &(VPoints[0]);
}

// ***************************************************************************

bool CPrimPath::read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config)
{
	// Read points
	VPoints.clear ();
	VPoints.reserve (CIXml::countChildren (xmlNode, "PT"));
	xmlNodePtr ptNode = CIXml::getFirstChildNode (xmlNode, "PT");
	if (ptNode)
	{
		do
		{
			// Read a vector
			VPoints.push_back (CPrimVector ());
			if (!ReadVector (VPoints.back (), filename, ptNode))
				return false;

			ptNode = CIXml::getNextChildNode (ptNode, "PT");
		}
		while (ptNode);
	}

	return IPrimitive::read (xmlNode, filename, version, config);
}

// ***************************************************************************

void CPrimPath::write (xmlNodePtr xmlNode, const char *filename) const
{
	// Save the points
	for (uint i=0; i<VPoints.size (); i++)
	{
		xmlNodePtr ptNode = xmlNewChild ( xmlNode, NULL, (const xmlChar*)"PT", NULL);
		WriteVector (VPoints[i], ptNode);
	}

	IPrimitive::write (xmlNode, filename);
}

// ***************************************************************************
// CPrimZone
// ***************************************************************************

uint CPrimZone::getNumVector () const
{
	return (uint)VPoints.size ();
}

// ***************************************************************************

const CPrimVector *CPrimZone::getPrimVector () const
{
	if (VPoints.empty())
		return NULL;
	return &(VPoints[0]);
}

// ***************************************************************************

NLLIGO::IPrimitive *CPrimZone::copy () const
{
	return new CPrimZone (*this);
}

// ***************************************************************************

CPrimVector	*CPrimZone::getPrimVector ()
{
	if(VPoints.empty()) return 0;
	return &(VPoints[0]);
}

// ***************************************************************************

bool CPrimZone::read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config)
{
	// Read points
	VPoints.clear ();
	VPoints.reserve (CIXml::countChildren (xmlNode, "PT"));
	xmlNodePtr ptNode = CIXml::getFirstChildNode (xmlNode, "PT");
	if (ptNode)
	{
		do
		{
			// Read a vector
			VPoints.push_back (CPrimVector ());
			if (!ReadVector (VPoints.back (), filename, ptNode))
				return false;

			ptNode = CIXml::getNextChildNode (ptNode, "PT");
		}
		while (ptNode);
	}

	return IPrimitive::read (xmlNode, filename, version, config);
}

// ***************************************************************************

void CPrimZone::write (xmlNodePtr xmlNode, const char *filename) const
{
	// Save the points
	for (uint i=0; i<VPoints.size (); i++)
	{
		xmlNodePtr ptNode = xmlNewChild ( xmlNode, NULL, (const xmlChar*)"PT", NULL);
		WriteVector (VPoints[i], ptNode);
	}

	IPrimitive::write (xmlNode, filename);
}

// ***************************************************************************

bool CPrimZone::contains (const NLMISC::CVector &v, const std::vector<CPrimVector> &points, float &distance, NLMISC::CVector &nearPos, bool isPath)
{
	H_AUTO(NLLIGO_Contains1)
	uint32 i;
	CVector vMin, vMax;
	float nearest = FLT_MAX;
	CVector pos;

	// Point or line can't contains !
	if (points.size() < 3 || isPath)
	{
		// only compute the distance.
		if (points.size() == 1)
		{
			distance = (points[0] - v).norm();
			nearPos = points[0];
		}
		else if (points.size() == 2)
		{
			distance = getSegmentDist(v, points[0], points[1], nearPos);
		}
		else
		{
			// compute nearest segment
			for (i = 0; i < points.size()-1; ++i)
			{
				const CVector &p1 = points[i];
				const CVector &p2 = points[i+1];

				float dist = getSegmentDist(v, p1, p2, pos);
				if( dist < nearest)
				{
					nearest = dist;
					nearPos = pos;
				}
			}
			distance = nearest;
		}
		return false;
	}

	// Get the bounding rectangle of the zone
	vMax = vMin = points[0];
	for (i = 0; i < points.size(); ++i)
	{
		vMin.x = min(vMin.x, points[i].x);
		vMin.y = min(vMin.y, points[i].y);
		vMax.x = max(vMax.x, points[i].x);
		vMax.y = max(vMax.y, points[i].y);
	}

	if ((v.x < vMin.x) || (v.y < vMin.y) || (v.x > vMax.x) || (v.y > vMax.y))
	{
		// find the nearest distance of all segment
		for (uint i=0; i<points.size(); ++i)
		{
			float dist = getSegmentDist(v, points[i], points[(i+1) % points.size()], pos);

			if (dist < nearest)
			{
				nearest = dist;
				nearPos = pos;
			}
		}
		distance = nearest;
		return false;
	}

	uint32 nNbIntersection = 0;
	for (i = 0; i < points.size(); ++i)
	{
		const CVector &p1 = points[i];
		const CVector &p2 = points[(i+1)%points.size()];

		float dist = getSegmentDist(v, p1, p2, pos);
		if( dist < nearest)
		{
			nearest = dist;
			nearPos = pos;
		}

		if (((p1.y-v.y) <= 0.0)&&((p2.y-v.y) <= 0.0))
			continue;
		if (((p1.y-v.y) > 0.0)&&((p2.y-v.y) > 0.0))
			continue;
		float xinter = p1.x + (p2.x-p1.x) * ((v.y-p1.y)/(p2.y-p1.y));
		if (xinter > v.x)
			++nNbIntersection;
	}

	distance = nearest;
	if ((nNbIntersection&1) == 1) // odd intersections so the vertex is inside
		return true;
	else
		return false;
}

// ***************************************************************************

bool CPrimZone::contains (const NLMISC::CVector &v, const std::vector<CVector> &points, float &distance, NLMISC::CVector &nearPos, bool isPath)
{
	H_AUTO(NLLIGO_Contains2)
	uint32 i;
	CVector vMin, vMax;
	float nearest = FLT_MAX;
	CVector pos;

	// Point or line can't contains !
	if (points.size() < 3 || isPath)
	{
		// only compute the distance.
		if (points.size() == 1)
		{
			distance = (points[0] - v).norm();
			nearPos = points[0];
		}
		else if (points.size() == 2)
		{
			distance = getSegmentDist(v, points[0], points[1], nearPos);
		}
		else
		{
			// compute nearest segment
			for (i = 0; i < points.size()-1; ++i)
			{
				const CVector &p1 = points[i];
				const CVector &p2 = points[i+1];

				float dist = getSegmentDist(v, p1, p2, pos);
				if( dist < nearest)
				{
					nearest = dist;
					nearPos = pos;
				}
			}
			distance = nearest;
		}
		return false;
	}

	// Get the bounding rectangle of the zone
	vMax = vMin = points[0];
	for (i = 0; i < points.size(); ++i)
	{
		vMin.x = min(vMin.x, points[i].x);
		vMin.y = min(vMin.y, points[i].y);
		vMax.x = max(vMax.x, points[i].x);
		vMax.y = max(vMax.y, points[i].y);
	}

	if ((v.x < vMin.x) || (v.y < vMin.y) || (v.x > vMax.x) || (v.y > vMax.y))
	{
		// find the nearest distance of all segment
		for (uint i=0; i<points.size(); ++i)
		{
			float dist = getSegmentDist(v, points[i], points[(i+1) % points.size()], pos);

			if (dist < nearest)
			{
				nearest = dist;
				nearPos = pos;
			}
		}
		distance = nearest;
		return false;
	}

	uint32 nNbIntersection = 0;
	for (i = 0; i < points.size(); ++i)
	{
		const CVector &p1 = points[i];
		const CVector &p2 = points[(i+1)%points.size()];

		float dist = getSegmentDist(v, p1, p2, pos);
		if( dist < nearest)
		{
			nearest = dist;
			nearPos = pos;
		}

		if (((p1.y-v.y) <= 0.0)&&((p2.y-v.y) <= 0.0))
			continue;
		if (((p1.y-v.y) > 0.0)&&((p2.y-v.y) > 0.0))
			continue;
		float xinter = p1.x + (p2.x-p1.x) * ((v.y-p1.y)/(p2.y-p1.y));
		if (xinter > v.x)
			++nNbIntersection;
	}

	distance = nearest;
	if ((nNbIntersection&1) == 1) // odd intersections so the vertex is inside
		return true;
	else
		return false;
}

// ***************************************************************************

float CPrimZone::getSegmentDist(const NLMISC::CVector v, const NLMISC::CVector &p1, const NLMISC::CVector &p2, NLMISC::CVector &nearPos)
{
	// two points, compute distance to the segment.
	CVector V = (p2-p1).normed();
	double	length= (p2-p1).norm();
	float distance;

	// case where p1==p2
	if(length==0.0)
	{
		nearPos= p1;
		distance = (p1-v).norm();
	}
	// standard case
	else
	{
		float t = (float)((double)((v-p1)*V)/length);
		if (t < 0.0f)
		{
			nearPos = p1;
			distance = (p1-v).norm();
		}
		else if (t > 1.0f)
		{
			nearPos = p2;
			distance = (p2-v).norm();
		}
		else
		{
			nearPos = p1 + t*(p2-p1);
			distance = (v-nearPos).norm();
		}
	}

	return distance;
}


// ***************************************************************************
NLMISC::CVector CPrimZone::getBarycentre() const
{
	CVector sum( CVector::Null );
	uint n = (uint)VPoints.size();
	if ( n != 0 )
	{
		for ( uint i=0; i!=n; ++i )
			sum += VPoints[i];
		return sum / (float)n;
	}
	else
		return sum;
}

// ***************************************************************************
void CPrimZone::getAABox( NLMISC::CVector& cornerMin, NLMISC::CVector& cornerMax ) const
{
	cornerMin.x = FLT_MAX;
	cornerMin.y = FLT_MAX;
	cornerMin.z = 0;
	cornerMax.x = -FLT_MAX;
	cornerMax.y = -FLT_MAX;
	cornerMax.z = 0;
	for ( uint i=0; i!=VPoints.size(); ++i )
	{
		const CVector& p = VPoints[i];
		if ( p.x < cornerMin.x )
			cornerMin.x = p.x;
		if ( p.x > cornerMax.x )
			cornerMax.x = p.x;
		if ( p.y < cornerMin.y )
			cornerMin.y = p.y;
		if ( p.y > cornerMax.y )
			cornerMax.y = p.y;
	}
}


// ***************************************************************************
float CPrimZone::getAreaOfAABox() const
{
	CVector cornerMin, cornerMax;
	getAABox( cornerMin, cornerMax );
	return (cornerMax.x-cornerMin.x) * (cornerMax.y-cornerMin.y);
}


// ***************************************************************************
void CPrimZone::serial (NLMISC::IStream &f)
{
	IPrimitive::serial(f);
	f.serialCont(VPoints);
}

// ***************************************************************************
void CPrimRegion::serial (NLMISC::IStream &f)
{
	f.xmlPushBegin ("REGION");

	f.xmlSetAttrib ("NAME");
	f.serial (Name);

	f.xmlPushEnd();

	sint version = 2;
	version = f.serialVersion (version);
	string check = "REGION";
	f.serialCheck (check);

	f.xmlPush ("POINTS");
		f.serialCont (VPoints);
	f.xmlPop ();
	f.xmlPush ("PATHES");
		f.serialCont (VPaths);
	f.xmlPop ();
	f.xmlPush ("ZONES");
		f.serialCont (VZones);
	f.xmlPop ();

	if (version > 1)
	{
		f.xmlPush ("HIDEPOINTS");
			f.serialCont (VHidePoints);
		f.xmlPop ();
		f.xmlPush ("HIDEZONES");
			f.serialCont (VHideZones);
		f.xmlPop ();
		f.xmlPush ("HIDEPATHS");
			f.serialCont (VHidePaths);
		f.xmlPop ();
	}
	else
	{
		VHidePoints.resize	(VPoints.size(), false);
		VHideZones.resize	(VZones.size(),	false);
		VHidePaths.resize	(VPaths.size(), false);
	}
}

// ***************************************************************************
// IPrimitive
// ***************************************************************************

IPrimitive::IPrimitive ()
{
	_Parent = NULL;
}


IPrimitive::IPrimitive (const IPrimitive &node) : IStreamable()
{
	_Parent = NULL;
	IPrimitive::operator= (node);
}

// ***************************************************************************

void IPrimitive::serial (NLMISC::IStream &f)
{
	// NB : unparsed parameters are not binary serialized !

	// serialize the property container
	if (f.isReading())
	{
		uint32 size;
		f.serial(size);
		for (uint i=0; i<size; ++i)
		{
			std::string s;
			f.serial(s);
			IProperty *&pp = _Properties[s];
			f.serialPolyPtr(pp);
		}
	}
	else
	{
		uint32 size = (uint32)_Properties.size();
		f.serial(size);
		std::map<std::string, IProperty*>::iterator first(_Properties.begin()), last(_Properties.end());
		for (; first != last; ++first)
		{
			std::string &s = const_cast<std::string&>(first->first);

			f.serial(s);
			f.serialPolyPtr(first->second);
		}
	}
	f.serial(_ChildId);

//	f.serial(Layer);
//	f.serial(Name);
//	f.serial(Expanded);

	// serial the childrens
	if (f.isReading())
	{
		std::vector<IPrimitive*> children;
		f.serialContPolyPtr(children);
		uint index = 0;
		for(std::vector<IPrimitive*>::iterator it = children.begin(); it != children.end(); ++it, ++index)
		{
			insertChild(*it, index);
		}
	}
	else
	{
		f.serialContPolyPtr(_Children);
	}

	if (f.isReading())
	{
		// reloc child link
		vector<IPrimitive*>::iterator first(_Children.begin()), last(_Children.end());
		for (; first != last; ++first)
		{
			if (*first)
				(*first)->_Parent = this;
		}
	}
}


// ***************************************************************************

void IPrimitive::updateChildId (uint index)
{
	uint i;
	uint count = (uint)_Children.size ();
	for (i=index; i<count; i++)
		_Children[i]->_ChildId = i;
}

// ***************************************************************************

void IPrimitive::branchLink()
{
	onBranchLink();
	std::vector<IPrimitive*>::iterator first(_Children.begin()), last(_Children.end());
	for (; first != last; ++first)
	{
		(*first)->branchLink();
	}
}

// ***************************************************************************

void IPrimitive::branchUnlink()
{
	onBranchUnlink();
	std::vector<IPrimitive*>::iterator first(_Children.begin()), last(_Children.end());
	for (; first != last; ++first)
	{
		(*first)->branchUnlink();
	}
}


// ***************************************************************************

void IPrimitive::operator= (const IPrimitive &node)
{
	// Clean dest
	removeChildren ();
	removeProperties ();

	// copy deprecated param
//	Layer = node.Layer;
//	Name = node.Name;

	// copy unparsed properties
	_UnparsedProperties = node._UnparsedProperties;

	// Copy the flags
//	Expanded = node.Expanded;
	_ChildId = node._ChildId;

	// Copy children
	_Children.resize (node._Children.size ());
	for (uint child = 0; child < node._Children.size (); child++)
	{
		// Copy the child
		_Children[child] = node._Children[child]->copy ();

		// Set the parent
		_Children[child]->_Parent = this;
	}

	// Copy properties
	std::map<std::string, IProperty*>::const_iterator ite = node._Properties.begin ();
	while (ite != node._Properties.end ())
	{
		// Get the property
		CPropertyString *propString = dynamic_cast<CPropertyString *>(ite->second);
		if (propString)
		{
			// New property
			CPropertyString *newProp = new CPropertyString ();
			*newProp = *propString;
			_Properties.insert (std::map<std::string, IProperty*>::value_type (ite->first, newProp));
		}
		else
		{
			CPropertyStringArray *propStringArray = dynamic_cast<CPropertyStringArray *>(ite->second);
			if (propStringArray)
			{
				// New property
				CPropertyStringArray *newProp = new CPropertyStringArray ();
				*newProp = *propStringArray;
				_Properties.insert (std::map<std::string, IProperty*>::value_type (ite->first, newProp));
			}
			else
			{
				CPropertyColor *propColor = dynamic_cast<CPropertyColor *>(ite->second);
				nlverify (propColor);

				// New property
				CPropertyColor *newProp = new CPropertyColor ();
				*newProp = *propColor;
				_Properties.insert (std::map<std::string, IProperty*>::value_type (ite->first, newProp));
			}
		}

		ite++;
	}

#ifdef NLLIGO_DEBUG
	_DebugClassName = node._DebugClassName;
	_DebugPrimitiveName = node._DebugPrimitiveName;
#endif
}


const	IPrimitive	*IPrimitive::getPrimitive	(const	std::string	&absoluteOrRelativePath)	const
{
	const	IPrimitive	*cursor=this;
	string	path=absoluteOrRelativePath;

	if (path.find("//")==0)	//	an absolute path.
	{
		while (cursor->getParent())
			cursor=cursor->getParent();
		path.erase(0,2);
	}

	while (path.size()>0)
	{
		if	(path.find("/")==0)
		{
			path.erase(0,1);
			continue;
		}
		if	(path.find("..")==0)
		{
			cursor=cursor->getParent();
			if	(!cursor)
				return	NULL;

			path.erase(0,2);
			continue;
		}

		string::size_type indexStr=path.find("/");
		string	          childName;
		if (indexStr==string::npos)
		{
			childName=path;
			path="";
		}
		else
		{
			childName=path.substr(0,indexStr);
			path.erase(0, indexStr);
		}
		childName=toUpper(childName);
		const	IPrimitive*child=NULL;
		uint	childIndex;
		for	(childIndex=0;childIndex<cursor->getNumChildren();childIndex++)
		{
			cursor->getChild(child,childIndex);
			string	name;
			if	(	child->getPropertyByName("class", name)
				&&	toUpper(name)==childName	)
				break;
		}
		if	(childIndex>=cursor->getNumChildren())
			return	NULL;

		cursor=child;
	}
	return	cursor;
}

// ***************************************************************************

bool IPrimitive::getProperty (uint index, std::string &property_name, const IProperty *&result) const
{
	// Look for the property
	std::map<std::string, IProperty*>::const_iterator ite = _Properties.begin ();
	while (ite != _Properties.end ())
	{
		if (index == 0)
		{
			property_name = ite->first;
			result = ite->second;
			return true;
		}
		index--;
		ite ++;
	}
	nlwarning ("NLLIGO::IPrimitive::getProperty : invalid index (index : %d, size : %d).", index, _Properties.size ());
	return false;
}

// ***************************************************************************

bool IPrimitive::getProperty (uint index, std::string &property_name, IProperty *&result)
{
	// Look for the property
	std::map<std::string, IProperty*>::iterator ite = _Properties.begin ();
	while (ite != _Properties.end ())
	{
		if (index == 0)
		{
			property_name = ite->first;
			result = ite->second;
			return true;
		}
		index--;
		ite ++;
	}
	nlwarning ("NLLIGO::IPrimitive::getProperty : invalid index (index : %d, size : %d).", index, _Properties.size ());
	return false;
}

// ***************************************************************************

bool IPrimitive::getPropertyByName (const char *property_name, const IProperty *&result) const
{
	// Look for the property
	std::map<std::string, IProperty*>::const_iterator ite = _Properties.find (property_name);
	if (ite != _Properties.end ())
	{
		result = ite->second;
		return true;
	}
	return false;
}

// ***************************************************************************

bool IPrimitive::getPropertyByName (const char *property_name, IProperty *&result) const
{
	// Look for the property
	std::map<std::string, IProperty*>::const_iterator ite = _Properties.find (property_name);
	if (ite != _Properties.end ())
	{
		result = ite->second;
		return true;
	}
	return false;
}

// ***************************************************************************

bool IPrimitive::getPropertyByName (const char *property_name, std::string *&result) const
{
	// Get the property
	IProperty *prop;
	if (getPropertyByName (property_name, prop))
	{
		CPropertyString *strProp = dynamic_cast<CPropertyString *> (prop);
		if (strProp)
		{
			result = &(strProp->String);
			return true;
		}
		else
		{
			nlwarning ("NLLIGO::IPrimitive::getPropertyByName : property (%s) in not a string.", property_name);
		}
	}
	return false;
}

// ***************************************************************************

bool IPrimitive::getPropertyByName (const char *property_name, std::string &result) const
{
	// Get the property
	const IProperty *prop;
	if (getPropertyByName (property_name, prop))
	{
		const CPropertyString *strProp = dynamic_cast<const CPropertyString *> (prop);
		if (strProp)
		{
			result = strProp->String;
			return true;
		}
		else
		{
			nlwarning ("NLLIGO::IPrimitive::getPropertyByName : property (%s) in not a string.", property_name);
		}
	}
	return false;
}

// ***************************************************************************

bool IPrimitive::getPropertyByName (const char *property_name, std::vector<std::string> *&result) const
{
	// Get the property
	IProperty *prop;
	if (getPropertyByName (property_name, prop))
	{
		CPropertyStringArray *strProp = dynamic_cast<CPropertyStringArray *> (prop);
		if (strProp)
		{
			result = &(strProp->StringArray);
			return true;
		}
		else
		{
			nlwarning ("NLLIGO::IPrimitive::getPropertyByName : property (%s) in not a string.", property_name);
		}
	}
	return false;
}

// ***************************************************************************

bool IPrimitive::getPropertyByName (const char *property_name, const std::vector<std::string> *&result) const
{
	// Get the property
	IProperty *prop;
	if (getPropertyByName (property_name, prop))
	{
		const CPropertyStringArray *strProp = dynamic_cast<const CPropertyStringArray *> (prop);
		if (strProp)
		{
			result = &(strProp->StringArray);
			return true;
		}
		else
		{
			nlwarning ("NLLIGO::IPrimitive::getPropertyByName : property (%s) in not a string.", property_name);
		}
	}
	return false;
}

// ***************************************************************************

bool IPrimitive::getPropertyByName (const char *property_name, NLMISC::CRGBA &result) const
{
	// Get the property
	IProperty *prop;
	if (getPropertyByName (property_name, prop))
	{
		const CPropertyColor *colorProp = dynamic_cast<const CPropertyColor *> (prop);
		if (colorProp)
		{
			result = colorProp->Color;
			return true;
		}
		else
		{
			nlwarning ("NLLIGO::IPrimitive::getPropertyByName : property (%s) in not a color.", property_name);
		}
	}
	return false;
}

// ***************************************************************************

bool IPrimitive::removeProperty (uint index)
{
	// Look for the property
	std::map<std::string, IProperty*>::iterator ite = _Properties.begin ();
	while (ite != _Properties.end ())
	{
		if (index == 0)
		{
			_Properties.erase (ite);
			return true;
		}
		index--;
		ite ++;
	}
	nlwarning ("NLLIGO::IPrimitive::removeProperty : invalid index (index : %d, size : %d).", index, _Properties.size ());
	return false;
}

// ***************************************************************************

bool IPrimitive::removePropertyByName (const char *property_name)
{
	// Look for the property
	std::map<std::string, IProperty*>::iterator ite = _Properties.find (property_name);
	if (ite != _Properties.end ())
	{
		_Properties.erase (ite);
		return true;
	}
	return false;
}

// ***************************************************************************

void IPrimitive::removeProperties ()
{
	std::map<std::string, IProperty*>::iterator ite = _Properties.begin ();
	while (ite != _Properties.end ())
	{
		delete ite->second;
		ite++;
	}
	_Properties.clear ();
}

// ***************************************************************************

bool IPrimitive::getChild (const IPrimitive *&result, uint childId) const
{
	if (childId < _Children.size ())
	{
		result = _Children[childId];
		return true;
	}
	else
	{
		nlwarning ("NLLIGO::IPrimitive::getChild : invalid index (index : %d, size %d).", childId, _Children.size ());
	}
	return false;
}

// ***************************************************************************

bool IPrimitive::getChild (IPrimitive *&result, uint childId)
{
	if (childId < _Children.size ())
	{
		result = _Children[childId];
		return true;
	}
	else
	{
		nlwarning ("NLLIGO::IPrimitive::getChild : invalid index (index : %d, size %d).", childId, _Children.size ());
	}
	return false;
}

// ***************************************************************************

bool IPrimitive::removeChild (IPrimitive *child)
{
	uint childId;
	if (getChildId(childId, child))
	{
		return removeChild(childId);
	}
	else
	{
		nlwarning("NLLIGO::IPrimitive::removeChild : invalid child, can't remove (child : %p)", child);
	}
	return false;
}

// ***************************************************************************

bool IPrimitive::removeChild (uint childId)
{
	if (childId < _Children.size ())
	{
		delete _Children[childId];
		_Children.erase (_Children.begin()+childId);
		updateChildId (childId);
		return true;
	}
	else
	{
		nlwarning ("NLLIGO::IPrimitive::removeChild : invalid index (index : %d, size %d).", childId, _Children.size ());
	}
	return false;
}

// ***************************************************************************

void IPrimitive::removeChildren ()
{
	// Erase children
	for (uint i=0; i<_Children.size (); i++)
	{
		delete _Children[i];
	}
	_Children.clear ();
}

// ***************************************************************************

bool IPrimitive::unlinkChild(IPrimitive *child)
{
	uint childId;
	if (getChildId(childId, child))
	{
		child->onUnlinkFromParent();
		child->branchUnlink();
		_Children.erase (_Children.begin()+childId);
		updateChildId (childId);
		child->_Parent = NULL;
		child->_ChildId = 0;
		return true;
	}
	else
	{
		nlwarning("NLLIGO::IPrimitive::unlinkChild : invalid child, can't unlink (child : %p)", child);
	}
	return false;
}

// ***************************************************************************

bool IPrimitive::insertChild (IPrimitive *primitive, uint index)
{
	// At the end ?
	if (index == AtTheEnd)
		index = (uint)_Children.size ();

	// Index valid ?
	if (index>_Children.size ())
		return false;

	// Insert
	_Children.insert (_Children.begin () + index, primitive);

	// Update child id
	updateChildId (index);

	// Link to the parent
	primitive->_Parent = this;

	// signaling
	primitive->onLinkToParent();
	primitive->branchLink();

	return true;
}

// ***************************************************************************

IPrimitive::~IPrimitive ()
{
	// Remove children
	removeChildren ();

	// Erase properties
	removeProperties ();
}

// ***************************************************************************
bool IPrimitive::checkProperty(const std::string &property_name) const
{
	if (_Properties.find(property_name) == _Properties.end())
		return false;
	return true;
}

// ***************************************************************************

bool IPrimitive::addPropertyByName (const char *property_name, IProperty *result)
{
	bool inserted = _Properties.insert (std::map<std::string, IProperty*>::value_type (property_name, result)).second;
	if (inserted)
	{
		return true;
	}
	return false;
}

// ***************************************************************************

bool IPrimitive::read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config)
{
	// Erase old properties
	_Properties.clear ();

	// Read the unparsed properties (for editor view)
	xmlNodePtr commentNode = CIXml::getFirstChildNode(xmlNode, XML_COMMENT_NODE);
	if (commentNode)
	{
		 if (!CIXml::getContentString(_UnparsedProperties, commentNode))
			_UnparsedProperties = "";
	}

	// Read the expanded flag
//	string expanded;
//	Expanded = true;
//	if (CIXml::getPropertyString (expanded, xmlNode, "EXPANDED"))
//		Expanded = (expanded != "false");

	// Read the properties
	xmlNodePtr propNode;
	propNode = CIXml::getFirstChildNode (xmlNode, "PROPERTY");
	if (propNode)
	{
		do
		{
			// Read the name
			string name;
			if (GetNodeString (name, filename, propNode, "NAME"))
			{
				// Get the property type
				string type;
				if (GetPropertyString (type, filename, propNode, "TYPE"))
				{
					// The property
					IProperty *property = NULL;

					// Check the type
					if (type == "string")
					{
						// Create a new property
						CPropertyString *propertyString = new CPropertyString;
						property = propertyString;

						// Read it
						if (!GetNodeString (propertyString->String, filename, propNode, "STRING"))
						{
							return false;
						}
					}
					else if (type == "string_array")
					{
						// Create a new property
						CPropertyStringArray *propertyStringArray = new CPropertyStringArray;
						property = propertyStringArray;

						// Read strings
						xmlNodePtr stringNode;
						propertyStringArray->StringArray.reserve (CIXml::countChildren (propNode, "STRING"));
						stringNode = CIXml::getFirstChildNode (propNode, "STRING");
						if (stringNode)
						{
							do
							{
								// Add the string
								string content;
								GetContentString (content, filename, stringNode);
								propertyStringArray->StringArray.push_back (content);

								stringNode = CIXml::getNextChildNode (stringNode, "STRING");
							}
							while (stringNode);
						}
					}
					else if (type == "color")
					{
						// Create a new property
						CPropertyColor *propertyColor= new CPropertyColor;
						property = propertyColor;

						// Read strings
						xmlNodePtr colorNode;
						colorNode = CIXml::getFirstChildNode (xmlNode, "COLOR");
						string R, G, B, A;
						if (GetPropertyString (R, filename, colorNode, "R") &&
							GetPropertyString (G, filename, colorNode, "G") &&
							GetPropertyString (B, filename, colorNode, "B") &&
							GetPropertyString (A, filename, colorNode, "A"))
						{
							sint32 sR=0, sG=0, sB=0, sA=255;
							sR = atoi (R.c_str ());
							clamp (sR, 0, 255);
							sG = atoi (G.c_str ());
							clamp (sG, 0, 255);
							sB = atoi (B.c_str ());
							clamp (sB, 0, 255);
							sA = atoi (A.c_str ());
							clamp (sR, 0, 255);
							propertyColor->Color.R = (uint8)sR;
							propertyColor->Color.G = (uint8)sG;
							propertyColor->Color.B = (uint8)sB;
							propertyColor->Color.A = (uint8)sA;
						}
						else
							return false;
					}

					// Property found ?
					if (property == NULL)
					{
						XMLError (propNode, filename, "IPrimitive::read : Unknown property type (%s)", type.c_str ());
						return false;
					}

					// Add it
					_Properties.insert (std::map<std::string, IProperty*>::value_type (name, property));
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			propNode = CIXml::getNextChildNode (propNode, "PROPERTY");
		}
		while (propNode);
	}

	// Initialise default value
	initDefaultValues (config);

	// Read children
	xmlNodePtr childNode;
	childNode = CIXml::getFirstChildNode (xmlNode, "CHILD");
	if (childNode)
	{
		do
		{
			// Get the property class
			string type;
			if (GetPropertyString (type, filename, childNode, "TYPE"))
			{
				// Primitive
				if (type=="node")
					type="CPrimNode";
				if (type=="point")
					type="CPrimPoint";
				if (type=="path")
					type="CPrimPath";
				if (type=="zone")
					type="CPrimZone";
				if (type=="alias")
					type="CPrimAlias";
				IPrimitive *primitive = static_cast<IPrimitive *> (CClassRegistry::create (type));

				// Primitive type not found ?
				if (primitive == NULL)
				{
					XMLError (childNode, filename, "IPrimitive::read : Unknown primitive type (%s)", type.c_str ());
					return false;
				}

				// Read it
				primitive->read (childNode, filename, version, config);

				// Add it
				insertChild (primitive);

			}
			else
			{
				return false;
			}

			childNode = CIXml::getNextChildNode (childNode, "CHILD");
		}
		while (childNode);
	}

#ifdef NLLIGO_DEBUG
	// store debug data
 	getPropertyByName("class", _DebugClassName);
	getPropertyByName("name", _DebugPrimitiveName);
#endif
	// Done
	return true;
}

// ***************************************************************************

void IPrimitive::initDefaultValues (CLigoConfig &config)
{
	// Get the primitive class
	const CPrimitiveClass *primitiveClass = config.getPrimitiveClass (*this);
	if (primitiveClass)
	{
		// For each properties
		uint count = (uint)primitiveClass->Parameters.size ();
		uint i;
		for (i=0; i<count; i++)
		{
			const CPrimitiveClass::CParameter &parameter = primitiveClass->Parameters[i];

			// Get the property
			IProperty *result;
			if (!getPropertyByName (parameter.Name.c_str(), result))
			{
				// Create the property
				if ((parameter.Type == CPrimitiveClass::CParameter::StringArray) || (parameter.Type == CPrimitiveClass::CParameter::ConstStringArray))
					result = new CPropertyStringArray();
				else
					result = new CPropertyString();
				nlverify (addPropertyByName (parameter.Name.c_str(), result));
			}
		}

		// Set the default values
		for (i=0; i<count; i++)
		{
			const CPrimitiveClass::CParameter &parameter = primitiveClass->Parameters[i];

			CPropertyString *pString = NULL;
			CPropertyStringArray *pStringArray = NULL;

			IProperty *result;
			nlverify (getPropertyByName (parameter.Name.c_str(), result));
			pString = dynamic_cast<CPropertyString*>(result);
			if (!pString)
				pStringArray = dynamic_cast<CPropertyStringArray*>(result);

			// Property have default values ?
			if (pString)
			{
				// Empty string ?
				if (pString->String.empty())
				{
					// Set as default
					pString->Default = true;
					parameter.getDefaultValue (pString->String, *this, *primitiveClass);
				}
			}
			else if (pStringArray)
			{
				// Empty string array ?
				if (pStringArray->StringArray.empty())
				{
					// Set as default
					pStringArray->Default = true;
					parameter.getDefaultValue (pStringArray->StringArray, *this, *primitiveClass);
				}
			}
		}
	}
}

// ***************************************************************************

void IPrimitive::write (xmlNodePtr xmlNode, const char *filename) const
{
	// Save the expanded flag
//	if (!Expanded)
//		xmlSetProp (xmlNode, (const xmlChar*)"EXPANDED", (const xmlChar*)"false");

	// Set the type
	xmlSetProp (xmlNode, (const xmlChar*)"TYPE", (const xmlChar*)(const_cast<IPrimitive*> (this)->getClassName ().c_str ()));

	// Save the unparsed property
	if (!_UnparsedProperties.empty())
	{
		xmlNodePtr commentNode = xmlNewComment((const xmlChar*)(_UnparsedProperties.c_str()));
		nlverify(commentNode);
		xmlAddChild(xmlNode, commentNode);
	}

	// Save the properties
	std::map<std::string, IProperty*>::const_iterator ite =	_Properties.begin ();
	while (ite != _Properties.end ())
	{
		// Not a default property ?
		if (!ite->second->Default)
		{
			// Create new nodes
			xmlNodePtr propNode = xmlNewChild ( xmlNode, NULL, (const xmlChar*)"PROPERTY", NULL);
			xmlNodePtr nameNode = xmlNewChild ( propNode, NULL, (const xmlChar*)"NAME", NULL);
			xmlNodePtr textNode = xmlNewText ((const xmlChar *)(ite->first.c_str ()));
			xmlAddChild (nameNode, textNode);

			// Type
			const CPropertyString *str = dynamic_cast<const CPropertyString *> (ite->second);
			if (str)
			{
				// Set the type
				xmlSetProp (propNode, (const xmlChar*)"TYPE", (const xmlChar*)"string");

				// Create new nodes
				xmlNodePtr stringNode = xmlNewChild ( propNode, NULL, (const xmlChar*)"STRING", NULL);
				xmlNodePtr textNode = xmlNewText ((const xmlChar *)(str->String.c_str ()));
				xmlAddChild (stringNode, textNode);
			}
			else
			{
				// Should be an array
				const CPropertyStringArray *array = dynamic_cast<const CPropertyStringArray *> (ite->second);
				if (array)
				{
					// Set the type
					xmlSetProp (propNode, (const xmlChar*)"TYPE", (const xmlChar*)"string_array");

					// For each strings in the array
					for (uint i=0; i<array->StringArray.size (); i++)
					{
						// Create new nodes
						xmlNodePtr stringNode = xmlNewChild ( propNode, NULL, (const xmlChar*)"STRING", NULL);
						xmlNodePtr textNode = xmlNewText ((const xmlChar *)(array->StringArray[i].c_str ()));
						xmlAddChild (stringNode, textNode);
					}
				}
				else
				{
					// Should be a color
					const CPropertyColor *color = safe_cast<const CPropertyColor *> (ite->second);

					// Set the type
					xmlSetProp (propNode, (const xmlChar*)"TYPE", (const xmlChar*)"color");

					// Create new nodes
					xmlNodePtr colorNode = xmlNewChild ( propNode, NULL, (const xmlChar*)"COLOR", NULL);
					xmlSetProp (colorNode, (const xmlChar*)"R", (const xmlChar*)toString (color->Color.R).c_str ());
					xmlSetProp (colorNode, (const xmlChar*)"G", (const xmlChar*)toString (color->Color.G).c_str ());
					xmlSetProp (colorNode, (const xmlChar*)"B", (const xmlChar*)toString (color->Color.B).c_str ());
					xmlSetProp (colorNode, (const xmlChar*)"A", (const xmlChar*)toString (color->Color.A).c_str ());
				}
			}
		}

		ite++;
	}

	// Save the children
	for (uint i=0; i<_Children.size (); i++)
	{
		// New node
		xmlNodePtr childNode = xmlNewChild ( xmlNode, NULL, (const xmlChar*)"CHILD", NULL);

		// Write it
		_Children[i]->write (childNode, filename);
	}
}

// ***************************************************************************

bool IPrimitive::getChildId (uint &childId, const IPrimitive *child) const
{
	childId = child->_ChildId;
	return true;
}

// ***************************************************************************

uint IPrimitive::getNumProperty () const
{
	return (uint)_Properties.size ();
}

// ***************************************************************************

std::string		IPrimitive::getName() const
{
	std::string	ret;
	getPropertyByName("name", ret);
	return ret;
}

// ***************************************************************************

const std::string &IPrimitive::getUnparsedProperties() const
{
	return _UnparsedProperties;
}

// ***************************************************************************

void IPrimitive::setUnparsedProperties(const std::string &unparsedProperties) const
{
	_UnparsedProperties = unparsedProperties;
}

// ***************************************************************************
// CPrimAlias
// ***************************************************************************

CPrimAlias::CPrimAlias() :
	_Alias(0),
	_Container(NULL)
{
}

CPrimAlias::CPrimAlias(const CPrimAlias &other)
	: IPrimitive(other)
{
	// clear the container reference and alias
	_Container = NULL;
	_Alias = other._Alias;
}

CPrimAlias::~CPrimAlias()
{
	if (_Container)
		onBranchUnlink();
}

void CPrimAlias::onBranchLink()
{
	CPrimitiveContext	&ctx = CPrimitiveContext::instance();
	// context must be set when handling alias
	nlassert(ctx.CurrentPrimitive);
	nlassert(_Container ==  NULL || _Container == ctx.CurrentPrimitive);

	_Container = ctx.CurrentPrimitive;

	// generate a new alias, eventually keeping the current one if any and if still available
	_Alias = _Container->genAlias(this, _Alias);
}

void CPrimAlias::onBranchUnlink()
{
	nlassert(_Container !=  NULL);
	_Container->releaseAlias(this, _Alias);
	_Container = NULL;

	// NB : we keep the alias value for next linkage
}

uint32	CPrimAlias::getAlias() const
{
	return _Alias;
}

uint32	CPrimAlias::getFullAlias() const
{
	nlassert(_Container != NULL);
	return _Container->buildFullAlias(_Alias);
}

void CPrimAlias::regenAlias()
{
	// container must exist
	nlassert(_Container);
	// generate a new alias, eventually keeping the current one if any and if still available
	_Alias = _Container->genAlias(this, _Alias);
}


// Read the primitive
bool CPrimAlias::read (xmlNodePtr xmlNode, const char *filename, uint version, CLigoConfig &config)
{
	// Read alias
	xmlNodePtr ptNode = CIXml::getFirstChildNode (xmlNode, "ALIAS");
	if (ptNode)
	{
		sint val = 0;
		if (ReadInt ("VALUE", val, filename, ptNode))
		{
			_Alias = uint32(val);

//			nlassert( CPrimitiveContext::instance().CurrentPrimitive);
////			nlassert(_Container);
//			 CPrimitiveContext::instance().CurrentPrimitive->reserveAlias(_Alias);
//			// set to null, it will be rewrited by onBranchLink callback
////			_Container = NULL;
		}
		else
		{
			// error in format !
			nlwarning("CPrimAlias::read: Can't find xml property 'VALUE' in element <ALIAS>");
			return false;
		}
	}
	else
	{
		// error in format !
		nlwarning("CPrimAlias::read: Can't find xml element <ALIAS>");
		return false;
	}

	return IPrimitive::read (xmlNode, filename, version, config);
}
// Write the primitive
void CPrimAlias::write (xmlNodePtr xmlNode, const char *filename) const
{
	// Write alias
	xmlNodePtr ptNode = xmlNewChild(xmlNode, NULL, (const xmlChar*)"ALIAS", NULL);
	WriteInt("VALUE", int(_Alias), ptNode);

	IPrimitive::write (xmlNode, filename);
}

// Create a copy of this primitive
IPrimitive *CPrimAlias::copy () const
{
	// NB : this will not call the reserveAlias on the container
	CPrimAlias *pa = new CPrimAlias(*this);

	// clear the alias and container reference
//	pa->_Alias = 0;
//	pa->_Container = 0;

	return pa;
}
// serial for binary save
void CPrimAlias::serial (NLMISC::IStream &f)
{
	IPrimitive::serial(f);

	f.serial(_Alias);

//	if (f.isReading())
//	{
//		nlassert(_Container);
//		_Container->reserveAlias(_Alias);
//	}
}


// ***************************************************************************
// CPrimitives
// ***************************************************************************

CPrimitives::CPrimitives () :
	_LigoConfig(NULL)
{
	// init the alias generator
	_LastGeneratedAlias = 0;
	_AliasStaticPart = 0;

	RootNode = static_cast<CPrimNode *> (CClassRegistry::create ("CPrimNode"));

	// get the current ligo context (if any)
	_LigoConfig = CPrimitiveContext::instance().CurrentLigoConfig;
}

// ***************************************************************************

CPrimitives::CPrimitives (const CPrimitives &other)
{
	operator =(other);
//	_LastGeneratedAlias = other._LastGeneratedAlias;
//	// get the current ligo context (if any)
//	_LigoConfig = CPrimitiveContext::instance().CurrentLigoConfig;
//
//	CPrimitives *temp = CPrimitiveContext::instance().CurrentPrimitive;
//	CPrimitiveContext::instance().CurrentPrimitive = this;
//	// copy the nodes
//	RootNode = static_cast<CPrimNode *> (((IPrimitive*)other.RootNode)->copy ());
//	RootNode->branchLink();
//
//	CPrimitiveContext::instance().CurrentPrimitive = temp;
}

// ***************************************************************************

CPrimitives::~CPrimitives ()
{
	delete RootNode;
}

// ***************************************************************************

uint32 CPrimitives::getAliasStaticPart()
{
	return _AliasStaticPart;
}

// ***************************************************************************

void CPrimitives::setAliasStaticPart(uint32 staticPart)
{
	_AliasStaticPart = staticPart;
}

// ***************************************************************************

uint32 CPrimitives::buildFullAlias(uint32 dynamicPart)
{
	if (_LigoConfig)
	{
		return _LigoConfig->buildAlias(_AliasStaticPart, dynamicPart, true);
	}
	else
		return dynamicPart;
}


// ***************************************************************************

uint32 CPrimitives::genAlias(IPrimitive *prim, uint32 preferedAlias)
{
	nlassert(_LigoConfig);
	uint32 ret;

	if (preferedAlias != 0)
	{
		// only dynamic part allowed here
		nlassert(preferedAlias == (preferedAlias & _LigoConfig->getDynamicAliasMask()));
		// check is the prefered alias is not already in use
		map<uint32, IPrimitive*>::iterator it(_AliasInUse.find(preferedAlias));
		if (it == _AliasInUse.end())
		{
			// this alias is available, just use it
//			nldebug("Alias: added alias %u, %u alias used", preferedAlias, _AliasInUse.size()+1);
			_AliasInUse.insert(make_pair(preferedAlias, prim));
			return preferedAlias;
		}
		else
		{
			// check who own the alias now
			if (it->second == prim)
			{
				// ok, the alias is already own by this primitive
//				nldebug("Alias: using alias %u, %u alias used", preferedAlias, _AliasInUse.size()+1);
				return preferedAlias;
			}
		}
	}

	// make sure there are some free aliases
	uint32 mask = _LigoConfig->getDynamicAliasMask();
	nlassert(_AliasInUse.size() < mask);

	// increment alias counter
	++_LastGeneratedAlias;
	// mask with the dynamic alias mask
	_LastGeneratedAlias &= _LigoConfig->getDynamicAliasMask();

	ret = _LastGeneratedAlias;

	while (_AliasInUse.find(ret) != _AliasInUse.end())
	{
		// this alias is already in use ! generate a new one
		// increment, mask, affect...
		++_LastGeneratedAlias;
		_LastGeneratedAlias &= _LigoConfig->getDynamicAliasMask();
		ret = _LastGeneratedAlias;
	}

	// insert the alias
//	nldebug("Alias: added alias %u, %u alias in use", ret, _AliasInUse.size()+1);
	_AliasInUse.insert(make_pair(ret, prim));

	// callback
	prim->onModifyPrimitive (*this);

	return ret;
}

//void CPrimitives::reserveAlias(uint32 dynamicAlias)
//{
//	// need ligo config
//	nlassert(_LigoConfig);
//	// only dynamic part allowed here
//	nlassert(dynamicAlias == (dynamicAlias & _LigoConfig->getDynamicAliasMask()));
//	std::set<uint32>::iterator it(_AliasInUse.find(dynamicAlias));
//	// warn if already found
//	if (it != _AliasInUse.end())
//	{
//		const string &fileName = _LigoConfig->getFileNameForStaticAlias(_AliasStaticPart);
//		if (fileName.empty())
//			nlwarning("Dynamic Alias %u is already in use");
//		else
//			nlwarning("Dynamic Alias %u is already in use in file '%s'",
//				dynamicAlias,
//				fileName.c_str());
//		return;
//	}
//
//	// insert the alias
//	nldebug("Alias: added alias %u, %u alias in use", dynamicAlias, _AliasInUse.size()+1);
//	_AliasInUse.insert(dynamicAlias);
//}
//
void CPrimitives::releaseAlias(IPrimitive *prim, uint32 alias)
{
	// need ligo config
	nlassert(_LigoConfig);
	// only dynamic part allowed here
	nlassert(alias == (alias & _LigoConfig->getDynamicAliasMask()));
	std::map<uint32, IPrimitive*>::iterator it(_AliasInUse.find(alias));
	// need to be found
	nlassert(it != _AliasInUse.end());

	if (it->second != prim)
	{
		nlwarning("CPrimitives::releaseAlias: The alias %u is own by another primitive !", alias);
		return;
	}

	// remove this alias
//	nldebug("Alias: remove alias %u, %u alias left", it->first, _AliasInUse.size()-1);
	_AliasInUse.erase(it);
}

// ***************************************************************************

void CPrimitives::forceAlias(CPrimAlias *prim, uint32 alias)
{
	// need ligo config
	nlassert(_LigoConfig);
	// only dynamic part allowed here
	nlassert(alias == (alias & _LigoConfig->getDynamicAliasMask()));

	// store the alias in the primitive
	prim->_Alias = alias;

	std::map<uint32, IPrimitive*>::iterator it(_AliasInUse.find(alias));
	if (it != _AliasInUse.end() && it->second != prim)
	{
		// we need to alloc and set a new alias for the current alias holder
		CPrimAlias *pa = static_cast<CPrimAlias*>(const_cast<IPrimitive*>(it->second));

		// reserve the alias for the new primitive
		it->second = prim;


		// and regen an alias for the old
		pa->regenAlias();
	}
	else
	{
		// just store the association
		_AliasInUse.insert(make_pair(alias, prim));
	}

}

// ***************************************************************************

uint32 CPrimitives::getLastGeneratedAlias()
{
	return _LastGeneratedAlias;
}

// ***************************************************************************

IPrimitive		*CPrimitives::getPrimitiveByAlias(uint32 primAlias)
{
	// check the static part of the alias
	uint32 staticAlias = _LigoConfig->getStaticAliasMask() & primAlias;
	staticAlias = staticAlias >> _LigoConfig->getDynamicAliasSize();
	if (staticAlias != _AliasStaticPart)
		return NULL;

	// clear the static part before searching
	primAlias &= _LigoConfig->getDynamicAliasMask();

	std::map<uint32, IPrimitive*>::const_iterator it(_AliasInUse.find(primAlias));

	if (it != _AliasInUse.end())
		return it->second->getParent();
	else
		return NULL;
}

// ***************************************************************************

void		CPrimitives::buildPrimitiveWithAliasList(std::map<uint32, IPrimitive*> &result)
{
	nlassert(_LigoConfig != NULL);

	std::map<uint32, IPrimitive*>::iterator first(_AliasInUse.begin()), last(_AliasInUse.end());
	for (; first != last; ++first)
	{
		result.insert(make_pair(_LigoConfig->buildAlias(_AliasStaticPart, first->first), first->second->getParent()));
	}
}

// ***************************************************************************

CPrimitives& CPrimitives::operator= (const CPrimitives &other)
{
//	RootNode = static_cast<CPrimNode *> (((IPrimitive*)other.RootNode)->copy ());
//	return *this;

	_AliasStaticPart = other._AliasStaticPart;
	_LastGeneratedAlias = other._LastGeneratedAlias;
	// get the current ligo context (if any)
	_LigoConfig = CPrimitiveContext::instance().CurrentLigoConfig;

	CPrimitives *temp = CPrimitiveContext::instance().CurrentPrimitive;
	CPrimitiveContext::instance().CurrentPrimitive = this;
	// copy the nodes
	RootNode = static_cast<CPrimNode *> (((IPrimitive*)other.RootNode)->copy ());
	RootNode->branchLink();

	CPrimitiveContext::instance().CurrentPrimitive = temp;

	return *this;
}

// ***************************************************************************

bool CPrimitives::read (xmlNodePtr xmlNode, const char *filename, CLigoConfig &config)
{
	nlassert (xmlNode);

	_Filename = CFile::getFilename(filename);
	if (_LigoConfig)
	{
		// try to get the static alias mapping
		_AliasStaticPart = _LigoConfig->getFileStaticAliasMapping(CFile::getFilename(filename));
	}

	// Clear the primitives
	RootNode->removeChildren ();
	RootNode->removeProperties ();

	// Get the name
	if (strcmp ((const char*)xmlNode->name, "PRIMITIVES") == 0)
	{
		// Get the version
		string versionName = "0";
		if (GetPropertyString (versionName, filename, xmlNode, "VERSION"))
		{
			// Get the version
			uint32 version = atoi (versionName.c_str ());

			// Check the version
			if (version <= NLLIGO_PRIMITIVE_VERSION)
			{
				// Read the primitives
				xmlNode = GetFirstChildNode (xmlNode, filename, "ROOT_PRIMITIVE");
				if (xmlNode)
				{
					if (version > 0)
					{
						xmlNodePtr subNode = GetFirstChildNode(xmlNode, filename, "ALIAS");
						if (subNode)
						{
							uint temp;
							ReadUInt("LAST_GENERATED", temp, filename, subNode);
							_LastGeneratedAlias = temp;
						}
						else
							_LastGeneratedAlias = 0;
					}
					else
						_LastGeneratedAlias = 0;

					// Read the primitive tree
					((IPrimitive*)RootNode)->read (xmlNode, filename, version, config);
				}
			}
			else
			{
				Error (filename, "CPrimitives::read : Unknown file version (%d)", version);
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		XMLError (xmlNode, filename, "This XML document is not a NeL primitive file");
		return false;
	}

	return true;
}

// ***************************************************************************

void CPrimitives::write (xmlDocPtr doc, const char *filename) const
{
	nlassert (doc);

	// Primitive node
	xmlNodePtr primNode = xmlNewDocNode (doc, NULL, (const xmlChar*)"PRIMITIVES", NULL);
	xmlDocSetRootElement (doc, primNode);

	write (primNode, filename);
}

// ***************************************************************************

void CPrimitives::write (xmlNodePtr root, const char *filename) const
{
	nlassert (root);

	// Version node
	xmlSetProp (root, (const xmlChar*)"VERSION", (const xmlChar*)toString (NLLIGO_PRIMITIVE_VERSION).c_str ());

	// The primitive root node
	xmlNodePtr nameNode = xmlNewChild ( root, NULL, (const xmlChar*)"ROOT_PRIMITIVE", NULL);
	xmlNodePtr subNode = xmlNewChild ( nameNode, NULL, (const xmlChar*)"ALIAS", NULL);
	WriteUInt("LAST_GENERATED", _LastGeneratedAlias, subNode);

	// Write the primitive tree
	((IPrimitive*)RootNode)->write (nameNode, filename);
}

// ***************************************************************************

void CPrimitives::serial(NLMISC::IStream &f)
{
	uint currentVersion = NLLIGO_PRIMITIVE_VERSION;
	f.serialVersion(currentVersion);

	if (currentVersion == 0)
	{
		f.serial(_LastGeneratedAlias);
	}

	if (f.isReading())
	{
		RootNode->removeChildren ();
		RootNode->removeProperties ();
	}
	f.serialPolyPtr(RootNode);
	f.serial(_Filename);
	if (f.isReading() && _LigoConfig)
	{
		_AliasStaticPart = _LigoConfig->getFileStaticAliasMapping(_Filename);
	}
}

// ***************************************************************************

void CPrimitives::convertAddPrimitive (IPrimitive *child, const IPrimitive *prim, bool hidden)
{
	// The primitve
	IPrimitive *primitive = NULL;

	// What kind of primitive ?
	const CPrimPoint *oldPoint = dynamic_cast<const CPrimPoint *>(prim);
	if (oldPoint)
	{
		// Create a primitive
		CPrimPoint *point = static_cast<CPrimPoint *> (CClassRegistry::create ("CPrimPoint"));
		primitive = point;

		// Copy it
		*point = *oldPoint;
	}
	else
	{
		// Path ?
		const CPrimPath *oldPath = dynamic_cast<const CPrimPath *>(prim);
		if (oldPath)
		{
			// Create a primitive
			CPrimPath *path = static_cast<CPrimPath *> (CClassRegistry::create ("CPrimPath"));
			primitive = path;

			// Copy it
			*path = *oldPath;
		}
		else
		{
			const CPrimZone *oldZone = safe_cast<const CPrimZone *>(prim);
			if (oldZone)
			{
				// Create a primitive
				CPrimZone *zone = static_cast<CPrimZone *> (CClassRegistry::create ("CPrimZone"));
				primitive = zone;

				// Copy it
				*zone = *oldZone;
			}
		}
	}

	// Primitive has been created ?
	if (primitive)
	{
		// Create a property for the name
		CPropertyString *nameProp = new CPropertyString;
//		nameProp->String = prim->Name;

		// Add the property
		primitive->addPropertyByName ("name", nameProp);

		// The primitive is hidden ?
		if (hidden)
		{
			// Create a property for hidden
			nameProp = new CPropertyString;

			// Add the property
			primitive->addPropertyByName ("hidden", nameProp);
		}

		// Add the child
		child->insertChild (primitive);
	}
}

// ***************************************************************************

void CPrimitives::convertPrimitive (const IPrimitive *prim, bool hidden)
{
	// Look for the group
	uint numChildren = RootNode->getNumChildren ();
	uint j;
	for (j=0; j<numChildren; j++)
	{
		IPrimitive *child;
		nlverify (RootNode->getChild (child, j));
		const IProperty *prop;
		if (child->getPropertyByName ("name", prop))
		{
			// Prop string
			const CPropertyString *name = dynamic_cast<const CPropertyString *>(prop);
			if (name)
			{
				// This one ?
/*				if (name->String == prim->Layer)
				{
					convertAddPrimitive (child, prim, hidden);
					break;
				}
*/			}
		}
	}

	// Not found ?
	if (j==numChildren)
	{
		// Create a node
		CPrimNode *primNode = static_cast<CPrimNode *> (CClassRegistry::create ("CPrimNode"));

		// Create a property for the layer
		CPropertyString *nameProp = new CPropertyString;
//		nameProp->String = prim->Layer;

		// Add the property
		primNode->addPropertyByName ("name", nameProp);

		// Add the child
		RootNode->insertChild (primNode);

		// Add the primitive
		convertAddPrimitive (primNode, prim, hidden);
	}
}

// ***************************************************************************

void CPrimitives::convert (const CPrimRegion &region)
{
	// Delete
	RootNode->removeChildren ();
	RootNode->removeProperties ();

	// For each primitives
	uint i;
	for (i=0; i<region.VPoints.size (); i++)
	{
		convertPrimitive (&(region.VPoints[i]), region.VHidePoints[i]);
	}
	for (i=0; i<region.VPaths.size (); i++)
	{
		convertPrimitive (&(region.VPaths[i]), region.VHidePaths[i]);
	}
	for (i=0; i<region.VZones.size (); i++)
	{
		convertPrimitive (&(region.VZones[i]), region.VHideZones[i]);
	}
}

// ***************************************************************************

CPrimitiveContext::CPrimitiveContext():
	CurrentLigoConfig(NULL),
	CurrentPrimitive(NULL)
{
}


void Register ()
{
	NLMISC_REGISTER_CLASS(CPropertyString);
	NLMISC_REGISTER_CLASS(CPropertyStringArray);
	NLMISC_REGISTER_CLASS(CPropertyColor);
	NLMISC_REGISTER_CLASS(CPrimNode);
	NLMISC_REGISTER_CLASS(CPrimPoint);
	NLMISC_REGISTER_CLASS(CPrimPath);
	NLMISC_REGISTER_CLASS(CPrimZone);
	NLMISC_REGISTER_CLASS(CPrimAlias);
}

// ***************************************************************************

} // namespace NLLIGO


