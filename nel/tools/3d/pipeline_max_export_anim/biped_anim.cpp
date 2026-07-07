/**
 * \file biped_anim.cpp
 * \brief Biped animation-key decode + evaluation. See biped_anim.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */

/*
 * Copyright (C) 2026  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "biped_anim.h"

#include <algorithm>
#include <cmath>

// M_PI is not standard C++ (MSVC 9.0 / VS2008 does not define it in <cmath> without _USE_MATH_DEFINES);
// define it portably to the same double value glibc uses so both toolchains agree bit-for-bit.
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <cstring>

#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/biped/biped_driven.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PMAX_RIG;

namespace BIPANIM {

// ---------------------------------------------------------------------------------------------
// quat math (double precision, standard column-vector convention)

QuatD qMul(const QuatD &a, const QuatD &b)
{
	return QuatD(a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
	             a.w*b.y + a.y*b.w + a.z*b.x - a.x*b.z,
	             a.w*b.z + a.z*b.w + a.x*b.y - a.y*b.x,
	             a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z);
}

QuatD qConj(const QuatD &q) { return QuatD(-q.x, -q.y, -q.z, q.w); }

QuatD qNorm(const QuatD &q)
{
	double n = sqrt(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
	if (n <= 0.0) return q;
	return QuatD(q.x/n, q.y/n, q.z/n, q.w/n);
}

NLMISC::CVectorD qRotate(const QuatD &q, const NLMISC::CVectorD &v)
{
	double tx = 2*(q.y*v.z - q.z*v.y), ty = 2*(q.z*v.x - q.x*v.z), tz = 2*(q.x*v.y - q.y*v.x);
	return NLMISC::CVectorD(v.x + q.w*tx + (q.y*tz - q.z*ty),
	                        v.y + q.w*ty + (q.z*tx - q.x*tz),
	                        v.z + q.w*tz + (q.x*ty - q.y*tx));
}

QuatD qAxisAngle(const NLMISC::CVectorD &axis, double angle)
{
	double s = sin(angle * 0.5);
	return QuatD(axis.x*s, axis.y*s, axis.z*s, cos(angle * 0.5));
}

static QuatD qMirrorLR(const QuatD &q) { return QuatD(q.x, q.y, -q.z, -q.w); }

static QuatD qClosest(const QuatD &q, const QuatD &ref)
{
	double d = q.x*ref.x + q.y*ref.y + q.z*ref.z + q.w*ref.w;
	if (d < 0.0) return QuatD(-q.x, -q.y, -q.z, -q.w);
	return q;
}

static QuatD qLog(const QuatD &q)
{
	double n = sqrt(q.x*q.x + q.y*q.y + q.z*q.z);
	if (n < 1e-15) return QuatD(0, 0, 0, 0);
	double w = std::min(1.0, std::max(-1.0, q.w));
	double a = acos(w);
	double s = a / n;
	return QuatD(q.x*s, q.y*s, q.z*s, 0.0);
}

static QuatD qExp(const QuatD &q)
{
	double a = sqrt(q.x*q.x + q.y*q.y + q.z*q.z);
	if (a < 1e-15) return QuatD(0, 0, 0, 1);
	double s = sin(a) / a;
	return QuatD(q.x*s, q.y*s, q.z*s, cos(a));
}

static QuatD qSlerp(const QuatD &a, const QuatD &b, double t)
{
	double dot = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
	bool flip = dot < 0.0;
	if (flip) dot = -dot;
	if (dot > 0.99999)
	{
		// near-parallel: component lerp + renormalize
		double bx = flip ? -b.x : b.x, by = flip ? -b.y : b.y, bz = flip ? -b.z : b.z, bw = flip ? -b.w : b.w;
		return qNorm(QuatD(a.x + t*(bx - a.x), a.y + t*(by - a.y), a.z + t*(bz - a.z), a.w + t*(bw - a.w)));
	}
	double th = acos(std::min(1.0, dot));
	double sa = sin((1.0-t)*th)/sin(th);
	double sbb = sin(t*th)/sin(th);
	if (flip) sbb = -sbb;
	return QuatD(sa*a.x + sbb*b.x, sa*a.y + sbb*b.y, sa*a.z + sbb*b.z, sa*a.w + sbb*b.w);
}

// ---------------------------------------------------------------------------------------------
// TCB channels (Max/NeL TCB formulas + the biped boundary rule)

static void tcbFactors(float tens, float cont, float bias, double timeBefore, double time,
                       double timeAfter, bool firstKey, bool endKey,
                       double &ksm, double &ksp, double &kdm, double &kdp)
{
	double fp = 1.0, fn = 1.0;
	if (!firstKey && !endKey)
	{
		double dtm = 0.5 * (timeAfter - timeBefore);
		fp = (time - timeBefore) / dtm;
		fn = (timeAfter - time) / dtm;
		double c = fabs((double)cont);
		fp = fp + c - c * fp;
		fn = fn + c - c * fn;
	}
	double cm = 1.0 - cont;
	double tm = 0.5 * (1.0 - tens);
	double cp = 2.0 - cm;
	double bm = 1.0 - bias;
	double bp = 2.0 - bm;
	double tmcm = tm * cm, tmcp = tm * cp;
	ksm = tmcm * bp * fp; ksp = tmcp * bm * fp;
	kdm = tmcp * bp * fn; kdp = tmcm * bm * fn;
}

void TCBScalarChannel::compile()
{
	size_t n = Keys.size();
	TanFrom.assign(n, 0.0);
	TanTo.assign(n, 0.0);
	if (n <= 1) return;
	for (size_t i = 1; i + 1 < n; ++i)
	{
		double ksm, ksp, kdm, kdp;
		tcbFactors(Keys[i].Tens, Keys[i].Cont, Keys[i].Bias,
		           Keys[i-1].Time, Keys[i].Time, Keys[i+1].Time, false, false, ksm, ksp, kdm, kdp);
		double delm = Keys[i].Value - Keys[i-1].Value;
		double delp = Keys[i+1].Value - Keys[i].Value;
		TanTo[i] = delm*ksm + delp*ksp;
		TanFrom[i] = delm*kdm + delp*kdp;
	}
	// biped boundary rule: plain forward/backward difference scaled by (1 - tension)
	TanFrom[0] = (Keys[1].Value - Keys[0].Value) * (1.0 - Keys[0].Tens);
	TanTo[n-1] = (Keys[n-1].Value - Keys[n-2].Value) * (1.0 - Keys[n-1].Tens);
}

// Max segment ease curve: piecewise-quadratic time warp; a = start key's easeFrom, b = end
// key's easeTo, both 0..1. Classic Max SDK formula (identical in the 2004-era exporter's Max).
static double easeWarp(double u, double a, double b)
{
	double s = a + b;
	if (u <= 0.0 || u >= 1.0 || s <= 0.0) return u;
	if (s > 1.0) { a /= s; b /= s; }
	double k = 1.0 / (2.0 - a - b);
	if (u < a) return (k / a) * u * u;
	if (u < 1.0 - b) return k * (2.0 * u - a);
	u = 1.0 - u;
	return 1.0 - (k / b) * u * u;
}

double TCBScalarChannel::eval(double t) const
{
	size_t n = Keys.size();
	if (n == 0) return 0.0;
	if (t <= Keys.front().Time) return Keys.front().Value;
	if (t >= Keys.back().Time) return Keys.back().Value;
	for (size_t i = 0; i + 1 < n; ++i)
	{
		if (t >= Keys[i].Time && t <= Keys[i+1].Time)
		{
			double d = (t - Keys[i].Time) / (double)(Keys[i+1].Time - Keys[i].Time);
			if (CosineEase)
			{
				d = 0.5 * (1.0 - cos(d * M_PI));
				return Keys[i].Value * (1.0 - d) + Keys[i+1].Value * d;
			}
			d = easeWarp(d, Keys[i].EaseFrom, Keys[i+1].EaseTo);
			double d2 = d*d, d3 = d2*d;
			double a = 3.0*d2 - 2.0*d3;
			return Keys[i].Value*(1.0-a) + Keys[i+1].Value*a
			     + TanFrom[i]*(d3 - 2.0*d2 + d) + TanTo[i+1]*(d3 - d2);
		}
	}
	return Keys.back().Value;
}

void TCBQuatChannel::compile()
{
	size_t n = Keys.size();
	A.assign(n, QuatD());
	B.assign(n, QuatD());
	if (n <= 1) return;
	// chain makeClosest on absolutes
	for (size_t i = 1; i < n; ++i)
		Keys[i].Quat = qClosest(Keys[i].Quat, Keys[i-1].Quat);
	for (size_t i = 0; i < n; ++i)
	{
		bool first = (i == 0), last = (i == n-1);
		QuatD qm, qp;
		if (!first)
			qm = qLog(qMul(qConj(qClosest(Keys[i-1].Quat, Keys[i].Quat)), Keys[i].Quat));
		if (!last)
			qp = qLog(qMul(qConj(Keys[i].Quat), qClosest(Keys[i+1].Quat, Keys[i].Quat)));
		if (first) qm = qp;
		if (last) qp = qm;
		double tb = first ? Keys[i].Time : Keys[i-1].Time;
		double ta = last ? Keys[i].Time : Keys[i+1].Time;
		double ksm, ksp, kdm, kdp;
		tcbFactors(Keys[i].Tens, Keys[i].Cont, Keys[i].Bias, tb, Keys[i].Time, ta, first, last, ksm, ksp, kdm, kdp);
		QuatD qa(0.5*(qm.x*kdm + qp.x*(kdp-1.0)), 0.5*(qm.y*kdm + qp.y*(kdp-1.0)),
		         0.5*(qm.z*kdm + qp.z*(kdp-1.0)), 0.5*(qm.w*kdm + qp.w*(kdp-1.0)));
		QuatD qb(0.5*(qm.x*(1.0-ksm) - qp.x*ksp), 0.5*(qm.y*(1.0-ksm) - qp.y*ksp),
		         0.5*(qm.z*(1.0-ksm) - qp.z*ksp), 0.5*(qm.w*(1.0-ksm) - qp.w*ksp));
		A[i] = qMul(Keys[i].Quat, qExp(qa));
		B[i] = qMul(Keys[i].Quat, qExp(qb));
	}
}

QuatD TCBQuatChannel::eval(double t) const
{
	size_t n = Keys.size();
	if (n == 0) return QuatD();
	if (t <= Keys.front().Time) return Keys.front().Quat;
	if (t >= Keys.back().Time) return Keys.back().Quat;
	for (size_t i = 0; i + 1 < n; ++i)
	{
		if (t >= Keys[i].Time && t <= Keys[i+1].Time)
		{
			double d = (t - Keys[i].Time) / (double)(Keys[i+1].Time - Keys[i].Time);
			if (CosineEase)
				return qSlerp(Keys[i].Quat, Keys[i+1].Quat, 0.5 * (1.0 - cos(d * M_PI)));
			d = easeWarp(d, Keys[i].EaseFrom, Keys[i+1].EaseTo);
			QuatD s0 = qSlerp(Keys[i].Quat, Keys[i+1].Quat, d);
			QuatD s1 = qSlerp(A[i], B[i+1], d);
			return qSlerp(s0, s1, 2.0*d*(1.0-d));
		}
	}
	return Keys.back().Quat;
}

// ---------------------------------------------------------------------------------------------
// keytrack parsing

static const float *sysChunkFloats(CSceneClass *sys, uint16 chunkId, size_t &countOut)
{
	countOut = 0;
	IStorageObject *chunk = findChunkAnywhere(sys, chunkId);
	if (!chunk) return NULL;
	CStorageRaw *raw = dynamic_cast<CStorageRaw *>(chunk);
	if (!raw || raw->Value.size() < 4) return NULL;
	countOut = raw->Value.size() / 4;
	return reinterpret_cast<const float *>(nlVectorData(raw->Value));
}

static uint32 fBits(float f) { uint32 u; memcpy(&u, &f, 4); return u; }

// Parse one keytrack chunk pair. dataHdr: dwords before the first record in the data chunk.
// recSize 0 = infer from (n - dataHdr) / count. vertical=true parses only the first of the
// two banks (hdr 1 + count*13 each).
static void parseTrack(CSceneClass *sys, uint16 dataId, uint16 timeId, int dataHdr, int recSize,
                       SBipKeyTrack &out, bool vertical = false)
{
	size_t dn = 0, tn = 0;
	const float *d = sysChunkFloats(sys, dataId, dn);
	const float *t = sysChunkFloats(sys, timeId, tn);
	if (!d || !t || dn < 1 || tn < 8) return;
	sint32 count = (sint32)fBits(d[0]);
	if (count <= 0 || count > 100000) return;
	sint32 tcount = (sint32)fBits(t[0]);
	if (tcount != count) return;
	// time records
	if ((tn - 7) % count) return;
	size_t trec = (tn - 7) / count;
	if (trec < 10) return;
	// data records
	size_t rec;
	if (vertical)
	{
		// two banks: 1 + count*13 + 1 + count*13
		rec = 13;
		if (dn < 1 + (size_t)count * rec) return;
	}
	else if (recSize > 0)
	{
		rec = (size_t)recSize;
		if (dn < (size_t)dataHdr + (size_t)count * rec) return;
	}
	else
	{
		if (((sint32)dn - dataHdr) % count) return;
		rec = ((sint32)dn - dataHdr) / count;
	}
	out.Times.resize(count);
	out.Recs.resize(count);
	out.Tens.resize(count);
	out.Cont.resize(count);
	out.Bias.resize(count);
	out.EaseTo.resize(count);
	out.EaseFrom.resize(count);
	for (sint32 k = 0; k < count; ++k)
	{
		const float *tr = t + 7 + (size_t)k * trec;
		sint32 timeTicks;
		memcpy(&timeTicks, &tr[0], 4);
		out.Times[k] = timeTicks;
		// TCB in UI units (0..50, default 25) -> internal -1..1. Stored record order is
		// (easeTo, easeFrom, tension, BIAS, CONTINUITY) at [5..9] — bias BEFORE continuity,
		// pinned by the differential anim dataset's a_tcb_cont0/a_tcb_bias0 pair (the earlier
		// t,c,b assumption was undetectable while every corpus key sat at the 25 defaults).
		out.Tens[k] = (tr[7] - 25.0f) / 25.0f;
		out.Bias[k] = (tr[8] - 25.0f) / 25.0f;
		out.Cont[k] = (tr[9] - 25.0f) / 25.0f;
		out.EaseTo[k] = tr[5] / 50.0f;
		out.EaseFrom[k] = tr[6] / 50.0f;
		const float *dr = d + dataHdr + (size_t)k * rec;
		out.Recs[k].assign(dr, dr + rec);
	}
}

void parseBipAnimKeys(CSceneClass *sys, SBipAnimKeys &out)
{
	parseTrack(sys, 0x012c, 0x012d, 1, 10, out.Horizontal);
	parseTrack(sys, 0x012e, 0x012f, 3, 13, out.Turn);
	parseTrack(sys, 0x0130, 0x0131, 1, 13, out.Vertical, true);
	parseTrack(sys, 0x0132, 0x0133, 3, 7, out.Pelvis);
	parseTrack(sys, 0x0134, 0x0135, 4, 110, out.ArmR);
	parseTrack(sys, 0x0136, 0x0137, 4, 110, out.ArmL);
	parseTrack(sys, 0x0138, 0x0139, 4, 110, out.LegR);
	parseTrack(sys, 0x013a, 0x013b, 4, 110, out.LegL);
	parseTrack(sys, 0x013c, 0x013d, 4, 0, out.Spine);
	parseTrack(sys, 0x013e, 0x013f, 3, 0, out.Head);
	parseTrack(sys, 0x0142, 0x0143, 4, 0, out.Tail);
	parseTrack(sys, 0x0147, 0x0148, 4, 0, out.Pony1);
	parseTrack(sys, 0x0149, 0x014a, 4, 0, out.Pony2);
	// The range union (HasRange/RangeMin/RangeMax) is computed by CBipedAnimEval's constructor
	// over the keytracks of the bones that actually exist in the scene; see SBipAnimKeys.
}

// ---------------------------------------------------------------------------------------------
// evaluator

// crisp constants from the decode (see pipeline_max_design.md §10c)
static const QuatD Q_C(0.70710678118654752, 0.0, 0.0, 0.70710678118654752); // Rx(+90) — Y-up basis
static const QuatD Q_PELVIS_A(1.0, 0.0, 0.0, 0.0);                          // Rx(pi)
static const QuatD Q_PELVIS_B(-0.5, -0.5, 0.5, -0.5);
static const QuatD Q_UPPERARM_A(0.5, 0.5, -0.5, 0.5);
static const QuatD Q_UPPERARM_B(1.0, 0.0, 0.0, 0.0);
static const QuatD Q_THIGH_A(0.5, 0.5, 0.5, -0.5);
static const QuatD Q_THIGH_B(0.0, 1.0, 0.0, 0.0);
static const QuatD Q_HEAD_B(-0.70710678118654752, -0.70710678118654752, 0.0, 0.0);

// chain angle composition R = Rx(a3) * Rz(-a1) * Ry(a2) (double)
static QuatD chainAngleQuatD(double a1, double a2, double a3)
{
	QuatD qx = qAxisAngle(NLMISC::CVectorD(1, 0, 0), a3);
	QuatD qz = qAxisAngle(NLMISC::CVectorD(0, 0, 1), -a1);
	QuatD qy = qAxisAngle(NLMISC::CVectorD(0, 1, 0), a2);
	return qNorm(qMul(qMul(qx, qz), qy));
}

static NLMISC::CVectorD yupPos(const float *p)
{
	return NLMISC::CVectorD(p[0], -p[2], p[1]);
}

void CBipedAnimEval::quatChannelFrom(const SBipKeyTrack &tr, int off, TCBQuatChannel &out)
{
	out.Keys.clear();
	for (size_t k = 0; k < tr.Times.size(); ++k)
	{
		if (tr.Recs[k].size() < (size_t)off + 4) return;
		TCBQuatKey key;
		key.Time = tr.Times[k];
		key.Quat = qNorm(QuatD(tr.Recs[k][off], tr.Recs[k][off+1], tr.Recs[k][off+2], tr.Recs[k][off+3]));
		key.Tens = tr.Tens[k]; key.Cont = tr.Cont[k]; key.Bias = tr.Bias[k];
		key.EaseTo = tr.EaseTo[k]; key.EaseFrom = tr.EaseFrom[k];
		out.Keys.push_back(key);
	}
	out.compile();
}

void CBipedAnimEval::scalarChannelFrom(const SBipKeyTrack &tr, int off, TCBScalarChannel &out)
{
	out.Keys.clear();
	for (size_t k = 0; k < tr.Times.size(); ++k)
	{
		if (tr.Recs[k].size() < (size_t)off + 1) return;
		TCBScalarKey key;
		key.Time = tr.Times[k];
		key.Value = tr.Recs[k][off];
		key.Tens = tr.Tens[k]; key.Cont = tr.Cont[k]; key.Bias = tr.Bias[k];
		key.EaseTo = tr.EaseTo[k]; key.EaseFrom = tr.EaseFrom[k];
		out.Keys.push_back(key);
	}
	out.compile();
}

// Angle channel: same as scalarChannelFrom but unwraps each key to within pi of the previous
// one (the stored chain/bend angles wrap around 2*pi — e.g. 0 -> 6.22 -> 0 across keys — and
// interpolating the raw values would sweep the whole circle).
static void angleChannelFrom(const SBipKeyTrack &tr, int off, TCBScalarChannel &out)
{
	out.Keys.clear();
	double prev = 0.0;
	for (size_t k = 0; k < tr.Times.size(); ++k)
	{
		if (tr.Recs[k].size() < (size_t)off + 1) return;
		TCBScalarKey key;
		key.Time = tr.Times[k];
		double v = tr.Recs[k][off];
		if (k > 0)
		{
			while (v - prev > M_PI) v -= 2.0 * M_PI;
			while (v - prev < -M_PI) v += 2.0 * M_PI;
		}
		prev = v;
		key.Value = v;
		key.Tens = tr.Tens[k]; key.Cont = tr.Cont[k]; key.Bias = tr.Bias[k];
		key.EaseTo = tr.EaseTo[k]; key.EaseFrom = tr.EaseFrom[k];
		out.Keys.push_back(key);
	}
	out.compile();
}

static void vec3ChannelFrom(const SBipKeyTrack &tr, int ox, int oy, int oz, TCBVec3Channel &out)
{
	out.X.Keys.clear(); out.Y.Keys.clear(); out.Z.Keys.clear();
	for (size_t k = 0; k < tr.Times.size(); ++k)
	{
		TCBScalarKey kx, ky, kz;
		kx.Time = ky.Time = kz.Time = tr.Times[k];
		kx.Tens = ky.Tens = kz.Tens = tr.Tens[k];
		kx.Cont = ky.Cont = kz.Cont = tr.Cont[k];
		kx.Bias = ky.Bias = kz.Bias = tr.Bias[k];
		kx.EaseTo = ky.EaseTo = kz.EaseTo = tr.EaseTo[k];
		kx.EaseFrom = ky.EaseFrom = kz.EaseFrom = tr.EaseFrom[k];
		kx.Value = tr.Recs[k][ox]; ky.Value = tr.Recs[k][oy]; kz.Value = tr.Recs[k][oz];
		out.X.Keys.push_back(kx); out.Y.Keys.push_back(ky); out.Z.Keys.push_back(kz);
	}
	out.compile();
}

CBipedAnimEval::CBipedAnimEval(CSceneClass *rigSys, SBipedRig &rig,
                               const std::vector<Bone> &bones,
                               const std::map<INode *, size_t> &boneOfNode)
	: m_Sys(rigSys), m_Rig(&rig), m_HaveFigPelvis(false)
{
	parseBipAnimKeys(rigSys, m_Keys);

	// Collect this rig's nodes in walk (bone) order: the COM node and every BipDriven bone whose
	// controller references this system.
	for (size_t i = 0; i < bones.size(); ++i)
	{
		INode *node = bones[i].Node;
		if (!node) continue;
		CSceneClass *sys = bipedSystemOfCtrl(node->getReference(0));
		if (sys != rigSys) continue;
		SNodeInfo ni;
		ni.Node = node;
		ni.Parent = node->parent();
		ni.IsCom = isBipedComNode(node);
		ni.Id = 0; ni.Link = 0;
		ni.HasIdLink = readBipDrivenIdLink(node, ni.Id, ni.Link);
		ni.FigWorldRot = QuatD(bones[i].WorldTM.getRot());
		NLMISC::CVector wp = bones[i].WorldTM.getPos();
		ni.FigWorldPos = NLMISC::CVectorD(wp.x, wp.y, wp.z);
		ni.FigLocalRot = QuatD(bones[i].OrigRot);
		ni.FigLocalPos = NLMISC::CVectorD(bones[i].OrigPos.x, bones[i].OrigPos.y, bones[i].OrigPos.z);
		if (ni.IsCom)
		{
			m_FigComRot = ni.FigWorldRot;
			m_FigComPos = ni.FigWorldPos;
		}
		if (ni.HasIdLink && ni.Id == BID_PELVIS)
		{
			m_FigPelvisRot = ni.FigWorldRot;
			m_FigPelvisPos = ni.FigWorldPos;
			m_HaveFigPelvis = true;
		}
		m_NodeIdx[node] = m_Nodes.size();
		m_Nodes.push_back(ni);
	}

	// Range: the reference exporter unions the key spans of the EXISTING biped nodes'
	// controllers via IKeyControl (buildBipedInformation walks INodes), so keytracks without a
	// corresponding node (e.g. leg tracks on a first-person arms-only rig) must not contribute
	// — and neither does the COM (BIPBODY) controller: its horizontal/vertical/turn keys are
	// invisible to that enumeration (verified against fy_hof_co_fus_tir / fy_hom_co_pa_tir_
	// 1stperson, whose COM tracks extend past every limb track while the reference range stops
	// at the limbs). Recompute over m_Nodes.
	{
		bool has = false;
		sint32 mn = 0, mx = 0;
		bool idSeen[32] = { false };
		for (size_t i = 0; i < m_Nodes.size(); ++i)
		{
			if (!m_Nodes[i].IsCom && m_Nodes[i].HasIdLink && m_Nodes[i].Id < 32)
				idSeen[m_Nodes[i].Id] = true;
		}
		const SBipKeyTrack *byId[32] = { NULL };
		byId[0] = &m_Keys.ArmL; byId[1] = &m_Keys.ArmR;
		byId[2] = &m_Keys.ArmL; byId[3] = &m_Keys.ArmR;   // fingers ride the arm tracks
		byId[4] = &m_Keys.LegL; byId[5] = &m_Keys.LegR;
		byId[6] = &m_Keys.LegL; byId[7] = &m_Keys.LegR;   // toes ride the leg tracks
		byId[8] = &m_Keys.Spine; byId[9] = &m_Keys.Tail;
		byId[10] = &m_Keys.Head; byId[11] = &m_Keys.Pelvis;
		byId[16] = &m_Keys.Head; byId[17] = &m_Keys.Pony1; byId[18] = &m_Keys.Pony2;
		byId[22] = &m_Keys.ArmR; byId[23] = &m_Keys.ArmL;
		byId[24] = &m_Keys.LegL; byId[25] = &m_Keys.LegR;
		byId[26] = &m_Keys.Tail; byId[27] = &m_Keys.Head; byId[28] = &m_Keys.Pony1; byId[29] = &m_Keys.Head;
		for (int id = 0; id < 32; ++id)
		{
			if (!idSeen[id] || !byId[id] || byId[id]->empty()) continue;
			sint32 a = byId[id]->Times.front(), b = byId[id]->Times.back();
			if (!has) { mn = a; mx = b; has = true; }
			else { mn = std::min(mn, a); mx = std::max(mx, b); }
		}
		m_Keys.HasRange = has;
		m_Keys.RangeMin = mn;
		m_Keys.RangeMax = mx;
	}

	buildChannels();
}

void CBipedAnimEval::buildChannels()
{
	if (!m_Keys.Horizontal.empty())
		vec3ChannelFrom(m_Keys.Horizontal, 0, 1, 2, m_ChHorizontal); // Y-up stored (x, y_up, z_up)
	if (!m_Keys.Vertical.empty())
		scalarChannelFrom(m_Keys.Vertical, 2, m_ChVertical);
	if (!m_Keys.Turn.empty())
		quatChannelFrom(m_Keys.Turn, 4, m_ChTurn);
	if (!m_Keys.Pelvis.empty())
		quatChannelFrom(m_Keys.Pelvis, 0, m_ChPelvis);
	const SBipKeyTrack *arms[2] = { &m_Keys.ArmR, &m_Keys.ArmL };
	const SBipKeyTrack *legs[2] = { &m_Keys.LegR, &m_Keys.LegL };
	int legShift = 0;
	if (m_Rig) legShift = std::max(0, 2 * (m_Rig->MaxLegLink - 2));
	for (int s = 0; s < 2; ++s)
	{
		if (!arms[s]->empty())
		{
			angleChannelFrom(*arms[s], 0, m_ChHinge[s]);
			quatChannelFrom(*arms[s], 2, m_ChUpper[s]);
			quatChannelFrom(*arms[s], 28, m_ChEnd[s]);
			angleChannelFrom(*arms[s], 9, m_ChClavA[s]);
			angleChannelFrom(*arms[s], 10, m_ChClavB[s]);
			scalarChannelFrom(*arms[s], 12, m_ChIkBlend[0][s]);
			vec3ChannelFrom(*arms[s], 18, 19, 20, m_ChIkTarget[0][s]);
			// fingers: base quat [46+10k], bends [54+10k], [55+10k]
			size_t nf = m_Rig ? std::max(m_Rig->Fingers[0].size(), m_Rig->Fingers[1].size()) : 0;
			m_ChFingerBase[s].resize(nf);
			m_ChFingerBend[s].resize(nf * 2);
			for (size_t f = 0; f < nf; ++f)
			{
				quatChannelFrom(*arms[s], 46 + 10*(int)f, m_ChFingerBase[s][f]);
				// finger bases stay TCB — only TOE bases cosine-ease (a_fk_finger vs a_fk_toe)
				angleChannelFrom(*arms[s], 54 + 10*(int)f, m_ChFingerBend[s][f*2]);
				angleChannelFrom(*arms[s], 55 + 10*(int)f, m_ChFingerBend[s][f*2+1]);
			}
		}
		if (!legs[s]->empty())
		{
			angleChannelFrom(*legs[s], 0, m_ChLegHinge[s]);
			angleChannelFrom(*legs[s], 1, m_ChLegAnkle[s]);
			quatChannelFrom(*legs[s], 2 + legShift, m_ChLegUpper[s]);
			quatChannelFrom(*legs[s], 28 + legShift, m_ChLegEnd[s]);
			scalarChannelFrom(*legs[s], 12, m_ChIkBlend[1][s]);
			vec3ChannelFrom(*legs[s], 18, 19, 20, m_ChIkTarget[1][s]);
			size_t nt = m_Rig ? std::max(m_Rig->Toes[0].size(), m_Rig->Toes[1].size()) : 0;
			m_ChToeBase[s].resize(nt);
			m_ChToeBend[s].resize(nt * 2);
			for (size_t f = 0; f < nt; ++f)
			{
				quatChannelFrom(*legs[s], 46 + legShift + 10*(int)f, m_ChToeBase[s][f]);
				m_ChToeBase[s][f].CosineEase = true;
				angleChannelFrom(*legs[s], 54 + legShift + 10*(int)f, m_ChToeBend[s][f*2]);
				angleChannelFrom(*legs[s], 55 + legShift + 10*(int)f, m_ChToeBend[s][f*2+1]);
				// toe BENDS cosine-ease like the toe bases (b_fk_toebend); finger bends stay TCB
				m_ChToeBend[s][f*2].CosineEase = true;
				m_ChToeBend[s][f*2+1].CosineEase = true;
			}
		}
	}
	// spine/tail/pony: per-angle scalar channels (rec = count marker + angles)
	struct { const SBipKeyTrack *tr; std::vector<TCBScalarChannel> *ch; } chains[] = {
		{ &m_Keys.Spine, &m_ChSpineAng }, { &m_Keys.Tail, &m_ChTailAng },
		{ &m_Keys.Pony1, &m_ChPony1Ang }, { &m_Keys.Pony2, &m_ChPony2Ang } };
	for (size_t c = 0; c < 4; ++c)
	{
		const SBipKeyTrack &tr = *chains[c].tr;
		if (tr.empty()) continue;
		uint32 cnt = fBits(tr.Recs[0][0]);
		if (cnt > 3 * 64 || 1 + cnt > tr.Recs[0].size()) continue;
		chains[c].ch->resize(cnt);
		for (uint32 a = 0; a < cnt; ++a)
			angleChannelFrom(tr, 1 + (int)a, (*chains[c].ch)[a]);
	}
	// head: quat [0..3] + neck angles ([8]=count marker, [9..])
	if (!m_Keys.Head.empty())
	{
		quatChannelFrom(m_Keys.Head, 0, m_ChHead);
		uint32 cnt = fBits(m_Keys.Head.Recs[0][7]);
		if (cnt <= 3 * 64 && 8 + cnt <= m_Keys.Head.Recs[0].size())
		{
			m_ChNeckAng.resize(cnt);
			for (uint32 a = 0; a < cnt; ++a)
				angleChannelFrom(m_Keys.Head, 8 + (int)a, m_ChNeckAng[a]);
		}
	}
}

void CBipedAnimEval::evalAt(double t, std::map<INode *, SBipNodeState> &out)
{
	// --- COM ---
	QuatD comRot = m_FigComRot;
	if (!m_ChTurn.empty())
	{
		QuatD s = m_ChTurn.eval(t);
		comRot = qNorm(qMul(qMul(Q_C, qConj(s)), qConj(Q_C)));
	}
	NLMISC::CVectorD comPos = m_FigComPos;
	if (!m_ChHorizontal.empty())
	{
		NLMISC::CVectorD h = m_ChHorizontal.eval(t); // stored Y-up (x, y_up, z_up)
		comPos.x = h.x;
		comPos.y = -h.z;
	}
	if (!m_ChVertical.empty())
		comPos.z = m_ChVertical.eval(t);

	// per-eval state
	QuatD pelvisRot = comRot;      // updated when the pelvis is seen
	NLMISC::CVectorD pelvisPos = comPos;
	QuatD lastSpineRot = comRot;   // last spine link world state (clavicle attach frame)
	NLMISC::CVectorD lastSpinePos = comPos;
	QuatD figLastSpineRot = m_FigComRot;
	NLMISC::CVectorD figLastSpinePos = m_FigComPos;
	bool haveLastSpine = false;

	// figure-frame inverse for COM-relative fallbacks/offsets
	QuatD figComInv = qConj(m_FigComRot);

	for (size_t i = 0; i < m_Nodes.size(); ++i)
	{
		SNodeInfo &ni = m_Nodes[i];
		SBipNodeState st;
		st.HasPos = true;

		// parent state (walk order guarantees parents first); root biped node's parent may be
		// outside the rig (scene root) — treat as identity at origin then.
		QuatD parentRot;
		NLMISC::CVectorD parentPos(0, 0, 0);
		std::map<INode *, size_t>::const_iterator pit = ni.Parent ? m_NodeIdx.find(ni.Parent) : m_NodeIdx.end();
		bool haveParent = false;
		if (pit != m_NodeIdx.end())
		{
			std::map<INode *, SBipNodeState>::const_iterator ps = out.find(ni.Parent);
			if (ps != out.end())
			{
				parentRot = ps->second.WorldRot;
				parentPos = ps->second.WorldPos;
				haveParent = true;
			}
		}

		if (ni.IsCom)
		{
			st.WorldRot = comRot;
			st.WorldPos = comPos;
			out[ni.Node] = st;
			continue;
		}

		// default: figure local relative to the animated parent
		QuatD worldRot = haveParent ? qMul(parentRot, ni.FigLocalRot) : ni.FigWorldRot;
		NLMISC::CVectorD worldPos = haveParent ? parentPos + qRotate(parentRot, ni.FigLocalPos) : ni.FigWorldPos;

		if (ni.HasIdLink)
		{
			bool isLeftArm = (ni.Id == BID_LARM || ni.Id == BID_LFINGERS);
			bool isLeftLeg = (ni.Id == BID_LLEG || ni.Id == BID_LTOES);
			int sArm = isLeftArm ? 1 : 0;
			int sLeg = isLeftLeg ? 1 : 0;
			int maxLegLink = m_Rig ? m_Rig->MaxLegLink : 2;

			switch (ni.Id)
			{
			case BID_PELVIS:
				if (!m_ChPelvis.empty())
				{
					QuatD s = m_ChPelvis.eval(t);
					worldRot = qNorm(qMul(comRot, qMul(qMul(Q_PELVIS_A, qConj(s)), Q_PELVIS_B)));
				}
				else
					worldRot = qNorm(qMul(comRot, qMul(figComInv, ni.FigWorldRot)));
				// pelvis attach: fixed offset from the COM node in the pelvis frame (figure)
				{
					NLMISC::CVectorD off = qRotate(qConj(ni.FigWorldRot), ni.FigWorldPos - m_FigComPos);
					worldPos = comPos + qRotate(worldRot, off);
				}
				break;
			case BID_SPINE:
				if (ni.Link == 0)
				{
					if (!m_ChSpineAng.empty() && m_ChSpineAng.size() >= 3)
					{
						QuatD figAng = chainAngleQuatD(m_Rig && !m_Rig->Spine.Angles.empty() ? m_Rig->Spine.Angles[0].x : 0.0,
						                               m_Rig && !m_Rig->Spine.Angles.empty() ? m_Rig->Spine.Angles[0].y : 0.0,
						                               m_Rig && !m_Rig->Spine.Angles.empty() ? m_Rig->Spine.Angles[0].z : 0.0);
						QuatD C = qMul(qMul(figComInv, ni.FigWorldRot), qConj(figAng));
						QuatD R = chainAngleQuatD(m_ChSpineAng[0].eval(t), m_ChSpineAng[1].eval(t), m_ChSpineAng[2].eval(t));
						worldRot = qNorm(qMul(comRot, qMul(C, R)));
					}
					else
						worldRot = qNorm(qMul(comRot, qMul(figComInv, ni.FigWorldRot)));
					// spine base attach: rigid in the COM frame (figure offset)
					NLMISC::CVectorD off = qRotate(figComInv, ni.FigWorldPos - m_FigComPos);
					worldPos = comPos + qRotate(comRot, off);
				}
				else if (m_ChSpineAng.size() >= (ni.Link + 1) * 3)
				{
					QuatD figAng = chainAngleQuatD(
						m_Rig && ni.Link < m_Rig->Spine.Angles.size() ? m_Rig->Spine.Angles[ni.Link].x : 0.0,
						m_Rig && ni.Link < m_Rig->Spine.Angles.size() ? m_Rig->Spine.Angles[ni.Link].y : 0.0,
						m_Rig && ni.Link < m_Rig->Spine.Angles.size() ? m_Rig->Spine.Angles[ni.Link].z : 0.0);
					QuatD C = qMul(ni.FigLocalRot, qConj(figAng));
					QuatD R = chainAngleQuatD(m_ChSpineAng[ni.Link*3].eval(t), m_ChSpineAng[ni.Link*3+1].eval(t), m_ChSpineAng[ni.Link*3+2].eval(t));
					worldRot = qNorm(qMul(parentRot, qMul(C, R)));
				}
				lastSpineRot = worldRot;
				lastSpinePos = worldPos;
				figLastSpineRot = ni.FigWorldRot;
				figLastSpinePos = ni.FigWorldPos;
				haveLastSpine = true;
				break;
			case BID_TAIL:
				if (m_ChTailAng.empty() && ni.Link == 0)
				{
					// unkeyed tail: base holds its figure orientation in the COM frame (world-hold
					// rule from the differential anim dataset: unkeyed world/COM-frame roles do NOT
					// follow the node hierarchy — a_fk_spine's tail/legs/head/arms all stay put).
					worldRot = qNorm(qMul(comRot, qMul(figComInv, ni.FigWorldRot)));
				}
				if (!m_ChTailAng.empty() && m_ChTailAng.size() >= (ni.Link + 1) * 3)
				{
					QuatD figAng = chainAngleQuatD(
						m_Rig && ni.Link < m_Rig->Tail.Angles.size() ? m_Rig->Tail.Angles[ni.Link].x : 0.0,
						m_Rig && ni.Link < m_Rig->Tail.Angles.size() ? m_Rig->Tail.Angles[ni.Link].y : 0.0,
						m_Rig && ni.Link < m_Rig->Tail.Angles.size() ? m_Rig->Tail.Angles[ni.Link].z : 0.0);
					QuatD R = chainAngleQuatD(m_ChTailAng[ni.Link*3].eval(t), m_ChTailAng[ni.Link*3+1].eval(t), m_ChTailAng[ni.Link*3+2].eval(t));
					if (ni.Link == 0)
					{
						QuatD C = qMul(qMul(figComInv, ni.FigWorldRot), qConj(figAng));
						worldRot = qNorm(qMul(comRot, qMul(C, R)));
					}
					else
					{
						QuatD C = qMul(ni.FigLocalRot, qConj(figAng));
						worldRot = qNorm(qMul(parentRot, qMul(C, R)));
					}
				}
				if (ni.Link == 0)
				{
					// tail base attach: rigid in the COM frame (figure offset)
					NLMISC::CVectorD off = qRotate(figComInv, ni.FigWorldPos - m_FigComPos);
					worldPos = comPos + qRotate(comRot, off);
				}
				break;
			case BID_PONY1:
				if (!m_ChPony1Ang.empty() && m_ChPony1Ang.size() >= (ni.Link + 1) * 3)
				{
					QuatD figAng = chainAngleQuatD(
						m_Rig && ni.Link < m_Rig->Pony1.Angles.size() ? m_Rig->Pony1.Angles[ni.Link].x : 0.0,
						m_Rig && ni.Link < m_Rig->Pony1.Angles.size() ? m_Rig->Pony1.Angles[ni.Link].y : 0.0,
						m_Rig && ni.Link < m_Rig->Pony1.Angles.size() ? m_Rig->Pony1.Angles[ni.Link].z : 0.0);
					QuatD C = qMul(ni.FigLocalRot, qConj(figAng));
					QuatD R = chainAngleQuatD(m_ChPony1Ang[ni.Link*3].eval(t), m_ChPony1Ang[ni.Link*3+1].eval(t), m_ChPony1Ang[ni.Link*3+2].eval(t));
					worldRot = qNorm(qMul(parentRot, qMul(C, R)));
				}
				break;
			case BID_PONY2:
				if (!m_ChPony2Ang.empty() && m_ChPony2Ang.size() >= (ni.Link + 1) * 3)
				{
					QuatD figAng = chainAngleQuatD(
						m_Rig && ni.Link < m_Rig->Pony2.Angles.size() ? m_Rig->Pony2.Angles[ni.Link].x : 0.0,
						m_Rig && ni.Link < m_Rig->Pony2.Angles.size() ? m_Rig->Pony2.Angles[ni.Link].y : 0.0,
						m_Rig && ni.Link < m_Rig->Pony2.Angles.size() ? m_Rig->Pony2.Angles[ni.Link].z : 0.0);
					QuatD C = qMul(ni.FigLocalRot, qConj(figAng));
					QuatD R = chainAngleQuatD(m_ChPony2Ang[ni.Link*3].eval(t), m_ChPony2Ang[ni.Link*3+1].eval(t), m_ChPony2Ang[ni.Link*3+2].eval(t));
					worldRot = qNorm(qMul(parentRot, qMul(C, R)));
				}
				break;
			case BID_NECK:
				if (!m_ChNeckAng.empty() && m_ChNeckAng.size() >= (ni.Link + 1) * 3)
				{
					QuatD figAng = chainAngleQuatD(
						m_Rig && ni.Link < m_Rig->NeckAngles.size() ? m_Rig->NeckAngles[ni.Link].x : 0.0,
						m_Rig && ni.Link < m_Rig->NeckAngles.size() ? m_Rig->NeckAngles[ni.Link].y : 0.0,
						m_Rig && ni.Link < m_Rig->NeckAngles.size() ? m_Rig->NeckAngles[ni.Link].z : 0.0);
					QuatD C = qMul(ni.FigLocalRot, qConj(figAng));
					QuatD R = chainAngleQuatD(m_ChNeckAng[ni.Link*3].eval(t), m_ChNeckAng[ni.Link*3+1].eval(t), m_ChNeckAng[ni.Link*3+2].eval(t));
					worldRot = qNorm(qMul(parentRot, qMul(C, R)));
				}
				break;
			case BID_HEAD:
				if (!m_ChHead.empty())
				{
					QuatD s = m_ChHead.eval(t);
					worldRot = qNorm(qMul(comRot, qMul(qMul(Q_C, qConj(s)), Q_HEAD_B)));
				}
				else
					worldRot = qNorm(qMul(comRot, qMul(figComInv, ni.FigWorldRot))); // world-hold (see BID_TAIL)
				break;
			case BID_LARM: case BID_RARM:
			{
				const TCBQuatChannel &up = m_ChUpper[sArm];
				if (ni.Link == 0) // clavicle
				{
					double a = m_ChClavA[sArm].empty() ? 0.0 : m_ChClavA[sArm].eval(t);
					double figB = m_Rig ? m_Rig->ClavicleB[sArm] : 0.0;
					double figA = m_Rig ? m_Rig->ClavicleA[sArm] : 0.0;
					double b = figB + (m_ChClavB[sArm].empty() ? 0.0 : m_ChClavB[sArm].eval(t));
					if (!m_ChClavA[sArm].empty() || !m_ChClavB[sArm].empty())
					{
						a += figA;
						double phi = M_PI / 4.0 + b * 0.5;
						QuatD qb(cos(phi), 0.0, sin(phi), 0.0);
						QuatD qa = qAxisAngle(NLMISC::CVectorD(1, 0, 0), -a);
						QuatD qe = qAxisAngle(NLMISC::CVectorD(0, 0, 1), -(double)BIPED_EPS_TWIST);
						QuatD rel = qMul(qMul(qa, qe), qb);
						if (!isLeftArm) rel = qMirrorLR(rel);
						worldRot = qNorm(qMul(haveLastSpine ? lastSpineRot : parentRot, rel));
					}
					// clavicle attach: fixed offset off the LAST SPINE LINK (validated 3.6e-5 on
					// the character corpus; the parent-relative default would be neck-rigid, which
					// drifts as the neck rotates against the spine).
					if (haveLastSpine)
					{
						NLMISC::CVectorD off = qRotate(qConj(figLastSpineRot), ni.FigWorldPos - figLastSpinePos);
						worldPos = lastSpinePos + qRotate(lastSpineRot, off);
					}
					break;
				}
				else if (ni.Link == 1) // upperarm
				{
					if (!up.empty())
					{
						QuatD s = up.eval(t);
						if (isLeftArm) s = qMirrorLR(s);
						worldRot = qNorm(qMul(comRot, qMul(qMul(Q_UPPERARM_A, qConj(s)), Q_UPPERARM_B)));
					}
					else
						worldRot = qNorm(qMul(comRot, qMul(figComInv, ni.FigWorldRot))); // world-hold (see BID_TAIL)
				}
				else if (ni.Link == 2) // forearm (elbow hinge)
				{
					if (!m_ChHinge[sArm].empty())
					{
						double a = m_ChHinge[sArm].eval(t);
						worldRot = qNorm(qMul(parentRot, qAxisAngle(NLMISC::CVectorD(0, 0, 1), a - M_PI)));
					}
				}
				else if (ni.Link == 3) // hand
				{
					if (!m_ChEnd[sArm].empty())
					{
						QuatD s = m_ChEnd[sArm].eval(t);
						QuatD local;
						if (isLeftArm)
							local = qMul(qConj(s), qAxisAngle(NLMISC::CVectorD(1, 0, 0), -M_PI/2.0));
						else
							local = qMul(qConj(qMirrorLR(s)), qAxisAngle(NLMISC::CVectorD(1, 0, 0), M_PI/2.0));
						worldRot = qNorm(qMul(parentRot, local));
					}
				}
				break;
			}
			case BID_LLEG: case BID_RLEG:
			{
				if (ni.Link == 0) // thigh
				{
					if (!m_ChLegUpper[sLeg].empty())
					{
						QuatD s = m_ChLegUpper[sLeg].eval(t);
						if (!isLeftLeg) s = qMirrorLR(s);
						worldRot = qNorm(qMul(comRot, qMul(qMul(Q_THIGH_A, qConj(s)), Q_THIGH_B)));
					}
					else
						worldRot = qNorm(qMul(comRot, qMul(figComInv, ni.FigWorldRot))); // world-hold: COM-anchored even under pelvis keys (b_fk_pelvis)
					// thigh attach: fixed offset in the PELVIS frame — the walk parent differs by
					// era (fresh Max 9 rigs hang thighs off the lowest spine link) but the attach
					// doesn't (differential anim dataset: a_fk_spine's legs stay put).
					if (m_HaveFigPelvis)
					{
						NLMISC::CVectorD off = qRotate(qConj(m_FigPelvisRot), ni.FigWorldPos - m_FigPelvisPos);
						worldPos = pelvisPos + qRotate(pelvisRot, off);
					}
				}
				else if (ni.Link == 1) // calf (knee hinge)
				{
					if (!m_ChLegHinge[sLeg].empty())
					{
						double a = m_ChLegHinge[sLeg].eval(t);
						worldRot = qNorm(qMul(parentRot, qAxisAngle(NLMISC::CVectorD(0, 0, 1), a - M_PI)));
					}
				}
				else if ((int)ni.Link == 2 && maxLegLink == 3) // horse ankle hinge
				{
					if (!m_ChLegAnkle[sLeg].empty())
					{
						double a = m_ChLegAnkle[sLeg].eval(t);
						worldRot = qNorm(qMul(parentRot, qAxisAngle(NLMISC::CVectorD(0, 0, 1), a)));
					}
				}
				else if ((int)ni.Link == maxLegLink) // foot
				{
					if (!m_ChLegEnd[sLeg].empty())
					{
						QuatD s = m_ChLegEnd[sLeg].eval(t);
						worldRot = qNorm(qMul(comRot, qMul(Q_C, qConj(s))));
					}
					else
						worldRot = qNorm(qMul(comRot, qMul(figComInv, ni.FigWorldRot))); // world-hold (see BID_TAIL)
				}
				break;
			}
			case BID_LFINGERS: case BID_RFINGERS:
			{
				int side = (ni.Id == BID_LFINGERS) ? 1 : 0;
				// The anim base delta is relative to the bone's OWN half's record matrix (unlike the
				// figure decode, which reads the right half for both sides): L: local = M_L*conj(s),
				// R: local = mirror(M_R*conj(s)). Validated on the character corpus (3e-4 residual).
				int srcHalf = (m_Rig && !m_Rig->Fingers[side].empty()) ? side : (side ? 0 : 1);
				const std::vector<SBipedFinger> &fingers = m_Rig ? m_Rig->Fingers[srcHalf] : std::vector<SBipedFinger>();
				int fi = 0, sub = 0;
				if (m_Rig && locateChainSub(fingers, ni.Link, fi, sub))
				{
					if (sub == 0)
					{
						if ((size_t)fi < m_ChFingerBase[side].size() && !m_ChFingerBase[side][fi].empty())
						{
							QuatD s = m_ChFingerBase[side][fi].eval(t);
							QuatD local = qMul(QuatD(fingers[fi].Rot), qConj(s));
							if (side == 0) local = qMirrorLR(local);
							worldRot = qNorm(qMul(parentRot, local));
						}
					}
					else
					{
						size_t bendIdx = (size_t)fi * 2 + (size_t)(sub - 1);
						if (bendIdx < m_ChFingerBend[side].size() && !m_ChFingerBend[side][bendIdx].empty())
						{
							double a = m_ChFingerBend[side][bendIdx].eval(t);
							worldRot = qNorm(qMul(parentRot, qAxisAngle(NLMISC::CVectorD(0, 0, 1), a)));
						}
					}
				}
				break;
			}
			case BID_LTOES: case BID_RTOES:
			{
				int side = (ni.Id == BID_LTOES) ? 1 : 0;
				// Unlike the fingers, the toe base uses the RIGHT half's record matrix for BOTH
				// sides (the left half's own matrices use another basis — same era caveat as the
				// figure decode): R: local = M_R*conj(s_R); L: local = mirror(M_R*conj(s_L)).
				// Validated to float noise on the character corpus. NB: SBipedToe.Rot already
				// carries the mirrorQuatLR z-flip from parseLegRecord, matching the raw
				// rows-as-IJK decode this rule was solved against.
				int srcHalf = (m_Rig && !m_Rig->Toes[0].empty()) ? 0 : 1;
				const std::vector<SBipedToe> &toes = m_Rig ? m_Rig->Toes[srcHalf] : std::vector<SBipedToe>();
				int ti = 0, sub = 0;
				if (m_Rig && locateChainSub(toes, ni.Link, ti, sub))
				{
					if (sub == 0)
					{
						if ((size_t)ti < m_ChToeBase[side].size() && !m_ChToeBase[side][ti].empty())
						{
							QuatD s = m_ChToeBase[side][ti].eval(t);
							QuatD local;
							if (m_Rig && m_Rig->FigureVersion == 0)
							{
								// Fresh-format rule = the figure decode's (toe.Rot keeps the z-flip;
								// right direct, left = LR mirror of the composed local) — pinned by
								// the differential anim dataset's a_fk_toe.
								local = qMul(QuatD(toes[ti].Rot), qConj(s));
							}
							else
							{
								// Legacy: raw rows-as-IJK matrix (undo the z-flip).
								QuatD Mraw = qMirrorLR(QuatD(toes[ti].Rot));
								local = qMul(Mraw, qConj(s));
							}
							if (side == 1) local = qMirrorLR(local);
							worldRot = qNorm(qMul(parentRot, local));
						}
					}
					else
					{
						size_t bendIdx = (size_t)ti * 2 + (size_t)(sub - 1);
						if (bendIdx < m_ChToeBend[side].size() && !m_ChToeBend[side][bendIdx].empty())
						{
							double a = m_ChToeBend[side][bendIdx].eval(t);
							worldRot = qNorm(qMul(parentRot, qAxisAngle(NLMISC::CVectorD(0, 0, 1), a)));
						}
					}
				}
				break;
			}
			default:
				break; // nubs, props, footprints: figure fallback
			}

			if (ni.Id == BID_PELVIS)
			{
				pelvisRot = worldRot;
				pelvisPos = worldPos;
			}
		}

		st.WorldRot = worldRot;
		st.WorldPos = worldPos;
		out[ni.Node] = st;
	}

	// --- IK pass: legs and arms with IK blend keys. Solves thigh/calf (upperarm/forearm) toward
	// the interpolated world end-effector position; end rotation stays the channel value.
	// Approximation for in-between frames (exact at keys); see the wiki open-work note.
	static const bool s_enableIk = (getenv("PMB_BIPED_IK") != NULL); // experimental — see wiki open-work note
	for (int limb = 0; limb < 2 && s_enableIk; ++limb) // 0=arm, 1=leg
	{
		for (int side = 0; side < 2; ++side) // 0=R, 1=L
		{
			const TCBScalarChannel &blendCh = m_ChIkBlend[limb][side];
			if (blendCh.empty()) continue;
			// end-key rule: use the interval's END key blend gate (validated corpus behavior:
			// alpha 1->0 intervals evaluate pure FK)
			double alpha = 0.0;
			{
				const std::vector<TCBScalarKey> &ks = blendCh.Keys;
				if (t <= ks.front().Time) alpha = std::min(1.0, std::max(0.0, ks.front().Value));
				else if (t >= ks.back().Time) alpha = std::min(1.0, std::max(0.0, ks.back().Value));
				else
				{
					for (size_t i = 0; i + 1 < ks.size(); ++i)
					{
						if (t >= ks[i].Time && t <= ks[i+1].Time)
						{
							double a1 = std::min(1.0, std::max(0.0, ks[i+1].Value));
							if (a1 <= 0.0) { alpha = 0.0; break; }
							alpha = std::min(1.0, std::max(0.0, blendCh.eval(t)));
							break;
						}
					}
				}
			}
			if (alpha <= 0.0) continue;
			// find the chain nodes
			uint32 upperId = limb ? (side ? BID_LLEG : BID_RLEG) : (side ? BID_LARM : BID_RARM);
			uint32 upLink = limb ? 0 : 1;
			INode *nUp = NULL, *nMid = NULL, *nEnd = NULL;
			int maxLegLink = m_Rig ? m_Rig->MaxLegLink : 2;
			for (size_t i = 0; i < m_Nodes.size(); ++i)
			{
				if (!m_Nodes[i].HasIdLink || m_Nodes[i].Id != upperId) continue;
				uint32 l = m_Nodes[i].Link;
				if (l == upLink) nUp = m_Nodes[i].Node;
				else if (l == upLink + 1) nMid = m_Nodes[i].Node;
				else if ((limb && (int)l == maxLegLink) || (!limb && l == 3)) nEnd = m_Nodes[i].Node;
			}
			if (!nUp || !nMid || !nEnd) continue;
			SBipNodeState &stUp = out[nUp];
			SBipNodeState &stMid = out[nMid];
			SBipNodeState &stEnd = out[nEnd];
			size_t iUp = m_NodeIdx[nUp], iMid = m_NodeIdx[nMid], iEnd = m_NodeIdx[nEnd];
			double L1 = (m_Nodes[iMid].FigWorldPos - m_Nodes[iUp].FigWorldPos).norm();
			double L2 = (m_Nodes[iEnd].FigWorldPos - m_Nodes[iMid].FigWorldPos).norm();
			if (L1 <= 0.0 || L2 <= 0.0) continue;
			NLMISC::CVectorD H = stUp.WorldPos;
			NLMISC::CVectorD ankFk = stEnd.WorldPos;
			NLMISC::CVectorD Tik = ankFk;
			if (!m_ChIkTarget[limb][side].empty())
			{
				NLMISC::CVectorD raw = m_ChIkTarget[limb][side].eval(t);
				Tik = NLMISC::CVectorD(raw.x, -raw.z, raw.y); // Y-up -> Z-up
			}
			NLMISC::CVectorD T = ankFk + (Tik - ankFk) * alpha;
			double d = (T - H).norm();
			double dmin = fabs(L1 - L2) + 1e-9, dmax = L1 + L2 - 1e-9;
			if (d < dmin) d = dmin;
			if (d > dmax) d = dmax;
			NLMISC::CVectorD uFk = ankFk - H;
			double nFk = uFk.norm();
			if (nFk < 1e-9) continue;
			uFk = uFk / nFk;
			NLMISC::CVectorD u = T - H;
			double nu = u.norm();
			if (nu < 1e-9) continue;
			u = u / nu;
			// align rotation uFk -> u
			NLMISC::CVectorD c(uFk.y*u.z - uFk.z*u.y, uFk.z*u.x - uFk.x*u.z, uFk.x*u.y - uFk.y*u.x);
			double dotv = std::min(1.0, std::max(-1.0, uFk * u));
			QuatD Ral(0, 0, 0, 1);
			if (dotv > -0.99999)
			{
				double s2 = sqrt((1.0 + dotv) * 2.0);
				Ral = qNorm(QuatD(c.x / s2, c.y / s2, c.z / s2, s2 / 2.0));
			}
			QuatD thigh1 = qMul(Ral, stUp.WorldRot);
			// knee interior angle + hip flexion
			double aIk = acos(std::min(1.0, std::max(-1.0, (L1*L1 + L2*L2 - d*d) / (2.0*L1*L2))));
			double phiT = acos(std::min(1.0, std::max(-1.0, (L1*L1 + d*d - L2*L2) / (2.0*L1*d))));
			NLMISC::CVectorD thX = qRotate(thigh1, NLMISC::CVectorD(1, 0, 0));
			double phiNow = acos(std::min(1.0, std::max(-1.0, thX * u)));
			NLMISC::CVectorD hz = qRotate(thigh1, NLMISC::CVectorD(0, 0, 1));
			NLMISC::CVectorD cx(u.y*thX.z - u.z*thX.y, u.z*thX.x - u.x*thX.z, u.x*thX.y - u.y*thX.x);
			double sgn = (cx * hz) >= 0.0 ? 1.0 : -1.0;
			QuatD thigh2 = qMul(qAxisAngle(hz, (phiT - phiNow) * sgn), thigh1);
			QuatD calf2 = qMul(thigh2, qAxisAngle(NLMISC::CVectorD(0, 0, 1), aIk - M_PI));
			// blend from FK by alpha
			stUp.WorldRot = qSlerp(stUp.WorldRot, qNorm(thigh2), alpha);
			QuatD midNew = qSlerp(stMid.WorldRot, qNorm(calf2), alpha);
			// positions follow: knee at hip + thigh*L1local (local pos of mid is figure-static)
			NLMISC::CVectorD midLocal = m_Nodes[iMid].FigLocalPos;
			NLMISC::CVectorD endLocal = m_Nodes[iEnd].FigLocalPos;
			stMid.WorldRot = midNew;
			stMid.WorldPos = stUp.WorldPos + qRotate(stUp.WorldRot, midLocal);
			stEnd.WorldPos = stMid.WorldPos + qRotate(stMid.WorldRot, endLocal);
			// horse-link legs: the intermediate link between calf and foot keeps its hinge but
			// follows the chain (approximation).
			// children of the end bone (toes/fingers) are re-based below.
			// re-derive descendants' positions/rotations that hang off the adjusted chain
			for (size_t i = 0; i < m_Nodes.size(); ++i)
			{
				SNodeInfo &ni2 = m_Nodes[i];
				if (ni2.Node == nUp || ni2.Node == nMid || ni2.Node == nEnd) continue;
				// only fix nodes parented under the adjusted bones (one level is enough for
				// hands/toes/fingers chains: walk order re-application)
				std::map<INode *, SBipNodeState>::iterator ps = out.find(ni2.Parent);
				if (ps == out.end()) continue;
				std::map<INode *, size_t>::iterator pidx = m_NodeIdx.find(ni2.Parent);
				if (pidx == m_NodeIdx.end()) continue;
				// recompute only pure-parent-relative followers (identity treatment): skip nodes
				// with world-frame channels (they were computed against COM which is unchanged)
				if (ni2.HasIdLink && (ni2.Id == upperId || (limb == 0 && (ni2.Id == (side ? BID_LFINGERS : BID_RFINGERS))) || (limb == 1 && (ni2.Id == (side ? BID_LTOES : BID_RTOES)))))
				{
					// re-derive: keep the node's local rotation (world = parent * local), and the
					// figure-static local position
					QuatD localRot = qMul(qConj(ps->second.WorldRot), out[ni2.Node].WorldRot);
					// NOTE: for foot/hand (world-frame channels) keep world rotation; only fix pos
					bool worldFrame = (ni2.HasIdLink && ((limb == 1 && (int)ni2.Link == maxLegLink && ni2.Id == upperId) || (limb == 0 && ni2.Link == 3 && ni2.Id == upperId)));
					(void)worldFrame; (void)localRot;
					out[ni2.Node].WorldPos = ps->second.WorldPos + qRotate(ps->second.WorldRot, ni2.FigLocalPos);
				}
			}
		}
	}
}

} /* namespace BIPANIM */

/* end of file */
