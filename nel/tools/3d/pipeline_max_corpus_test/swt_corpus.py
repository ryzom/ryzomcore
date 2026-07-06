#!/usr/bin/env python3
"""Corpus driver for the swt .max -> .swt pipeline (skeleton weight templates).

Enumerates SwtSourceDirectories from the workspace, runs T1/T2 roundtrip via
pipeline_max_corpus_test and T3 export via pipeline_max_export_swt, byte-comparing against
the direct reference exports. Reference-era tolerance: nodes SWT-flagged in the .max after
the reference export appear as extra entries; a file passes when the reference entry list is
an in-order subset of ours and every extra node is in the documented era list below.
"""

import argparse, os, struct, subprocess, sys

SKIP_CODE = 77

# Nodes whose SWT flag postdates the reference export (max file authoritative, see
# pipeline_max_design.md T3 triage rules). Keyed by source basename.
ERA_EXTRA_NODES = {
    "max_top": {"box_arme", "box_arme_gauche"},
}

def parse_workspace_dirs(path, key):
    import re
    dirs = []
    with open(path) as f:
        for line in f:
            m = re.match(r'\s*' + key + r'\s*\+=\s*\[\s*"([^"]+)"\s*\]', line)
            if m: dirs.append(m.group(1))
    return dirs

def parse_swt(path):
    d = open(path, 'rb').read()
    if d[0:4] != b'SKWT':
        raise ValueError('bad magic in ' + path)
    o = 5
    cnt = struct.unpack_from('<I', d, o)[0]; o += 4
    out = []
    for _ in range(cnt):
        o += 1
        n = struct.unpack_from('<I', d, o)[0]; o += 4
        name = d[o:o+n].decode('latin1'); o += n
        w = struct.unpack_from('<f', d, o)[0]; o += 4
        out.append((name, w))
    return out

def main():
    home = os.path.expanduser("~")
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--graphics", default=os.path.join(home, "ryzomcore_graphics"))
    ap.add_argument("--workspace", default=os.path.join(home, "ryzomcore_leveldesign/workspace"))
    ap.add_argument("--bin", default=os.path.join(home, "ryzomcore/build/nel-pipeline/bin"))
    ap.add_argument("--ref", default=os.path.join(home, "pipeline_export/common/characters/swt"))
    args = ap.parse_args()

    if not os.path.isdir(args.graphics) or not os.path.isdir(args.workspace):
        print("SKIP: asset checkouts not present")
        sys.exit(SKIP_CODE)
    corpus_test = os.path.join(args.bin, "pipeline_max_corpus_test")
    export_swt = os.path.join(args.bin, "pipeline_max_export_swt")
    if not (os.path.isfile(corpus_test) and os.path.isfile(export_swt)):
        print("SKIP: missing binaries (build first)")
        sys.exit(SKIP_CODE)

    files = []
    cfg = os.path.join(args.workspace, "common", "characters", "directories.py")
    for d in parse_workspace_dirs(cfg, "SwtSourceDirectories"):
        full = os.path.join(args.graphics, d)
        if not os.path.isdir(full): continue
        for name in sorted(os.listdir(full)):
            if name.lower().endswith(".max"):
                files.append(os.path.join(full, name))
    print("corpus: %d swt-source .max files" % len(files))
    if not files:
        sys.exit(SKIP_CODE)

    fails = 0
    for full in files:
        base = os.path.splitext(os.path.basename(full))[0]
        r = subprocess.run([corpus_test, "--parse", full], capture_output=True, text=True, timeout=300)
        if r.returncode != 0 or "FAIL" in r.stdout:
            print("T1/T2 FAIL %s: %s" % (base, r.stdout.strip()[:160])); fails += 1
            continue
        out = "/tmp/swt_corpus.%d.%s.swt" % (os.getpid(), base)
        r = subprocess.run([export_swt, full, out], capture_output=True, text=True, timeout=300)
        if r.returncode != 0:
            print("T3 FAIL %s: exporter rc=%d %s" % (base, r.returncode, r.stderr.strip()[:160])); fails += 1
            continue
        ref = os.path.join(args.ref, base + ".swt")
        if not os.path.isfile(ref):
            print("T3 %s: no reference, export ok (%d entries)" % (base, len(parse_swt(out))))
            os.unlink(out)
            continue
        if open(out, 'rb').read() == open(ref, 'rb').read():
            print("T3 %s: byte-identical" % base)
            os.unlink(out)
            continue
        ours, theirs = parse_swt(out), parse_swt(ref)
        it = iter(ours)
        subset = all(any(x == want for x in it) for want in theirs)
        extras = set(n.rsplit('.', 1)[0] for n, _ in set(ours) - set(theirs))
        allowed = ERA_EXTRA_NODES.get(base, set())
        if subset and extras and extras <= allowed:
            print("T3 %s: era-superset ok (extra nodes: %s)" % (base, ", ".join(sorted(extras))))
        else:
            print("T3 FAIL %s: entries differ beyond the era allowance (extras=%s)" % (base, sorted(extras)[:5]))
            fails += 1
        os.unlink(out)
    sys.exit(1 if fails else 0)

if __name__ == "__main__":
    main()
