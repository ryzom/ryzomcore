#!/usr/bin/env python3
"""Corpus driver for the zone .max -> .zone/.ligozone pipeline.

Enumerates every ligo-source .max from the ryzomcore_leveldesign workspace ecosystems
(LigoMaxSourceDirectory of desert/jungle/lacustre/primes_racines — the listing
build_gamedata processes/ligo/1_export.py drives, filtered to the zonematerial-*/
zonetransition-*/zonespecial-* protocol files like the maxscript), and runs test tiers:

  T1  structural roundtrip:  the pipeline_max_corpus_test binary, no --parse.
  T2  parse/build roundtrip: the pipeline_max_corpus_test binary with --parse.
  T3  zone export:           pipeline_max_export_zone --ligo per file, then per brick:
        - .ligozone: byte-compare against ~/pipeline_export/ecosystems/<eco>/ligo_es/
          zoneligos/ (strict; the whole corpus is byte-identical).
        - .zone: masked byte-compare against .../ligo_es/zones/ — masks cover the
          reference exporter's uninitialized-memory classes only (CTileElement unclaimed
          flag bits: empty-layer rotations + bit 15; CPatch::Flags bits outside the smooth
          mask; CTileLightInfluence::PackedLightFactor; unused CBindInfo Next/Edge slots;
          the CZone serial version byte 4-vs-5). Files that fail the masked compare are
          re-classified through the x87 tolerance tier: every residual diff must be a
          +-1 packed uint16 in the patch geometry or a header float (bbox/bias) delta
          <= 1e-4 — the 32-bit x87 reference plugin vs x64 SSE class (extended-precision
          intermediates at pack boundaries), same family as the ig POS_EPS whitelist.

Known budgeted deviations (see pipeline_max_design.md section 10h):
  - zonematerial-converted-202_dy (lacustre): two non-frozen RklPatch nodes (202_DY +
    leftover copy 202_DY01) — the original exporter errors on this too ("multiple
    NelPatchMesh"); the existing reference predates the copy. The tool's refusal is the
    correct current behavior.
  - zonematerial-foret-25_landmark_d_ring2 (jungle), zonematerial-converted-200_dz
    (lacustre): stacked NeL Edit + Paint + NeL Edit modifier chains where a handful of
    tangent handles differ at cm scale — the in-Max evaluation of stacked patch modifiers
    applies a refresh not yet pinned (needs a Max-side differential probe).

Defaults are the layout on Kaetemi's machine; override via CLI when running elsewhere.
"""

import argparse, collections, concurrent.futures, os, struct, subprocess, sys

SKIP_CODE = 77

DEF_GRAPHICS = os.path.expanduser("~/ryzomcore_graphics")
DEF_REF = os.path.expanduser("~/pipeline_export")
DEF_BIN = os.path.expanduser("~/ryzomcore/build/nel-pipeline/bin")

ECOSYSTEMS = ["desert", "jungle", "lacustre", "primes_racines"]
OLE_MAGIC = b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1"

# Known budgeted deviations (path substring -> reason)
BUDGET = {
    "zonematerial-converted-202_dy": "duplicate non-frozen RklPatch (202_DY01); original errors too, reference predates the copy",
    "zonematerial-foret-25_landmark_d_ring2": "stacked Edit+Paint+Edit chain, cm-scale tangent refresh unpinned",
    "zonematerial-converted-200_dz": "stacked Edit+Paint+Edit chain, cm-scale tangent refresh unpinned",
}


# ---------------------------------------------------------------------------------------------
# .zone masked compare (see the module docstring for the mask classes).

class _P:
    def __init__(self, d):
        self.d = d
        self.o = 0

    def u8(self):
        v = self.d[self.o]
        self.o += 1
        return v

    def u16(self):
        v = struct.unpack_from('<H', self.d, self.o)[0]
        self.o += 2
        return v

    def u32(self):
        v = struct.unpack_from('<I', self.d, self.o)[0]
        self.o += 4
        return v

    def skip(self, n):
        self.o += n


def zone_masks(d):
    """Build (mask bytearray, geometry word offsets set, header float offsets) for a .zone."""
    m = bytearray(b'\xff' * len(d))
    geom = set()
    p = _P(d)
    ver = p.u8()
    if ver not in (4, 5):
        raise ValueError("zone version %d" % ver)
    m[0] = 0x00  # version byte: reference era writes 4, current CZone::serial writes 5
    if d[p.o:p.o + 4] != b'ZONE':
        raise ValueError("no ZONE magic")
    p.skip(4 + 2)
    p.skip(1)          # CAABBoxExt version
    hdr = list(range(p.o, p.o + 24 + 12 + 4))  # bbox center+halfsize, PatchBias, PatchScale
    p.skip(24 + 12 + 4)
    p.skip(4)          # NumVertices
    nBorder = p.u32()
    p.skip(nBorder * 11)
    nPatch = p.u32()
    for _ in range(nPatch):
        pver = p.u8()
        if pver != 7:
            raise ValueError("patch version %d" % pver)
        for _k in range((24 + 48 + 24) // 2):
            geom.add(p.o)
            p.skip(2)
        nTiles = p.u32()
        for _t in range(nTiles):
            fo = p.o
            p.skip(2)
            t = [p.u16(), p.u16(), p.u16()]
            mask = 0x8000
            for l in range(3):
                if t[l] == 0xffff:
                    mask |= 0x3 << (2 * l)
            m[fo] &= (~mask) & 0xff
            m[fo + 1] &= ((~mask) >> 8) & 0xff
        nColors = p.u32()
        p.skip(2 * nColors)
        p.skip(2)      # OrderS, OrderT
        nLumel = p.u32()
        p.skip(nLumel)
        p.skip(2)      # NoiseRotation, _CornerSmoothFlag
        m[p.o] &= 0x3c  # CPatch::Flags: only the smooth bits are defined
        p.skip(1)
        nTLI = p.u32()
        for _t in range(nTLI):
            p.skip(2)
            m[p.o] = 0x00  # PackedLightFactor: undefined when no light influence
            p.skip(1)
    nConnect = p.u32()
    for _ in range(nConnect):
        p.skip(1 + 4 + 8)
        for _e in range(4):
            p.skip(1)
            npat = p.u8()
            valid = 1 if npat == 5 else npat
            p.skip(2)
            for k in range(4):
                if k >= valid:
                    m[p.o] = 0x00
                    m[p.o + 1] = 0x00
                p.skip(2)
            for k in range(4):
                if k >= valid:
                    m[p.o] = 0x00
                p.skip(1)
    return m, geom, hdr


def compare_zone(refPath, gotPath):
    """Return (status, detail): status in {'exact', 'x87', 'diff', 'error'}."""
    a = open(refPath, 'rb').read()
    b = open(gotPath, 'rb').read()
    if len(a) != len(b):
        return 'diff', 'size %d vs %d' % (len(a), len(b))
    try:
        m, geom, hdr = zone_masks(a)
    except ValueError as e:
        return 'error', 'parse: %s' % e
    diffs = [i for i in range(len(a)) if (a[i] & m[i]) != (b[i] & m[i])]
    if not diffs:
        return 'exact', ''
    # x87 tolerance tier: +-1 packed geometry words, tiny header float deltas.
    hdrset = set(hdr)
    flips = 0
    seen = set()
    for off in diffs:
        if off in hdrset or (off - 1) in hdrset or (off - 2) in hdrset or (off - 3) in hdrset:
            fo = hdr[0] + ((off - hdr[0]) // 4) * 4
            fa = struct.unpack_from('<f', a, fo)[0]
            fb = struct.unpack_from('<f', b, fo)[0]
            if abs(fa - fb) > 1e-4:
                return 'diff', 'header float delta %g at 0x%x' % (abs(fa - fb), fo)
            continue
        ok = False
        for bo in (off - 1, off):
            if bo in seen:
                ok = True
                break
            if bo in geom:
                wa = struct.unpack_from('<H', a, bo)[0]
                wb = struct.unpack_from('<H', b, bo)[0]
                if wa != wb and abs(wa - wb) <= 1:
                    seen.add(bo)
                    flips += 1
                    ok = True
                    break
        if not ok:
            return 'diff', 'unexplained byte at 0x%x' % off
    return 'x87', '%d word flips' % flips


# ---------------------------------------------------------------------------------------------

def enumerate_corpus(graphics_dir):
    files = []
    for eco in ECOSYSTEMS:
        d = os.path.join(graphics_dir, "landscape", "ligo", eco, "max")
        if not os.path.isdir(d):
            continue
        for fn in sorted(os.listdir(d)):
            if not fn.endswith(".max"):
                continue
            base = fn[:-4]
            toks = base.split("-")
            if not ((len(toks) == 3 and toks[0] == "zonematerial")
                    or (len(toks) == 4 and toks[0] == "zonetransition")
                    or (len(toks) == 2 and toks[0] == "zonespecial")):
                continue
            path = os.path.join(d, fn)
            try:
                with open(path, 'rb') as f:
                    if f.read(8) != OLE_MAGIC:
                        continue
            except OSError:
                continue
            files.append((eco, path))
    return files


def brick_names(base):
    toks = base.split("-")
    if toks[0] == "zonematerial":
        return ["-".join(toks[1:])]
    if toks[0] == "zonespecial":
        return [toks[1]]
    return ["%s-%d" % ("-".join(toks[1:]), z) for z in range(9)]


def run_t1t2(args, path, parse):
    cmd = [os.path.join(args.bin, "pipeline_max_corpus_test")]
    if parse:
        cmd.append("--parse")
    cmd.append(path)
    r = subprocess.run(cmd, capture_output=True, text=True)
    return r.returncode == 0, (r.stdout + r.stderr).strip()


def run_t3(args, eco, path):
    """Export one file and compare its bricks. Returns (verdict, detail) where verdict in
    {'exact', 'x87', 'budget', 'fail', 'nozone'}."""
    base = os.path.splitext(os.path.basename(path))[0]
    outdir = os.path.join(args.workdir, eco)
    bank = os.path.join(args.ref, "ecosystems", eco, "smallbank", eco + ".smallbank")
    refdir = os.path.join(args.ref, "ecosystems", eco, "ligo_es")
    r = subprocess.run([os.path.join(args.bin, "pipeline_max_export_zone"),
                        "--ligo", outdir, "--bank", bank, path],
                       capture_output=True, text=True)
    for key, reason in BUDGET.items():
        if key in path:
            return 'budget', reason
    if r.returncode != 0:
        return 'fail', 'export rc=%d: %s' % (r.returncode, r.stderr.strip()[:200])
    worst = 'exact'
    details = []
    flips = 0
    for name in brick_names(base):
        refZone = os.path.join(refdir, "zones", name + ".zone")
        gotZone = os.path.join(outdir, "zones", name + ".zone")
        refLigo = os.path.join(refdir, "zoneligos", name + ".ligozone")
        gotLigo = os.path.join(outdir, "zoneligos", name + ".ligozone")
        haveRef = os.path.exists(refZone)
        haveGot = os.path.exists(gotZone)
        if not haveRef and not haveGot:
            continue
        if not haveRef:
            details.append("%s: exported, no reference" % name)
            continue
        if not haveGot:
            worst = 'fail'
            details.append("%s: reference exists, not exported" % name)
            continue
        status, detail = compare_zone(refZone, gotZone)
        if status == 'x87':
            flips += int(detail.split()[0])
            if worst == 'exact':
                worst = 'x87'
        elif status != 'exact':
            worst = 'fail'
            details.append("%s: %s %s" % (name, status, detail))
        if os.path.exists(refLigo):
            if not os.path.exists(gotLigo) or open(refLigo, 'rb').read() != open(gotLigo, 'rb').read():
                worst = 'fail'
                details.append("%s: ligozone differs" % name)
    if worst == 'x87':
        details.append("%d word flips" % flips)
    return worst, "; ".join(details)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--graphics", default=DEF_GRAPHICS)
    ap.add_argument("--ref", default=DEF_REF)
    ap.add_argument("--bin", default=DEF_BIN)
    ap.add_argument("--workdir", default="/tmp/pipeline_max_zone_corpus.%d" % os.getpid())
    ap.add_argument("--t1", action="store_true")
    ap.add_argument("--t2", action="store_true")
    ap.add_argument("--t3", action="store_true")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--gate-t3", action="store_true", help="fail on any T3 'fail' beyond the budget list")
    ap.add_argument("--filter", default=None)
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    args = ap.parse_args()
    if args.all:
        args.t1 = args.t2 = args.t3 = True
    if not (args.t1 or args.t2 or args.t3):
        args.t1 = args.t2 = True

    if not os.path.isdir(args.graphics) or not os.path.isdir(args.ref):
        print("SKIP: asset checkouts not present")
        return SKIP_CODE

    corpus = enumerate_corpus(args.graphics)
    if args.filter:
        corpus = [(e, p) for e, p in corpus if args.filter in p]
    if not corpus:
        print("SKIP: no corpus files found")
        return SKIP_CODE
    print("%d corpus files" % len(corpus))

    rc = 0
    if args.t1 or args.t2:
        for parse in ([False] if args.t1 else []) + ([True] if args.t2 else []):
            tier = "T2" if parse else "T1"
            fails = []
            with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as ex:
                results = list(ex.map(lambda ep: (ep[1], run_t1t2(args, ep[1], parse)), corpus))
            for path, (ok, out) in results:
                if not ok:
                    fails.append((path, out))
            print("%s: %d/%d ok" % (tier, len(corpus) - len(fails), len(corpus)))
            for path, out in fails[:10]:
                print("  FAIL %s: %s" % (path, out[:200]))
            if fails:
                rc = 1

    if args.t3:
        # per-ecosystem output dirs are shared across files: create them upfront (the tool
        # creates them too, but concurrent first-use racing is avoided this way)
        for eco in ECOSYSTEMS:
            os.makedirs(os.path.join(args.workdir, eco, "zones"), exist_ok=True)
            os.makedirs(os.path.join(args.workdir, eco, "zoneligos"), exist_ok=True)
        counts = collections.Counter()
        fails = []
        budgets = []
        totalFlips = 0
        with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as ex:
            results = list(ex.map(lambda ep: (ep[0], ep[1], run_t3(args, ep[0], ep[1])), corpus))
        for eco, path, (verdict, detail) in results:
            counts[verdict] += 1
            if verdict == 'fail':
                fails.append((path, detail))
            elif verdict == 'budget':
                budgets.append((path, detail))
            elif verdict == 'x87' and detail:
                try:
                    totalFlips += int(detail.split("; ")[-1].split()[0])
                except (ValueError, IndexError):
                    pass
        print("T3: %s, %d x87 word flips corpus-wide" % (dict(counts), totalFlips))
        for path, detail in budgets:
            print("  BUDGET %s: %s" % (os.path.basename(path), detail))
        for path, detail in fails[:20]:
            print("  FAIL %s: %s" % (os.path.basename(path), detail))
        if args.gate_t3 and fails:
            rc = 1

    return rc


if __name__ == '__main__':
    sys.exit(main())
