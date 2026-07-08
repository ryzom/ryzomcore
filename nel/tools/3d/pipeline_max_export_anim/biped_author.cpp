/**
 * \file biped_author.cpp
 * \brief Programmatic biped animation authoring — the asymmetric jump emote generator.
 *
 * Pipeline (see biped_author.h and pipeline_max_design.md §10u):
 *   1. Read the IDLE POSE from an existing animation's first-key records (typed CBipedSystem
 *      tracks of the idle source — bit-exact templates; unidentified cache slots are carried
 *      verbatim, corpus-proven pose-inert (§10c: same stored pose across files with different
 *      cache values)).
 *   2. Load the skeleton .max, walk the rig (PMAX_RIG), and evaluate the idle templates on it
 *      (CBipedAnimEval override-keys ctor) to calibrate: COM-local axes, limb attach offsets,
 *      2-bone axis conventions + twist residuals, foot pivot arm (pLocal).
 *   3. Author the choreography as WORLD/COM-relative channel tables (piecewise-linear author
 *      channels sampled at each track's key times; the stored TCB then interpolates through the
 *      authored key values), run a pass-A evaluation for the time-varying attach frames
 *      (hip/shoulder positions), 2-bone-solve the limbs, and invert the §10c conversions into
 *      stored record fields.
 *   4. Verify in memory (pass-B eval: end-effector-at-target, plant invariance, boundary pose),
 *      write the .max through the typed tracks (all other streams verbatim), reload from disk
 *      and re-verify.
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

#include <nel/misc/types_nl.h>
#include "biped_author.h"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <nel/misc/file.h>
#include <nel/misc/path.h>

#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/storage_ole.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class_registry.h"
#include "../pipeline_max/builtin/builtin.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/update1/update1.h"
#include "../pipeline_max/epoly/epoly.h"
#include "../pipeline_max/biped/biped.h"
#include "../pipeline_max/biped/biped_system.h"
#include "../pipeline_max/biped/biped_anim_track.h"

#include "../pipeline_max_export_common/biped_rig.h"
#include "biped_anim.h"

using namespace std;
using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PMAX_RIG;
using namespace BIPANIM;
using PIPELINE::MAX::BIPED::CBipedSystem;
using PIPELINE::MAX::BIPED::CBipedAnimTrack;

namespace BIPAUTHOR {

// =============================================================================================
// small math
// =============================================================================================

typedef NLMISC::CVectorD Vec;

static QuatD qMirrorLR(const QuatD &q) { return QuatD(q.x, q.y, -q.z, -q.w); }

// crisp frame constants from the §10c decode (duplicated from biped_anim.cpp, where they are
// file-local)
static const QuatD Q_C(0.70710678118654752, 0.0, 0.0, 0.70710678118654752); // Rx(+90)
static const QuatD Q_PELVIS_A(1.0, 0.0, 0.0, 0.0);                          // Rx(pi)
static const QuatD Q_PELVIS_B(-0.5, -0.5, 0.5, -0.5);
static const QuatD Q_UPPERARM_A(0.5, 0.5, -0.5, 0.5);
static const QuatD Q_UPPERARM_B(1.0, 0.0, 0.0, 0.0);
static const QuatD Q_THIGH_A(0.5, 0.5, 0.5, -0.5);
static const QuatD Q_THIGH_B(0.0, 1.0, 0.0, 0.0);
static const QuatD Q_HEAD_B(-0.70710678118654752, -0.70710678118654752, 0.0, 0.0);

// shortest rotation mapping unit vector a onto unit vector b
static QuatD qBetween(const Vec &a, const Vec &b)
{
	double d = a * b;
	if (d > 0.999999) return QuatD();
	if (d < -0.999999)
	{
		// pick any axis perpendicular to a
		Vec ax = fabs(a.x) < 0.9 ? Vec(1, 0, 0) : Vec(0, 1, 0);
		Vec p = a ^ ax; p.normalize();
		return qAxisAngle(p, 3.14159265358979323846);
	}
	Vec c = a ^ b;
	double s = sqrt((1.0 + d) * 2.0);
	return qNorm(QuatD(c.x / s, c.y / s, c.z / s, s / 2.0));
}

// Y-up storage <-> NeL Z-up world
static Vec yupToNel(double x, double yUp, double zUp) { return Vec(x, -zUp, yUp); }
static void nelToYup(const Vec &v, float out[3]) { out[0] = (float)v.x; out[1] = (float)v.z; out[2] = (float)(-v.y); }

// =============================================================================================
// author channels: piecewise-linear (frame, value) tables, sampled at track key times; the
// stored TCB then interpolates through the sampled values.
// =============================================================================================

struct SK1 { double f, v; };
struct SK2 { double f, a, b; };
struct SK3 { double f, a, b, c; };
struct SK4 { double f, a, b, c, d; };

static double chan1(const SK1 *t, size_t n, double f)
{
	if (f <= t[0].f) return t[0].v;
	if (f >= t[n-1].f) return t[n-1].v;
	for (size_t i = 0; i + 1 < n; ++i)
		if (f >= t[i].f && f <= t[i+1].f)
		{
			double u = (f - t[i].f) / (t[i+1].f - t[i].f);
			return t[i].v + (t[i+1].v - t[i].v) * u;
		}
	return t[n-1].v;
}

static Vec chan3(const SK3 *t, size_t n, double f)
{
	if (f <= t[0].f) return Vec(t[0].a, t[0].b, t[0].c);
	if (f >= t[n-1].f) return Vec(t[n-1].a, t[n-1].b, t[n-1].c);
	for (size_t i = 0; i + 1 < n; ++i)
		if (f >= t[i].f && f <= t[i+1].f)
		{
			double u = (f - t[i].f) / (t[i+1].f - t[i].f);
			return Vec(t[i].a + (t[i+1].a - t[i].a) * u,
			           t[i].b + (t[i+1].b - t[i].b) * u,
			           t[i].c + (t[i+1].c - t[i].c) * u);
		}
	return Vec(t[n-1].a, t[n-1].b, t[n-1].c);
}

static void chan2(const SK2 *t, size_t n, double f, double &a, double &b)
{
	if (f <= t[0].f) { a = t[0].a; b = t[0].b; return; }
	if (f >= t[n-1].f) { a = t[n-1].a; b = t[n-1].b; return; }
	for (size_t i = 0; i + 1 < n; ++i)
		if (f >= t[i].f && f <= t[i+1].f)
		{
			double u = (f - t[i].f) / (t[i+1].f - t[i].f);
			a = t[i].a + (t[i+1].a - t[i].a) * u;
			b = t[i].b + (t[i+1].b - t[i].b) * u;
			return;
		}
	a = t[n-1].a; b = t[n-1].b;
}

static void chan4(const SK4 *t, size_t n, double f, double &a, double &b, double &c, double &d)
{
	if (f <= t[0].f) { a = t[0].a; b = t[0].b; c = t[0].c; d = t[0].d; return; }
	if (f >= t[n-1].f) { a = t[n-1].a; b = t[n-1].b; c = t[n-1].c; d = t[n-1].d; return; }
	for (size_t i = 0; i + 1 < n; ++i)
		if (f >= t[i].f && f <= t[i+1].f)
		{
			double u = (f - t[i].f) / (t[i+1].f - t[i].f);
			a = t[i].a + (t[i+1].a - t[i].a) * u;
			b = t[i].b + (t[i+1].b - t[i].b) * u;
			c = t[i].c + (t[i+1].c - t[i].c) * u;
			d = t[i].d + (t[i+1].d - t[i].d) * u;
			return;
		}
	a = t[n-1].a; b = t[n-1].b; c = t[n-1].c; d = t[n-1].d;
}

#define CHAN_N(tbl) (sizeof(tbl) / sizeof((tbl)[0]))

// =============================================================================================
// choreography — the asymmetric jump, 30 fps, frames 0..56 (160 ticks/frame)
//
// Phases: idle 0 / anticipation 4-14 (weight onto the right leg, arms sweep back — the LEFT
// further, spine flexes, head nods) / drive 16-18 (extension, heels roll over the ball pivots,
// STAGGERED toe-off: right at 17, left at 18) / flight 18-32 (ballistic COM, asymmetric tuck:
// right knee pulled high, left leg trailing; left arm punches overhead, right arm abducted;
// slight left body twist; hair trails) / staggered landing (right 32, left 34) / absorb 36 /
// recover with overshoot 40-48 / settle to the exact idle key at 56.
// =============================================================================================

static const double TICKS = 160.0; // ticks per frame
static const double LAST_FRAME = 56.0;

// COM: (fwd delta, right delta, ABSOLUTE z). Flight z keys follow the exact ballistic arc
// (toe-off 0.985 at f18, v0 = 2.126 m/s, apex ~1.215 at f24.5, touchdown 0.909 at f32 — sized
// so the staggered toe-off keeps the last planted leg within its 0.894 m reach).
static const SK3 kCom[] = {
	{ 0, 0.000, 0.000, 0.9766 }, { 4, 0.000, 0.015, 0.9550 }, { 10, -0.010, 0.030, 0.7900 },
	{ 14, -0.015, 0.035, 0.7100 }, { 16, 0.000, 0.020, 0.8600 }, { 17, 0.005, 0.015, 0.9250 },
	{ 18, 0.010, 0.010, 0.9850 }, { 20, 0.015, 0.005, 1.1049 }, { 22, 0.020, 0.000, 1.1812 },
	{ 24, 0.025, -0.005, 1.2140 }, { 26, 0.030, -0.008, 1.2030 }, { 28, 0.035, -0.008, 1.1486 },
	{ 30, 0.042, -0.006, 1.0506 }, { 32, 0.050, -0.010, 0.9088 }, { 34, 0.050, -0.005, 0.8500 },
	{ 36, 0.050, 0.010, 0.7800 }, { 40, 0.035, 0.008, 0.9000 }, { 44, 0.020, 0.002, 0.9800 },
	{ 48, 0.000, 0.000, 0.9766 }, { 52, 0.000, 0.000, 0.9766 }, { 56, 0.000, 0.000, 0.9766 },
};

// COM yaw delta (rad; wind-up right, twist left through the flight)
static const SK1 kYaw[] = {
	{ 0, 0.0 }, { 4, -0.01 }, { 10, -0.06 }, { 14, -0.09 }, { 16, -0.04 }, { 18, 0.02 },
	{ 22, 0.12 }, { 26, 0.18 }, { 30, 0.12 }, { 32, 0.08 }, { 36, 0.05 }, { 40, 0.03 },
	{ 44, 0.015 }, { 48, 0.0 }, { 56, 0.0 },
};

// pelvis (pitch about COM-local right — positive = forward lean; roll about COM-local fwd)
static const SK2 kPelvis[] = {
	{ 0, 0.00, 0.00 }, { 4, 0.03, 0.02 }, { 10, 0.20, 0.05 }, { 14, 0.26, 0.06 },
	{ 16, 0.05, 0.02 }, { 18, -0.06, 0.00 }, { 22, -0.03, -0.04 }, { 26, 0.00, -0.05 },
	{ 30, 0.04, -0.02 }, { 32, 0.10, -0.03 }, { 36, 0.24, 0.04 }, { 40, 0.14, 0.02 },
	{ 44, 0.04, 0.00 }, { 48, 0.00, 0.00 }, { 56, 0.00, 0.00 },
};

// spine sagittal deltas per link (angle component 0; NEGATIVE = bend forward, per the bow decode)
static const SK3 kSpineSag[] = {
	{ 0, 0.00, 0.00, 0.00 }, { 4, -0.03, -0.02, -0.01 }, { 10, -0.18, -0.14, -0.10 },
	{ 14, -0.24, -0.18, -0.12 }, { 16, -0.02, 0.02, 0.04 }, { 18, 0.05, 0.06, 0.06 },
	{ 22, 0.02, 0.03, 0.04 }, { 26, 0.00, 0.01, 0.02 }, { 30, -0.04, -0.02, 0.00 },
	{ 32, -0.10, -0.08, -0.05 }, { 36, -0.26, -0.20, -0.13 }, { 40, -0.16, -0.12, -0.08 },
	{ 44, -0.05, -0.04, -0.02 }, { 48, 0.00, 0.00, 0.00 }, { 56, 0.00, 0.00, 0.00 },
};

// spine twist delta (angle component 1, all links — follows the flight yaw twist)
static const SK1 kSpineTwist[] = {
	{ 0, 0.0 }, { 14, -0.02 }, { 18, 0.02 }, { 22, 0.05 }, { 26, 0.06 }, { 30, 0.03 },
	{ 34, 0.01 }, { 40, 0.0 }, { 56, 0.0 },
};

// head: nod delta (stored-quat z; positive = look down per the bow decode) + lateral tilt (x)
static const SK2 kHead[] = {
	{ 0, 0.00, 0.00 }, { 4, 0.02, 0.00 }, { 10, 0.14, 0.00 }, { 14, 0.18, 0.01 },
	{ 16, 0.02, 0.00 }, { 18, -0.10, 0.00 }, { 22, -0.14, 0.05 }, { 26, -0.10, 0.05 },
	{ 30, -0.02, 0.02 }, { 32, 0.06, 0.00 }, { 36, 0.16, -0.02 }, { 40, 0.08, 0.00 },
	{ 44, 0.02, 0.00 }, { 48, 0.00, 0.00 }, { 56, 0.00, 0.00 },
};

// ponytail sagittal delta (component 0 of each link, scaled 1 / 1.5 / 2 down the chain) — lags
// the body: back on the rise, forward after the landing.
static const SK1 kPony[] = {
	{ 0, 0.00 }, { 6, 0.04 }, { 12, 0.10 }, { 16, -0.06 }, { 19, -0.16 }, { 23, -0.22 },
	{ 27, -0.12 }, { 31, 0.02 }, { 34, 0.16 }, { 38, 0.24 }, { 42, 0.12 }, { 47, 0.05 },
	{ 52, 0.01 }, { 56, 0.00 },
};

// clavicles: (L a-delta, L b-delta, R a-delta, R b-delta) added to the idle stored deltas
static const SK4 kClav[] = {
	{ 0, 0.00, 0.00, 0.00, 0.00 }, { 10, 0.06, 0.00, 0.05, 0.00 }, { 14, 0.07, 0.01, 0.06, 0.01 },
	{ 18, -0.04, 0.00, -0.02, 0.00 }, { 22, -0.08, -0.01, -0.03, 0.00 },
	{ 26, -0.10, -0.02, -0.04, -0.01 }, { 30, -0.05, 0.00, -0.02, 0.00 },
	{ 34, 0.03, 0.00, 0.03, 0.00 }, { 36, 0.05, 0.01, 0.04, 0.01 }, { 40, 0.03, 0.00, 0.02, 0.00 },
	{ 44, 0.01, 0.00, 0.01, 0.00 }, { 48, 0.00, 0.00, 0.00, 0.00 }, { 56, 0.00, 0.00, 0.00, 0.00 },
};

// wrist targets: deltas (fwd, up, right) on the idle wrist position in the COM-local frame.
// LEFT = the lead arm (sweeps back further, then the overhead punch at the apex).
static const SK3 kWristL[] = {
	{ 0, 0.00, 0.00, 0.00 }, { 4, -0.04, -0.01, 0.00 }, { 10, -0.26, -0.06, 0.03 },
	{ 14, -0.32, -0.08, 0.04 }, { 16, -0.05, 0.15, 0.00 }, { 18, 0.20, 0.45, -0.03 },
	{ 22, 0.15, 0.80, -0.08 }, { 26, 0.04, 0.94, -0.12 }, { 28, 0.14, 0.76, -0.09 },
	{ 30, 0.18, 0.50, -0.05 }, { 33, 0.15, 0.15, -0.02 }, { 36, 0.12, -0.08, 0.02 },
	{ 40, 0.05, -0.06, 0.01 }, { 44, 0.02, -0.02, 0.00 }, { 48, 0.00, 0.00, 0.00 },
	{ 56, 0.00, 0.00, 0.00 },
};
// RIGHT = the trailing arm (abducted out to the side at the apex)
static const SK3 kWristR[] = {
	{ 0, 0.00, 0.00, 0.00 }, { 4, -0.03, -0.01, 0.00 }, { 10, -0.20, -0.05, 0.02 },
	{ 14, -0.26, -0.06, 0.03 }, { 16, -0.02, 0.10, 0.05 }, { 18, 0.15, 0.30, 0.12 },
	{ 22, 0.10, 0.42, 0.30 }, { 26, 0.00, 0.48, 0.42 }, { 28, 0.04, 0.40, 0.32 },
	{ 30, 0.08, 0.30, 0.20 }, { 33, 0.12, 0.10, 0.08 }, { 36, 0.10, -0.06, 0.03 },
	{ 40, 0.04, -0.05, 0.01 }, { 44, 0.015, -0.02, 0.00 }, { 48, 0.00, 0.00, 0.00 },
	{ 56, 0.00, 0.00, 0.00 },
};

// elbow swivel (rad, about the shoulder->target axis; L elbow points out during the punch)
static const SK1 kSwivelArmL[] = { { 0, 0.0 }, { 18, 0.0 }, { 22, 0.3 }, { 26, 0.5 }, { 30, 0.2 }, { 34, 0.0 }, { 56, 0.0 } };
static const SK1 kSwivelArmR[] = { { 0, 0.0 }, { 18, 0.0 }, { 22, -0.15 }, { 26, -0.25 }, { 30, -0.1 }, { 34, 0.0 }, { 56, 0.0 } };
static const SK1 kSwivelLegR[] = { { 0, 0.0 }, { 18, 0.0 }, { 20, 0.15 }, { 28, 0.15 }, { 31, 0.0 }, { 56, 0.0 } };
static const SK1 kSwivelLegL[] = { { 0, 0.0 }, { 19, 0.0 }, { 21, -0.10 }, { 29, -0.10 }, { 33, 0.0 }, { 56, 0.0 } };

// hand wrist-flex delta (rad, local)
static const SK1 kHandL[] = { { 0, 0.0 }, { 10, -0.15 }, { 14, -0.20 }, { 18, 0.05 }, { 22, 0.10 }, { 26, 0.12 }, { 33, 0.0 }, { 36, -0.05 }, { 44, 0.0 }, { 56, 0.0 } };
static const SK1 kHandR[] = { { 0, 0.0 }, { 10, -0.10 }, { 14, -0.14 }, { 18, 0.03 }, { 22, 0.07 }, { 26, 0.08 }, { 33, 0.0 }, { 36, -0.03 }, { 44, 0.0 }, { 56, 0.0 } };

// finger curl (added to the idle bend angles; negative = open/spread). L fist through the
// anticipation, open hand at the apex; R milder.
static const SK1 kCurlL[] = { { 0, 0.0 }, { 10, 0.85 }, { 14, 0.85 }, { 18, 0.40 }, { 22, -0.12 }, { 26, -0.12 }, { 30, 0.0 }, { 33, 0.20 }, { 36, 0.10 }, { 44, 0.0 }, { 56, 0.0 } };
static const SK1 kCurlR[] = { { 0, 0.0 }, { 10, 0.55 }, { 14, 0.55 }, { 18, 0.25 }, { 22, -0.08 }, { 26, -0.08 }, { 30, 0.0 }, { 33, 0.13 }, { 36, 0.07 }, { 44, 0.0 }, { 56, 0.0 } };

// ankle targets during FLIGHT: deltas (fwd, up, right) on the idle ankle in the COM-local frame
// (right leg tucks high and forward, left trails)
static const SK3 kAnkleR[] = {
	{ 18, 0.02, 0.10, 0.00 }, { 20, 0.10, 0.32, 0.02 }, { 24, 0.16, 0.48, 0.03 },
	{ 28, 0.06, 0.24, 0.01 }, { 30, 0.02, 0.10, 0.00 }, { 31, 0.00, 0.04, 0.00 },
};
static const SK3 kAnkleL[] = {
	{ 19, 0.00, 0.06, 0.00 }, { 21, -0.06, 0.22, -0.02 }, { 25, -0.08, 0.34, -0.02 },
	{ 29, -0.04, 0.26, -0.01 }, { 31, -0.01, 0.16, 0.00 }, { 33, 0.00, 0.06, 0.00 },
};

// foot pitch (rad about the idle COM-right axis; positive = heel up / toes down, sign verified
// through the in-tool semantic report)
static const SK1 kFootPitchR[] = {
	{ 0, 0.0 }, { 10, 0.0 }, { 14, 0.06 }, { 16, 0.45 }, { 17, 0.75 }, { 20, 0.55 },
	{ 24, 0.35 }, { 28, 0.20 }, { 30, 0.25 }, { 31, 0.28 }, { 32, 0.30 }, { 34, 0.10 },
	{ 36, 0.02 }, { 40, 0.0 }, { 56, 0.0 },
};
static const SK1 kFootPitchL[] = {
	{ 0, 0.0 }, { 10, 0.0 }, { 14, 0.04 }, { 16, 0.42 }, { 18, 1.15 }, { 21, 0.75 },
	{ 25, 0.45 }, { 29, 0.25 }, { 31, 0.28 }, { 33, 0.30 }, { 34, 0.32 }, { 36, 0.12 },
	{ 40, 0.02 }, { 44, 0.0 }, { 56, 0.0 },
};

// toe flex (rad, toe-base delta; positive = toes bend up relative to the foot — the push-off
// dorsiflexion over the ball pivot; negative = pointed)
static const SK1 kToeR[] = {
	{ 0, 0.0 }, { 14, 0.05 }, { 16, 0.35 }, { 17, 0.55 }, { 20, -0.15 }, { 24, -0.15 },
	{ 28, 0.0 }, { 30, 0.10 }, { 32, 0.40 }, { 34, 0.20 }, { 36, 0.05 }, { 40, 0.0 }, { 56, 0.0 },
};
static const SK1 kToeL[] = {
	{ 0, 0.0 }, { 14, 0.04 }, { 16, 0.30 }, { 18, 0.55 }, { 21, -0.15 }, { 25, -0.15 },
	{ 29, 0.0 }, { 31, 0.10 }, { 34, 0.42 }, { 36, 0.20 }, { 38, 0.05 }, { 42, 0.0 }, { 56, 0.0 },
};

// per-track key times (frames)
static const double kTimesCom[] = { 0, 4, 10, 14, 16, 17, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 40, 44, 48, 52, 56 };
static const double kTimesPelvis[] = { 0, 4, 10, 14, 16, 18, 22, 26, 30, 32, 36, 40, 44, 48, 56 };
static const double kTimesSpine[] = { 0, 4, 10, 14, 16, 18, 22, 26, 30, 32, 36, 40, 44, 48, 56 };
static const double kTimesHead[] = { 0, 4, 10, 14, 16, 18, 22, 26, 30, 32, 36, 40, 44, 48, 56 };
static const double kTimesPony[] = { 0, 6, 12, 16, 19, 23, 27, 31, 34, 38, 42, 47, 52, 56 };
static const double kTimesArm[] = { 0, 4, 10, 14, 16, 18, 20, 22, 26, 28, 30, 33, 36, 40, 44, 48, 52, 56 };
static const double kTimesLegR[] = { 0, 4, 10, 14, 16, 17, 18, 20, 24, 28, 30, 31, 32, 34, 36, 40, 44, 48, 52, 56 };
static const double kTimesLegL[] = { 0, 4, 10, 14, 16, 18, 19, 21, 25, 29, 31, 33, 34, 36, 40, 44, 48, 52, 56 };

// leg plant state: planted while <= toe-off frame and >= touchdown frame
static const double TOEOFF_R = 17.0, TOUCH_R = 32.0;
static const double TOEOFF_L = 18.0, TOUCH_L = 34.0;

// =============================================================================================
// scene loading / rig context
// =============================================================================================

struct SLoadedMax
{
	CDllDirectory *Dll;
	CClassDirectory3 *Cd;
	CScene *Scene;
	// raw streams for verbatim write-back
	std::vector<std::string> StreamNames;
	std::vector<std::vector<uint8> > StreamBytes;
	uint8 OleClassId[16];
	bool HaveClassId;
	SLoadedMax() : Dll(NULL), Cd(NULL), Scene(NULL), HaveClassId(false) { }
};

static bool loadMax(const char *path, CSceneClassRegistry *reg, SLoadedMax &out, bool keepRaw)
{
	CStorageOleIn in;
	if (!in.open(path)) { std::cerr << "ERROR: not an OLE compound file: " << path << "\n"; return false; }
	if (keepRaw)
	{
		static const char *kStreams[] = {
			"VideoPostQueue", "Config", "ClassData", "DllDirectory", "ClassDirectory3", "Scene",
			"\05SummaryInformation", "\05DocumentSummaryInformation", NULL
		};
		for (const char **n = kStreams; *n; ++n)
		{
			std::vector<uint8> b;
			if (in.readStream(*n, b)) { out.StreamNames.push_back(*n); out.StreamBytes.push_back(b); }
		}
		out.HaveClassId = in.getClassId(out.OleClassId);
	}
	out.Dll = new CDllDirectory();
	{ std::vector<uint8> b; if (!in.readStream("DllDirectory", b)) { std::cerr << "ERROR: no DllDirectory in " << path << "\n"; return false; } CStorageStream st(b); out.Dll->serial(st); }
	out.Dll->parse(VersionUnknown);
	out.Cd = new CClassDirectory3(out.Dll);
	{ std::vector<uint8> b; if (!in.readStream("ClassDirectory3", b)) { std::cerr << "ERROR: no ClassDirectory3 in " << path << "\n"; return false; } CStorageStream st(b); out.Cd->serial(st); }
	out.Cd->parse(VersionUnknown);
	out.Scene = new CScene(reg, out.Dll, out.Cd);
	{ std::vector<uint8> b; if (!in.readStream("Scene", b)) { std::cerr << "ERROR: no Scene in " << path << "\n"; return false; } CStorageStream st(b); out.Scene->serial(st); }
	out.Scene->parse(VersionUnknown);
	return true;
}

static CBipedSystem *findBipedSystem(CSceneClassContainer *ssc)
{
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CBipedSystem *b = dynamic_cast<CBipedSystem *>(it->second);
		if (b) return b;
	}
	return NULL;
}

struct SRigCtx
{
	CSceneClassContainer *Ssc;
	CBipedSystem *Sys;
	SBipedRig *Rig;
	std::vector<Bone> Bones;
	std::map<INode *, size_t> BoneOf;
	INode *Com;
	// (boneId, link) -> node
	std::map<std::pair<uint32, uint32>, INode *> ByIdLink;

	INode *node(uint32 id, uint32 link) const
	{
		std::map<std::pair<uint32, uint32>, INode *>::const_iterator it = ByIdLink.find(std::make_pair(id, link));
		return it == ByIdLink.end() ? NULL : it->second;
	}
	Vec localPos(INode *n) const
	{
		std::map<INode *, size_t>::const_iterator it = BoneOf.find(n);
		if (it == BoneOf.end()) return Vec(0, 0, 0);
		const NLMISC::CVector &p = Bones[it->second].OrigPos;
		return Vec(p.x, p.y, p.z);
	}
};

static bool buildRigCtx(CScene *scene, SRigCtx &ctx)
{
	ctx.Ssc = scene->container();
	ctx.Sys = findBipedSystem(ctx.Ssc);
	if (!ctx.Sys) { std::cerr << "ERROR: no Biped (0x9155) system object\n"; return false; }

	INode *bip01 = NULL;
	for (CStorageContainer::TStorageObjectConstIt it = ctx.Ssc->chunks().begin(); it != ctx.Ssc->chunks().end() && !bip01; ++it)
	{
		CNodeImpl *n = dynamic_cast<CNodeImpl *>(it->second);
		if (n && NLMISC::toLower(ucstring(n->userName()).toUtf8()) == "bip01") bip01 = n;
	}
	if (!bip01) { std::cerr << "ERROR: no Bip01 node\n"; return false; }

	g_bipedRigs.clear();
	g_rig = NULL;
	g_msBones.clear();
	std::set<std::string> nameSet;
	NLMISC::CMatrix rootMat; rootMat.identity();
	walkNode(bip01, -1, rootMat, ctx.Ssc, ctx.Bones, nameSet);
	patchFootstepsGround(ctx.Bones);

	std::map<CSceneClass *, SBipedRig>::iterator rit = g_bipedRigs.find(ctx.Sys);
	if (rit == g_bipedRigs.end()) { std::cerr << "ERROR: walk produced no rig for the system object\n"; return false; }
	ctx.Rig = &rit->second;
	g_rig = ctx.Rig;

	ctx.Com = NULL;
	for (size_t i = 0; i < ctx.Bones.size(); ++i)
	{
		INode *n = ctx.Bones[i].Node;
		if (!n) continue;
		ctx.BoneOf[n] = i;
		if (isBipedComNode(n)) { if (!ctx.Com) ctx.Com = n; continue; }
		uint32 id = 0, link = 0;
		if (readBipDrivenIdLink(n, id, link))
			ctx.ByIdLink[std::make_pair(id, link)] = n;
	}
	if (!ctx.Com) { std::cerr << "ERROR: no COM node in the walked rig\n"; return false; }
	return true;
}

// =============================================================================================
// record editing helpers (uint32 rows)
// =============================================================================================

static float rowF(const std::vector<uint32> &r, size_t i) { return CBipedAnimTrack::asF(r[i]); }
static void rowSetF(std::vector<uint32> &r, size_t i, double v) { r[i] = CBipedAnimTrack::fBits((float)v); }
static void rowSetI(std::vector<uint32> &r, size_t i, sint32 v) { uint32 b; memcpy(&b, &v, 4); r[i] = b; }
static void rowSetQ(std::vector<uint32> &r, size_t i, const QuatD &q)
{
	rowSetF(r, i, q.x); rowSetF(r, i + 1, q.y); rowSetF(r, i + 2, q.z); rowSetF(r, i + 3, q.w);
}
static QuatD rowQ(const std::vector<uint32> &r, size_t i)
{
	return QuatD(rowF(r, i), rowF(r, i + 1), rowF(r, i + 2), rowF(r, i + 3));
}
static void rowSetYup(std::vector<uint32> &r, size_t i, const Vec &nel)
{
	float y[3]; nelToYup(nel, y);
	rowSetF(r, i, y[0]); rowSetF(r, i + 1, y[1]); rowSetF(r, i + 2, y[2]);
}
static Vec rowYup(const std::vector<uint32> &r, size_t i)
{
	return yupToNel(rowF(r, i), rowF(r, i + 1), rowF(r, i + 2));
}

static std::vector<uint32> makeTimeRec(size_t index, sint32 ticks)
{
	std::vector<uint32> t(10, 0);
	rowSetI(t, 0, ticks);
	rowSetF(t, 1, (double)index);
	rowSetF(t, 7, 25.0); rowSetF(t, 8, 25.0); rowSetF(t, 9, 25.0);
	return t;
}

// =============================================================================================
// stored-field inversions (§10c conversions in reverse)
// =============================================================================================

static QuatD storedTurnFromYaw(double angle) // angle = the stored [1] value
{
	// s = AxisAngle(Y-up, -angle)
	return QuatD(0.0, sin(-angle * 0.5), 0.0, cos(-angle * 0.5));
}
static QuatD comRotFromStoredTurn(const QuatD &s)
{
	return qNorm(qMul(qMul(Q_C, qConj(s)), qConj(Q_C)));
}
static QuatD storedFromPelvis(const QuatD &world, const QuatD &comRot)
{
	QuatD c = qMul(qMul(qConj(Q_PELVIS_A), qConj(comRot)), qMul(world, qConj(Q_PELVIS_B)));
	return qNorm(qConj(c));
}
static QuatD storedFromHead(const QuatD &world, const QuatD &comRot)
{
	QuatD c = qMul(qMul(qConj(Q_C), qConj(comRot)), qMul(world, qConj(Q_HEAD_B)));
	return qNorm(qConj(c));
}
static QuatD storedFromFoot(const QuatD &world, const QuatD &comRot)
{
	QuatD c = qMul(qConj(Q_C), qMul(qConj(comRot), world));
	return qNorm(qConj(c));
}
static QuatD footFromStored(const QuatD &s, const QuatD &comRot)
{
	return qNorm(qMul(comRot, qMul(Q_C, qConj(s))));
}
static QuatD storedFromThigh(const QuatD &world, const QuatD &comRot, bool leftLeg)
{
	QuatD c = qMul(qMul(qConj(Q_THIGH_A), qConj(comRot)), qMul(world, qConj(Q_THIGH_B)));
	QuatD sUsed = qNorm(qConj(c));
	return leftLeg ? sUsed : qMirrorLR(sUsed);
}
static QuatD thighFromStored(const QuatD &stored, const QuatD &comRot, bool leftLeg)
{
	QuatD sUsed = leftLeg ? stored : qMirrorLR(stored);
	return qNorm(qMul(comRot, qMul(qMul(Q_THIGH_A, qConj(sUsed)), Q_THIGH_B)));
}
static QuatD storedFromUpperArm(const QuatD &world, const QuatD &comRot, bool leftArm)
{
	QuatD c = qMul(qMul(qConj(Q_UPPERARM_A), qConj(comRot)), qMul(world, qConj(Q_UPPERARM_B)));
	QuatD sUsed = qNorm(qConj(c));
	return leftArm ? qMirrorLR(sUsed) : sUsed;
}
static QuatD upperArmFromStored(const QuatD &stored, const QuatD &comRot, bool leftArm)
{
	QuatD sUsed = leftArm ? qMirrorLR(stored) : stored;
	return qNorm(qMul(comRot, qMul(qMul(Q_UPPERARM_A, qConj(sUsed)), Q_UPPERARM_B)));
}
static QuatD handLocalFromStored(const QuatD &stored, bool leftArm)
{
	if (leftArm) return qMul(qConj(stored), qAxisAngle(Vec(1, 0, 0), -3.14159265358979323846 / 2.0));
	return qMul(qConj(qMirrorLR(stored)), qAxisAngle(Vec(1, 0, 0), 3.14159265358979323846 / 2.0));
}
static QuatD storedFromHandLocal(const QuatD &local, bool leftArm)
{
	if (leftArm) return qNorm(qConj(qMul(local, qAxisAngle(Vec(1, 0, 0), 3.14159265358979323846 / 2.0))));
	return qMirrorLR(qNorm(qConj(qMul(local, qAxisAngle(Vec(1, 0, 0), -3.14159265358979323846 / 2.0)))));
}

// =============================================================================================
// 2-bone limb solve with exact figure-local offsets
// =============================================================================================

struct SLimbGeom
{
	Vec O1, O2;        // FigLocalPos of the mid bone (in the upper frame) and end bone (in the mid frame)
	QuatD Twist;       // residual calibrated at the idle pose (identity when the frame model is exact)
	double HingeIdle;  // idle interior angle (diagnostics)
};

// chain vector in the upper-bone frame for hinge angle a (local mid = Rz(a - pi))
static Vec chainVec(const SLimbGeom &g, double a)
{
	QuatD rz = qAxisAngle(Vec(0, 0, 1), a - 3.14159265358979323846);
	return g.O1 + qRotate(rz, g.O2);
}

// solve the hinge angle so |chainVec| == d (bisect; |chainVec| is monotone in a over [0.05, pi])
static double solveHinge(const SLimbGeom &g, double d)
{
	double lo = 0.05, hi = 3.14159265358979323846 - 1e-6;
	double dlo = chainVec(g, lo).norm(), dhi = chainVec(g, hi).norm();
	if (d <= dlo) return lo;
	if (d >= dhi) return hi;
	for (int i = 0; i < 60; ++i)
	{
		double mid = 0.5 * (lo + hi);
		if (chainVec(g, mid).norm() < d) lo = mid; else hi = mid;
	}
	return 0.5 * (lo + hi);
}

// Solve the upper-bone world rotation and hinge angle placing the end bone at target T from
// attach H, with the hinge plane normal steered toward nWorld. Exact at the idle pose by the
// Twist calibration.
static void solveLimb(const SLimbGeom &g, const Vec &H, const Vec &T, const Vec &nWorld,
                      QuatD &upperWorld, double &hinge)
{
	Vec toT = T - H;
	double d = toT.norm();
	if (d < 1e-9) { hinge = g.HingeIdle; upperWorld = QuatD(); return; }
	hinge = solveHinge(g, d);
	Vec v = chainVec(g, hinge);
	double vn = v.norm();
	Vec vHat = v / vn;
	Vec uHat = toT / d;
	// align chain onto target dir, then spin about the target dir to put local z toward nWorld
	QuatD w0 = qBetween(qRotate(g.Twist, vHat), uHat); // pre-twist so the calibration is exact
	QuatD base = qMul(w0, g.Twist);
	Vec z0 = qRotate(base, Vec(0, 0, 1));
	// project both onto the plane perpendicular to uHat
	Vec zp = z0 - uHat * (z0 * uHat);
	Vec np = nWorld - uHat * (nWorld * uHat);
	double zpn = zp.norm(), npn = np.norm();
	if (zpn > 1e-9 && npn > 1e-9)
	{
		zp = zp / zpn; np = np / npn;
		double c = std::max(-1.0, std::min(1.0, zp * np));
		Vec cr = zp ^ np;
		double sgn = (cr * uHat) >= 0.0 ? 1.0 : -1.0;
		QuatD spin = qAxisAngle(uHat, sgn * acos(c));
		base = qMul(spin, base);
	}
	upperWorld = qNorm(base);
}

// =============================================================================================
// track assembly (typed rows + evaluator float views)
// =============================================================================================

struct SBuiltTrack
{
	std::vector<sint32> Ticks;
	std::vector<std::vector<uint32> > Recs;
	void clear() { Ticks.clear(); Recs.clear(); }
	void add(sint32 t, const std::vector<uint32> &r) { Ticks.push_back(t); Recs.push_back(r); }
	// Make the stored quat at the given record offset sign-continuous across keys (the record
	// inversions return arbitrary hemispheres; Character Studio stores sign-continuous
	// sequences and the TCB squad takes the long way around on a flip — a mid-interval pop).
	void signChain(size_t off)
	{
		for (size_t k = 1; k < Recs.size(); ++k)
		{
			if (Recs[k].size() < off + 4 || Recs[k-1].size() < off + 4) return;
			QuatD a = rowQ(Recs[k-1], off), b = rowQ(Recs[k], off);
			if (a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w < 0.0)
				rowSetQ(Recs[k], off, QuatD(-b.x, -b.y, -b.z, -b.w));
		}
	}
};

static void toKeyTrack(const SBuiltTrack &b, BIPANIM::SBipKeyTrack &out)
{
	size_t n = b.Ticks.size();
	out.Times.resize(n);
	out.Recs.resize(n);
	out.Tens.assign(n, 0.0f); out.Cont.assign(n, 0.0f); out.Bias.assign(n, 0.0f);
	out.EaseTo.assign(n, 0.0f); out.EaseFrom.assign(n, 0.0f);
	for (size_t k = 0; k < n; ++k)
	{
		out.Times[k] = b.Ticks[k];
		out.Recs[k].resize(b.Recs[k].size());
		for (size_t i = 0; i < b.Recs[k].size(); ++i)
			out.Recs[k][i] = rowF(b.Recs[k], i);
	}
}

static void storeTrack(CBipedSystem *sys, CBipedAnimTrack::ETrack t, const SBuiltTrack &b, bool dualBank)
{
	CBipedAnimTrack *tr = sys->trackForEdit(t);
	if (!tr) { std::cerr << "ERROR: track " << (int)t << " not lifted in the skeleton file\n"; exit(1); }
	tr->Keys.clear();
	tr->Keys.resize(b.Ticks.size());
	tr->Bank2.clear();
	for (size_t k = 0; k < b.Ticks.size(); ++k)
	{
		tr->Keys[k].TimeRec = makeTimeRec(k, b.Ticks[k]);
		tr->Keys[k].DataRec = b.Recs[k];
	}
	if (dualBank)
		for (size_t k = 0; k < b.Recs.size(); ++k)
			tr->Bank2.push_back(b.Recs[k]);
}

// =============================================================================================
// the generator
// =============================================================================================

struct SIdleTemplates
{
	std::vector<uint32> Horizontal, Turn, Vertical, Pelvis, ArmR, ArmL, LegR, LegL, Spine, Head, Pony1;
};

static bool grabTemplates(CBipedSystem *sys, SIdleTemplates &out)
{
	struct { CBipedAnimTrack::ETrack T; std::vector<uint32> *Dst; const char *Name; } req[] = {
		{ CBipedAnimTrack::TrackHorizontal, &out.Horizontal, "horizontal" },
		{ CBipedAnimTrack::TrackTurn, &out.Turn, "turn" },
		{ CBipedAnimTrack::TrackVertical, &out.Vertical, "vertical" },
		{ CBipedAnimTrack::TrackPelvis, &out.Pelvis, "pelvis" },
		{ CBipedAnimTrack::TrackArmR, &out.ArmR, "armR" },
		{ CBipedAnimTrack::TrackArmL, &out.ArmL, "armL" },
		{ CBipedAnimTrack::TrackLegR, &out.LegR, "legR" },
		{ CBipedAnimTrack::TrackLegL, &out.LegL, "legL" },
		{ CBipedAnimTrack::TrackSpine, &out.Spine, "spine" },
		{ CBipedAnimTrack::TrackHead, &out.Head, "head" },
		{ CBipedAnimTrack::TrackPony1, &out.Pony1, "pony1" },
	};
	for (size_t i = 0; i < sizeof(req) / sizeof(req[0]); ++i)
	{
		const CBipedAnimTrack *tr = sys->track(req[i].T);
		if (!tr || tr->Keys.empty()) { std::cerr << "ERROR: idle source has no " << req[i].Name << " keys\n"; return false; }
		*req[i].Dst = tr->Keys[0].DataRec;
	}
	return true;
}

// per-key evaluated attach state (pass A)
struct SAttach
{
	QuatD ComRot; Vec ComPos;
	Vec Hip[2];      // [0]=R, [1]=L (thigh node pos)
	Vec Shoulder[2]; // upperarm node pos
};

int runAuthorJump(const char *skelMax, const char *idleMax, const char *outMax)
{
	CSceneClassRegistry reg;
	CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	PIPELINE::MAX::BIPED::CBiped::registerClasses(&reg);

	// --- idle templates ---------------------------------------------------------------------
	SIdleTemplates tpl;
	{
		SLoadedMax idle;
		if (!loadMax(idleMax, &reg, idle, false)) return 1;
		CBipedSystem *idleSys = findBipedSystem(idle.Scene->container());
		if (!idleSys) { std::cerr << "ERROR: idle source has no typed Biped system\n"; return 1; }
		if (!grabTemplates(idleSys, tpl)) return 1;
		// leak idle scene objects intentionally: cheap one-shot tool, shared node pointers make
		// teardown ordering fiddly and the process exits right after
	}

	// --- skeleton scene -----------------------------------------------------------------------
	SLoadedMax skel;
	if (!loadMax(skelMax, &reg, skel, true)) return 1;
	SRigCtx ctx;
	if (!buildRigCtx(skel.Scene, ctx)) return 1;

	// sanity: template record sizes must match a 2-link-leg, 3-spine-link, 5-finger rig
	if (tpl.LegR.size() != 110 || tpl.ArmR.size() != 110 || tpl.Spine.size() != 10 || tpl.Head.size() != 12)
	{
		std::cerr << "ERROR: unexpected idle record sizes (legR " << tpl.LegR.size() << ", armR " << tpl.ArmR.size()
		          << ", spine " << tpl.Spine.size() << ", head " << tpl.Head.size() << ")\n";
		return 1;
	}

	// --- idle keys on the skeleton (calibration eval) ----------------------------------------
	SBipAnimKeys idleKeys;
	{
		struct { const std::vector<uint32> *Rec; BIPANIM::SBipKeyTrack *Tr; } m[] = {
			{ &tpl.Horizontal, &idleKeys.Horizontal }, { &tpl.Turn, &idleKeys.Turn },
			{ &tpl.Vertical, &idleKeys.Vertical }, { &tpl.Pelvis, &idleKeys.Pelvis },
			{ &tpl.ArmR, &idleKeys.ArmR }, { &tpl.ArmL, &idleKeys.ArmL },
			{ &tpl.LegR, &idleKeys.LegR }, { &tpl.LegL, &idleKeys.LegL },
			{ &tpl.Spine, &idleKeys.Spine }, { &tpl.Head, &idleKeys.Head }, { &tpl.Pony1, &idleKeys.Pony1 },
		};
		for (size_t i = 0; i < sizeof(m) / sizeof(m[0]); ++i)
		{
			SBuiltTrack b;
			b.add(0, *m[i].Rec);
			toKeyTrack(b, *m[i].Tr);
		}
	}
	CBipedAnimEval evalIdle(ctx.Sys, *ctx.Rig, ctx.Bones, ctx.BoneOf, &idleKeys);
	std::map<INode *, SBipNodeState> stIdle;
	evalIdle.evalAt(0.0, stIdle);

	// node handles
	INode *nCom = ctx.Com;
	INode *nPelvis = ctx.node(BID_PELVIS, 0);
	INode *nHead = ctx.node(BID_HEAD, 0);
	INode *nThigh[2] = { ctx.node(BID_RLEG, 0), ctx.node(BID_LLEG, 0) };
	INode *nCalf[2] = { ctx.node(BID_RLEG, 1), ctx.node(BID_LLEG, 1) };
	INode *nFoot[2] = { ctx.node(BID_RLEG, 2), ctx.node(BID_LLEG, 2) };
	INode *nToe[2] = { ctx.node(BID_RTOES, 0), ctx.node(BID_LTOES, 0) };
	INode *nUpArm[2] = { ctx.node(BID_RARM, 1), ctx.node(BID_LARM, 1) };
	INode *nForearm[2] = { ctx.node(BID_RARM, 2), ctx.node(BID_LARM, 2) };
	INode *nHand[2] = { ctx.node(BID_RARM, 3), ctx.node(BID_LARM, 3) };
	if (!nPelvis || !nHead) { std::cerr << "ERROR: missing pelvis/head nodes\n"; return 1; }
	for (int s = 0; s < 2; ++s)
		if (!nThigh[s] || !nCalf[s] || !nFoot[s] || !nUpArm[s] || !nForearm[s] || !nHand[s] || !nToe[s])
		{ std::cerr << "ERROR: missing limb nodes (side " << s << ")\n"; return 1; }

	// --- calibration --------------------------------------------------------------------------
	const QuatD comRotIdle = stIdle[nCom].WorldRot;
	const Vec comPosIdle = stIdle[nCom].WorldPos;

	// COM-local axes: up from world up; fwd from the toe-ankle ground direction
	Vec upL = qRotate(qConj(comRotIdle), Vec(0, 0, 1));
	Vec fwdW;
	{
		Vec a = stIdle[nToe[0]].WorldPos - stIdle[nFoot[0]].WorldPos;
		a.z = 0.0;
		a.normalize();
		fwdW = a;
	}
	Vec fwdL = qRotate(qConj(comRotIdle), fwdW);
	// orthonormalize fwd against up in local coords
	fwdL = fwdL - upL * (fwdL * upL); fwdL.normalize();
	Vec rightL = fwdL ^ upL; rightL.normalize();

	const double yawIdle = rowF(tpl.Turn, 1);

	// limb geometry + twist calibration
	SLimbGeom legGeom[2], armGeom[2];
	Vec relAnkleIdle[2], relWristIdle[2], legNIdle[2], armNIdle[2];
	QuatD footWorldIdle[2], handLocalIdle[2];
	Vec pLocal[2];   // foot pivot arm (pivot [98] of the idle template)
	Vec pAIdle[2];   // idle pivot world position (NeL)
	for (int s = 0; s < 2; ++s)
	{
		const std::vector<uint32> &legT = s ? tpl.LegL : tpl.LegR;
		const std::vector<uint32> &armT = s ? tpl.ArmL : tpl.ArmR;
		// legs
		legGeom[s].O1 = ctx.localPos(nCalf[s]);
		legGeom[s].O2 = ctx.localPos(nFoot[s]);
		legGeom[s].Twist = QuatD();
		Vec H = stIdle[nThigh[s]].WorldPos;
		Vec A = stIdle[nFoot[s]].WorldPos;
		double hinge = rowF(legT, 0);
		legGeom[s].HingeIdle = hinge;
		// residual: reconstruct with identity twist, compare against the evaluated thigh rot
		{
			QuatD wIdeal = stIdle[nThigh[s]].WorldRot;
			Vec v = chainVec(legGeom[s], hinge);
			Vec vHat = v / v.norm();
			Vec uHat = (A - H); uHat.normalize();
			QuatD w0 = qBetween(vHat, uHat);
			// twist = conj(w0) * actual; then solveLimb's pre-twist alignment makes idle exact
			legGeom[s].Twist = qNorm(qMul(qConj(w0), wIdeal));
		}
		legNIdle[s] = qRotate(qConj(comRotIdle), qRotate(stIdle[nThigh[s]].WorldRot, Vec(0, 0, 1)));
		relAnkleIdle[s] = qRotate(qConj(comRotIdle), A - comPosIdle);
		footWorldIdle[s] = stIdle[nFoot[s]].WorldRot;
		// pivot arm from the idle template pivots (corrected to the BALL pivot below)
		pAIdle[s] = rowYup(legT, 101);
		pLocal[s] = qRotate(qConj(footWorldIdle[s]), pAIdle[s] - A);
		// arms
		armGeom[s].O1 = ctx.localPos(nForearm[s]);
		armGeom[s].O2 = ctx.localPos(nHand[s]);
		armGeom[s].Twist = QuatD();
		Vec HS = stIdle[nUpArm[s]].WorldPos;
		Vec W = stIdle[nHand[s]].WorldPos;
		double eHinge = rowF(armT, 0);
		armGeom[s].HingeIdle = eHinge;
		{
			QuatD wIdeal = stIdle[nUpArm[s]].WorldRot;
			Vec v = chainVec(armGeom[s], eHinge);
			Vec vHat = v / v.norm();
			Vec uHat = (W - HS); uHat.normalize();
			QuatD w0 = qBetween(vHat, uHat);
			armGeom[s].Twist = qNorm(qMul(qConj(w0), wIdeal));
		}
		armNIdle[s] = qRotate(qConj(comRotIdle), qRotate(stIdle[nUpArm[s]].WorldRot, Vec(0, 0, 1)));
		relWristIdle[s] = qRotate(qConj(comRotIdle), W - comPosIdle);
		handLocalIdle[s] = handLocalFromStored(rowQ(armT, 28), s == 1);
	}
	// BALL pivot for BOTH feet. The corpus pivot ids are consistent per foot across files
	// (piv 5 = ball, piv 2 = heel — bye legR piv5/legL piv2, victory legR piv2+5/legL piv5 at
	// the same idle pose). The idle template's R leg carries piv 5 (the push-off/landing roll
	// pivot we want); its L leg carries the HEEL pivot, whose short arm cannot lift the ankle
	// during the toe-off roll. Construct the L ball pivot as the sagittal mirror of the R ball
	// offset (world, at idle — the offset is almost purely sagittal so the mirror is nearly the
	// identity on it) and write [98] = 5 on both sides' records.
	{
		// The mirror lives in the FOOT frame (the idle stance splays the feet, so a world-plane
		// mirror lands off the foot): pLocal_L = LR-mirror of pLocal_R = (x, y, -z) under the
		// qMirrorLR convention. Verified against fy_hom_emot_victory's stored legL piv-5 pA at
		// the same idle pose (NeL (0.1378, -0.0384, -0.0005)).
		Vec ankL = stIdle[nFoot[1]].WorldPos;
		pLocal[1] = Vec(pLocal[0].x, pLocal[0].y, -pLocal[0].z);
		pAIdle[1] = ankL + qRotate(footWorldIdle[1], pLocal[1]);
		fprintf(stderr, "CAL: L ball pivot constructed pA=(%.4f,%.4f,%.4f) NeL, |arm|=%.4f (R |arm|=%.4f)\n",
		        pAIdle[1].x, pAIdle[1].y, pAIdle[1].z, pLocal[1].norm(), pLocal[0].norm());
	}

	const QuatD pelvisWorldIdle = stIdle[nPelvis].WorldRot;
	const QuatD relPelvisIdle = qMul(qConj(comRotIdle), pelvisWorldIdle);

	fprintf(stderr, "CAL: com=(%.4f,%.4f,%.4f) yawIdle=%.4f fwdW=(%.3f,%.3f,%.3f)\n",
	        comPosIdle.x, comPosIdle.y, comPosIdle.z, yawIdle, fwdW.x, fwdW.y, fwdW.z);
	for (int s = 0; s < 2; ++s)
		fprintf(stderr, "CAL: side %d ankleIdle rel=(%.4f,%.4f,%.4f) wristIdle rel=(%.4f,%.4f,%.4f) pLocal=(%.4f,%.4f,%.4f) hingeIdle leg=%.3f arm=%.3f\n",
		        s, relAnkleIdle[s] * fwdL, relAnkleIdle[s] * upL, relAnkleIdle[s] * rightL,
		        relWristIdle[s] * fwdL, relWristIdle[s] * upL, relWristIdle[s] * rightL,
		        pLocal[s].x, pLocal[s].y, pLocal[s].z, legGeom[s].HingeIdle, armGeom[s].HingeIdle);

	// --- authored COM state at any frame ------------------------------------------------------
	struct SComState
	{
		double Yaw; QuatD Rot; Vec Pos;
	};
	struct FCom
	{
		double yawIdle; QuatD comRotIdle; Vec comPosIdle; Vec fwdL, rightL, upL;
		SComState at(double f) const
		{
			SComState o;
			o.Yaw = yawIdle + chan1(kYaw, CHAN_N(kYaw), f);
			QuatD s = storedTurnFromYaw(o.Yaw);
			o.Rot = comRotFromStoredTurn(s);
			Vec d = chan3(kCom, CHAN_N(kCom), f);
			// fwd/right deltas ride the IDLE COM frame (ground-plane displacement)
			Vec disp = qRotate(comRotIdle, fwdL * d.x + rightL * d.y);
			o.Pos = Vec(comPosIdle.x + disp.x, comPosIdle.y + disp.y, d.z);
			return o;
		}
	} fCom;
	fCom.yawIdle = yawIdle; fCom.comRotIdle = comRotIdle; fCom.comPosIdle = comPosIdle;
	fCom.fwdL = fwdL; fCom.rightL = rightL; fCom.upL = upL;

	// world axes for the foot pitch (fixed idle frame; plants are world-anchored)
	Vec rightWIdle = qRotate(comRotIdle, rightL);

	// --- build the non-limb tracks ------------------------------------------------------------
	SBuiltTrack bHorizontal, bTurn, bVertical, bPelvis, bSpine, bHead, bPony;

	for (size_t k = 0; k < CHAN_N(kTimesCom); ++k)
	{
		double f = kTimesCom[k];
		sint32 t = (sint32)(f * TICKS);
		SComState cs = fCom.at(f);
		// horizontal: rec 10, pos Y-up at [0..2], [3]=1, [4]=NaN filler (template), rest template
		std::vector<uint32> h = tpl.Horizontal;
		rowSetYup(h, 0, cs.Pos);
		bHorizontal.add(t, h);
		// turn: [0]/[2] lean slots stay template(0-ish at idle), [1] angle, [4..7] quat
		std::vector<uint32> tu = tpl.Turn;
		rowSetF(tu, 0, 0.0); rowSetF(tu, 2, 0.0);
		rowSetF(tu, 1, cs.Yaw);
		rowSetQ(tu, 4, storedTurnFromYaw(cs.Yaw));
		bTurn.add(t, tu);
		// vertical: [2]=[3]=z
		std::vector<uint32> v = tpl.Vertical;
		rowSetF(v, 2, cs.Pos.z); rowSetF(v, 3, cs.Pos.z);
		bVertical.add(t, v);
	}

	for (size_t k = 0; k < CHAN_N(kTimesPelvis); ++k)
	{
		double f = kTimesPelvis[k];
		SComState cs = fCom.at(f);
		double pitch, roll;
		chan2(kPelvis, CHAN_N(kPelvis), f, pitch, roll);
		QuatD delta = qMul(qAxisAngle(rightL, pitch), qAxisAngle(fwdL, roll));
		QuatD world = qNorm(qMul(cs.Rot, qMul(delta, relPelvisIdle)));
		std::vector<uint32> r = tpl.Pelvis;
		rowSetQ(r, 0, storedFromPelvis(world, cs.Rot));
		bPelvis.add((sint32)(f * TICKS), r);
	}

	for (size_t k = 0; k < CHAN_N(kTimesSpine); ++k)
	{
		double f = kTimesSpine[k];
		Vec sag = chan3(kSpineSag, CHAN_N(kSpineSag), f);
		double tw = chan1(kSpineTwist, CHAN_N(kSpineTwist), f);
		std::vector<uint32> r = tpl.Spine;
		// [0] = int count marker (template), then 9 angles (3 per link, stored order a1 a2 a3)
		double sagd[3] = { sag.x, sag.y, sag.z };
		for (int l = 0; l < 3; ++l)
		{
			rowSetF(r, 1 + l * 3 + 0, rowF(tpl.Spine, 1 + l * 3 + 0) + sagd[l]);
			rowSetF(r, 1 + l * 3 + 1, rowF(tpl.Spine, 1 + l * 3 + 1) + tw);
		}
		bSpine.add((sint32)(f * TICKS), r);
	}

	for (size_t k = 0; k < CHAN_N(kTimesHead); ++k)
	{
		double f = kTimesHead[k];
		double nod, tilt;
		chan2(kHead, CHAN_N(kHead), f, nod, tilt);
		std::vector<uint32> r = tpl.Head;
		QuatD s = rowQ(tpl.Head, 0);
		s = qNorm(qMul(s, qMul(qAxisAngle(Vec(0, 0, 1), nod), qAxisAngle(Vec(1, 0, 0), tilt))));
		rowSetQ(r, 0, s);
		bHead.add((sint32)(f * TICKS), r);
	}

	for (size_t k = 0; k < CHAN_N(kTimesPony); ++k)
	{
		double f = kTimesPony[k];
		double p = chan1(kPony, CHAN_N(kPony), f);
		static const double linkScale[3] = { 1.0, 1.5, 2.0 };
		std::vector<uint32> r = tpl.Pony1;
		for (int l = 0; l < 3; ++l)
			rowSetF(r, 1 + l * 3 + 0, rowF(tpl.Pony1, 1 + l * 3 + 0) + p * linkScale[l]);
		bPony.add((sint32)(f * TICKS), r);
	}

	// --- pass A: attach frames at every limb key time -----------------------------------------
	// arms with authored clav deltas but idle limbs; legs idle
	std::set<double> limbFrames;
	for (size_t k = 0; k < CHAN_N(kTimesArm); ++k) limbFrames.insert(kTimesArm[k]);
	for (size_t k = 0; k < CHAN_N(kTimesLegR); ++k) limbFrames.insert(kTimesLegR[k]);
	for (size_t k = 0; k < CHAN_N(kTimesLegL); ++k) limbFrames.insert(kTimesLegL[k]);

	SBipAnimKeys keysA;
	{
		SBuiltTrack aArmR, aArmL, aLegR, aLegL;
		for (size_t k = 0; k < CHAN_N(kTimesArm); ++k)
		{
			double f = kTimesArm[k];
			double la, lb, ra, rb;
			chan4(kClav, CHAN_N(kClav), f, la, lb, ra, rb);
			std::vector<uint32> rr = tpl.ArmR, rl = tpl.ArmL;
			rowSetF(rr, 9, rowF(tpl.ArmR, 9) + ra); rowSetF(rr, 10, rowF(tpl.ArmR, 10) + rb);
			rowSetF(rl, 9, rowF(tpl.ArmL, 9) + la); rowSetF(rl, 10, rowF(tpl.ArmL, 10) + lb);
			aArmR.add((sint32)(f * TICKS), rr);
			aArmL.add((sint32)(f * TICKS), rl);
		}
		aLegR.add(0, tpl.LegR);
		aLegL.add(0, tpl.LegL);
		toKeyTrack(bHorizontal, keysA.Horizontal);
		toKeyTrack(bTurn, keysA.Turn);
		toKeyTrack(bVertical, keysA.Vertical);
		toKeyTrack(bPelvis, keysA.Pelvis);
		toKeyTrack(bSpine, keysA.Spine);
		toKeyTrack(bHead, keysA.Head);
		toKeyTrack(bPony, keysA.Pony1);
		toKeyTrack(aArmR, keysA.ArmR);
		toKeyTrack(aArmL, keysA.ArmL);
		toKeyTrack(aLegR, keysA.LegR);
		toKeyTrack(aLegL, keysA.LegL);
	}
	CBipedAnimEval evalA(ctx.Sys, *ctx.Rig, ctx.Bones, ctx.BoneOf, &keysA);
	std::map<double, SAttach> attach;
	for (std::set<double>::const_iterator it = limbFrames.begin(); it != limbFrames.end(); ++it)
	{
		std::map<INode *, SBipNodeState> st;
		evalA.evalAt(*it * TICKS, st);
		SAttach a;
		a.ComRot = st[nCom].WorldRot; a.ComPos = st[nCom].WorldPos;
		for (int s = 0; s < 2; ++s)
		{
			a.Hip[s] = st[nThigh[s]].WorldPos;
			a.Shoulder[s] = st[nUpArm[s]].WorldPos;
		}
		attach[*it] = a;
	}

	// --- legs ----------------------------------------------------------------------------------
	SBuiltTrack bLegR, bLegL;
	Vec lastPlantPA[2]; // for stale pivots on free keys
	lastPlantPA[0] = pAIdle[0]; lastPlantPA[1] = pAIdle[1];
	for (int s = 0; s < 2; ++s)
	{
		const std::vector<uint32> &tplLeg = s ? tpl.LegL : tpl.LegR;
		const double *times = s ? kTimesLegL : kTimesLegR;
		size_t nTimes = s ? CHAN_N(kTimesLegL) : CHAN_N(kTimesLegR);
		const SK1 *pitchCh = s ? kFootPitchL : kFootPitchR;
		size_t pitchN = s ? CHAN_N(kFootPitchL) : CHAN_N(kFootPitchR);
		const SK1 *toeCh = s ? kToeL : kToeR;
		size_t toeN = s ? CHAN_N(kToeL) : CHAN_N(kToeR);
		const SK3 *ankCh = s ? kAnkleL : kAnkleR;
		size_t ankN = s ? CHAN_N(kAnkleL) : CHAN_N(kAnkleR);
		const SK1 *swCh = s ? kSwivelLegL : kSwivelLegR;
		size_t swN = s ? CHAN_N(kSwivelLegL) : CHAN_N(kSwivelLegR);
		double toeOff = s ? TOEOFF_L : TOEOFF_R;
		double touch = s ? TOUCH_L : TOUCH_R;
		SBuiltTrack &out = s ? bLegL : bLegR;

		bool prevPlanted = false;
		for (size_t k = 0; k < nTimes; ++k)
		{
			double f = times[k];
			const SAttach &a = attach[f];
			bool planted = (f <= toeOff) || (f >= touch);
			double pitch = chan1(pitchCh, pitchN, f);
			double toe = chan1(toeCh, toeN, f);
			double swivel = chan1(swCh, swN, f);

			// foot world rotation: pitch about the idle right axis, world-anchored. The tables
			// hold heel-up-positive; rotation about +right takes fwd toward up (toes up), so the
			// heel-up roll over the ball pivot is the NEGATIVE angle (pinned by the semantic
			// report: the first run put the toe above the ankle at push-off).
			QuatD footRot = qNorm(qMul(qAxisAngle(rightWIdle, -pitch), footWorldIdle[s]));
			Vec ankleT;
			if (planted)
				ankleT = pAIdle[s] - qRotate(footRot, pLocal[s]);
			else
			{
				Vec d = chan3(ankCh, ankN, f);
				Vec rel = relAnkleIdle[s] + fwdL * d.x + upL * d.y + rightL * d.z;
				ankleT = a.ComPos + qRotate(a.ComRot, rel);
				// free foot rides the body's turn delta
				QuatD ride = qMul(a.ComRot, qConj(comRotIdle));
				footRot = qNorm(qMul(ride, qMul(qAxisAngle(rightWIdle, -pitch), footWorldIdle[s])));
			}

			// knee plane normal, COM-carried + swivel about the chain axis
			Vec n = qRotate(a.ComRot, legNIdle[s]);
			if (swivel != 0.0)
			{
				Vec axis = ankleT - a.Hip[s];
				double an = axis.norm();
				if (an > 1e-9) n = qRotate(qAxisAngle(axis / an, swivel), n);
			}

			QuatD thighW; double hinge;
			solveLimb(legGeom[s], a.Hip[s], ankleT, n, thighW, hinge);

			std::vector<uint32> r = tplLeg;
			rowSetF(r, 0, hinge);
			rowSetQ(r, 2, storedFromThigh(thighW, a.ComRot, s == 1));
			rowSetI(r, 11, planted ? 2 : 0);
			rowSetF(r, 12, planted ? 1.0 : 0.0);
			rowSetF(r, 25, (planted && prevPlanted) ? 1.0 : 0.0);
			rowSetYup(r, 18, ankleT);
			rowSetF(r, 21, 1.0);
			rowSetQ(r, 28, storedFromFoot(footRot, a.ComRot));
			// toe base: conj(s') = conj(s)·Rz(delta)  ->  s' = Rz(-delta)·s
			{
				QuatD toeIdle = rowQ(tplLeg, 46);
				rowSetQ(r, 46, qNorm(qMul(qAxisAngle(Vec(0, 0, 1), -toe), toeIdle)));
			}
			// pivots: ball pivot (id 5) on both sides — see the calibration note
			rowSetI(r, 98, 5);
			Vec pa = planted ? pAIdle[s] : lastPlantPA[s];
			rowSetYup(r, 101, pa); rowSetF(r, 104, 1.0);
			rowSetYup(r, 105, pa); rowSetF(r, 108, 1.0);
			rowSetF(r, 100, 0.0);
			if (planted) lastPlantPA[s] = pa;
			out.add((sint32)(f * TICKS), r);
			prevPlanted = planted;
		}
	}

	// --- arms ----------------------------------------------------------------------------------
	SBuiltTrack bArmR, bArmL;
	for (int s = 0; s < 2; ++s)
	{
		const std::vector<uint32> &tplArm = s ? tpl.ArmL : tpl.ArmR;
		const SK3 *wrCh = s ? kWristL : kWristR;
		size_t wrN = s ? CHAN_N(kWristL) : CHAN_N(kWristR);
		const SK1 *swCh = s ? kSwivelArmL : kSwivelArmR;
		size_t swN = s ? CHAN_N(kSwivelArmL) : CHAN_N(kSwivelArmR);
		const SK1 *hfCh = s ? kHandL : kHandR;
		size_t hfN = s ? CHAN_N(kHandL) : CHAN_N(kHandR);
		const SK1 *cuCh = s ? kCurlL : kCurlR;
		size_t cuN = s ? CHAN_N(kCurlL) : CHAN_N(kCurlR);
		SBuiltTrack &out = s ? bArmL : bArmR;

		for (size_t k = 0; k < CHAN_N(kTimesArm); ++k)
		{
			double f = kTimesArm[k];
			const SAttach &a = attach[f];
			Vec d = chan3(wrCh, wrN, f);
			double swivel = chan1(swCh, swN, f);
			double flex = chan1(hfCh, hfN, f);
			double curl = chan1(cuCh, cuN, f);
			double la, lb, ra, rb;
			chan4(kClav, CHAN_N(kClav), f, la, lb, ra, rb);

			Vec rel = relWristIdle[s] + fwdL * d.x + upL * d.y + rightL * d.z;
			Vec wristT = a.ComPos + qRotate(a.ComRot, rel);
			Vec n = qRotate(a.ComRot, armNIdle[s]);
			if (swivel != 0.0)
			{
				Vec axis = wristT - a.Shoulder[s];
				double an = axis.norm();
				if (an > 1e-9) n = qRotate(qAxisAngle(axis / an, swivel), n);
			}
			QuatD upW; double hinge;
			solveLimb(armGeom[s], a.Shoulder[s], wristT, n, upW, hinge);

			std::vector<uint32> r = tplArm;
			rowSetF(r, 0, hinge);
			rowSetQ(r, 2, storedFromUpperArm(upW, a.ComRot, s == 1));
			rowSetF(r, 9, rowF(tplArm, 9) + (s ? la : ra));
			rowSetF(r, 10, rowF(tplArm, 10) + (s ? lb : rb));
			// hand local flex
			QuatD handLocal = qMul(handLocalIdle[s], qAxisAngle(Vec(0, 0, 1), flex));
			rowSetQ(r, 28, storedFromHandLocal(handLocal, s == 1));
			// world wrist (informational slot; arms are never pinned here)
			rowSetYup(r, 18, wristT);
			rowSetF(r, 21, 1.0);
			// finger curls: bends are absolute angles at [54+10k],[55+10k]; thumb (k=0) at half
			for (int fg = 0; fg < 5; ++fg)
			{
				double c = curl * (fg == 0 ? 0.5 : 1.0);
				double b1 = rowF(tplArm, 54 + 10 * fg) + c;
				double b2 = rowF(tplArm, 55 + 10 * fg) + c;
				if (b1 < 0.0) b1 = 0.0;
				if (b2 < 0.0) b2 = 0.0;
				rowSetF(r, 54 + 10 * fg, b1);
				rowSetF(r, 55 + 10 * fg, b2);
			}
			out.add((sint32)(f * TICKS), r);
		}
	}

	// sign-continuity across keys for every stored quat field (see SBuiltTrack::signChain)
	bTurn.signChain(4);
	bPelvis.signChain(0);
	bHead.signChain(0);
	for (int s = 0; s < 2; ++s)
	{
		SBuiltTrack &leg = s ? bLegL : bLegR;
		leg.signChain(2); leg.signChain(28); leg.signChain(46);
		SBuiltTrack &arm = s ? bArmL : bArmR;
		arm.signChain(2); arm.signChain(28);
		for (int fg = 0; fg < 5; ++fg) arm.signChain(46 + 10 * fg);
	}

	// --- pass B: verification eval -------------------------------------------------------------
	SBipAnimKeys keysB;
	toKeyTrack(bHorizontal, keysB.Horizontal);
	toKeyTrack(bTurn, keysB.Turn);
	toKeyTrack(bVertical, keysB.Vertical);
	toKeyTrack(bPelvis, keysB.Pelvis);
	toKeyTrack(bSpine, keysB.Spine);
	toKeyTrack(bHead, keysB.Head);
	toKeyTrack(bPony, keysB.Pony1);
	toKeyTrack(bArmR, keysB.ArmR);
	toKeyTrack(bArmL, keysB.ArmL);
	toKeyTrack(bLegR, keysB.LegR);
	toKeyTrack(bLegL, keysB.LegL);
	CBipedAnimEval evalB(ctx.Sys, *ctx.Rig, ctx.Bones, ctx.BoneOf, &keysB);

	double worstAnkle = 0.0, worstPlant = 0.0;
	for (int s = 0; s < 2; ++s)
	{
		const double *times = s ? kTimesLegL : kTimesLegR;
		size_t nTimes = s ? CHAN_N(kTimesLegL) : CHAN_N(kTimesLegR);
		double toeOff = s ? TOEOFF_L : TOEOFF_R;
		double touch = s ? TOUCH_L : TOUCH_R;
		for (size_t k = 0; k < nTimes; ++k)
		{
			double f = times[k];
			std::map<INode *, SBipNodeState> st;
			evalB.evalAt(f * TICKS, st);
			bool planted = (f <= toeOff) || (f >= touch);
			if (planted)
			{
				Vec pivot = st[nFoot[s]].WorldPos + qRotate(st[nFoot[s]].WorldRot, pLocal[s]);
				double dv = (pivot - pAIdle[s]).norm();
				if (dv > worstPlant) worstPlant = dv;
				if (dv > 0.002)
				{
					double pitch = chan1(s ? kFootPitchL : kFootPitchR, s ? CHAN_N(kFootPitchL) : CHAN_N(kFootPitchR), f);
					QuatD wantRot = qNorm(qMul(qAxisAngle(rightWIdle, -pitch), footWorldIdle[s]));
					Vec wantAnk = pAIdle[s] - qRotate(wantRot, pLocal[s]);
					Vec gotAnk = st[nFoot[s]].WorldPos;
					QuatD dq = qMul(qConj(wantRot), st[nFoot[s]].WorldRot);
					double rotErr = 2.0 * acos(std::min(1.0, fabs(dq.w)));
					fprintf(stderr, "  PLANT-MISS side=%d f=%g drift=%.4f ankMiss=%.4f rotErr=%.4f (hip-dist %.4f reach %.4f)\n",
					        s, f, dv, (gotAnk - wantAnk).norm(), rotErr,
					        (wantAnk - st[nThigh[s]].WorldPos).norm(),
					        legGeom[s].O1.norm() + legGeom[s].O2.norm());
				}
			}
		}
	}
	// semantic report at landmark frames
	{
		static const double marks[] = { 0, 14, 17, 26, 32, 36, 56 };
		for (size_t i = 0; i < sizeof(marks) / sizeof(marks[0]); ++i)
		{
			double f = marks[i];
			std::map<INode *, SBipNodeState> st;
			evalB.evalAt(f * TICKS, st);
			Vec com = st[nCom].WorldPos;
			Vec headRel = qRotate(qConj(comRotIdle), st[nHead].WorldPos - com);
			Vec wl = qRotate(qConj(st[nCom].WorldRot), st[nHand[1]].WorldPos - com);
			Vec al = qRotate(qConj(st[nCom].WorldRot), st[nFoot[0]].WorldPos - com);
			Vec toeR = st[nToe[0]].WorldPos;
			Vec ankR = st[nFoot[0]].WorldPos;
			fprintf(stderr, "SEM f=%g comZ=%.4f headRel(fwd,up)=(%.3f,%.3f) wristL(fwd,up,right)=(%.3f,%.3f,%.3f) ankleR(fwd,up)=(%.3f,%.3f) toeR z=%.4f ankR z=%.4f\n",
			        f, com.z, headRel * fwdL, headRel * upL,
			        wl * fwdL, wl * upL, wl * rightL, al * fwdL, al * upL, toeR.z, ankR.z);
		}
	}
	// end-effector-at-target check: recompute targets and compare
	for (int s = 0; s < 2; ++s)
	{
		for (size_t k = 0; k < CHAN_N(kTimesArm); ++k)
		{
			double f = kTimesArm[k];
			std::map<INode *, SBipNodeState> st;
			evalB.evalAt(f * TICKS, st);
			const SAttach &a = attach[f];
			const SK3 *wrCh = s ? kWristL : kWristR;
			size_t wrN = s ? CHAN_N(kWristL) : CHAN_N(kWristR);
			Vec d = chan3(wrCh, wrN, f);
			Vec rel = relWristIdle[s] + fwdL * d.x + upL * d.y + rightL * d.z;
			Vec wristT = a.ComPos + qRotate(a.ComRot, rel);
			double dv = (st[nHand[s]].WorldPos - wristT).norm();
			// reach-clamped keys legitimately miss the raw target; report only surprises
			double reach = (armGeom[s].O1.norm() + armGeom[s].O2.norm()) * 0.999;
			double want = (wristT - a.Shoulder[s]).norm();
			if (want < reach && dv > worstAnkle) worstAnkle = dv;
		}
	}
	fprintf(stderr, "VERIFY(mem): worst plant-pivot drift %.6f m, worst in-reach end-effector miss %.6f m\n",
	        worstPlant, worstAnkle);

	// quarter-frame sweep: plant hold between keys (through the exporter's pivot model) and a
	// pop detector (max per-quarter-frame displacement of the COM and end effectors)
	{
		double worstHold[2] = { 0.0, 0.0 };
		double worstStep = 0.0, worstStepT = 0.0;
		Vec prevCom, prevHand[2], prevFoot[2];
		bool have = false;
		for (double f = 0.0; f <= LAST_FRAME + 1e-9; f += 0.25)
		{
			std::map<INode *, SBipNodeState> st;
			evalB.evalAt(f * TICKS, st);
			for (int s = 0; s < 2; ++s)
			{
				double toeOff = s ? TOEOFF_L : TOEOFF_R;
				double touch = s ? TOUCH_L : TOUCH_R;
				if (f <= toeOff || f >= touch)
				{
					Vec pivot = st[nFoot[s]].WorldPos + qRotate(st[nFoot[s]].WorldRot, pLocal[s]);
					double dv = (pivot - pAIdle[s]).norm();
					if (dv > worstHold[s]) worstHold[s] = dv;
				}
			}
			if (have)
			{
				double step = (st[nCom].WorldPos - prevCom).norm();
				const char *who = "com";
				for (int s = 0; s < 2; ++s)
				{
					double d1 = (st[nHand[s]].WorldPos - prevHand[s]).norm();
					double d2 = (st[nFoot[s]].WorldPos - prevFoot[s]).norm();
					if (d1 > step) { step = d1; who = s ? "handL" : "handR"; }
					if (d2 > step) { step = d2; who = s ? "footL" : "footR"; }
				}
				if (step > worstStep) { worstStep = step; worstStepT = f; }
				if (step > 0.10)
					fprintf(stderr, "  POP f=%.2f %s step=%.4f\n", f, who, step);
			}
			prevCom = st[nCom].WorldPos;
			for (int s = 0; s < 2; ++s) { prevHand[s] = st[nHand[s]].WorldPos; prevFoot[s] = st[nFoot[s]].WorldPos; }
			have = true;
		}
		fprintf(stderr, "VERIFY(mem): in-between plant hold R %.4f m / L %.4f m; max quarter-frame step %.4f m at f=%.2f\n",
		        worstHold[0], worstHold[1], worstStep, worstStepT);
	}

	// boundary keys must equal the idle template pose exactly (records are template-copied at
	// f=0/f=56 only if every authored delta is zero there — verify via eval)
	{
		std::map<INode *, SBipNodeState> st0, st56, sti;
		evalB.evalAt(0.0, st0);
		evalB.evalAt(LAST_FRAME * TICKS, st56);
		evalIdle.evalAt(0.0, sti);
		double worst = 0.0;
		for (std::map<INode *, SBipNodeState>::iterator it = sti.begin(); it != sti.end(); ++it)
		{
			std::map<INode *, SBipNodeState>::iterator a = st0.find(it->first), b = st56.find(it->first);
			if (a == st0.end() || b == st56.end()) continue;
			double d1 = (a->second.WorldPos - it->second.WorldPos).norm();
			double d2 = (b->second.WorldPos - it->second.WorldPos).norm();
			if (d1 > worst) worst = d1;
			if (d2 > worst) worst = d2;
		}
		fprintf(stderr, "VERIFY(mem): boundary-vs-idle worst node position delta %.6f m\n", worst);
	}

	// --- write ----------------------------------------------------------------------------------
	// Write through a FRESHLY loaded scene: the walked/evaluated instance has read-side parse
	// state (node AppData parsed on demand during the rig walk) that the clean/build cycle
	// rejects; the fresh instance follows exactly the corpus-proven parse->edit->build path.
	{
		SLoadedMax wr;
		if (!loadMax(skelMax, &reg, wr, true)) return 1;
		CBipedSystem *wrSys = findBipedSystem(wr.Scene->container());
		if (!wrSys) { std::cerr << "ERROR: no typed Biped system in the write scene\n"; return 1; }
		// Chunk 0x0109 = the Figure Mode flag: 1 on the skeleton source, 0 on every corpus
		// animation file, and the SINGLE-VARIABLE isolation diff (fy_hom_skel vs the animode
		// probe's b00_baseline, whose only change is the figureMode=false commit) moves exactly
		// this chunk 1 -> 0 plus the §10n current-position/shadow-bank caches. Clear it so the
		// authored file opens in Animation Mode. (This corrects §10p's earlier 0x0109 = twist
		// attribution — that probe case toggled modes as a side effect.) The commit's cache
		// side-effect chunks are left as-is: they only drive UNKEYED channels (§10n/§10o) and
		// every channel here is keyed.
		{
			IStorageObject *fm = PMAX_RIG::findChunkAnywhere(wrSys, 0x0109);
			CStorageRaw *fmRaw = dynamic_cast<CStorageRaw *>(fm);
			if (fmRaw && fmRaw->Value.size() == 4)
			{
				fmRaw->Value[0] = 0; fmRaw->Value[1] = 0; fmRaw->Value[2] = 0; fmRaw->Value[3] = 0;
				fprintf(stderr, "FIGMODE: cleared 0x0109 (Figure Mode) in the output\n");
			}
			else
				fprintf(stderr, "FIGMODE: WARNING 0x0109 not found/unexpected size — output may open in Figure Mode\n");
		}
		storeTrack(wrSys, CBipedAnimTrack::TrackHorizontal, bHorizontal, false);
		storeTrack(wrSys, CBipedAnimTrack::TrackTurn, bTurn, false);
		storeTrack(wrSys, CBipedAnimTrack::TrackVertical, bVertical, true);
		storeTrack(wrSys, CBipedAnimTrack::TrackPelvis, bPelvis, false);
		storeTrack(wrSys, CBipedAnimTrack::TrackArmR, bArmR, false);
		storeTrack(wrSys, CBipedAnimTrack::TrackArmL, bArmL, false);
		storeTrack(wrSys, CBipedAnimTrack::TrackLegR, bLegR, false);
		storeTrack(wrSys, CBipedAnimTrack::TrackLegL, bLegL, false);
		storeTrack(wrSys, CBipedAnimTrack::TrackSpine, bSpine, false);
		storeTrack(wrSys, CBipedAnimTrack::TrackHead, bHead, false);
		storeTrack(wrSys, CBipedAnimTrack::TrackPony1, bPony, false);

		wr.Scene->clean();
		wr.Scene->build(VersionUnknown);
		wr.Scene->disown();
		std::string tempPath = std::string(outMax) + ".scene.tmp";
		std::vector<uint8> sceneBytes;
		{
			{
				NLMISC::COFile of(tempPath);
				wr.Scene->serial(of, 0);
			}
			std::ifstream ifs(tempPath.c_str(), std::ios::binary);
			ifs.seekg(0, std::ios::end);
			std::streampos end = ifs.tellg();
			ifs.seekg(0);
			sceneBytes.resize((size_t)end);
			if ((size_t)end) ifs.read((char *)nlVectorData(sceneBytes), (std::streamsize)end);
		}
		remove(tempPath.c_str());
		CStorageOleOut oleOut;
		for (size_t i = 0; i < wr.StreamNames.size(); ++i)
		{
			if (wr.StreamNames[i] == "Scene") oleOut.addStream("Scene", sceneBytes);
			else oleOut.addStream(wr.StreamNames[i], wr.StreamBytes[i]);
		}
		if (wr.HaveClassId) oleOut.setClassId(wr.OleClassId);
		if (!oleOut.write(outMax)) { std::cerr << "ERROR: cannot write " << outMax << "\n"; return 1; }
	}
	fprintf(stderr, "WROTE %s\n", outMax);

	// --- reload + final verification -------------------------------------------------------------
	{
		SLoadedMax check;
		if (!loadMax(outMax, &reg, check, false)) return 1;
		SRigCtx cctx;
		if (!buildRigCtx(check.Scene, cctx)) return 1;
		CBipedAnimEval evalF(cctx.Sys, *cctx.Rig, cctx.Bones, cctx.BoneOf); // parse from the file
		if (!evalF.keys().HasRange)
		{
			std::cerr << "ERROR: reloaded file has no key range\n";
			return 1;
		}
		fprintf(stderr, "RELOAD: key range [%d..%d] ticks (%g..%g frames)\n",
		        evalF.keys().RangeMin, evalF.keys().RangeMax,
		        evalF.keys().RangeMin / TICKS, evalF.keys().RangeMax / TICKS);
		// cross-check a few frames against the in-memory pass-B eval
		INode *cCom = cctx.Com;
		INode *cHandL = cctx.node(BID_LARM, 3);
		INode *cFootR = cctx.node(BID_RLEG, 2);
		double worst = 0.0;
		static const double marks[] = { 0, 10, 17, 24, 26, 32, 36, 44, 56 };
		for (size_t i = 0; i < sizeof(marks) / sizeof(marks[0]); ++i)
		{
			std::map<INode *, SBipNodeState> stf, stb;
			evalF.evalAt(marks[i] * TICKS, stf);
			evalB.evalAt(marks[i] * TICKS, stb);
			double d1 = (stf[cCom].WorldPos - stb[nCom].WorldPos).norm();
			double d2 = (stf[cHandL].WorldPos - stb[nHand[1]].WorldPos).norm();
			double d3 = (stf[cFootR].WorldPos - stb[nFoot[0]].WorldPos).norm();
			if (d1 > worst) worst = d1;
			if (d2 > worst) worst = d2;
			if (d3 > worst) worst = d3;
		}
		fprintf(stderr, "VERIFY(file): reloaded-vs-memory worst delta %.9f m\n", worst);
	}
	return 0;
}

} /* namespace BIPAUTHOR */

/* end of file */
