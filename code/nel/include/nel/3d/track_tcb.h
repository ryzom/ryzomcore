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


#ifndef NL_TRACK_H
#error "internal file included from track.h"
#endif


// ***************************************************************************
// ***************************************************************************
// TCB Keyframes.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
/**
 * TCB Track tools (for both normal TCB, and quat TCB). internal use.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT, class T, class TMapTimeCKey>
class CTCBTools
{
protected:
	typedef typename TMapTimeCKey::iterator		TMapTimeCKeyIterator;



	/// compute TCB ease information.
	void compileTCBEase(TMapTimeCKey &mapKey, bool loopMode)
	{
		TMapTimeCKeyIterator	it= mapKey.begin();
		for(;it!=mapKey.end();it++)
		{
			TMapTimeCKeyIterator	next= it;
			next++;

			// loop mgt. must compute ease from last to first (useful if _RangeLock is false).
			if(next==mapKey.end() && loopMode && mapKey.size()>1)
				next= mapKey.begin();

			// Ease Precompute.
			//=================
			CKeyT	&key= it->second;
			if(next!=mapKey.end())
			{
				float	e0= it->second.EaseFrom;
				float	e1= next->second.EaseTo;
				float	s =  e0 + e1;

				// "normalize".
				if (s > 1.0f)
				{
					e0 = e0/s;
					e1 = e1/s;
				}

				// precalc ease factors.
				key.Ease0= e0;
				key.Ease1= e1;
				key.EaseK= 1/(2.0f - e0 - e1);
				if(e0)
					key.EaseKOverEase0= key.EaseK / e0;
				if(e1)
					key.EaseKOverEase1= key.EaseK / e1;
			}
			else
			{
				// force ease() to just return d (see ease()).
				key.EaseK = 0.5f;
			}

		}
	}

	// ease time.
	float ease(const CKeyT *key, float d)
	{
		if (d==0.0f || d==1.0f) return d;
		// k==0.5f <=> e0+e1 == 0.
		if (key->EaseK == 0.5f) return d;

		if (d < key->Ease0)
			return key->EaseKOverEase0 * d*d;
		else if (d < 1.0f - key->Ease1)
			return key->EaseK * (2.0f*d - key->Ease0);
		else
		{
			d = 1.0f - d;
			return 1.0f - key->EaseKOverEase1 * d*d;
		}
	}

	// compute hermite factors.
	void computeHermiteBasis(float d, float hb[4])
	{
		float d2,d3,a;

		d2 = d*d;
		d3 = d2*d;
		a  = 3.0f*d2 - 2.0f*d3;
		hb[0] = 1.0f - a;
		hb[1] = a;
		hb[2] = d3 - 2.0f*d2 + d;
		hb[3] = d3 - d2;
	}


	// compute TCB tangents factors.
	void computeTCBFactors(const CKeyT &key, float timeBefore, float time, float timeAfter,
		float rangeDelta, bool firstKey, bool endKey, bool isLoop, float &ksm, float &ksp, float &kdm, float &kdp)
	{
		float fp,fn;

		if(isLoop || (!firstKey && !endKey))
		{
			float	dtm;
			// Compute Time deltas.
			if (firstKey)
			{
				dtm = 0.5f * (rangeDelta + timeAfter - time);
				fp = rangeDelta / dtm;
				fn = (timeAfter - time) / dtm;
			}
			else if (endKey)
			{
				dtm = 0.5f * (rangeDelta + time - timeBefore);
				fp = rangeDelta / dtm;
				fn = (time - timeBefore) / dtm;
			}
			else
			{
				dtm = 0.5f * (timeAfter - timeBefore);
				fp = (time - timeBefore) / dtm;
				fn = (timeAfter - time) / dtm;
			}
			float	c= (float)fabs( key.Continuity );
			fp = fp + c - c * fp;
			fn = fn + c - c * fn;
		}
		else
		{
			// firstkey and lastkey of not loop track.
			fp = 1.0f;
			fn = 1.0f;
		}

		// Compute tangents factors.
		float	tm,cm,cp,bm,bp,tmcm,tmcp;

		cm = 1.0f - key.Continuity;
		tm = 0.5f * ( 1.0f - key.Tension );
		cp = 2.0f - cm;
		bm = 1.0f - key.Bias;
		bp = 2.0f - bm;
		tmcm = tm*cm;	tmcp = tm*cp;

		// tgts factors.
		ksm = tmcm*bp*fp;	ksp = tmcp*bm*fp;
		kdm = tmcp*bp*fn; 	kdp = tmcm*bm*fn;

	}

};


// ***************************************************************************
/**
 * ITrack implementation for TCB keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT, class T>
class CTrackKeyFramerTCB : public ITrackKeyFramer<CKeyT>, private CTCBTools<CKeyT, T, std::map<TAnimationTime, CKeyT> >
{
public:

protected:


	typedef typename CKeyT::TValueType		TKeyValueType;



	/// \name From ITrackKeyFramer
	// @{

	/// evalKey (runtime).
	virtual void evalKey (	const CKeyT* previous, const CKeyT* next,
							TAnimationTime datePrevious, TAnimationTime /* dateNext */,
							TAnimationTime date, IAnimatedValue &result )
	{
		CAnimatedValueBlendable<T>	&resultVal= static_cast<CAnimatedValueBlendable<T>&>(result);

		if(previous && next)
		{
			// lerp from previous to cur.
			date-= datePrevious;
			date*= previous->OODeltaTime;
			NLMISC::clamp(date, 0,1);

			date = this->ease(previous, (float)date);

			float hb[4];
			this->computeHermiteBasis(date, hb);
			copyToValue(resultVal.Value,
				previous->Value*hb[0] + next->Value*hb[1] +
				previous->TanFrom*hb[2] + next->TanTo*hb[3]);
		}
		else
		{
			if (previous)
				copyToValue(resultVal.Value, previous->Value);
			else
				if (next)
					copyToValue(resultVal.Value, next->Value);
		}
	}

	/// compile (precalc).
	virtual void compile()
	{
		ITrackKeyFramer<CKeyT>::compile();

		// Ease Precompute.
		this->compileTCBEase(this->_MapKey, this->getLoopMode());


		// Tangents Precompute.
		sint	nKeys= (sint)this->_MapKey.size();
		if(nKeys<=1)
			return;

		typename std::map<TAnimationTime, CKeyT>::iterator	it= this->_MapKey.begin();				// first key.
		typename std::map<TAnimationTime, CKeyT>::iterator	itNext= it; itNext++;				// second key.
		typename std::map<TAnimationTime, CKeyT>::iterator	itPrev= this->_MapKey.end(); itPrev--;	// last key.

		if(nKeys==2 && !this->getLoopMode())
		{
			computeTCBKeyLinear( it->second, itNext->second );
		}
		else
		{
			// rangeDelta is the length of effective Range - length of LastKey-FirstKey.
			// NB: if RangeLock, rangeDelta==0.
			float	rangeDelta;
			// NB: _RangeDelta has just been compiled in ITrackKeyFramer<CKeyT>::compile().
			rangeDelta= this->getCompiledRangeDelta();

			// Compute all middle keys.
			for(;it!=this->_MapKey.end();)
			{
				// Do the first key and the last key only in LoopMode.
				// NB: we are the last if itNext==_MapKey.begin().
				if(this->getLoopMode() || (it!=this->_MapKey.begin() && itNext!=this->_MapKey.begin()) )
				{
					computeTCBKey(itPrev->second, it->second, itNext->second,
						itPrev->first, it->first, itNext->first, rangeDelta,
						it==this->_MapKey.begin(), itNext==this->_MapKey.begin(), this->getLoopMode());
				}

				// Next key!!
				itPrev= it;
				it++;
				itNext++;
				// loop.
				if(itNext==this->_MapKey.end())
					itNext= this->_MapKey.begin();
			}

			// In not loop mode, compute first and last key, AFTER middle keys computed.
			if(!this->getLoopMode())
			{
				typename std::map<TAnimationTime, CKeyT>::iterator	it0= this->_MapKey.begin();				// first key.
				typename std::map<TAnimationTime, CKeyT>::iterator	it1= it0; it1++;					// second key.
				typename std::map<TAnimationTime, CKeyT>::iterator	itLast= this->_MapKey.end();itLast--;		// last key.
				typename std::map<TAnimationTime, CKeyT>::iterator	itLastPrev= itLast;itLastPrev--;	// prev of last key.

				computeFirstKey(it0->second, it1->second);
				computeLastKey(itLast->second, itLastPrev->second);
			}
		}
	}

	// @}


// *****************
private:


	void computeTCBKey(CKeyT &keyBefore, CKeyT &key, CKeyT &keyAfter, float timeBefore, float time, float timeAfter,
		float rangeDelta, bool firstKey, bool endKey, bool isLoop)
	{
		float	ksm,ksp,kdm,kdp;

		// compute tangents factors.
		this->computeTCBFactors(key, timeBefore, time, timeAfter, rangeDelta, firstKey, endKey, isLoop, ksm,ksp,kdm,kdp);

		// Delta.
		TKeyValueType	delm, delp;
		delm = key.Value - keyBefore.Value;
		delp = keyAfter.Value - key.Value;

		// Tangents.
		key.TanTo	= delm*ksm + delp*ksp;
		key.TanFrom= delm*kdm + delp*kdp;

	}

	// compute 2 TCB keys for a not-loop track => "linear".
	void computeTCBKeyLinear(CKeyT &key0, CKeyT &key1)
	{
		float f0, f1;
		TKeyValueType	dv;

		f0 = 1.0f - key0.Tension;
		f1 = 1.0f - key1.Tension;
		dv = key1.Value - key0.Value;
		key0.TanFrom= dv * f0;
		key1.TanTo= dv * f1;
	}

	// compute this AFTER computing key1.
	void computeFirstKey(CKeyT &keyFirst, CKeyT &keyAfter)
	{
		float tm;
		tm = 0.5f * (1.0f - keyFirst.Tension);
		keyFirst.TanFrom= tm * ((keyAfter.Value - keyFirst.Value) * 3.0f - keyAfter.TanTo);
	}

	// compute this AFTER computing key(n-2).
	void computeLastKey(CKeyT &keyLast, CKeyT &keyBefore)
	{
		float tm;
		tm = 0.5f * (1.0f - keyLast.Tension);
		keyLast.TanTo= tm * ((keyLast.Value - keyBefore.Value) * 3.0f - keyBefore.TanFrom);
	}


};


// ***************************************************************************
/**
 * ITrack implementation for CQuat TCB keyframer.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
template<> class CTrackKeyFramerTCB<CKeyTCBQuat, NLMISC::CAngleAxis> : public ITrackKeyFramer<CKeyTCBQuat>,
	private CTCBTools<CKeyTCBQuat, NLMISC::CAngleAxis, std::map<TAnimationTime, CKeyTCBQuat> >
{
public:

	/// \name From ITrackKeyFramer
	// @{

	/// evalKey (runtime).
	virtual void evalKey (	const CKeyTCBQuat* previous, const CKeyTCBQuat* next,
							TAnimationTime datePrevious, TAnimationTime /* dateNext */,
							TAnimationTime date, IAnimatedValue &result )
	{
		CAnimatedValueQuat	&resultVal= static_cast<CAnimatedValueQuat&>(result);

		if(previous && next)
		{
			// lerp from previous to cur.
			date-= datePrevious;
			date*= previous->OODeltaTime;
			NLMISC::clamp(date, 0,1);

			// ease.
			date = ease(previous, date);

			// quad slerp.
			resultVal.Value= CQuat::squadrev(next->LocalAngleAxis, previous->Quat, previous->A, next->B, next->Quat, date);
		}
		else
		{
			if (previous)
				resultVal.Value= previous->Quat;
			else
				if (next)
					resultVal.Value= next->Quat;
		}

	}

	/// compile (precalc).
	virtual void compile()
	{
		ITrackKeyFramer<CKeyTCBQuat>::compile();

		// Ease Precompute.
		this->compileTCBEase(_MapKey, getLoopMode());

		TMapTimeCKey::iterator	it;
		TMapTimeCKey::iterator	itNext;
		TMapTimeCKey::iterator	itPrev;

		// Compute absolute quaternions.
		for (it= _MapKey.begin();it!=_MapKey.end();)
		{
			CKeyTCBQuat		&key= it->second;

			// Compute Local AngleAxis.
			if(it!= _MapKey.begin())
			{
				NLMISC::CMatrix		mat;
				mat.setRot(itPrev->second.Quat);
				mat.invert();
				key.LocalAngleAxis.Axis= mat*key.Value.Axis;
				key.LocalAngleAxis.Angle= key.Value.Angle;
			}
			else
				key.LocalAngleAxis= key.Value;


			key.LocalAngleAxis.Axis.normalize();
			// make angle positive.
			if(key.LocalAngleAxis.Angle<0.f)
			{
				key.LocalAngleAxis.Axis= -key.LocalAngleAxis.Axis;
				key.LocalAngleAxis.Angle= -key.LocalAngleAxis.Angle;
			}

			// relative quat
			key.Quat.setAngleAxis(key.LocalAngleAxis);

			// absolute quat
			if (it!= _MapKey.begin())
				key.Quat = itPrev->second.Quat * key.Quat;

			// next key.
			itPrev= it;
			it++;
		}

		// Tangents Precompute.
		sint	nKeys= (sint)_MapKey.size();
		if(nKeys<=1)
			return;

		// rangeDelta is the length of effective Range - length of LastKey-FirstKey.
		// NB: if RangeLock, rangeDelta==0.
		float	rangeDelta;
		// NB: _RangeDelta has just been compiled in ITrackKeyFramer<CKeyTCBQuat>::compile().
		rangeDelta= getCompiledRangeDelta();

		it= _MapKey.begin();				// first key.
		itNext= it; itNext++;				// second key.
		itPrev= _MapKey.end(); itPrev--;	// last key.

		// Compute all keys.
		for(;it!=_MapKey.end();)
		{
			// NB: we are the last key if itNext==_MapKey.begin().
			computeTCBKey(itPrev->second, it->second, itNext->second,
				itPrev->first, it->first, itNext->first, rangeDelta, it==_MapKey.begin(), itNext==_MapKey.begin(), getLoopMode());

			// Next key!!
			itPrev= it;
			it++;
			itNext++;
			// loop.
			if(itNext==_MapKey.end())
				itNext= _MapKey.begin();
		}

	}

	// @}


// *****************
private:

	void computeTCBKey(CKeyTCBQuat &keyBefore, CKeyTCBQuat &key, CKeyTCBQuat &keyAfter, float timeBefore, float time, float timeAfter,
		float rangeDelta, bool firstKey, bool endKey, bool isLoop)
	{
		CQuat  qp, qm;

		// compute qm.
		if (!firstKey || isLoop)
		{
			float	angle= key.LocalAngleAxis.Angle;
			CVector	&axis= key.LocalAngleAxis.Axis;

			if (angle > 2*NLMISC::Pi- NLMISC::QuatEpsilon)
			{
				qm.set(axis.x, axis.y, axis.z, 0.0f);
				qm = qm.log();
			}
			else
			{
				CQuat	qprev= keyBefore.Quat;
				qprev.makeClosest(key.Quat);
				qm = CQuat::lnDif(qprev, key.Quat);
			}
		}

		// compute qp.
		if (!endKey || isLoop)
		{
			float	angle= keyAfter.LocalAngleAxis.Angle;
			CVector	&axis= keyAfter.LocalAngleAxis.Axis;

			if (angle > 2*NLMISC::Pi- NLMISC::QuatEpsilon)
			{
				qp.set(axis.x, axis.y, axis.z, 0.0f);
				qp = qp.log();
			}
			else
			{
				CQuat	qnext= keyAfter.Quat;
				qnext.makeClosest(key.Quat);
				qp = CQuat::lnDif(key.Quat, qnext);
			}
		}

		// not loop mgt.
		if (firstKey && !isLoop)
			qm = qp;
		if (endKey && !isLoop)
			qp = qm;


		// compute tangents factors.
		float	ksm, ksp, kdm, kdp;
		computeTCBFactors(key, timeBefore, time, timeAfter, rangeDelta, firstKey, endKey, isLoop, ksm,ksp,kdm,kdp);


		// A/B.
		CQuat	qa, qb;
		qb= (qm * (1.0f-ksm) + qp * (-ksp)	  ) * 0.5f;
		qa= (qm * kdm		 + qp * (kdp-1.0f) ) * 0.5f;
		qa = qa.exp();
		qb = qb.exp();

		key.A = key.Quat * qa;
		key.B = key.Quat * qb;
	}



};


