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

"""Read Ryzom/NeL .hlsinfo files (panoply_maker's per-texture HLS build cache, one per
colorisable item, later concatenated by hls_bank_maker into a .hlsbank -- see
hls_texture_bank.py).

Format reverse-engineered from nel/tools/3d/panoply_maker/hls_bank_texture_info.h/.cpp
(CHLSBankTextureInfo::serial and friends):

- CHLSBankTextureInfo: version(0), DividedBy2 (bool), SrcBitmap (CDXTCBitmap), then
  serialCont(Masks) (vector<CMaskBitmap>), serialCont(Instances)
  (vector<CTextureInstance>).
- CDXTCBitmap: version(0), then serialCont(_Data) -- _Data is a **complete raw .dds
  file** (CS3TCCompressor::compress() writes "DDS " + DDS_HEADER + mipmap chain
  directly into this stream, byte-for-byte the same container format documented in
  nel/tools/forgery/docs/dds_export.md / built by ryzom_forgery.dds_export). Not
  decoded further here -- exposed as raw bytes; use ryzom_forgery.dds_export or any
  DDS reader if the actual pixels are needed.
- CMaskBitmap: version(0), Width/Height (uint32 each), then serialCont(Pixels) --
  Width*Height raw uint8 luminance values (single channel, no compression). This is
  the alpha-mask source CHLSColorTexture.addMask() consumes (via buildBitmap(), which
  replicates it to R=G=B=A).
- CTextureInstance: version(0), Name (NeL length-prefixed string, includes the
  ".tga"/".dds" extension), then serialCont(Mods) (vector<CHLSMod>) -- one CHLSMod per
  mask, in the same order as CHLSBankTextureInfo.Masks.
- CHLSMod: version(0), DHue/DLum/DSat (float32 each) -- uncompressed HLS deltas
  (DHue in degrees 0..360, DLum/DSat in -1..1) as configured in panoply_*.cfg. These
  get compressed into hls_texture_bank.ColorDelta's 0..255/-127..127 encoding only at
  hls_bank_maker time (see hls_bank_maker.cpp's addTextToBank(), and
  hls_texture_bank.compress_hls_mod() here, which mirrors that exact compression).

Usage:
	from pynel import hls_bank_texture_info as hlsinfo
	info = hlsinfo.load_hlsinfo("ryw_hom_caster01_pantabottes_c2.hlsinfo")
	print(len(info.masks), len(info.instances))
"""

import argparse
import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import BinaryIO, List, Union


class HlsInfoParseError(Exception):
	pass


@dataclass
class HLSMod:
	"""CHLSBankTextureInfo::CHLSMod. Uncompressed float deltas, as configured in
	panoply_*.cfg (hues in degrees 0..360, lightness/saturation/luminosity/contrast
	folded by panoply_maker into DLum/DSat, both -1..1)."""
	d_hue: float
	d_lum: float
	d_sat: float


@dataclass
class MaskBitmap:
	"""CHLSBankTextureInfo::CMaskBitmap: a raw single-channel (luminance) alpha mask,
	uncompressed, at the source texture's native resolution."""
	width: int
	height: int
	pixels: bytes  # width * height, one uint8 per pixel


@dataclass
class TextureInstance:
	"""CHLSBankTextureInfo::CTextureInstance: one baked color variant's name + its
	per-mask HLS deltas (mods[i] applies to masks[i] of the parent HLSBankTextureInfo)."""
	name: str
	mods: List[HLSMod] = field(default_factory=list)


@dataclass
class HLSBankTextureInfo:
	divided_by_2: bool
	src_bitmap_dds: bytes  # complete raw .dds file bytes (see module docstring)
	masks: List[MaskBitmap] = field(default_factory=list)
	instances: List[TextureInstance] = field(default_factory=list)


class _Reader:
	"""Minimal binary reader matching NeL's CIFile little-endian encoding."""

	def __init__(self, data: bytes):
		self._data = data
		self._pos = 0

	def _take(self, size: int) -> bytes:
		end = self._pos + size
		if end > len(self._data):
			raise HlsInfoParseError(
				f"unexpected end of file at offset {self._pos} (needed {size} bytes)"
			)
		chunk = self._data[self._pos:end]
		self._pos = end
		return chunk

	def u8(self) -> int:
		return self._take(1)[0]

	def u32(self) -> int:
		return struct.unpack("<I", self._take(4))[0]

	def s32(self) -> int:
		return struct.unpack("<i", self._take(4))[0]

	def f32(self) -> float:
		return struct.unpack("<f", self._take(4))[0]

	def boolean(self) -> bool:
		return self.u8() != 0

	def version(self) -> int:
		b = self.u8()
		if b == 0xFF:
			return self.u32()
		return b

	def string(self) -> str:
		length = self.u32()
		return self._take(length).decode("latin-1")

	def cont_len(self) -> int:
		"""Length prefix used by serialCont() for generic containers."""
		return self.s32()

	def raw(self, size: int) -> bytes:
		return self._take(size)

	def eof(self) -> bool:
		return self._pos >= len(self._data)


class _Writer:
	"""Minimal binary writer matching NeL's COFile little-endian encoding."""

	def __init__(self):
		self._chunks: List[bytes] = []

	def u8(self, v: int) -> None:
		self._chunks.append(struct.pack("<B", v))

	def u32(self, v: int) -> None:
		self._chunks.append(struct.pack("<I", v))

	def s32(self, v: int) -> None:
		self._chunks.append(struct.pack("<i", v))

	def f32(self, v: float) -> None:
		self._chunks.append(struct.pack("<f", v))

	def boolean(self, v: bool) -> None:
		self.u8(1 if v else 0)

	def version(self, v: int) -> None:
		if v < 0xFF:
			self.u8(v)
		else:
			self.u8(0xFF)
			self.u32(v)

	def string(self, s: str) -> None:
		encoded = s.encode("latin-1")
		self.u32(len(encoded))
		self.raw(encoded)

	def cont_len(self, n: int) -> None:
		self.s32(n)

	def raw(self, data: bytes) -> None:
		self._chunks.append(data)

	def getvalue(self) -> bytes:
		return b"".join(self._chunks)


def _parse_hls_mod(f: _Reader) -> HLSMod:
	f.version()
	return HLSMod(d_hue=f.f32(), d_lum=f.f32(), d_sat=f.f32())


def _parse_mask_bitmap(f: _Reader) -> MaskBitmap:
	f.version()
	width = f.u32()
	height = f.u32()
	pixel_len = f.cont_len()
	pixels = f.raw(pixel_len)
	return MaskBitmap(width=width, height=height, pixels=pixels)


def _parse_texture_instance(f: _Reader) -> TextureInstance:
	f.version()
	name = f.string()
	n_mods = f.cont_len()
	mods = [_parse_hls_mod(f) for _ in range(n_mods)]
	return TextureInstance(name=name, mods=mods)


def parse_hlsinfo(data: bytes) -> HLSBankTextureInfo:
	f = _Reader(data)

	f.version()
	divided_by_2 = f.boolean()

	# CDXTCBitmap: version(0), then serialCont(_Data) (a raw .dds file)
	f.version()
	dds_len = f.cont_len()
	src_bitmap_dds = f.raw(dds_len)

	n_masks = f.cont_len()
	masks = [_parse_mask_bitmap(f) for _ in range(n_masks)]

	n_instances = f.cont_len()
	instances = [_parse_texture_instance(f) for _ in range(n_instances)]

	if not f.eof():
		raise HlsInfoParseError(f"{len(data) - f._pos} trailing bytes after parsing .hlsinfo content")

	return HLSBankTextureInfo(
		divided_by_2=divided_by_2, src_bitmap_dds=src_bitmap_dds, masks=masks, instances=instances,
	)


def _write_hls_mod(f: _Writer, mod: HLSMod) -> None:
	f.version(0)
	f.f32(mod.d_hue)
	f.f32(mod.d_lum)
	f.f32(mod.d_sat)


def _write_mask_bitmap(f: _Writer, mask: MaskBitmap) -> None:
	f.version(0)
	f.u32(mask.width)
	f.u32(mask.height)
	f.cont_len(len(mask.pixels))
	f.raw(mask.pixels)


def _write_texture_instance(f: _Writer, inst: TextureInstance) -> None:
	f.version(0)
	f.string(inst.name)
	f.cont_len(len(inst.mods))
	for mod in inst.mods:
		_write_hls_mod(f, mod)


def dumps_hlsinfo(info: HLSBankTextureInfo) -> bytes:
	"""Re-serialize a .hlsinfo to bytes. Round-trip (load_hlsinfo -> dumps_hlsinfo) must
	be byte-identical to the original file -- same validation approach as
	hls_texture_bank.dumps_hlsbank()."""
	f = _Writer()

	f.version(0)
	f.boolean(info.divided_by_2)

	# CDXTCBitmap: version(0), then serialCont(_Data) (a raw .dds file)
	f.version(0)
	f.cont_len(len(info.src_bitmap_dds))
	f.raw(info.src_bitmap_dds)

	f.cont_len(len(info.masks))
	for mask in info.masks:
		_write_mask_bitmap(f, mask)

	f.cont_len(len(info.instances))
	for inst in info.instances:
		_write_texture_instance(f, inst)

	return f.getvalue()


def save_hlsinfo(path: Union[str, Path, BinaryIO], info: HLSBankTextureInfo) -> None:
	data = dumps_hlsinfo(info)
	if hasattr(path, "write"):
		path.write(data)
	else:
		Path(path).write_bytes(data)


def load_hlsinfo(path: Union[str, Path, BinaryIO]) -> HLSBankTextureInfo:
	if hasattr(path, "read"):
		data = path.read()
	else:
		data = Path(path).read_bytes()
	return parse_hlsinfo(data)


def _dump(info: HLSBankTextureInfo) -> None:
	print(f"divided_by_2: {info.divided_by_2}")
	print(f"src_bitmap: {len(info.src_bitmap_dds)} bytes (raw .dds)")
	print(f"masks: {len(info.masks)}")
	for i, m in enumerate(info.masks):
		print(f"  [{i}] {m.width}x{m.height}")
	print(f"instances: {len(info.instances)}")
	for inst in info.instances:
		mods = ", ".join(f"({m.d_hue:.1f},{m.d_lum:.2f},{m.d_sat:.2f})" for m in inst.mods)
		print(f"  {inst.name}: {mods}")


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Read Ryzom .hlsinfo files (panoply_maker's per-texture HLS build cache)")
	sub = parser.add_subparsers(dest="command", required=True)

	p_dump = sub.add_parser("dump", help="print a summary of a .hlsinfo file")
	p_dump.add_argument("path", type=Path)

	return parser


def _main() -> None:
	args = _build_arg_parser().parse_args()
	info = load_hlsinfo(args.path)

	if args.command == "dump":
		_dump(info)


if __name__ == "__main__":
	_main()
