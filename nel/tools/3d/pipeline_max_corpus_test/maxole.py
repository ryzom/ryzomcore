#!/usr/bin/env python3
"""Minimal pure-Python OLE2 compound file reader + .max chunk-stream walker.

Survey/harness tooling: lets the corpus drivers peek into .max files (stream listing, Scene
chunk structure, ClassDirectory3) without the native pipeline_max binaries. Handles both
512-byte (v3) and 4096-byte (v4) sector formats; sector offset = (sector + 1) * sector_size
per MS-CFB (the header occupies the first sector slot).

walk_chunks() yields the Max chunk-stream structure: (id, isContainer, payloadOffset,
payloadSize, depth), understanding the 64-bit chunk header extension (size32 == 0).
"""
import struct

class OLEError(Exception):
    pass

class OLEFile:
    MAGIC = b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1"

    def __init__(self, path):
        with open(path, "rb") as f:
            self.data = f.read()
        if self.data[:8] != self.MAGIC:
            raise OLEError("not OLE")
        (self.sect_shift,) = struct.unpack_from("<H", self.data, 30)
        (self.mini_shift,) = struct.unpack_from("<H", self.data, 32)
        self.sect_size = 1 << self.sect_shift
        self.mini_size = 1 << self.mini_shift
        (self.num_fat,) = struct.unpack_from("<I", self.data, 44)
        (self.dir_start,) = struct.unpack_from("<I", self.data, 48)
        (self.mini_cutoff,) = struct.unpack_from("<I", self.data, 56)
        (self.minifat_start,) = struct.unpack_from("<I", self.data, 60)
        (self.num_minifat,) = struct.unpack_from("<I", self.data, 64)
        (self.difat_start,) = struct.unpack_from("<I", self.data, 68)
        (self.num_difat,) = struct.unpack_from("<I", self.data, 72)
        # DIFAT: first 109 entries in header
        difat = list(struct.unpack_from("<109I", self.data, 76))
        sect = self.difat_start
        for _ in range(self.num_difat):
            if sect in (0xFFFFFFFE, 0xFFFFFFFF):
                break
            off = (sect + 1) * self.sect_size
            ents = struct.unpack_from("<%dI" % (self.sect_size // 4), self.data, off)
            difat.extend(ents[:-1])
            sect = ents[-1]
        # FAT
        self.fat = []
        for s in difat:
            if s in (0xFFFFFFFE, 0xFFFFFFFF):
                continue
            off = (s + 1) * self.sect_size
            self.fat.extend(struct.unpack_from("<%dI" % (self.sect_size // 4), self.data, off))
        # Directory
        dirdata = self._read_chain(self.dir_start)
        self.entries = []
        for i in range(0, len(dirdata), 128):
            e = dirdata[i:i + 128]
            if len(e) < 128:
                break
            (nlen,) = struct.unpack_from("<H", e, 64)
            if nlen < 2 or nlen > 64:
                continue
            typ = e[66]
            if typ not in (1, 2, 5):
                continue
            name = e[: nlen - 2].decode("utf-16-le", "replace")
            (start,) = struct.unpack_from("<I", e, 116)
            (size,) = struct.unpack_from("<Q", e, 120)
            self.entries.append((name, typ, start, size))
        # MiniFAT + mini stream (root entry start)
        self.minifat = []
        s = self.minifat_start
        cnt = 0
        while s not in (0xFFFFFFFE, 0xFFFFFFFF) and cnt < self.num_minifat:
            off = (s + 1) * self.sect_size
            self.minifat.extend(struct.unpack_from("<%dI" % (self.sect_size // 4), self.data, off))
            s = self.fat[s]
            cnt += 1
        root = [e for e in self.entries if e[1] == 5][0]
        self.ministream = self._read_chain(root[2], root[3])

    def _read_chain(self, start, size=None):
        out = bytearray()
        s = start
        guard = len(self.fat) + 2
        while s < 0xFFFFFFFC and guard > 0:
            off = (s + 1) * self.sect_size
            out += self.data[off:off + self.sect_size]
            if s >= len(self.fat):
                break
            s = self.fat[s]
            guard -= 1
        if size is not None:
            out = out[:size]
        return bytes(out)

    def _read_minichain(self, start, size):
        out = bytearray()
        s = start
        guard = len(self.minifat) + 2
        while s < 0xFFFFFFFC and guard > 0:
            off = s * self.mini_size
            out += self.ministream[off:off + self.mini_size]
            if s >= len(self.minifat):
                break
            s = self.minifat[s]
            guard -= 1
        return bytes(out[:size])

    def stream(self, name):
        for n, typ, start, size in self.entries:
            if n == name and typ == 2:
                if size < self.mini_cutoff:
                    return self._read_minichain(start, size)
                return self._read_chain(start, size)
        return None


def walk_chunks(data, pos, end, depth=0):
    """Yield (id, container, payload_offset, payload_size, depth)."""
    while pos + 6 <= end:
        (cid, size) = struct.unpack_from("<HI", data, pos)
        hdr = 6
        if size == 0:
            (size64,) = struct.unpack_from("<Q", data, pos + 6)
            hdr = 14
            cont = bool(size64 & 0x8000000000000000)
            size = (size64 & 0x7FFFFFFFFFFFFFFF)
        else:
            cont = bool(size & 0x80000000)
            size &= 0x7FFFFFFF
        if size < hdr or pos + size > end:
            return
        yield (cid, cont, pos + hdr, size - hdr, depth)
        if cont:
            for t in walk_chunks(data, pos + hdr, pos + size, depth + 1):
                yield t
        pos += size
