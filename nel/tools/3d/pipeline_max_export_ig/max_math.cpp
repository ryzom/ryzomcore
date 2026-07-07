/**
 * \file max_math.cpp
 * \brief See max_math.h. The decomposition is a faithful float port of the Graphics Gems IV
 * "Polar Matrix Decomposition" reference code (Ken Shoemake), which is what the SDK's
 * decomp_affine wraps; the gems code carries no license restrictions ("free to reuse").
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
#include "max_math.h"

#include <cmath>
#include <cstring>

namespace MAXMATH {

Matrix3M Matrix3M::identity()
{
	Matrix3M r;
	memset(r.m, 0, sizeof(r.m));
	r.m[0][0] = r.m[1][1] = r.m[2][2] = 1.0f;
	return r;
}

Matrix3M operator*(const Matrix3M &a, const Matrix3M &b)
{
	// Max Matrix3 operator*: (a*b) applied to row vector v is (v*a)*b... The SDK stores the
	// product so that rows of the result = a.row[i] transformed by b; translation row picks up
	// b's translation.
	Matrix3M r;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			r.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j];
		}
	}
	r.m[3][0] += b.m[3][0];
	r.m[3][1] += b.m[3][1];
	r.m[3][2] += b.m[3][2];
	return r;
}

Matrix3M inverseM3(const Matrix3M &a)
{
	// Max Matrix3 Inverse: adjoint/determinant on the 3x3, translation = -T * inv3x3.
	Matrix3M r;
	r.m[0][0] = a.m[1][1] * a.m[2][2] - a.m[1][2] * a.m[2][1];
	r.m[0][1] = a.m[0][2] * a.m[2][1] - a.m[0][1] * a.m[2][2];
	r.m[0][2] = a.m[0][1] * a.m[1][2] - a.m[0][2] * a.m[1][1];
	r.m[1][0] = a.m[1][2] * a.m[2][0] - a.m[1][0] * a.m[2][2];
	r.m[1][1] = a.m[0][0] * a.m[2][2] - a.m[0][2] * a.m[2][0];
	r.m[1][2] = a.m[0][2] * a.m[1][0] - a.m[0][0] * a.m[1][2];
	r.m[2][0] = a.m[1][0] * a.m[2][1] - a.m[1][1] * a.m[2][0];
	r.m[2][1] = a.m[0][1] * a.m[2][0] - a.m[0][0] * a.m[2][1];
	r.m[2][2] = a.m[0][0] * a.m[1][1] - a.m[0][1] * a.m[1][0];
	float det = a.m[0][0] * r.m[0][0] + a.m[0][1] * r.m[1][0] + a.m[0][2] * r.m[2][0];
	float ooDet = 1.0f / det;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			r.m[i][j] *= ooDet;
	// translation: -T * inv3x3
	r.m[3][0] = -(a.m[3][0] * r.m[0][0] + a.m[3][1] * r.m[1][0] + a.m[3][2] * r.m[2][0]);
	r.m[3][1] = -(a.m[3][0] * r.m[0][1] + a.m[3][1] * r.m[1][1] + a.m[3][2] * r.m[2][1]);
	r.m[3][2] = -(a.m[3][0] * r.m[0][2] + a.m[3][1] * r.m[1][2] + a.m[3][2] * r.m[2][2]);
	return r;
}

Matrix3M quatToMatrix3(const QuatM &q)
{
	// Max Quat::MakeMatrix. Max quats follow the LEFT-handed/inverse convention relative to the
	// standard quat-rotation mapping (the corpus-established "rotation controllers store the
	// inverse convention"): the row matrix built from a stored controller quat is the transpose
	// of the standard R(q) — validated against the reference igs (the untransposed variant
	// produced conjugated instance rotations corpus-wide).
	Matrix3M r = Matrix3M::identity();
	float x = q.x, y = q.y, z = q.z, w = q.w;
	float xx = x * x, yy = y * y, zz = z * z;
	float xy = x * y, xz = x * z, yz = y * z;
	float wx = w * x, wy = w * y, wz = w * z;
	r.m[0][0] = 1.0f - 2.0f * (yy + zz); r.m[0][1] = 2.0f * (xy - wz);        r.m[0][2] = 2.0f * (xz + wy);
	r.m[1][0] = 2.0f * (xy + wz);        r.m[1][1] = 1.0f - 2.0f * (xx + zz); r.m[1][2] = 2.0f * (yz - wx);
	r.m[2][0] = 2.0f * (xz - wy);        r.m[2][1] = 2.0f * (yz + wx);        r.m[2][2] = 1.0f - 2.0f * (xx + yy);
	r.m[3][0] = r.m[3][1] = r.m[3][2] = 0.0f;
	return r;
}

Matrix3M composePRS(const Point3M &pos, const QuatM &rot, const ScaleValueM &scale)
{
	// M = R * S * T in this row-vector convention: the scale is applied in the PARENT frame
	// (after the rotation), which is what the polar stretch of the reference node matrices
	// shows — a rotated node with local scale (1.03,1,1) decomposes to Scale (1,1.03,1) in the
	// reference igs (the fy_asc_1porte doors). The rotation row block times the scale matrix.
	Matrix3M r = quatToMatrix3(rot);

	Matrix3M s = Matrix3M::identity();
	bool qIdentity = (scale.q.x == 0.0f && scale.q.y == 0.0f && scale.q.z == 0.0f);
	if (qIdentity)
	{
		s.m[0][0] = scale.s.x;
		s.m[1][1] = scale.s.y;
		s.m[2][2] = scale.s.z;
	}
	else
	{
		// Axis-system scale: stretch along the axis frame's axes.
		Matrix3M srtm = quatToMatrix3(scale.q);
		Matrix3M stm = Matrix3M::identity();
		stm.m[0][0] = scale.s.x;
		stm.m[1][1] = scale.s.y;
		stm.m[2][2] = scale.s.z;
		s = inverseM3(srtm) * stm * srtm;
	}

	Matrix3M m = r * s;
	m.m[3][0] = pos.x;
	m.m[3][1] = pos.y;
	m.m[3][2] = pos.z;
	return m;
}

Point3M transformPoint(const Point3M &v, const Matrix3M &m)
{
	// Max Point3 * Matrix3: per component x*m[0][j] + y*m[1][j] + z*m[2][j] + m[3][j].
	Point3M r;
	r.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
	r.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
	r.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];
	return r;
}

// ---------------------------------------------------------------------------------------------
// Graphics Gems IV decomp_affine port. HMatrix is the gems 4x4 float, column-vector convention
// (translation in column W). The SDK adapter copies the Matrix3 rows straight into the HMatrix
// rows (so the gems code effectively decomposes the TRANSPOSE of the row-vector matrix, which
// is the same matrix in column convention) and reads the translation from the Matrix3 row 3.

typedef float HMatrix[4][4];

struct QuatG
{
	float x, y, z, w;
};

enum { QX = 0, QY = 1, QZ = 2, QW = 3 };

static QuatG qtMake(float x, float y, float z, float w)
{
	QuatG q;
	q.x = x; q.y = y; q.z = z; q.w = w;
	return q;
}

static QuatG qtConj(const QuatG &q)
{
	return qtMake(-q.x, -q.y, -q.z, q.w);
}

static QuatG qtMul(const QuatG &qL, const QuatG &qR)
{
	QuatG qq;
	qq.w = qL.w * qR.w - qL.x * qR.x - qL.y * qR.y - qL.z * qR.z;
	qq.x = qL.w * qR.x + qL.x * qR.w + qL.y * qR.z - qL.z * qR.y;
	qq.y = qL.w * qR.y + qL.y * qR.w + qL.z * qR.x - qL.x * qR.z;
	qq.z = qL.w * qR.z + qL.z * qR.w + qL.x * qR.y - qL.y * qR.x;
	return qq;
}

static QuatG qtScale(const QuatG &q, float w)
{
	return qtMake(q.x * w, q.y * w, q.z * w, q.w * w);
}

static float matNorm(const HMatrix M, int tpose)
{
	float sum, max = 0.0f;
	for (int i = 0; i < 3; ++i)
	{
		if (tpose) sum = (float)(fabs(M[0][i]) + fabs(M[1][i]) + fabs(M[2][i]));
		else sum = (float)(fabs(M[i][0]) + fabs(M[i][1]) + fabs(M[i][2]));
		if (max < sum) max = sum;
	}
	return max;
}

static float normInf(const HMatrix M) { return matNorm(M, 0); }
static float normOne(const HMatrix M) { return matNorm(M, 1); }

static int findMaxCol(const HMatrix M)
{
	float abs, max = 0.0f;
	int col = -1;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
		{
			abs = M[i][j];
			if (abs < 0.0f) abs = -abs;
			if (abs > max) { max = abs; col = j; }
		}
	return col;
}

static void makeReflector(const float *v, float *u)
{
	float s = (float)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	u[0] = v[0];
	u[1] = v[1];
	u[2] = v[2] + ((v[2] < 0.0f) ? -s : s);
	s = (float)sqrt(2.0f / (u[0] * u[0] + u[1] * u[1] + u[2] * u[2]));
	u[0] = u[0] * s;
	u[1] = u[1] * s;
	u[2] = u[2] * s;
}

static void reflectCols(HMatrix M, const float *u)
{
	for (int i = 0; i < 3; ++i)
	{
		float s = u[0] * M[0][i] + u[1] * M[1][i] + u[2] * M[2][i];
		for (int j = 0; j < 3; ++j)
			M[j][i] -= u[j] * s;
	}
}

static void reflectRows(HMatrix M, const float *u)
{
	for (int i = 0; i < 3; ++i)
	{
		float s = u[0] * M[i][0] + u[1] * M[i][1] + u[2] * M[i][2];
		for (int j = 0; j < 3; ++j)
			M[i][j] -= u[j] * s;
	}
}

static void vcross(const float *va, const float *vb, float *v)
{
	v[0] = va[1] * vb[2] - va[2] * vb[1];
	v[1] = va[2] * vb[0] - va[0] * vb[2];
	v[2] = va[0] * vb[1] - va[1] * vb[0];
}

static float vdot(const float *va, const float *vb)
{
	return va[0] * vb[0] + va[1] * vb[1] + va[2] * vb[2];
}

static void adjointTranspose(const HMatrix M, HMatrix MadjT)
{
	vcross(M[1], M[2], MadjT[0]);
	vcross(M[2], M[0], MadjT[1]);
	vcross(M[0], M[1], MadjT[2]);
}

static void matTpose(HMatrix AT, const HMatrix A)
{
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			AT[i][j] = A[j][i];
}

static void matCopyEq(HMatrix C, const HMatrix A)
{
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			C[i][j] = A[i][j];
}

static void matPad(HMatrix A)
{
	A[QW][QX] = A[QX][QW] = A[QW][QY] = A[QY][QW] = A[QW][QZ] = A[QZ][QW] = 0.0f;
	A[QW][QW] = 1.0f;
}

static void matMultEq(const HMatrix A, const HMatrix B, HMatrix AB)
{
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			AB[i][j] = A[i][0] * B[0][j] + A[i][1] * B[1][j] + A[i][2] * B[2][j];
}

// Rank-deficient special cases of the polar decomposition.
static void doRank1(HMatrix M, HMatrix Q)
{
	float v1[3], v2[3], s;
	int col;
	// Q = identity
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			Q[i][j] = (i == j) ? 1.0f : 0.0f;
	// If rank(M) is 1, we should find a non-zero column in M
	col = findMaxCol(M);
	if (col < 0) return; // rank 0
	v1[0] = M[0][col]; v1[1] = M[1][col]; v1[2] = M[2][col];
	makeReflector(v1, v1); reflectCols(M, v1);
	v2[0] = M[2][0]; v2[1] = M[2][1]; v2[2] = M[2][2];
	makeReflector(v2, v2); reflectRows(M, v2);
	s = M[2][2];
	if (s < 0.0f) Q[2][2] = -1.0f;
	reflectCols(Q, v1); reflectRows(Q, v2);
}

static void doRank2(HMatrix M, const HMatrix MadjT, HMatrix Q)
{
	float v1[3], v2[3];
	float w, x, y, z, c, s, d;
	int col;
	// If rank(M) is 2, we should find a non-zero column in MadjT
	col = findMaxCol(MadjT);
	if (col < 0) { doRank1(M, Q); return; } // rank < 2
	v1[0] = MadjT[0][col]; v1[1] = MadjT[1][col]; v1[2] = MadjT[2][col];
	makeReflector(v1, v1); reflectCols(M, v1);
	vcross(M[0], M[1], v2);
	makeReflector(v2, v2); reflectRows(M, v2);
	w = M[0][0]; x = M[0][1]; y = M[1][0]; z = M[1][1];
	if (w * z > x * y)
	{
		c = z + w; s = y - x; d = (float)sqrt(c * c + s * s); c = c / d; s = s / d;
		Q[0][0] = Q[1][1] = c; Q[0][1] = -s; Q[1][0] = s;
	}
	else
	{
		c = z - w; s = y + x; d = (float)sqrt(c * c + s * s); c = c / d; s = s / d;
		Q[0][0] = c; Q[1][1] = -c; Q[0][1] = Q[1][0] = s;
	}
	Q[0][2] = Q[2][0] = Q[1][2] = Q[2][1] = 0.0f; Q[2][2] = 1.0f;
	reflectCols(Q, v1); reflectRows(Q, v2);
}

// Polar decomposition of M into Q*S, returning det(Q's pre-normalized form).
static float polarDecomp(const HMatrix M, HMatrix Q, HMatrix S)
{
	const float TOL = 1.0e-6f;
	HMatrix Mk, MadjTk, Ek;
	float det, M_one, M_inf, MadjT_one, MadjT_inf, E_one, gamma, g1, g2;
	matTpose(Mk, M);
	M_one = normOne(Mk);
	M_inf = normInf(Mk);
	do
	{
		adjointTranspose(Mk, MadjTk);
		det = vdot(Mk[0], MadjTk[0]);
		if (det == 0.0f) { doRank2(Mk, MadjTk, Mk); break; }
		MadjT_one = normOne(MadjTk);
		MadjT_inf = normInf(MadjTk);
		gamma = (float)sqrt(sqrt((MadjT_one * MadjT_inf) / (M_one * M_inf)) / fabs(det));
		g1 = gamma * 0.5f;
		g2 = 0.5f / (gamma * det);
		matCopyEq(Ek, Mk);
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				Mk[i][j] = g1 * Mk[i][j] + g2 * MadjTk[i][j];
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				Ek[i][j] = Ek[i][j] - Mk[i][j];
		E_one = normOne(Ek);
		M_one = normOne(Mk);
		M_inf = normInf(Mk);
	} while (E_one > (M_one * TOL));
	matTpose(Q, Mk); matPad(Q);
	matMultEq(Mk, M, S); matPad(S);
	for (int i = 0; i < 3; ++i)
		for (int j = i; j < 3; ++j)
			S[i][j] = S[j][i] = 0.5f * (S[i][j] + S[j][i]);
	return det;
}

// Quaternion from rotation matrix (gems Qt_FromMatrix, column-vector convention).
static QuatG qtFromMatrix(const HMatrix mat)
{
	QuatG qu;
	double tr, s;
	tr = mat[QX][QX] + mat[QY][QY] + mat[QZ][QZ];
	if (tr >= 0.0)
	{
		s = sqrt(tr + mat[QW][QW]);
		qu.w = (float)(s * 0.5);
		s = 0.5 / s;
		qu.x = (float)((mat[QZ][QY] - mat[QY][QZ]) * s);
		qu.y = (float)((mat[QX][QZ] - mat[QZ][QX]) * s);
		qu.z = (float)((mat[QY][QX] - mat[QX][QY]) * s);
	}
	else
	{
		int h = QX;
		if (mat[QY][QY] > mat[QX][QX]) h = QY;
		if (mat[QZ][QZ] > mat[h][h]) h = QZ;
		switch (h)
		{
		case QX:
			s = sqrt((mat[QX][QX] - (mat[QY][QY] + mat[QZ][QZ])) + mat[QW][QW]);
			qu.x = (float)(s * 0.5);
			s = 0.5 / s;
			qu.y = (float)((mat[QX][QY] + mat[QY][QX]) * s);
			qu.z = (float)((mat[QZ][QX] + mat[QX][QZ]) * s);
			qu.w = (float)((mat[QZ][QY] - mat[QY][QZ]) * s);
			break;
		case QY:
			s = sqrt((mat[QY][QY] - (mat[QZ][QZ] + mat[QX][QX])) + mat[QW][QW]);
			qu.y = (float)(s * 0.5);
			s = 0.5 / s;
			qu.z = (float)((mat[QY][QZ] + mat[QZ][QY]) * s);
			qu.x = (float)((mat[QX][QY] + mat[QY][QX]) * s);
			qu.w = (float)((mat[QX][QZ] - mat[QZ][QX]) * s);
			break;
		case QZ:
			s = sqrt((mat[QZ][QZ] - (mat[QX][QX] + mat[QY][QY])) + mat[QW][QW]);
			qu.z = (float)(s * 0.5);
			s = 0.5 / s;
			qu.x = (float)((mat[QZ][QX] + mat[QX][QZ]) * s);
			qu.y = (float)((mat[QY][QZ] + mat[QZ][QY]) * s);
			qu.w = (float)((mat[QY][QX] - mat[QX][QY]) * s);
			break;
		}
	}
	if (mat[QW][QW] != 1.0f) qu = qtScale(qu, (float)(1.0 / sqrt(mat[QW][QW])));
	return qu;
}

// Spectral decomposition of symmetric S: eigenvalues in the returned vector, eigenvectors as
// the rotation accumulated in U (jacobi sweeps, double accumulators like the gems code).
static void spectDecomp(const HMatrix S, HMatrix U, float kOut[3])
{
	double Diag[3], OffD[3];
	double g, h, fabsh, fabsOffDi, t, theta, c, s, tau, ta, OffDq, a, b;
	static const int nxt[] = { QY, QZ, QX };
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			U[i][j] = (i == j) ? 1.0f : 0.0f;
	matPad(U);
	Diag[QX] = S[QX][QX]; Diag[QY] = S[QY][QY]; Diag[QZ] = S[QZ][QZ];
	OffD[QX] = S[QY][QZ]; OffD[QY] = S[QZ][QX]; OffD[QZ] = S[QX][QY];
	for (int sweep = 20; sweep > 0; --sweep)
	{
		float sm = (float)(fabs(OffD[QX]) + fabs(OffD[QY]) + fabs(OffD[QZ]));
		if (sm == 0.0f) break;
		for (int i = QZ; i >= QX; --i)
		{
			int p = nxt[i];
			int q = nxt[p];
			fabsOffDi = fabs(OffD[i]);
			g = 100.0 * fabsOffDi;
			if (fabsOffDi > 0.0)
			{
				h = Diag[q] - Diag[p];
				fabsh = fabs(h);
				if (fabsh + g == fabsh)
				{
					t = OffD[i] / h;
				}
				else
				{
					theta = 0.5 * h / OffD[i];
					t = 1.0 / (fabs(theta) + sqrt(theta * theta + 1.0));
					if (theta < 0.0) t = -t;
				}
				c = 1.0 / sqrt(t * t + 1.0);
				s = t * c;
				tau = s / (c + 1.0);
				ta = t * OffD[i];
				OffD[i] = 0.0;
				Diag[p] -= ta;
				Diag[q] += ta;
				OffDq = OffD[q];
				OffD[q] -= s * (OffD[p] + tau * OffD[q]);
				OffD[p] += s * (OffDq - tau * OffD[p]);
				for (int j = QZ; j >= QX; --j)
				{
					a = U[j][p];
					b = U[j][q];
					U[j][p] -= (float)(s * (b + tau * a));
					U[j][q] += (float)(s * (a - tau * b));
				}
			}
		}
	}
	kOut[0] = (float)Diag[QX];
	kOut[1] = (float)Diag[QY];
	kOut[2] = (float)Diag[QZ];
}

// snuggle: given the stretch rotation q and stretch factors k, compute a permutation/reflection
// rotation p so U*p is as close to an axis permutation as possible and k is permuted to match.
static QuatG snuggle(QuatG q, float k[3])
{
#define SQRTHALF 0.7071067811865475244f
#define sgnf(n, v) ((n) ? -(v) : (v))
#define swapf(a, i, j) { a[3] = a[i]; a[i] = a[j]; a[j] = a[3]; }
#define cyclef(a, p)                                        \
	if (p) { a[3] = a[0]; a[0] = a[1]; a[1] = a[2]; a[2] = a[3]; } \
	else   { a[3] = a[2]; a[2] = a[1]; a[1] = a[0]; a[0] = a[3]; }

	QuatG p;
	float ka[4];
	int turn = -1;
	ka[QX] = k[0]; ka[QY] = k[1]; ka[QZ] = k[2];
	if (ka[QX] == ka[QY])
	{
		if (ka[QX] == ka[QZ]) turn = QW;
		else turn = QZ;
	}
	else
	{
		if (ka[QX] == ka[QZ]) turn = QY;
		else if (ka[QY] == ka[QZ]) turn = QX;
	}
	if (turn >= 0)
	{
		QuatG qtoz, qp;
		unsigned neg[3], win;
		double mag[3], t;
		static const QuatG qxtoz = { 0.0f, SQRTHALF, 0.0f, SQRTHALF };
		static const QuatG qytoz = { SQRTHALF, 0.0f, 0.0f, SQRTHALF };
		static const QuatG qppmm = { 0.5f, 0.5f, -0.5f, -0.5f };
		static const QuatG qpppp = { 0.5f, 0.5f, 0.5f, 0.5f };
		static const QuatG qmpmm = { -0.5f, 0.5f, -0.5f, -0.5f };
		static const QuatG qpppm = { 0.5f, 0.5f, 0.5f, -0.5f };
		static const QuatG q0001 = { 0.0f, 0.0f, 0.0f, 1.0f };
		static const QuatG q1000 = { 1.0f, 0.0f, 0.0f, 0.0f };
		switch (turn)
		{
		default:
			return qtConj(q);
		case QX:
			qtoz = qxtoz;
			q = qtMul(q, qtoz);
			swapf(ka, QX, QZ)
			break;
		case QY:
			qtoz = qytoz;
			q = qtMul(q, qtoz);
			swapf(ka, QY, QZ)
			break;
		case QZ:
			qtoz = q0001;
			break;
		}
		q = qtConj(q);
		mag[0] = (double)q.z * q.z + (double)q.w * q.w - 0.5;
		mag[1] = (double)q.x * q.z - (double)q.y * q.w;
		mag[2] = (double)q.y * q.z + (double)q.x * q.w;
		for (int i = 0; i < 3; ++i)
		{
			neg[i] = (mag[i] < 0.0);
			if (neg[i]) mag[i] = -mag[i];
		}
		if (mag[1] > mag[0])
		{
			if (mag[2] > mag[1]) win = 2;
			else win = 1;
		}
		else
		{
			if (mag[2] > mag[0]) win = 2;
			else win = 0;
		}
		switch (win)
		{
		case 0:
			if (neg[0]) p = q1000; else p = q0001;
			break;
		case 1:
			if (neg[1]) p = qppmm; else p = qpppp;
			cyclef(ka, 0)
			break;
		case 2:
			if (neg[2]) p = qmpmm; else p = qpppm;
			cyclef(ka, 1)
			break;
		}
		qp = qtMul(q, p);
		t = sqrt(mag[win] + 0.5);
		p = qtMul(p, qtMake(0.0f, 0.0f, (float)(-qp.z / t), (float)(qp.w / t)));
		p = qtMul(qtoz, qtConj(p));
	}
	else
	{
		float qa[4], pa[4];
		unsigned lo, hi, neg[4], par = 0;
		double all, big, two;
		qa[0] = q.x; qa[1] = q.y; qa[2] = q.z; qa[3] = q.w;
		for (int i = 0; i < 4; ++i)
		{
			pa[i] = 0.0f;
			neg[i] = (qa[i] < 0.0f);
			if (neg[i]) qa[i] = -qa[i];
			par ^= neg[i];
		}
		// Find the two largest components, indices in hi and lo
		if (qa[0] > qa[1]) lo = 0;
		else lo = 1;
		if (qa[2] > qa[3]) hi = 2;
		else hi = 3;
		if (qa[lo] > qa[hi])
		{
			if (qa[lo ^ 1] > qa[hi]) { hi = lo; lo ^= 1; }
			else { hi ^= lo; lo ^= hi; hi ^= lo; }
		}
		else
		{
			if (qa[hi ^ 1] > qa[lo]) lo = hi ^ 1;
		}
		all = (qa[0] + qa[1] + qa[2] + qa[3]) * 0.5;
		two = (qa[hi] + qa[lo]) * SQRTHALF;
		big = qa[hi];
		if (all > two)
		{
			if (all > big)
			{
				// all
				for (int i = 0; i < 4; ++i)
					pa[i] = sgnf(neg[i], 0.5f);
				cyclef(ka, par)
			}
			else
			{
				// big
				pa[hi] = sgnf(neg[hi], 1.0f);
			}
		}
		else
		{
			if (two > big)
			{
				// two
				pa[hi] = sgnf(neg[hi], SQRTHALF);
				pa[lo] = sgnf(neg[lo], SQRTHALF);
				if (lo > hi) { hi ^= lo; lo ^= hi; hi ^= lo; }
				if (hi == QW)
				{
					static const unsigned hipick[] = { 1, 2, 0 };
					hi = hipick[lo];
					lo = 3 - hi - lo;
				}
				swapf(ka, hi, lo)
			}
			else
			{
				// big
				pa[hi] = sgnf(neg[hi], 1.0f);
			}
		}
		p.x = -pa[0]; p.y = -pa[1]; p.z = -pa[2]; p.w = pa[3];
	}
	k[0] = ka[QX]; k[1] = ka[QY]; k[2] = ka[QZ];
	return p;

#undef SQRTHALF
#undef sgnf
#undef swapf
#undef cyclef
}

void decompAffine(const Matrix3M &a, AffinePartsM &parts)
{
	// SDK adaptation: Matrix3 rows into the HMatrix rows (the gems code then works on the
	// column-convention view of the same values), translation from row 3.
	HMatrix A, Q, S, U;
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			A[i][j] = a.m[i][j];
		A[i][3] = 0.0f;
	}
	A[3][0] = a.m[3][0]; A[3][1] = a.m[3][1]; A[3][2] = a.m[3][2]; A[3][3] = 1.0f;

	parts.t.x = a.m[3][0];
	parts.t.y = a.m[3][1];
	parts.t.z = a.m[3][2];

	float det = polarDecomp(A, Q, S);
	QuatG q, u, p;
	if (det < 0.0f)
	{
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				Q[i][j] = -Q[i][j];
		parts.f = -1.0f;
	}
	else
	{
		parts.f = 1.0f;
	}
	q = qtFromMatrix(Q);
	float k[3];
	spectDecomp(S, U, k);
	u = qtFromMatrix(U);
	p = snuggle(u, k);
	u = qtMul(u, p);

	parts.q.x = q.x; parts.q.y = q.y; parts.q.z = q.z; parts.q.w = q.w;
	parts.u.x = u.x; parts.u.y = u.y; parts.u.z = u.z; parts.u.w = u.w;
	parts.k.x = k[0]; parts.k.y = k[1]; parts.k.z = k[2];
}

} /* namespace MAXMATH */

/* end of file */
