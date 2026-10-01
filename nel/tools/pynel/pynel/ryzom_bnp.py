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

"""Reader/writer for Ryzom/NeL .bnp archives (NLMISC::CBigFile).

Format reverse-engineered from nel/include/nel/misc/big_file.h and
nel/src/misc/big_file.cpp (CBigFile::BNP::readHeader/appendHeader/appendFile),
cross-checked against the reference CLI tool nel/tools/misc/bnp_make/main.cpp
and empirically verified byte-for-byte against the real test archive
nel/tools/nel_unit_test/ut_misc_files/files.bnp.

On-disk layout (all little-endian, no compression):

	[raw data of file 0][raw data of file 1]...[raw data of file N-1]
	uint32 nNbFile
	for each file:
		uint8  nameLen
		char   name[nameLen]     -- flat file name, no path, not nul-terminated
		uint32 size               -- raw (uncompressed) size
		uint32 pos                -- absolute offset from the start of the archive
	uint32 offsetFromBeginning    -- always the last 4 bytes: absolute offset of "nNbFile"

Notes:
  - No sub-folder hierarchy: only base file names are stored (CBigFile::BNP::
    appendFile keeps only CFile::getFilename(filename)), so names must be
    unique within an archive regardless of source sub-directory.
  - No compression, no CRC, no timestamps.
  - Name lookup is case-insensitive at runtime (CBigFile lowercases names for
    its internal binary-search table). This library goes one step further and
    normalizes every entry name to lowercase on both read and write (list(),
    extract_all(), pack_directory(), add_file()...), rather than just
    comparing case-insensitively while preserving whatever case is on disk --
    real archives mix casing (e.g. "ZO_flag_AS.TGA" vs "zo-toit2.tga"), and
    it's one less thing to worry about when cross-referencing names against
    other tools (e.g. Ryzom Forgery) that also normalize to lowercase.

Usage:
	from ryzom_bnp import BnpReader, pack_directory, add_file, remove_file
	bnp = BnpReader("data.bnp")
	for entry in bnp.list():
		print(entry.name, entry.size)
	data = bnp.read_file("myshape.shape")
"""

import argparse
import os
import struct
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import BinaryIO, List, Union

MAX_NAME_LEN = 255


class BnpError(Exception):
	pass


@dataclass
class BnpEntry:
	name: str
	size: int
	pos: int  # absolute offset from the start of the archive


def _encode_name(name: str) -> bytes:
	raw = name.lower().encode("latin-1")
	if len(raw) > MAX_NAME_LEN:
		raise BnpError(f"file name too long for .bnp (max {MAX_NAME_LEN} bytes): {name!r}")
	return raw


def _write_table(out: BinaryIO, entries: List[BnpEntry]) -> None:
	offset_from_beginning = out.tell()
	out.write(struct.pack("<I", len(entries)))
	for entry in entries:
		raw_name = _encode_name(entry.name)
		out.write(struct.pack("<B", len(raw_name)))
		out.write(raw_name)
		out.write(struct.pack("<II", entry.size, entry.pos))
	out.write(struct.pack("<I", offset_from_beginning))


def _read_table(fh: BinaryIO) -> List[BnpEntry]:
	fh.seek(-4, os.SEEK_END)
	(offset_from_beginning,) = struct.unpack("<I", fh.read(4))
	fh.seek(offset_from_beginning, os.SEEK_SET)
	(n,) = struct.unpack("<I", fh.read(4))
	entries = []
	for _ in range(n):
		name_len = fh.read(1)[0]
		name = fh.read(name_len).decode("latin-1").lower()
		size, pos = struct.unpack("<II", fh.read(8))
		entries.append(BnpEntry(name=name, size=size, pos=pos))
	return entries


class BnpReader:
	"""Random-access reader: only the file table is loaded eagerly, file
	contents are read on demand via seek+read (archives can be large)."""

	def __init__(self, path: Union[str, Path]):
		self.path = Path(path)
		self._entries: List[BnpEntry] = self._load_entries()

	def _load_entries(self) -> List[BnpEntry]:
		with open(self.path, "rb") as fh:
			try:
				return _read_table(fh)
			except (struct.error, OSError) as exc:
				raise BnpError(f"cannot read .bnp table from {self.path}: {exc}")

	def list(self) -> List[BnpEntry]:
		return list(self._entries)

	def find(self, name: str) -> BnpEntry:
		for entry in self._entries:
			if entry.name.lower() == name.lower():
				return entry
		raise BnpError(f"no file named {name!r} in {self.path}")

	def read_file(self, name: str) -> bytes:
		entry = self.find(name)
		with open(self.path, "rb") as fh:
			fh.seek(entry.pos)
			return fh.read(entry.size)

	def extract_all(self, out_dir: Union[str, Path]) -> None:
		out_dir = Path(out_dir)
		out_dir.mkdir(parents=True, exist_ok=True)
		with open(self.path, "rb") as fh:
			for entry in self._entries:
				fh.seek(entry.pos)
				data = fh.read(entry.size)
				(out_dir / entry.name).write_bytes(data)


def pack_directory(src_dir: Union[str, Path], bnp_path: Union[str, Path]) -> None:
	"""Packs every file found recursively under src_dir into a new .bnp.

	Mirrors bnp_make --pack: only base file names are kept (the .bnp format
	has no sub-folder concept), sorted case-insensitively for a deterministic
	archive. Raises if two files share the same base name.
	"""
	src_dir = Path(src_dir)
	files = sorted((p for p in src_dir.rglob("*") if p.is_file()), key=lambda p: p.name.lower())

	seen = {}
	for p in files:
		key = p.name.lower()
		if key in seen:
			raise BnpError(f"duplicate file name across sub-directories: {p.name!r} ({seen[key]} and {p})")
		seen[key] = p

	bnp_path = Path(bnp_path)
	with open(bnp_path, "wb") as out:
		entries = []
		for p in files:
			data = p.read_bytes()
			pos = out.tell()
			out.write(data)
			entries.append(BnpEntry(name=p.name, size=len(data), pos=pos))
		_write_table(out, entries)


def add_file(bnp_path: Union[str, Path], file_path: Union[str, Path], as_name: str = None) -> None:
	"""Appends a single file to an existing .bnp (rebuilds data + table)."""
	bnp_path = Path(bnp_path)
	file_path = Path(file_path)
	name = as_name or file_path.name

	reader = BnpReader(bnp_path)
	if any(e.name.lower() == name.lower() for e in reader.list()):
		raise BnpError(f"a file named {name!r} already exists in {bnp_path}; remove it first or use a different name")

	new_data = file_path.read_bytes()
	tmp_path = bnp_path.with_name(bnp_path.name + ".tmp")
	with open(tmp_path, "wb") as out, open(bnp_path, "rb") as src:
		entries = []
		for e in reader.list():
			src.seek(e.pos)
			data = src.read(e.size)
			pos = out.tell()
			out.write(data)
			entries.append(BnpEntry(name=e.name, size=e.size, pos=pos))
		pos = out.tell()
		out.write(new_data)
		entries.append(BnpEntry(name=name, size=len(new_data), pos=pos))
		_write_table(out, entries)
	tmp_path.replace(bnp_path)


def remove_file(bnp_path: Union[str, Path], name: str) -> None:
	"""Removes a single file from an existing .bnp (rebuilds data + table)."""
	bnp_path = Path(bnp_path)
	reader = BnpReader(bnp_path)
	remaining = [e for e in reader.list() if e.name.lower() != name.lower()]
	if len(remaining) == len(reader.list()):
		raise BnpError(f"no file named {name!r} in {bnp_path}")

	tmp_path = bnp_path.with_name(bnp_path.name + ".tmp")
	with open(tmp_path, "wb") as out, open(bnp_path, "rb") as src:
		entries = []
		for e in remaining:
			src.seek(e.pos)
			data = src.read(e.size)
			pos = out.tell()
			out.write(data)
			entries.append(BnpEntry(name=e.name, size=e.size, pos=pos))
		_write_table(out, entries)
	tmp_path.replace(bnp_path)


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def _cmd_list(args: argparse.Namespace) -> None:
	reader = BnpReader(args.path)
	for entry in reader.list():
		print(f"{entry.pos:>10} {entry.size:>10} {entry.name}")


def _cmd_extract(args: argparse.Namespace) -> None:
	reader = BnpReader(args.path)

	if args.name is not None:
		data = reader.read_file(args.name)
		if args.stdout:
			sys.stdout.buffer.write(data)
			return
		out_dir = args.output or Path(".")
		out_dir = Path(out_dir)
		if out_dir.exists() and out_dir.is_dir():
			dest = out_dir / args.name
		else:
			dest = out_dir
		dest.parent.mkdir(parents=True, exist_ok=True)
		dest.write_bytes(data)
		print(f"wrote {dest}")
		return

	if args.stdout:
		raise SystemExit("--stdout can only be used when extracting a single named file")
	out_dir = args.output or Path(".")
	reader.extract_all(out_dir)
	print(f"extracted {len(reader.list())} files to {out_dir}")


def _cmd_pack(args: argparse.Namespace) -> None:
	pack_directory(args.directory, args.path)
	reader = BnpReader(args.path)
	print(f"packed {len(reader.list())} files into {args.path}")


def _cmd_add(args: argparse.Namespace) -> None:
	add_file(args.path, args.file, as_name=args.as_name)
	print(f"added {args.as_name or Path(args.file).name!r} to {args.path}")


def _cmd_remove(args: argparse.Namespace) -> None:
	remove_file(args.path, args.name)
	print(f"removed {args.name!r} from {args.path}")


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Read and edit Ryzom .bnp archives")
	sub = parser.add_subparsers(dest="command", required=True)

	p_list = sub.add_parser("list", help="list the files contained in a .bnp")
	p_list.add_argument("path", type=Path)
	p_list.set_defaults(func=_cmd_list)

	p_extract = sub.add_parser("extract", help="extract one file or the whole archive")
	p_extract.add_argument("path", type=Path)
	p_extract.add_argument("name", nargs="?", help="file to extract; omit to extract everything")
	p_extract.add_argument("-o", "--output", type=Path, help="output directory (or file path when --name is given)")
	p_extract.add_argument("--stdout", action="store_true", help="write the single extracted file to stdout")
	p_extract.set_defaults(func=_cmd_extract)

	p_pack = sub.add_parser("pack", help="pack a directory (recursively) into a new .bnp")
	p_pack.add_argument("directory", type=Path)
	p_pack.add_argument("path", type=Path)
	p_pack.set_defaults(func=_cmd_pack)

	p_add = sub.add_parser("add", help="add a single file to an existing .bnp")
	p_add.add_argument("path", type=Path)
	p_add.add_argument("file", type=Path)
	p_add.add_argument("--as", dest="as_name", help="name to store the file under (default: its base name)")
	p_add.set_defaults(func=_cmd_add)

	p_remove = sub.add_parser("remove", help="remove a file from an existing .bnp")
	p_remove.add_argument("path", type=Path)
	p_remove.add_argument("name")
	p_remove.set_defaults(func=_cmd_remove)

	return parser


def _main() -> None:
	args = _build_arg_parser().parse_args()
	try:
		args.func(args)
	except BnpError as exc:
		raise SystemExit(str(exc))


if __name__ == "__main__":
	_main()
