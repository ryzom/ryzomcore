// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/gui/interface_anim.h"

#include "nel/gui/widget_manager.h"
#include "nel/gui/interface_expr.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/action_handler.h"

// ----------------------------------------------------------------------------
using namespace std;
using namespace NL3D;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

	// ----------------------------------------------------------------------------
	// CInterfaceTrack
	// ----------------------------------------------------------------------------
	CInterfaceTrack::CInterfaceTrack()
	{
		_Type = Track_Linear;
		_TrackKeyFramer = NULL;
		_Dynamic = false;
	}

	// ----------------------------------------------------------------------------
	CInterfaceTrack::~CInterfaceTrack()
	{
		delete _TrackKeyFramer;
	}

	// ----------------------------------------------------------------------------
	bool CInterfaceTrack::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
	{
		CXMLAutoPtr ptr;

		ptr = xmlGetProp (cur, (xmlChar*)"dynamic");
		if (ptr) _Dynamic = CInterfaceElement::convertBool (ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"type");
		string sTmp = toLowerAscii(ptr.str());
		if (sTmp == "linear")
			_Type = Track_Linear;
		else if (sTmp == "bezier")
			_Type = Track_Bezier;
		else if (sTmp == "tcb")
			_Type = Track_TCB;
		else
			nlwarning ("track unknown type : %s, setting linear by default", (const char*)ptr);

		ptr = xmlGetProp (cur, (xmlChar*)"target");
		if (!ptr)
		{
			nlwarning ("no target for track");
			return false;
		}

		//
		if (!CInterfaceLink::splitLinkTargets (ptr.str(), parentGroup, _Targets))
		{
			nlwarning ("no target for track");
			return false;
		}

		// Initialize the track
		switch(_Type)
		{
			case Track_Linear:	_TrackKeyFramer = UTrackKeyframer::createLinearFloatTrack(); break;
			case Track_TCB:		_TrackKeyFramer = UTrackKeyframer::createTCBFloatTrack(); break;
			case Track_Bezier:	_TrackKeyFramer = UTrackKeyframer::createBezierFloatTrack(); break;
			default: nlstop;	break;
		}

		cur = cur->children;
		bool ok = true;

		if (_Dynamic)
		{
			while (cur)
			{
				// Check that this is a key node
				if ( stricmp((char*)cur->name,"key") != 0 )
				{
					cur = cur->next;
					continue;
				}

				// Read time and value the 2 main key attributes
				CXMLAutoPtr time, value;
				time = xmlGetProp (cur, (xmlChar*)"time");
				value = xmlGetProp (cur, (xmlChar*)"value");
				if (!time || !value)
				{
					nlwarning("track key with no time or no value");
					ok = false;
					cur = cur->next;
					continue;
				}

				SDynKey k;
				k.Value = (const char*)value;
				k.Time = (const char*)time;
				if (isdigit(k.Time[0]) || (k.Time[0] == '-'))
				{
					double dTime;
					fromString(k.Time, dTime);
					k.Time = toString(dTime * CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionMulCoefAnim).getValFloat());
				}

				// Depending on the type of the track read extra values
				switch(_Type)
				{
					case Track_Linear:
					break;

					case Track_Bezier:
					{
						CXMLAutoPtr inTan, outTan, step;
						inTan = xmlGetProp (cur, (xmlChar*)"intan");
						outTan = xmlGetProp (cur, (xmlChar*)"outtan");
						step = xmlGetProp (cur, (xmlChar*)"step");

						if (inTan)	k.InTan = (const char*)inTan;
						if (outTan) k.OutTan = (const char*)outTan;
						if (step)	k.Step = (const char*)step;
					}
					break;

					case Track_TCB:
					{
						CXMLAutoPtr tension, continuity, bias, easeto, easefrom;
						tension = xmlGetProp (cur, (xmlChar*)"tension");
						continuity = xmlGetProp (cur, (xmlChar*)"continuity");
						bias = xmlGetProp (cur, (xmlChar*)"bias");
						easeto = xmlGetProp (cur, (xmlChar*)"easeto");
						easefrom = xmlGetProp (cur, (xmlChar*)"easefrom");

						if (tension)	k.Tension = (const char*)tension;
						if (continuity)	k.Continuity = (const char*)continuity;
						if (bias)		k.Bias = (const char*)bias;
						if (easeto)		k.EaseTo = (const char*)easeto;
						if (easefrom)	k.EaseFrom = (const char*)easefrom;
					}
					break;
					default: nlstop; break;
				}
				_DynKeys.push_back(k);

				cur = cur->next;
			}
		}
		else // Static parsing
		{
			while (cur)
			{
				// Check that this is a key node
				if ( stricmp((char*)cur->name,"key") != 0 )
				{
					cur = cur->next;
					continue;
				}

				// Read time and value the 2 main key attributes
				CXMLAutoPtr time, value;
				time = xmlGetProp (cur, (xmlChar*)"time");
				value = xmlGetProp (cur, (xmlChar*)"value");
				if (!time || !value)
				{
					nlwarning("track key with no time or no value");
					ok = false;
					cur = cur->next;
					continue;
				}

				float fAnimTime;
				fromString((const char*)time, fAnimTime);
				TAnimationTime animTime = fAnimTime * CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionMulCoefAnim).getValFloat();
				double animValue;
				fromString(value.str(), animValue);

				// Depending on the type of the track add the key
				switch(_Type)
				{
					case Track_Linear:
					{
						UTrackKeyframer::UKeyLinearFloat k;
						k.Time = animTime;
						k.Value = (float)animValue;
						_TrackKeyFramer->addLinearFloatKey (k);
					}
					break;

					case Track_Bezier:
					{
						UTrackKeyframer::UKeyBezierFloat k;
						k.Time = animTime;
						k.Value = (float)animValue;

						// Read extra values
						CXMLAutoPtr inTan, outTan, step;
						inTan = xmlGetProp (cur, (xmlChar*)"intan");
						outTan = xmlGetProp (cur, (xmlChar*)"outtan");
						step = xmlGetProp (cur, (xmlChar*)"step");

						k.TanIn = 0.0f;
						k.TanOut = 0.0f;
						k.Step = false;

						if (inTan)	fromString((const char*)inTan, k.TanIn);
						if (outTan) fromString((const char*)outTan, k.TanOut);
						if (step)	k.Step = CInterfaceElement::convertBool(step);

						_TrackKeyFramer->addBezierFloatKey (k);
					}
					break;

					case Track_TCB:
					{
						UTrackKeyframer::UKeyTCBFloat k;
						k.Time = animTime;
						k.Value = (float)animValue;

						// Read extra values
						CXMLAutoPtr tension, continuity, bias, easeto, easefrom;
						tension = xmlGetProp (cur, (xmlChar*)"tension");
						continuity = xmlGetProp (cur, (xmlChar*)"continuity");
						bias = xmlGetProp (cur, (xmlChar*)"bias");
						easeto = xmlGetProp (cur, (xmlChar*)"easeto");
						easefrom = xmlGetProp (cur, (xmlChar*)"easefrom");

						k.Tension = 0.0f;
						k.Continuity = 0.0f;
						k.Bias = 0.0f;
						k.EaseTo = 0.0f;
						k.EaseFrom = 0.0f;

						if (tension)	fromString((const char*)tension, k.Tension);
						if (continuity)	fromString((const char*)continuity, k.Continuity);
						if (bias)		fromString((const char*)bias, k.Bias);
						if (easeto)		fromString((const char*)easeto, k.EaseTo);
						if (easefrom)	fromString((const char*)easefrom, k.EaseFrom);

						_TrackKeyFramer->addTCBFloatKey (k);
					}
					break;
					default: nlstop; break;
				}
				cur = cur->next;
			}
		}
		return true;
	}

	// ----------------------------------------------------------------------------
	void CInterfaceTrack::update (double currentTime)
	{
		float currentValue;
		UTrack *pTrack = dynamic_cast<UTrack*>(_TrackKeyFramer);
		if (pTrack == NULL) return;
		pTrack->interpolate((TAnimationTime)currentTime, currentValue);

		// Update the targets
		CInterfaceExprValue expr;
		expr.setDouble(currentValue);
		for (uint i = 0; i < _Targets.size(); ++i)
			_Targets[i].affect(expr);
	}

	// ----------------------------------------------------------------------------
	void CInterfaceTrack::eval()
	{
		if (!_Dynamic) return;

		if (_TrackKeyFramer != NULL)
			delete _TrackKeyFramer;

		switch(_Type)
		{
			case Track_Linear:
			{
				_TrackKeyFramer = UTrackKeyframer::createLinearFloatTrack();
				for (uint i = 0; i < _DynKeys.size(); ++i)
				{
					SDynKey &rKey = _DynKeys[i];
					UTrackKeyframer::UKeyLinearFloat k;
					CInterfaceExprValue res;
					if (!rKey.Time.empty() && CInterfaceExpr::eval(rKey.Time, res))
					{
						if (!res.toDouble())
							nlwarning ("cannot convert Time to double");
						else
							k.Time = (TAnimationTime)res.getDouble();
					}
					else
					{
						k.Time = 0;
					}

					if (!rKey.Value.empty() && CInterfaceExpr::eval(rKey.Value, res))
					{
						if (!res.toDouble())
							nlwarning ("cannot convert Value to double");
						else
							k.Value = (float)res.getDouble();
					}
					else
					{
						k.Value = 0;
					}

					_TrackKeyFramer->addLinearFloatKey (k);
				}
			}
			break;

			case Track_TCB:
			{
				_TrackKeyFramer = UTrackKeyframer::createTCBFloatTrack();
				for (uint i = 0; i < _DynKeys.size(); ++i)
				{
					SDynKey &rKey = _DynKeys[i];
					UTrackKeyframer::UKeyTCBFloat k;
					CInterfaceExprValue res;
					if (!rKey.Time.empty() && CInterfaceExpr::eval(rKey.Time, res))
					{
						if (!res.toDouble())
							nlwarning ("cannot convert Time to double");
						else
							k.Time = (TAnimationTime)res.getDouble();
					}
					else
					{
						k.Time = 0;
					}

					if (!rKey.Value.empty() && CInterfaceExpr::eval(rKey.Value, res))
					{
						if (!res.toDouble())
							nlwarning ("cannot convert Value to double");
						else
							k.Value = (float)res.getDouble();
					}
					else
					{
						k.Value = 0;
					}

					if (!rKey.Tension.empty() && CInterfaceExpr::eval(rKey.Tension, res))
					{
						if (!res.toDouble())
							nlwarning ("cannot convert Tension to double");
						else
							k.Tension = (float)res.getDouble();
					}
					else
					{
						k.Tension = 0;
					}

					if (!rKey.Continuity.empty() && CInterfaceExpr::eval(rKey.Continuity, res))
					{
						if (!res.toDouble())
							nlwarning ("cannot convert Continuity to double");
						else
							k.Continuity = (float)res.getDouble();
					}
					else
					{
						k.Continuity = 0;
					}

					if (!rKey.Bias.empty() && CInterfaceExpr::eval(rKey.Bias, res))
					{
						if (!res.toDouble())
							nlwarning ("cannot convert Bias to double");
						else
							k.Bias = (float)res.getDouble();
					}
					else
					{
						k.Bias = 0;
					}

					if (!rKey.EaseTo.empty() && CInterfaceExpr::eval(rKey.EaseTo, res))
					{
						if (!res.toDouble())
							nlwarning ("cannot convert EaseTo to double");
						else
							k.EaseTo = (float)res.getDouble();
					}
					else
					{
						k.EaseTo = 0;
					}

					if (!rKey.EaseFrom.empty() && CInterfaceExpr::eval(rKey.EaseFrom, res))
					{
						if (!res.toDouble())
							nlwarning ("cannot convert EaseFrom to double");
						else
							k.EaseFrom = (float)res.getDouble();
					}
					else
					{
						k.EaseFrom = 0;
					}

					_TrackKeyFramer->addTCBFloatKey (k);
				}
			}
			break;

			case Track_Bezier:
			{
				_TrackKeyFramer = UTrackKeyframer::createBezierFloatTrack(); break;
				for (uint i = 0; i < _DynKeys.size(); ++i)
				{
					SDynKey &rKey = _DynKeys[i];
					UTrackKeyframer::UKeyBezierFloat k;
					CInterfaceExprValue res;
					if (!rKey.Time.empty() && CInterfaceExpr::eval(rKey.Time, res))
					{
						if (!res.toDouble())
							nlwarning ("cannot convert Time to double");
						else
							k.Time = (TAnimationTime)res.getDouble();
					}
					else
					{
						k.Time = 0;
					}

					if (!rKey.Value.empty() && CInterfaceExpr::eval(rKey.Value, res))
					{
						if (!res.toDouble())
							nlwarning ("cannot convert Value to double");
						else
							k.Value = (float)res.getDouble();
					}
					else
					{
						k.Value = 0;
					}

					if (!rKey.InTan.empty() && CInterfaceExpr::eval(rKey.InTan, res))
					{
						if (!res.toDouble())
							nlwarning ("cannot convert TanIn to double");
						else
							k.TanIn = (float)res.getDouble();
					}
					else
					{
						k.TanIn = 0;
					}

					if (!rKey.OutTan.empty() && CInterfaceExpr::eval(rKey.OutTan, res))
					{
						if (!res.toDouble())
							nlwarning ("cannot convert TanOut to double");
						else
							k.TanOut = (float)res.getDouble();
					}
					else
					{
						k.TanOut = 0;
					}

					if (!rKey.Step.empty() && CInterfaceExpr::eval(rKey.Step, res))
					{
						if (!res.toBool())
							nlwarning ("cannot convert Step to bool");
						else
							k.Step = res.getBool();
					}
					else
					{
						k.Step = false;
					}

					_TrackKeyFramer->addBezierFloatKey (k);
				}
			}
			default: nlstop;	break;
		}

	}

	// ----------------------------------------------------------------------------
	// CInterfaceAnim
	// ----------------------------------------------------------------------------

	// ----------------------------------------------------------------------------
	CInterfaceAnim::CInterfaceAnim()
	{
		_CurrentTime = 0;
		_Finished = true;
		_Duration = 0;
		_DisableButtons = true;
		_AnimHasToBeStopped = false;
		_Parent = NULL;
	}

	// ----------------------------------------------------------------------------
	CInterfaceAnim::~CInterfaceAnim()
	{
		for (uint i = 0; i < _Tracks.size(); ++i)
			delete _Tracks[i];
	}

	// ----------------------------------------------------------------------------
	bool CInterfaceAnim::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
	{
		CXMLAutoPtr ptr;

		ptr = xmlGetProp (cur, (xmlChar*)"id");
		_Id = (const char*)ptr;

		ptr = xmlGetProp (cur, (xmlChar*)"duration");
		if (!ptr)
		{
			nlwarning("anim with no duration");
			return false;
		}
		fromString((const char*)ptr, _Duration);
		if (_Duration == 0)
			_Duration = 1.0;
		_Duration *= CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionMulCoefAnim).getValFloat();

		ptr = xmlGetProp (cur, (xmlChar*)"disable_buttons");
		if (ptr)
			_DisableButtons = CInterfaceElement::convertBool(ptr);

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"on_finish" );
		if (ptr) _AHOnFinish = (const char *) ptr;
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"on_finish_params" );
		if (ptr) _AHOnFinishParams = (const char *) ptr;

		cur = cur->children;

	//	bool ok = true;
		while (cur)
		{
			// Check that this is a key node
			if ( stricmp((char*)cur->name,"track") != 0 )
			{
				cur = cur->next;
				continue;
			}

			CInterfaceTrack *pTrack = new CInterfaceTrack;
			if (!pTrack->parse(cur,parentGroup))
			{
				delete pTrack;
				nlwarning("track not added to anim");
			}
			else
			{
				_Tracks.push_back(pTrack);
			}
			cur = cur->next;
		}

		_Parent = parentGroup;

		return true;
	}

	// ----------------------------------------------------------------------------
	void CInterfaceAnim::update()
	{
		if ((_Duration == 0) || (_Finished))
			return;

		const CWidgetManager::SInterfaceTimes &times = CWidgetManager::getInstance()->getInterfaceTimes();

		// Delta time limiter
		if ( ( times.frameDiffMs / 1000.0f )  > 0.1)
			_CurrentTime += 0.1;
		else
			_CurrentTime += ( times.frameDiffMs / 1000.0f );

		// Stop the anim if we have to
		if (_AnimHasToBeStopped)
		{
			stop();
			return;
		}

		// Do this here to let it play the last frame of the animation
		if (_CurrentTime >= _Duration)
		{
			_CurrentTime = _Duration;
			_AnimHasToBeStopped = true;
		}

		// Update tracks
		for (uint i = 0; i < _Tracks.size(); ++i)
		{
			CInterfaceTrack *pTrack = _Tracks[i];
			pTrack->update (_CurrentTime);
		}
	}

	// ----------------------------------------------------------------------------
	void CInterfaceAnim::start()
	{
		_CurrentTime = 0.0f;
		_Finished = false;
		_AnimHasToBeStopped = false;

		// Look if there are some dynamic tracks
		for (uint i = 0; i < _Tracks.size(); ++i)
		if (_Tracks[i]->isDynamic())
			_Tracks[i]->eval();

		// Play the first frame
		update();
	}

	// ----------------------------------------------------------------------------
	void CInterfaceAnim::stop()
	{
		_Finished = true;

		if (!_AHOnFinish.empty())
			CAHManager::getInstance()->runActionHandler(_AHOnFinish, _Parent, _AHOnFinishParams);
	}

}

