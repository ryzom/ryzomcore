#!/usr/bin/env python3
# Copyright (C) 2026  Nuno Gonçalves (Ulukyn) <nuno@troispetits.net>
# Copyright (C) 2026  Claude Sonnet 5 (Anthropic) <noreply@anthropic.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Read and write Ryzom/NeL .cmb files (indoor collision mesh interchange format).

Format reverse-engineered from nel/include/nel/pacs/collision_mesh_build.h
(CCollisionMeshBuild::serial, CCollisionFace::serial) — see
nel/tools/pynel/docs/pacs_format.md section 8 for the full writeup, including
the authoring conventions (Visibility semantics, MatID 666 on exterior faces,
one MatID per opening) sourced from
https://wiki.ryzom.dev/landscape/collisions_create.

.cmb is a flat, unversioned binary format: no magic, no serialVersion. It is
NOT a final PACS artifact by itself — it's the raw triangle-soup interchange
format produced by a 3D authoring tool (historically 3ds Max) and consumed by
build_indoor_rbank to build an indoor .lr retriever.

Usage:
	from pynel import collision_mesh_build as cmb
	mesh = cmb.load_cmb("apartment.cmb")
	print(len(mesh.vertices), len(mesh.faces))
	cmb.save_cmb("out.cmb", mesh)
"""

import argparse
import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import BinaryIO, List, Union


class CmbParseError(Exception):
	pass


class CmbWriteError(Exception):
	pass


@dataclass
class Vector3:
	x: float
	y: float
	z: float


@dataclass
class CollisionFace:
	"""NLPACS::CCollisionFace. Only the serialized fields — Edge[]/InternalSurface/
	EdgeFlags are computed at load time by CCollisionMeshBuild::link() and are not
	part of the file."""
	v: List[int]  # 3 vertex indices into CollisionMeshBuild.vertices
	visibility: List[bool]  # per-edge: True = hard boundary (never linked), False = opening (see docs/pacs_format.md 8.2b)
	surface: int  # -1 = exterior/boundary mesh face, >=0 = author-assigned interior room/floor tag
	material: int  # Max material id; must be 666 on every exterior-mesh face (see docs/pacs_format.md 8.2c)


@dataclass
class CollisionMeshBuild:
	vertices: List[Vector3] = field(default_factory=list)
	faces: List[CollisionFace] = field(default_factory=list)


class _Reader:
	"""Minimal binary reader matching NeL's CIFile little-endian encoding."""

	def __init__(self, data: bytes):
		self._data = data
		self._pos = 0

	def _take(self, size: int) -> bytes:
		end = self._pos + size
		if end > len(self._data):
			raise CmbParseError(
				f"unexpected end of file at offset {self._pos} (needed {size} bytes)"
			)
		chunk = self._data[self._pos:end]
		self._pos = end
		return chunk

	def u32(self) -> int:
		return struct.unpack("<I", self._take(4))[0]

	def s32(self) -> int:
		return struct.unpack("<i", self._take(4))[0]

	def f32(self) -> float:
		return struct.unpack("<f", self._take(4))[0]

	def boolean(self) -> bool:
		return self._take(1)[0] != 0

	def vector3(self) -> Vector3:
		return Vector3(self.f32(), self.f32(), self.f32())

	def cont_len(self) -> int:
		"""Length prefix used by serialCont() for generic containers."""
		return self.s32()

	def eof(self) -> bool:
		return self._pos >= len(self._data)


class _Writer:
	"""Minimal binary writer matching NeL's COFile little-endian encoding."""

	def __init__(self):
		self._chunks: List[bytes] = []

	def u32(self, value: int) -> None:
		self._chunks.append(struct.pack("<I", value))

	def s32(self, value: int) -> None:
		self._chunks.append(struct.pack("<i", value))

	def f32(self, value: float) -> None:
		self._chunks.append(struct.pack("<f", value))

	def boolean(self, value: bool) -> None:
		self._chunks.append(struct.pack("<B", 1 if value else 0))

	def vector3(self, v: Vector3) -> None:
		self.f32(v.x)
		self.f32(v.y)
		self.f32(v.z)

	def cont_len(self, length: int) -> None:
		self.s32(length)

	def getvalue(self) -> bytes:
		return b"".join(self._chunks)


def _parse_face(f: _Reader) -> CollisionFace:
	v = [f.u32(), f.u32(), f.u32()]
	visibility = [f.boolean(), f.boolean(), f.boolean()]
	surface = f.s32()
	material = f.s32()
	return CollisionFace(v=v, visibility=visibility, surface=surface, material=material)


def _write_face(f: _Writer, face: CollisionFace) -> None:
	if len(face.v) != 3 or len(face.visibility) != 3:
		raise CmbWriteError(f"face must have exactly 3 vertices/visibility flags, got {face}")
	for index in face.v:
		f.u32(index)
	for vis in face.visibility:
		f.boolean(vis)
	f.s32(face.surface)
	f.s32(face.material)


def parse_cmb(data: bytes) -> CollisionMeshBuild:
	f = _Reader(data)

	n_vertices = f.cont_len()
	vertices = [f.vector3() for _ in range(n_vertices)]

	n_faces = f.cont_len()
	faces = [_parse_face(f) for _ in range(n_faces)]

	if not f.eof():
		raise CmbParseError(f"{len(data) - f._pos} trailing bytes after parsing .cmb content")

	return CollisionMeshBuild(vertices=vertices, faces=faces)


def dumps(mesh: CollisionMeshBuild) -> bytes:
	f = _Writer()

	f.cont_len(len(mesh.vertices))
	for v in mesh.vertices:
		f.vector3(v)

	f.cont_len(len(mesh.faces))
	for face in mesh.faces:
		_write_face(f, face)

	return f.getvalue()


def load_cmb(path: Union[str, Path, BinaryIO]) -> CollisionMeshBuild:
	if hasattr(path, "read"):
		data = path.read()
	else:
		data = Path(path).read_bytes()
	return parse_cmb(data)


def save_cmb(path: Union[str, Path, BinaryIO], mesh: CollisionMeshBuild) -> None:
	data = dumps(mesh)
	if hasattr(path, "write"):
		path.write(data)
	else:
		Path(path).write_bytes(data)


def _dump(mesh: CollisionMeshBuild) -> None:
	print(f"vertices: {len(mesh.vertices)}")
	print(f"faces: {len(mesh.faces)}")

	surfaces = {}
	materials = {}
	visible_edges = 0
	invisible_edges = 0
	for face in mesh.faces:
		surfaces[face.surface] = surfaces.get(face.surface, 0) + 1
		materials[face.material] = materials.get(face.material, 0) + 1
		visible_edges += sum(1 for vis in face.visibility if vis)
		invisible_edges += sum(1 for vis in face.visibility if not vis)

	print(f"surfaces: {dict(sorted(surfaces.items()))}")
	if -1 in surfaces:
		print(f"  (-1 = exterior/boundary mesh faces: {surfaces[-1]})")
	print(f"materials: {dict(sorted(materials.items()))}")
	if -1 in surfaces and any(f.surface == -1 and f.material != 665 for f in mesh.faces):
		print("  WARNING: exterior faces present but not all use material 665 (0-based; the tutorial's "
			"'MatID 666' is 1-based, as entered in 3ds Max's Material ID field) — see docs/pacs_format.md 8.2c")
	print(f"edges: {visible_edges} visible (walls), {invisible_edges} invisible (openings)")


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Read Ryzom .cmb files (indoor collision mesh interchange)")
	sub = parser.add_subparsers(dest="command", required=True)

	p_dump = sub.add_parser("dump", help="print a summary of a .cmb file")
	p_dump.add_argument("path", type=Path)

	return parser


def _main() -> None:
	args = _build_arg_parser().parse_args()
	mesh = load_cmb(args.path)

	if args.command == "dump":
		_dump(mesh)


if __name__ == "__main__":
	_main()
