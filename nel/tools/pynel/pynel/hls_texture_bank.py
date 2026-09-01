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

"""Read Ryzom/NeL .hlsbank files (HLS-colorisable texture bank, e.g. characters.hlsbank).

Format reverse-engineered from nel/include/nel/3d/hls_texture_bank.h/.cpp
(CHLSTextureBank/CHLSTextureBank::CTextureInstance::serial) and
nel/include/nel/3d/hls_color_texture.h/.cpp (CHLSColorTexture/CHLSColorTexture::CMask/
CHLSColorDelta::serial). nel/tools/3d/hls_bank_maker/hls_bank_maker.cpp confirms the
on-disk file is a raw COFile::serial(bank) -- no magic, no header, no compression.

Layout:
- CHLSTextureBank: version(0), then serialCont of color_textures
  (vector<CHLSColorTexture>), a raw instance_data blob (vector<uint8>), and instances
  (vector<CTextureInstance>, each only storing a data offset + a color texture index --
  the real per-instance data lives in instance_data, decoded here into TextureInstance).
- Each instance's real data sits in instance_data at its data offset: a 0-terminated
  lowercase name string, immediately followed by
  color_textures[color_texture_id].num_masks() ColorDelta entries (3 bytes each: DHue
  uint8, DLum int8, DSat int8) -- num_masks comes from that color texture's own mask
  count, so instances are decoded only after all color textures are parsed.
- CHLSColorTexture: version(0), width/height/num_mipmap/block_to_compress_index (uint32
  each), then serialCont(texture) (raw DXTC5 bytes, not decoded here) and
  serialCont(masks) (vector<CMask>).
- CMask: version(0), full_block_index/mixt_block_index (uint32 each), then
  serialCont(data) (raw bytes, not decoded here).

Read-only: nothing currently needs to write a .hlsbank back out (unlike .cmb, which
build_indoor_rbank consumes as an interchange format authors actively produce), and the
source .hlsinfo files this bank is normally rebuilt from are not always available -- see
docs/hls_texture_bank.md.

Usage:
	from pynel import hls_texture_bank as hlsbank
	bank = hlsbank.load_hlsbank("characters.hlsbank")
	print(len(bank.color_textures), len(bank.instances))
	for inst in bank.instances:
		print(inst.name, len(inst.color_deltas))
"""

import argparse
import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import BinaryIO, List, Union


class HlsBankParseError(Exception):
	pass


@dataclass
class ColorDelta:
	"""CHLSColorDelta. d_hue is 0..255 (compressed from 0..360 degrees at bake time),
	d_lum/d_sat are -128..127 (compressed from -1..1)."""
	d_hue: int
	d_lum: int
	d_sat: int


@dataclass
class Mask:
	"""CHLSColorTexture::CMask. data is the raw per-block mask bitmask (mixt blocks'
	16 raw uint8 mix weights, then FullBlockIndex/MixtBlockIndex membership bits) --
	not decoded further here, only needed to reproduce buildColorVersion()."""
	full_block_index: int
	mixt_block_index: int
	data: bytes


@dataclass
class ColorTexture:
	"""CHLSColorTexture: the shared, uncolored source texture + its masks. Multiple
	TextureInstance entries can reference the same ColorTexture (one per distinct
	item texture that shares this base + set of masks)."""
	width: int
	height: int
	num_mipmap: int
	block_to_compress_index: int
	texture: bytes  # raw DXTC5 data, not decoded here
	masks: List[Mask] = field(default_factory=list)

	def num_masks(self) -> int:
		return len(self.masks)


@dataclass
class TextureInstance:
	"""CHLSTextureBank::CTextureInstance, with its data (name + per-mask color deltas)
	decoded from the bank's shared instance_data blob. name is always lowercase (the
	engine lowercases it at addTextureInstance() time)."""
	name: str
	color_texture_id: int
	color_deltas: List[ColorDelta] = field(default_factory=list)


@dataclass
class HLSTextureBank:
	color_textures: List[ColorTexture] = field(default_factory=list)
	instances: List[TextureInstance] = field(default_factory=list)


class _Reader:
	"""Minimal binary reader matching NeL's CIFile little-endian encoding."""

	def __init__(self, data: bytes):
		self._data = data
		self._pos = 0

	def _take(self, size: int) -> bytes:
		end = self._pos + size
		if end > len(self._data):
			raise HlsBankParseError(
				f"unexpected end of file at offset {self._pos} (needed {size} bytes)"
			)
		chunk = self._data[self._pos:end]
		self._pos = end
		return chunk

	def u8(self) -> int:
		return self._take(1)[0]

	def s8(self) -> int:
		return struct.unpack("<b", self._take(1))[0]

	def u32(self) -> int:
		return struct.unpack("<I", self._take(4))[0]

	def s32(self) -> int:
		return struct.unpack("<i", self._take(4))[0]

	def version(self) -> int:
		b = self.u8()
		if b == 0xFF:
			return self.u32()
		return b

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

	def s8(self, v: int) -> None:
		self._chunks.append(struct.pack("<b", v))

	def u32(self, v: int) -> None:
		self._chunks.append(struct.pack("<I", v))

	def s32(self, v: int) -> None:
		self._chunks.append(struct.pack("<i", v))

	def version(self, v: int) -> None:
		if v < 0xFF:
			self.u8(v)
		else:
			self.u8(0xFF)
			self.u32(v)

	def cont_len(self, n: int) -> None:
		self.s32(n)

	def raw(self, data: bytes) -> None:
		self._chunks.append(data)

	def getvalue(self) -> bytes:
		return b"".join(self._chunks)


def _parse_mask(f: _Reader) -> Mask:
	f.version()
	full_block_index = f.u32()
	mixt_block_index = f.u32()
	data_len = f.cont_len()
	data = f.raw(data_len)
	return Mask(full_block_index=full_block_index, mixt_block_index=mixt_block_index, data=data)


def _parse_color_texture(f: _Reader) -> ColorTexture:
	f.version()
	width = f.u32()
	height = f.u32()
	num_mipmap = f.u32()
	block_to_compress_index = f.u32()
	texture_len = f.cont_len()
	texture = f.raw(texture_len)
	n_masks = f.cont_len()
	masks = [_parse_mask(f) for _ in range(n_masks)]
	return ColorTexture(
		width=width, height=height, num_mipmap=num_mipmap,
		block_to_compress_index=block_to_compress_index, texture=texture, masks=masks,
	)


def parse_hlsbank(data: bytes) -> HLSTextureBank:
	f = _Reader(data)

	f.version()

	n_color_textures = f.cont_len()
	color_textures = [_parse_color_texture(f) for _ in range(n_color_textures)]

	instance_data_len = f.cont_len()
	instance_data = f.raw(instance_data_len)

	n_instances = f.cont_len()
	raw_instances = []
	for _ in range(n_instances):
		f.version()
		data_index = f.u32()
		color_texture_id = f.u32()
		raw_instances.append((data_index, color_texture_id))

	if not f.eof():
		raise HlsBankParseError(f"{len(data) - f._pos} trailing bytes after parsing .hlsbank content")

	instances = []
	for data_index, color_texture_id in raw_instances:
		if color_texture_id >= len(color_textures):
			raise HlsBankParseError(
				f"instance at data offset {data_index} references out-of-range color texture "
				f"{color_texture_id} (bank has {len(color_textures)})"
			)
		num_masks = color_textures[color_texture_id].num_masks()
		name_end = instance_data.index(b"\x00", data_index)
		name = instance_data[data_index:name_end].decode("latin-1")
		pos = name_end + 1
		deltas = []
		for _ in range(num_masks):
			d_hue = instance_data[pos]
			d_lum = struct.unpack("<b", instance_data[pos + 1:pos + 2])[0]
			d_sat = struct.unpack("<b", instance_data[pos + 2:pos + 3])[0]
			deltas.append(ColorDelta(d_hue=d_hue, d_lum=d_lum, d_sat=d_sat))
			pos += 3
		instances.append(TextureInstance(name=name, color_texture_id=color_texture_id, color_deltas=deltas))

	return HLSTextureBank(color_textures=color_textures, instances=instances)


def _write_mask(f: _Writer, mask: Mask) -> None:
	f.version(0)
	f.u32(mask.full_block_index)
	f.u32(mask.mixt_block_index)
	f.cont_len(len(mask.data))
	f.raw(mask.data)


def _write_color_texture(f: _Writer, tex: ColorTexture) -> None:
	f.version(0)
	f.u32(tex.width)
	f.u32(tex.height)
	f.u32(tex.num_mipmap)
	f.u32(tex.block_to_compress_index)
	f.cont_len(len(tex.texture))
	f.raw(tex.texture)
	f.cont_len(len(tex.masks))
	for mask in tex.masks:
		_write_mask(f, mask)


def dumps_hlsbank(bank: HLSTextureBank) -> bytes:
	"""Re-serialize a bank to bytes. Round-trip (load_hlsbank -> dumps_hlsbank) must be
	byte-identical to the original file for any bank that wasn't mutated -- used to
	validate this module's writer half against real production data before trusting
	append_texture_info()."""
	f = _Writer()

	f.version(0)

	f.cont_len(len(bank.color_textures))
	for tex in bank.color_textures:
		_write_color_texture(f, tex)

	instance_data = bytearray()
	offsets = []
	for inst in bank.instances:
		offsets.append(len(instance_data))
		instance_data += inst.name.lower().encode("latin-1") + b"\x00"
		for delta in inst.color_deltas:
			instance_data += struct.pack("<Bbb", delta.d_hue, delta.d_lum, delta.d_sat)
	f.cont_len(len(instance_data))
	f.raw(bytes(instance_data))

	f.cont_len(len(bank.instances))
	for inst, offset in zip(bank.instances, offsets):
		f.version(0)
		f.u32(offset)
		f.u32(inst.color_texture_id)

	return f.getvalue()


def save_hlsbank(path: Union[str, Path, BinaryIO], bank: HLSTextureBank) -> None:
	data = dumps_hlsbank(bank)
	if hasattr(path, "write"):
		path.write(data)
	else:
		Path(path).write_bytes(data)


# ***************************************************************************
# Writer: building new bank entries from a parsed .hlsinfo (pynel.hls_bank_texture_info)
# -- Python port of hls_bank_maker.cpp's addTextToBank() + CHLSColorTexture::setBitmap()/
# addMask(). See docs/hls_texture_bank.md for the algorithm writeup.
# ***************************************************************************

_BLOCK_ALPHA_SIZE = 16  # bytes of raw mixt-block alpha data per block (hls_color_texture.cpp BLOCK_ALPHA_SIZE)
_BLOCK_DXTC_SIZE = 16   # bytes per compressed DXTC5 block (BLOCK_DXTC_SIZE)

_MASK_BLOCK_EMPTY = 0
_MASK_BLOCK_FULL = 1
_MASK_BLOCK_MIXT = 2


def _dds_mip_levels(dds_bytes: bytes):
	"""Split a raw .dds file (as embedded in .hlsinfo's SrcBitmap, see
	hls_bank_texture_info.py) into its DXT5 mip levels. Returns
	(width, height, [level_bytes, ...]) -- level 0 is the base (largest) level.

	Header layout matches nel/tools/3d/s3tc_compressor_lib/s3tc_compressor.h's
	DDS_HEADER/DDS_PIXELFORMAT (same container documented in
	nel/tools/forgery/docs/dds_export.md): magic "DDS " (4 bytes) + 124-byte header,
	body starts at offset 128. Standard DXT block size is 16 bytes for DXT5."""
	if dds_bytes[:4] != b"DDS ":
		raise HlsBankParseError(f"not a DDS file (bad magic: {dds_bytes[:4]!r})")
	# DDS_HEADER starts right after the 4-byte "DDS " magic: dwSize(4) dwFlags(4)
	# dwHeight(4) dwWidth(4) dwLinearSize(4) dwDepth(4) dwMipMapCount(4) ...
	height, width, _linear_size, _depth, mip_count = struct.unpack("<5I", dds_bytes[12:32])
	body = dds_bytes[128:]
	levels = []
	pos = 0
	w, h = width, height
	for _ in range(max(mip_count, 1)):
		size = ((w + 3) // 4) * ((h + 3) // 4) * _BLOCK_DXTC_SIZE
		levels.append(body[pos:pos + size])
		pos += size
		w = max(1, w // 2)
		h = max(1, h // 2)
	return width, height, levels


def _mask_mip_chain(pixels: bytes, width: int, height: int) -> List[bytes]:
	"""Port of CBitmap::buildMipMaps()'s box filter (see
	nel/tools/forgery/ryzom_forgery/dds_export.py's _build_mip_chain, same algorithm),
	applied to a single-channel (luminance) mask instead of RGBA. Only defined for
	power-of-2 dimensions, matching the engine -- non-power-of-2 masks are not
	mipmapped (single level returned), same silent fallback as the original."""
	def is_pow2(n):
		return n > 0 and (n & (n - 1)) == 0

	levels = [pixels]
	if not (is_pow2(width) and is_pow2(height)):
		return levels

	w, h = width, height
	prev = pixels
	while w > 1 or h > 1:
		prev_w, prev_h = w, h
		w = (w + 1) // 2
		h = (h + 1) // 2
		mul_w = prev_w // w
		mul_h = prev_h // h
		level = bytearray(w * h)
		for y in range(h):
			y0 = y * mul_h
			y1 = y0 if mul_h == 1 else y0 + 1
			for x in range(w):
				x0 = x * mul_w
				x1 = x0 if mul_w == 1 else x0 + 1
				c0 = prev[y0 * prev_w + x0]
				c1 = prev[y0 * prev_w + x1]
				c2 = prev[y1 * prev_w + x0]
				c3 = prev[y1 * prev_w + x1]
				level[y * w + x] = (c0 + c1 + c2 + c3 + 2) // 4
		levels.append(bytes(level))
		prev = bytes(level)

	return levels


def build_color_texture(dds_bytes: bytes, mask_pixel_planes) -> ColorTexture:
	"""Port of CHLSColorTexture::setBitmap() + addMask(), one call per mask, called in
	the order masks must end up in (must match the order .hlsinfo's Instances[].mods
	entries are given in). `dds_bytes` is a raw .dds file (HLSBankTextureInfo.src_bitmap_dds
	-- already DXT5-compressed by the real panoply_maker/CS3TCCompressor, reused
	byte-for-byte here, not re-compressed). `mask_pixel_planes` is a list of
	(pixels: bytes, width: int, height: int) for each mask, same order as the masks
	must appear in the resulting ColorTexture.
	"""
	width, height, levels = _dds_mip_levels(dds_bytes)
	pixel_size = sum(len(level) for level in levels)
	num_total_blocks = sum(((w_ + 3) // 4) * ((h_ + 3) // 4) for w_, h_ in _mip_dims(width, height, len(levels)))
	block_to_compress_size = 4 * ((num_total_blocks + 31) // 32)

	texture = bytearray(b"".join(levels))
	texture += bytes(block_to_compress_size)  # zero-initialized "block to recompress" bitfield

	tex = ColorTexture(
		width=width, height=height, num_mipmap=len(levels),
		block_to_compress_index=pixel_size, texture=bytes(texture), masks=[],
	)

	for pixels, mask_width, mask_height in mask_pixel_planes:
		_add_mask(tex, pixels, mask_width, mask_height)

	return tex


def _mip_dims(width: int, height: int, num_mipmap: int):
	w, h = width, height
	for _ in range(num_mipmap):
		yield w, h
		w = max(1, w // 2)
		h = max(1, h // 2)


def _set_bit(data: bytearray, bit_id: int) -> None:
	data[bit_id // 8] |= 1 << (bit_id & 7)


def _add_mask(tex: ColorTexture, pixels: bytes, width: int, height: int, threshold: int = 15) -> None:
	if width != tex.width or height != tex.height:
		raise HlsBankParseError(
			f"mask size {width}x{height} doesn't match color texture size {tex.width}x{tex.height}"
		)

	mip_planes = _mask_mip_chain(pixels, width, height)
	if len(mip_planes) != tex.num_mipmap:
		# non-power-of-2 base texture: engine's setBitmap() still allows this (mmCount
		# may be 1), addMask()'s buildMipMaps() silently falls back to 1 level too --
		# only mismatches when one is pow2 and the other isn't, which shouldn't happen
		# for two bitmaps of the same dimensions.
		raise HlsBankParseError(
			f"mask mip chain has {len(mip_planes)} levels, color texture has {tex.num_mipmap}"
		)

	# classify every 4x4 block of every mip level as EMPTY/FULL/MIXT
	block_classes: List[List[int]] = []
	num_mixt_block = 0
	num_total_block = 0
	for (mm_width, mm_height), plane in zip(_mip_dims(tex.width, tex.height, tex.num_mipmap), mip_planes):
		w_block = (mm_width + 3) // 4
		h_block = (mm_height + 3) // 4
		classes = [_MASK_BLOCK_EMPTY] * (w_block * h_block)
		w = min(mm_width, 4)
		h = min(mm_height, 4)
		for yb in range(h_block):
			for xb in range(w_block):
				accum = 0
				for y in range(h):
					for x in range(w):
						y_pix = yb * 4 + y
						x_pix = xb * 4 + x
						alpha = plane[y_pix * mm_width + x_pix]
						if alpha < threshold:
							alpha = 0
						elif alpha > 255 - threshold:
							alpha = 255
						accum += alpha
				if accum == 0:
					cls = _MASK_BLOCK_EMPTY
				elif accum == w * h * 255:
					cls = _MASK_BLOCK_FULL
				else:
					cls = _MASK_BLOCK_MIXT
					num_mixt_block += 1
				classes[yb * w_block + xb] = cls
		block_classes.append(classes)
		num_total_block += len(classes)

	bit_data_size = 4 * ((num_total_block + 31) // 32)
	new_mask_data_size = num_mixt_block * _BLOCK_ALPHA_SIZE
	full_block_index = new_mask_data_size
	new_mask_data_size += bit_data_size
	mixt_block_index = new_mask_data_size
	new_mask_data_size += bit_data_size

	data = bytearray(new_mask_data_size)

	bit_id = 0
	mixt_block_id = 0
	for (mm_width, mm_height), plane, classes in zip(
		_mip_dims(tex.width, tex.height, tex.num_mipmap), mip_planes, block_classes
	):
		w_block = (mm_width + 3) // 4
		h_block = (mm_height + 3) // 4
		w = min(mm_width, 4)
		h = min(mm_height, 4)
		for yb in range(h_block):
			for xb in range(w_block):
				idx = yb * w_block + xb
				if classes[idx] == _MASK_BLOCK_MIXT:
					dst_off = mixt_block_id * _BLOCK_ALPHA_SIZE
					for y in range(h):
						for x in range(w):
							data[dst_off + y * 4 + x] = plane[(yb * 4 + y) * mm_width + (xb * 4 + x)]
					mixt_block_id += 1
		for cls in classes:
			if cls == _MASK_BLOCK_FULL:
				_set_bit(data, full_block_index * 8 + bit_id)
			elif cls == _MASK_BLOCK_MIXT:
				_set_bit(data, mixt_block_index * 8 + bit_id)
			bit_id += 1

	mask = Mask(full_block_index=full_block_index, mixt_block_index=mixt_block_index, data=bytes(data))
	tex.masks.append(mask)

	# OR this mask's mixt-block bits into the color texture's own trailing
	# "block to recompress" bitfield (CHLSColorTexture::addMask()'s last step)
	texture = bytearray(tex.texture)
	mixt_bits = mask.data[mixt_block_index:mixt_block_index + bit_data_size]
	if len(mixt_bits) != bit_data_size or len(texture) - tex.block_to_compress_index != bit_data_size:
		raise HlsBankParseError("block-to-compress bitfield size mismatch while adding mask")
	for i in range(bit_data_size):
		texture[tex.block_to_compress_index + i] |= mixt_bits[i]
	tex.texture = bytes(texture)


def compress_hls_mod(mod) -> ColorDelta:
	"""Port of hls_bank_maker.cpp's addTextToBank() HLS-delta compression: `mod` is a
	pynel.hls_bank_texture_info.HLSMod (float DHue 0..360 degrees, DLum/DSat -1..1)."""
	import math

	d_hue = math.fmod(mod.d_hue, 360)
	d_hue = math.fmod(d_hue + 360, 360)
	d_hue_u8 = int(math.floor(256 * d_hue / 360)) & 0xFF

	d_lum = max(-127, min(127, 127 * mod.d_lum))
	d_sat = max(-127, min(127, 127 * mod.d_sat))

	return ColorDelta(d_hue=d_hue_u8, d_lum=int(math.floor(d_lum)), d_sat=int(math.floor(d_sat)))


def append_texture_info(bank: HLSTextureBank, info) -> int:
	"""Port of hls_bank_maker.cpp's addTextToBank(): builds a new ColorTexture from a
	parsed .hlsinfo (pynel.hls_bank_texture_info.HLSBankTextureInfo) and appends it +
	all its instances to `bank`, in place. Returns the new color_texture_id.

	Every existing ColorTexture/TextureInstance in `bank` is left byte-for-byte
	untouched -- only new entries are appended (unlike hls_bank_maker, which always
	rebuilds the whole bank from every .hlsinfo in a directory)."""
	mask_planes = [(m.pixels, m.width, m.height) for m in info.masks]
	tex = build_color_texture(info.src_bitmap_dds, mask_planes)
	color_texture_id = len(bank.color_textures)
	bank.color_textures.append(tex)

	for inst in info.instances:
		if len(inst.mods) != len(info.masks):
			raise HlsBankParseError(
				f"instance {inst.name!r} has {len(inst.mods)} mods, expected {len(info.masks)} (one per mask)"
			)
		deltas = [compress_hls_mod(mod) for mod in inst.mods]
		bank.instances.append(TextureInstance(name=inst.name.lower(), color_texture_id=color_texture_id, color_deltas=deltas))

	return color_texture_id


def load_hlsbank(path: Union[str, Path, BinaryIO]) -> HLSTextureBank:
	if hasattr(path, "read"):
		data = path.read()
	else:
		data = Path(path).read_bytes()
	return parse_hlsbank(data)


def _dump(bank: HLSTextureBank) -> None:
	print(f"color textures: {len(bank.color_textures)}")
	print(f"instances: {len(bank.instances)}")
	print()
	for i, tex in enumerate(bank.color_textures):
		insts = [inst for inst in bank.instances if inst.color_texture_id == i]
		names = ", ".join(inst.name for inst in insts) if insts else "(unused)"
		print(f"[{i}] {tex.width}x{tex.height} mipmap={tex.num_mipmap} masks={tex.num_masks()}: {names}")


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Read Ryzom .hlsbank files (HLS-colorisable texture bank)")
	sub = parser.add_subparsers(dest="command", required=True)

	p_dump = sub.add_parser("dump", help="list color textures and item instances in a .hlsbank file")
	p_dump.add_argument("path", type=Path)

	p_add = sub.add_parser("add", help="append new .hlsinfo entries into a .hlsbank file")
	p_add.add_argument("bank", type=Path, help="existing .hlsbank to append to")
	p_add.add_argument("hlsinfo", type=Path, nargs="+", help="one or more .hlsinfo files to append")
	p_add.add_argument("-o", "--output", type=Path, required=True, help="path to write the resulting .hlsbank")

	return parser


def _main() -> None:
	from pynel.hls_bank_texture_info import load_hlsinfo

	args = _build_arg_parser().parse_args()

	if args.command == "dump":
		bank = load_hlsbank(args.path)
		_dump(bank)
	elif args.command == "add":
		bank = load_hlsbank(args.bank)
		for hlsinfo_path in args.hlsinfo:
			info = load_hlsinfo(hlsinfo_path)
			color_texture_id = append_texture_info(bank, info)
			print(f"appended {hlsinfo_path.name}: color texture [{color_texture_id}], {len(info.instances)} instances")
		save_hlsbank(args.output, bank)
		print(f"wrote {args.output}")


if __name__ == "__main__":
	_main()
