#!/usr/bin/env python3
"""Compare our `pipeline_max_export_anim --dump-samples` evaluation against the LIVE Max 9
ground truth from a Part A residual-probe manifest, and decompose the per-bone world-rotation
error of a 2-bone limb chain into the parts an IK model can act on.

The §10s biped-IK sessions repeatedly re-derived ~100-line ad-hoc scripts to do exactly this
alignment (Max Y-up positions/quats on both sides at quarter-frame). This tool captures that
once: it loads a CASE's dense GT block, loads our samples, aligns them by frame, and reports,
per frame:

  * per-bone world-rot qdist  (= max|x,y,z| of the double-cover-aligned difference quat;
    ≈ sin(θ/2) where θ is the rotation angle — OFFLINE_RESIDUAL confirms FA qdist 0.5116 ↔
    geodesic 1.0766 rad, so qdist*2.1 ≈ radians for ballpark reading),
  * the UPPER-bone POINTING tilt (angle between our shoulder→mid direction and Max's) — this
    is the "where the elbow sits" error, distinct from roll about the limb,
  * the SWIVEL (signed angle between our and Max's elbow-offset about the reach axis) — the
    roll/swivel DOF; ~0 means the pole vector is already correct,
  * the elbow and wrist POSITION deltas,
  * (with --ikdbg) the radial reach of our solve target T vs Max's wrist — whether the target
    is too straight (inward) or too far, which is what bends the elbow via law-of-cosines.

This decomposition is what pinned the §10s-oct Part A refinement: the LargePath strike-arm
residual is NOT swivel (swivel is exact to ~0.003 rad) — it is that Character Studio keeps the
arm EXTENDED (the wrist holds the stored-hinge-[0] reach, ≈0.496 m on coup_fort_03) while our
models over-bend (reach dips to ≈0.43 m), so the pointing tilt diverges to 0.4 rad.

Usage:
  cmp_gt_samples.py --gt <manifest.txt> --case <CASE_LABEL> --ours <our_samples.txt> \
      [--bones B1,B2,B3,B4] [--frame-min F] [--frame-max F] [--ikdbg <ikdbg.txt>] \
      [--target-side 0|1] [--bin-side-note "..."]

  --gt          the Part A manifest (~/biped_ik_residual_probe_new/manifest.txt).
  --case        the CASE label to locate (e.g. A_hof_l2m_coup_fort_03); its dense GT block is
                taken (TRUTH_DENSE .. TRUTH_SPARSE/next directive).
  --ours        our --dump-samples output (Max Y-up, same convention as the manifest).
  --bones       comma-separated chain, UPPER→MID→END (default the R arm: "Bip01 R UpperArm,
                Bip01 R Forearm,Bip01 R Hand"). The UPPER bone's world pos is the chain root
                (shoulder/hip); END is the wrist/ankle.
  --frame-min/max  restrict to a frame window (the plant interval of interest).
  --ikdbg       optional PMB_BIPED_IK_DEBUG log; extracts the solve target T + FK wrist (ankFk)
                for the --target-side limb to add the radial-reach comparison.

Both sample files use the SAME line shape:
    SAMPLE <frame> <bonename> pos <x>,<y>,<z> rot <qx>,<qy>,<qz>,<qw>
"""
import argparse, math, re, sys

# ---- small double-precision quat/vector math (Max/NeL column-vector convention) ----
def qmul(a, b):
    ax, ay, az, aw = a
    return (aw*b[0]+ax*b[3]+ay*b[2]-az*b[1],
            aw*b[1]-ax*b[2]+ay*b[3]+az*b[0],
            aw*b[2]+ax*b[1]-ay*b[0]+az*b[3],
            aw*b[3]-ax*b[0]-ay*b[1]-az*b[2])
def qconj(a): return (-a[0], -a[1], -a[2], a[3])
def qnorm(a):
    n = math.sqrt(sum(x*x for x in a))
    return tuple(x/n for x in a) if n > 1e-12 else (0.0, 0.0, 0.0, 1.0)
def qdist(a, b):
    """max|x,y,z| of the double-cover-aligned difference quat (≈ sin(θ/2))."""
    d = qnorm(qmul(qconj(a), b))
    if d[3] < 0: d = tuple(-x for x in d)
    return max(abs(d[0]), abs(d[1]), abs(d[2]))
def vsub(a, b): return (a[0]-b[0], a[1]-b[1], a[2]-b[2])
def vadd(a, b): return (a[0]+b[0], a[1]+b[1], a[2]+b[2])
def vscale(a, s): return (a[0]*s, a[1]*s, a[2]*s)
def vdot(a, b): return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]
def vcross(a, b): return (a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0])
def vnorm(a):
    n = math.sqrt(vdot(a, a))
    return (a[0]/n, a[1]/n, a[2]/n) if n > 1e-12 else (0.0, 0.0, 0.0)
def vmag(a): return math.sqrt(vdot(a, a))
def pdist(a, b): return vmag(vsub(a, b))

_SAMP = re.compile(r'SAMPLE\s+(\S+)\s+(.+?)\s+pos\s+(\S+),(\S+),(\S+)\s+rot\s+(\S+),(\S+),(\S+),(\S+)')

def parse_samples(path, want_bones, line_start=None, line_end=None):
    """{bone: {frame(float): (px,py,pz,qx,qy,qz,qw)}}, Max Y-up."""
    data = {}
    with open(path) as f:
        for i, ln in enumerate(f, 1):
            if line_start and i < line_start: continue
            if line_end and i > line_end: continue
            m = _SAMP.search(ln)
            if not m: continue
            bone = m.group(2).strip()
            if bone not in want_bones: continue
            data.setdefault(bone, {})[float(m.group(1))] = tuple(map(float, m.groups()[2:]))
    return data

def find_case_block(manifest, case):
    """Return (start_line, end_line) 1-based of the dense GT block for a CASE label.
    The dense block runs from the case's TRUTH_DENSE directive to the next TRUTH_SPARSE,
    IKORACLE, REEXPORT, or CASE directive (whichever comes first)."""
    start = end = None
    in_case = False
    dense_seen = False
    with open(manifest) as f:
        for i, ln in enumerate(f, 1):
            if ln.startswith("CASE\t"):
                if in_case and dense_seen:
                    end = i - 1
                    break
                in_case = (ln.strip() == "CASE\t" + case)
                dense_seen = False
                continue
            if in_case and "TRUTH_DENSE" in ln and not dense_seen:
                start = i
                dense_seen = True
            elif in_case and dense_seen and (ln.startswith("  TRUTH_SPARSE") or
                                              ln.startswith("  IKORACLE") or
                                              ln.startswith("  REEXPORT")):
                end = i - 1
                break
    if start and not end:
        end = sum(1 for _ in open(manifest))
    return start, end

def parse_ikdbg_targets(path, side, tmin, tmax):
    """{tick: (T_xyz, ankFk_xyz)} from PMB_BIPED_IK_DEBUG lines for the given side."""
    out = {}
    if not path: return out
    pat = re.compile(r'IKDBG side=%d t=(\S+) .*T=\(([^)]+)\) ankFk=\(([^)]+)\)' % side)
    with open(path) as f:
        for ln in f:
            m = pat.search(ln)
            if not m: continue
            t = int(float(m.group(1)))
            if t < tmin or t > tmax: continue
            T = tuple(map(float, m.group(2).split(',')))
            F = tuple(map(float, m.group(3).split(',')))
            out[t] = (T, F)
    return out

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--gt", required=True)
    ap.add_argument("--case", required=True)
    ap.add_argument("--ours", required=True)
    ap.add_argument("--bones", default="Bip01 R UpperArm,Bip01 R Forearm,Bip01 R Hand",
                    help="UPPER,MID,END chain (default R arm)")
    ap.add_argument("--frame-min", type=float, default=None)
    ap.add_argument("--frame-max", type=float, default=None)
    ap.add_argument("--ikdbg", default=None)
    ap.add_argument("--target-side", type=int, default=0, help="IKDBG side filter (0=R,1=L)")
    args = ap.parse_args()

    bones = [b.strip() for b in args.bones.split(",")]
    if len(bones) != 3:
        sys.exit("--bones needs exactly UPPER,MID,END")
    b_up, b_mid, b_end = bones

    start, end = find_case_block(args.gt, args.case)
    if not start:
        sys.exit("CASE %s not found (or no TRUTH_DENSE) in %s" % (args.case, args.gt))
    gt = parse_samples(args.gt, set(bones), start, end)
    ours = parse_samples(args.ours, set(bones))
    for b in bones:
        if b not in gt or not gt[b]:
            print("WARNING: bone %s missing from GT dense block" % b, file=sys.stderr)
        if b not in ours or not ours[b]:
            print("WARNING: bone %s missing from our samples" % b, file=sys.stderr)

    frames = sorted(set(ours.get(b_up, {})) & set(gt.get(b_up, {})))
    if args.frame_min is not None: frames = [f for f in frames if f >= args.frame_min]
    if args.frame_max is not None: frames = [f for f in frames if f <= args.frame_max]

    ik = parse_ikdbg_targets(args.ikdbg, args.target_side,
                             int((frames[0] if frames else 0)*160),
                             int((frames[-1] if frames else 1e9)*160)) if args.ikdbg else {}

    print("=== %s : %s ===" % (args.case, " -> ".join(bones)))
    hdr = "frame  UAqdist  midQdist  endQdist  pointTilt  swivel  elbowΔpos  wristΔpos"
    if ik: hdr += "  |T-endGT|  reach_T  reach_endGT"
    print(hdr)
    w = {"up": 0.0, "mid": 0.0, "end": 0.0, "tilt": 0.0, "sw": 0.0, "elb": 0.0, "wr": 0.0}
    for f in frames:
        ou = ours[b_up][f]; om = ours[b_mid][f]; oe = ours[b_end][f]
        mu = gt[b_up][f];   mm = gt[b_mid][f];   me = gt[b_end][f]
        d_up = qdist(ou[3:7], mu[3:7]); d_mid = qdist(om[3:7], mm[3:7]); d_end = qdist(oe[3:7], me[3:7])
        # UPPER pointing: shoulder(UPPER pos) -> mid; tilt = angle between ours and Max directions.
        sh_o = ou[0:3]; el_o = om[0:3]; wr_o = oe[0:3]
        sh_m = mu[0:3]; el_m = mm[0:3]; wr_m = me[0:3]
        pto = vnorm(vsub(el_o, sh_o)); ptm = vnorm(vsub(el_m, sh_m))
        tilt = math.acos(max(-1.0, min(1.0, vdot(pto, ptm))))
        # swivel: signed angle between elbow-offsets (⊥ reach) about the reach axis.
        reach = vnorm(vsub(wr_o, sh_o))
        off_o = vsub(vsub(el_o, sh_o), vscale(reach, vdot(vsub(el_o, sh_o), reach)))
        reach_m = vnorm(vsub(wr_m, sh_m))
        off_m = vsub(vsub(el_m, sh_m), vscale(reach_m, vdot(vsub(el_m, sh_m), reach_m)))
        cr = vcross(off_o, off_m)
        sw = math.atan2(vdot(cr, reach), vdot(off_o, off_m)) if (vmag(off_o) > 1e-9 and vmag(off_m) > 1e-9) else 0.0
        el_d = pdist(el_o, el_m); wr_d = pdist(wr_o, wr_m)
        w["up"] = max(w["up"], d_up); w["mid"] = max(w["mid"], d_mid); w["end"] = max(w["end"], d_end)
        w["tilt"] = max(w["tilt"], tilt); w["sw"] = max(w["sw"], abs(sw))
        w["elb"] = max(w["elb"], el_d); w["wr"] = max(w["wr"], wr_d)
        line = "%.2f  %.4f  %.4f  %.4f  %.4f  %+.4f  %.4f  %.4f" % (
            f, d_up, d_mid, d_end, tilt, sw, el_d, wr_d)
        if ik:
            tick = int(round(f * 160))
            tk = min(ik.keys(), key=lambda k: abs(k - tick)) if ik else None
            if tk is not None:
                T, _F = ik[tk]
                dT = pdist(T, me); rT = pdist(T, sh_o); rM = pdist(me, sh_m)
                line += "  %.4f  %.4f  %.4f" % (dT, rT, rM)
            else:
                line += "  -  -  -"
        print(line)
    print("\nPEAK  up=%.4f mid=%.4f end=%.4f tilt=%.4f |swivel|=%.4f elbow=%.4f wrist=%.4f" %
          (w["up"], w["mid"], w["end"], w["tilt"], w["sw"], w["elb"], w["wr"]))
    print("Reading: qdist≈sin(θ/2). tilt is the elbow-placement/pointing error; swivel≈0 ⇒ the")
    print("pole/swivel is already correct and the residual is pointing+reach, not roll.")

if __name__ == "__main__":
    main()
