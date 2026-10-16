#!/usr/bin/env python3
"""Analyze the output of gen_biped_fields_probe.ms: --diff-rig every single-
variable case against f00_baseline.max and produce a field->chunk map, flagging
which changed chunks are still UNDECODED by the headless biped decode.

Run gen_biped_fields_probe.ms in Max 9 first (it writes f00_baseline.max +
fNN_<field>.max into its output folder), copy that folder somewhere reachable,
then:  python3 gen_biped_fields_diff.py <folder> [--bin DIR]

Output:
  * per-case: the chunk ids that changed vs baseline, each tagged DECODED/UNKNOWN
  * inverse map: chunk id -> which field(s) move it (the decode targets)
"""
import argparse, os, re, subprocess, sys
from collections import defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

def main():
    home = os.path.expanduser("~")
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("folder", help="folder holding f00_baseline.max + fNN_*.max from the probe")
    ap.add_argument("--bin", default=os.path.join(home, "ryzomcore/build/nel-pipeline/bin"))
    ap.add_argument("--out", default=None, help="dir for the per-case diff txts (default: <folder>/diffs)")
    args = ap.parse_args()

    try:
        from biped_coverage import DECODED
    except Exception:
        DECODED = set()
    diffrig = os.path.join(args.bin, "pipeline_max_export_anim")
    baseline = os.path.join(args.folder, "f00_baseline.max")
    if not os.path.isfile(diffrig):
        print(f"SKIP: missing {diffrig}"); sys.exit(77)
    if not os.path.isfile(baseline):
        print(f"ERROR: no f00_baseline.max in {args.folder}"); sys.exit(1)
    outdir = args.out or os.path.join(args.folder, "diffs")
    os.makedirs(outdir, exist_ok=True)

    cases = sorted(f for f in os.listdir(args.folder)
                   if re.match(r'f[0-9A-Za-z]+_.*\.max$', f) and f != "f00_baseline.max")
    chunk_re = re.compile(r'chunk 0x([0-9a-f]{4})')
    case_chunks = {}   # case -> set(chunk ids)
    chunk_cases = defaultdict(list)  # chunk id -> [cases]
    for c in cases:
        name = os.path.splitext(c)[0]
        outtxt = os.path.join(outdir, name + ".diff.txt")
        try:
            subprocess.run([diffrig, "--diff-rig", baseline, os.path.join(args.folder, c), outtxt],
                           capture_output=True, text=True, timeout=120)
            txt = open(outtxt).read()
        except Exception as e:
            print(f"  {name}: diff FAILED ({e})"); continue
        ids = set()
        for m in re.finditer(r'^SYSOBJ \d+ chunk 0x([0-9a-f]{4})', txt, re.M):
            ids.add(int(m.group(1), 16))
        case_chunks[name] = ids
        for cid in ids: chunk_cases[cid].append(name)

    print("== PER-CASE changed chunks (U=undecoded target, .=already decoded) ==")
    for name in sorted(case_chunks):
        ids = sorted(case_chunks[name])
        if not ids:
            print(f"  {name:22} (no change — field ignored or not stored)")
            continue
        tags = " ".join(("U:0x%04x" % c) if c not in DECODED else ("0x%04x" % c) for c in ids)
        print(f"  {name:22} {tags}")

    print("\n== INVERSE MAP: undecoded chunk -> fields that move it (decode targets) ==")
    unknown = sorted(c for c in chunk_cases if c not in DECODED)
    if not unknown:
        print("  (every changed chunk is already decoded)")
    for cid in unknown:
        print(f"  0x{cid:04x}: {', '.join(sorted(set(chunk_cases[cid])))}")

    print("\n== decoded chunks that also moved (sanity — expected) ==")
    known = sorted(c for c in chunk_cases if c in DECODED)
    print("  " + " ".join("0x%04x" % c for c in known))

if __name__ == "__main__":
    main()
