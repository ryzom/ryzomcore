#!/usr/bin/env python3
"""Offline checker for the Map Extender drop-in plugin validation run.

Consumes the manifest written by gen_mapext_plugin_validate.ms (run in 3ds Max 2010 with the
rebuilt mapext198m3.dlm installed) and verifies, per exporting node, that the plugin's
AT-POSITION output (the map channel evaluated with every modifier above the Map Extender
disabled) is IDENTICAL to the LocalModData cache stored in the .max — i.e. the drop-in replays
the stored map verbatim, which is the corpus-proven semantic of the original
(design doc §10z-quatorze/§10z-quinze/§10z-seize; max_geometry_formats Part P).

Cache→node pairing is by (nFaces, nVerts) match among the file's Map Extender mod-apps (node
resolution through the derived-object wiring isn't needed: sizes are unique per node in
practice, and an ambiguous match still requires SOME cache to be bit-equal).

Usage: mapext_plugin_check.py <manifest.txt> [--graphics ~/ryzomcore_graphics]
"""

import argparse
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from maxole import OLEFile


def children(data, pos, end):
    out = []
    while pos + 6 <= end:
        cid, size = struct.unpack_from('<HI', data, pos)
        hdr = 6
        cont = bool(size & 0x80000000)
        size &= 0x7FFFFFFF
        if size == 0:
            size2, = struct.unpack_from('<Q', data, pos + 6)
            cont = bool(size2 >> 63)
            size = size2 & ((1 << 63) - 1)
            hdr = 14
        out.append((cid, pos + hdr, pos + size, cont))
        pos += size
    return out


def tolerant(data, pos, end):
    """Parse a leaf 0x2512 payload as a chunk stream; [] when it isn't one."""
    out = []
    p = pos
    while p + 6 <= end:
        cid, size = struct.unpack_from('<HI', data, p)
        cont = bool(size & 0x80000000)
        size &= 0x7FFFFFFF
        if size < 6 or p + size > end:
            return []
        out.append((cid, p + 6, p + size, cont))
        p += size
    return out if p == end else []


def mapext_caches(path):
    """Every Map Extender LocalModData cache in the file: (channel, nVerts, uvw, nFaces, corners)."""
    f = OLEFile(path)
    scene = f.stream('Scene')
    top = children(scene, 0, len(scene))
    objs = children(scene, top[0][1], top[0][2])
    caches = []
    for cid, s, e, cont in objs:
        for c2, s2, e2, cont2 in children(scene, s, e):
            if c2 != 0x2500:
                continue
            for c3, s3, e3, cont3 in children(scene, s2, e2):
                if c3 != 0x2512:
                    continue
                kids = tolerant(scene, s3, e3) if not cont3 else children(scene, s3, e3)
                d = {k[0]: (k[1], k[2]) for k in kids}
                if 0x03e9 not in d or 0x03e8 not in d or 0x03ea not in d or 0x03eb not in d:
                    continue
                nv, = struct.unpack_from('<I', scene, d[0x03e8][0])
                nf, = struct.unpack_from('<I', scene, d[0x03ea][0])
                ch = 1
                if 0x03f3 in d:
                    ch, = struct.unpack_from('<I', scene, d[0x03f3][0])
                uvw = list(struct.unpack_from('<%df' % (nv * 3), scene, d[0x03e9][0]))
                fcs = list(struct.unpack_from('<%dI' % (nf * 3), scene, d[0x03eb][0]))
                caches.append((ch, nv, uvw, nf, fcs))
    return caches


def parse_manifest(path):
    """Yield (file, node, pluginstate, atpos) where atpos = {channel: (nv, uvw floats, nf,
    corners)}. dumpChannel emits EVERY supported map channel (1 and 2) — an earlier revision
    kept only the last CHAN record, so any node whose mesh also carries a second channel
    compared the wrong (pass-through) channel against the cache and spuriously failed."""
    cur_file = None
    cur_node = None
    state = None
    at = None      # dict channel -> [nv, uvw, nf, corners]
    cur = None     # channel being filled
    results = []

    def push():
        if cur_node is not None:
            results.append((cur_file, cur_node, state, at))

    for line in open(path):
        t = line.split()
        if not t:
            continue
        if t[0] == 'FILE':
            push()
            cur_file, cur_node, state, at, cur = t[1], None, None, None, None
        elif t[0] == 'NODE':
            push()
            cur_node, state, at, cur = t[1], None, None, None
            if len(t) > 2 and t[2] == 'MISSING':
                state = 'node-missing'
        elif t[0] == 'PLUGINSTATE':
            state = ' '.join(t[1:])
        elif t[0] == 'ATPOS' and t[1] == 'CHAN':
            if at is None:
                at = {}
            cur = int(t[2])
            at[cur] = (int(t[4]), [], int(t[6]), [])
        elif t[0] == 'ATPOS' and t[1] == 'MV' and at is not None and cur is not None:
            at[cur][1].extend([float(t[3]), float(t[4]), float(t[5])])
        elif t[0] == 'ATPOS' and t[1] == 'MF' and at is not None and cur is not None:
            # MAXScript map-face indices are 1-based; the cache is 0-based.
            at[cur][3].extend([int(t[3]) - 1, int(t[4]) - 1, int(t[5]) - 1])
        elif t[0] == 'FULL':
            cur = None
    push()
    return results


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('manifest')
    ap.add_argument('--graphics', default=os.path.expanduser('~/ryzomcore_graphics'))
    ap.add_argument('--tol', type=float, default=1e-6,
                    help='per-component UV tolerance (manifest floats print at 9 sig digits)')
    args = ap.parse_args()

    ok = fail = skip = 0
    cache_cache = {}
    for relfile, node, state, at in parse_manifest(args.manifest):
        tag = '%s:%s' % (relfile, node)
        if state in ('node-missing',) or state is None or not state.startswith('ok'):
            print('SKIP %s (%s)' % (tag, state))
            skip += 1
            continue
        if not at:
            print('FAIL %s: plugin resolved but produced no map channel' % tag)
            fail += 1
            continue
        path = os.path.join(args.graphics, relfile.replace('\\', '/'))
        if path not in cache_cache:
            cache_cache[path] = mapext_caches(path)
        matched = False
        why = 'no cache matches any dumped channel (%s)' % ', '.join(
            'ch%d %dv/%df' % (c, v[0], v[2]) for c, v in sorted(at.items()))
        for cch, cnv, cuvw, cnf, cfcs in cache_cache[path]:
            if cch not in at:
                continue
            nv, uvw, nf, fcs = at[cch]
            if cnf != nf or cnv != nv:
                continue
            if cfcs != fcs:
                why = 'size-matched cache corner indices differ'
                continue
            bad = sum(1 for a, b in zip(uvw, cuvw) if abs(a - b) > args.tol)
            if bad:
                why = 'size-matched cache has %d UV components off > %g' % (bad, args.tol)
                continue
            matched = True
            break
        if matched:
            ok += 1
        else:
            print('FAIL %s: %s' % (tag, why))
            fail += 1
    print('mapext plugin check: %d ok, %d fail, %d skip' % (ok, fail, skip))
    return 1 if fail else 0


if __name__ == '__main__':
    sys.exit(main())
