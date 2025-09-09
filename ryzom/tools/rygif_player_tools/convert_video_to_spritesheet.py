#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Minimal pipeline runner:
- Accept a video path + optional parameters (fps, scale, quality, base/output).
- Execute two commands:
  1) ffmpeg -> extract frames into <frames_dir>/frame_%05d.jpg
  2) create_spritesheet.py -> build (multi) spritesheet(s) from frames

Defaults:
- fps = 20
- scale width = 360 (height auto; Lanczos)
- quality = medium (maps to ffmpeg -q:v 3)
- max atlas size = 2048 (pass-through to builder; tune as needed)
- output image = <output_dir>/<base>.<ext> (default ext=png)
- frames_dir = <output_dir>/<base>

Notes:
- This script expects create_spritesheet.py to be accessible.
  By default, it looks in the current working directory.
  You can override with --builder "path\to\create_spritesheet.py".
- ffmpeg must be on PATH, or pass --ffmpeg "C:\path\to\ffmpeg.exe".
"""

import argparse
import shutil
import subprocess
import sys
from pathlib import Path
import re

def map_quality_to_qscale(quality: str) -> int:
    """Map human-friendly quality to ffmpeg JPEG -q:v value (lower = better)."""
    q = (quality or "").strip().lower()
    if q in ("low", "lo", "l"):
        return 5
    if q in ("medium", "med", "mid", "m"):
        return 3
    if q in ("high", "hi", "h"):
        return 2
    try:
        val = int(q)
        return max(1, min(31, val))
    except Exception:
        return 3

def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)

def sanitize_token(s: str) -> str:
    # erlaubt Buchstaben/Zahlen/_ . - ; alles andere wird zu _
    return re.sub(r"[^A-Za-z0-9_.-]+", "_", s)

def main():
    ap = argparse.ArgumentParser(description="Run ffmpeg to extract frames, then call create_spritesheet.py")
    ap.add_argument("video", help="Input video file (e.g., MP4).")
    ap.add_argument("--base", help="Base name for output files (default: video stem).")
    ap.add_argument("--output-dir", help="Directory for output atlases (default: same as video).")
    ap.add_argument("--frames-dir", help="Directory for extracted frames (default: <output-dir>/<base>).")
    ap.add_argument("--fps", type=int, default=20, help="Frames per second to extract (default: 20).")
    ap.add_argument("--scale", type=int, default=360, help="Scale width in pixels (height auto; default: 360).")
    ap.add_argument("--quality", default="medium", help="JPEG quality: low|medium|high or 1..31 (default: medium~3).")
    ap.add_argument("--format", choices=["png", "tga"], default="png", help="Output image format (default: png).")
    ap.add_argument("--max-size", type=int, default=2048, help="Max atlas size (default: 2048; use 4096 for huge sheets).")
    ap.add_argument("--builder", default="create_spritesheet.py", help="Path to create_spritesheet.py (default: in CWD).")
    ap.add_argument("--ffmpeg", default="ffmpeg", help="ffmpeg executable path (default: 'ffmpeg' on PATH).")
    ap.add_argument("--overwrite", action="store_true", help="Delete frames directory if it exists before extracting.")
    args = ap.parse_args()

    video_path = Path(args.video)
    if not video_path.exists():
        print(f"Error: video not found: {video_path}", file=sys.stderr)
        sys.exit(1)

    base = args.base or video_path.stem
    safe_base = sanitize_token(base)
    output_dir = Path(args.output_dir) if args.output_dir else video_path.parent
    frames_dir = Path(args.frames_dir) if args.frames_dir else (output_dir / base)
    builder_path = Path(args.builder)

    # Resolve paths
    output_dir = output_dir.resolve()
    frames_dir = frames_dir.resolve()
    builder_path = builder_path.resolve()
    video_path = video_path.resolve()

    if not builder_path.exists():
        print(f"Error: builder not found: {builder_path}", file=sys.stderr)
        sys.exit(2)

    # Build output image path using chosen format
    output_image = (output_dir / f"{base}").with_suffix("." + args.format)

    # Ensure directories
    ensure_dir(output_dir)
    if args.overwrite and frames_dir.exists():
        shutil.rmtree(frames_dir, ignore_errors=True)
    ensure_dir(frames_dir)

    # 1) Run ffmpeg to extract frames
    vf = f"fps={args.fps},scale={args.scale}:-1:flags=lanczos"
    qv = str(map_quality_to_qscale(args.quality))
    frame_pattern = str(frames_dir / f"{safe_base}_%05d.jpg")

    ffmpeg_cmd = [
        args.ffmpeg,
        "-hide_banner",
        "-loglevel", "error",
        "-i", str(video_path),
        "-vf", vf,
        "-q:v", qv,
        frame_pattern,
    ]

    print("[1/2] ffmpeg extracting frames...")
    try:
        subprocess.run(ffmpeg_cmd, check=True)
    except FileNotFoundError:
        print("Error: ffmpeg not found. Pass --ffmpeg with full path or add to PATH.", file=sys.stderr)
        sys.exit(3)
    except subprocess.CalledProcessError as e:
        print(f"Error: ffmpeg failed with exit code {e.returncode}", file=sys.stderr)
        sys.exit(e.returncode)

    # 2) Call create_spritesheet.py with important defaults
    #    python create_spritesheet.py <output_image> <frames_dir> --max-size <N>
    builder_cmd = [
        sys.executable,                  # use current Python to run builder
        str(builder_path),
        str(output_image),
        str(frames_dir),
        "--max-size", str(args.max_size),
    ]

    print("[2/2] building spritesheets...")
    try:
        subprocess.run(builder_cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error: builder failed with exit code {e.returncode}", file=sys.stderr)
        sys.exit(e.returncode)

    print("Done.")
    print(f"Atlases written to: {output_dir}")
    print(f"Frames in: {frames_dir}")

if __name__ == "__main__":
    main()
