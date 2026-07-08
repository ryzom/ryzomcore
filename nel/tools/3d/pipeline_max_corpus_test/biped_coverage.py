#!/usr/bin/env python3
"""Biped (0x9155) system-object chunk coverage: how much of the biped rig format do the headless
decoders (pipeline_max_export_common/biped_rig.cpp + pipeline_max_export_anim/biped_anim.cpp)
actually read, vs. what still rides through as raw pass-through?

Runs `pipeline_max_export_anim --dump-rig` over the biped anim-source corpus, collects every
distinct chunk id that appears on a 0x9155 object with its occurrence count and total byte weight,
and classifies each as DECODED (a chunk id our decode reads) or UNKNOWN. Reports coverage by
distinct-id count and by byte weight. Read-only, no reference data needed.

Usage: python3 biped_coverage.py [--sample N] [--bin DIR] [--graphics DIR] [--workspace DIR]
"""
import argparse, os, re, subprocess, sys
from collections import defaultdict

# Chunk ids the headless biped decode reads off the 0x9155 system object (kept in sync with the
# chunk-reader call sites in biped_rig.cpp / biped_anim.cpp; see the design doc §10c/§10e/§10o).
DECODED = set(int(x, 16) for x in """
0x000b 0x000c 0x000d 0x000e 0x000f 0x0010 0x0012 0x0013 0x0014
0x0064 0x0065 0x0066 0x0067 0x0068 0x0069 0x006a 0x006c 0x006d 0x006e
0x0100 0x0104 0x0115 0x0117 0x0260
0x012c 0x012d 0x012e 0x012f 0x0130 0x0131 0x0132 0x0133 0x0134 0x0135
0x0136 0x0137 0x0138 0x0139 0x013a 0x013b 0x013c 0x013d 0x013e 0x013f
0x0142 0x0143 0x0147 0x0148 0x0149 0x014a
""".split())

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

def main():
    home = os.path.expanduser("~")
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--bin", default=os.path.join(home, "ryzomcore/build/nel-pipeline/bin"))
    ap.add_argument("--graphics", default=os.path.join(home, "ryzomcore_graphics"))
    ap.add_argument("--workspace", default=os.path.join(home, "ryzomcore_leveldesign/workspace"))
    ap.add_argument("--sample", type=int, default=0, help="cap at N biped files (0 = all)")
    args = ap.parse_args()

    from anim_corpus import enumerate_corpus, is_biped, is_git_lfs_stub
    dump = os.path.join(args.bin, "pipeline_max_export_anim")
    if not os.path.isfile(dump):
        print(f"SKIP: missing {dump}"); sys.exit(77)
    if not os.path.isdir(args.graphics):
        print(f"SKIP: no graphics checkout"); sys.exit(77)

    files = [f for f in enumerate_corpus(args.graphics, args.workspace)
             if not is_git_lfs_stub(f[3]) and is_biped(f[3])]
    if args.sample and len(files) > args.sample:
        files = files[::max(1, len(files) // args.sample)][:args.sample]
    print(f"biped coverage over {len(files)} files")

    occ = defaultdict(int)      # chunk id -> number of (file,object) it appears on
    byts = defaultdict(int)     # chunk id -> total bytes across all appearances
    chunk_re = re.compile(r'chunk 0x([0-9a-f]{4}) bytes=(\d+)')
    n = 0
    for _, _, name, full in files:
        try:
            r = subprocess.run([dump, "--dump-rig", full], capture_output=True, text=True, timeout=120)
        except Exception:
            continue
        for m in chunk_re.finditer(r.stdout):
            cid = int(m.group(1), 16); occ[cid] += 1; byts[cid] += int(m.group(2))
        n += 1
        if n % 250 == 0: print(f"  ...{n}", file=sys.stderr)

    ids = sorted(occ)
    dec_ids = [c for c in ids if c in DECODED]
    unk_ids = [c for c in ids if c not in DECODED]
    tot_bytes = sum(byts.values()); dec_bytes = sum(byts[c] for c in dec_ids)
    print(f"\ndistinct 0x9155 chunk ids seen: {len(ids)}")
    print(f"  decoded: {len(dec_ids)}  ({100.0*len(dec_ids)/max(1,len(ids)):.1f}% by id count)")
    print(f"  unknown: {len(unk_ids)}")
    print(f"byte weight: decoded {dec_bytes}/{tot_bytes} = {100.0*dec_bytes/max(1,tot_bytes):.1f}%")
    print(f"\ntop unknown chunks by byte weight (id: files, total bytes):")
    for c in sorted(unk_ids, key=lambda c: byts[c], reverse=True)[:25]:
        print(f"  0x{c:04x}: {occ[c]:5d} files, {byts[c]:10d} bytes")

if __name__ == "__main__":
    main()
