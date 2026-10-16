#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Improved sprite sheet builder inspired by Ryzom's original build_interface tool.

What's new in this version
--------------------------
- **Multi-Atlas Output**: Wenn die Summe der Sprites nicht in eine Atlas-Textur bis zur Maximalgröße passt,
  wird automatisch in mehrere Sheets gesplittet:  base_1.(png|tga), base_2.(png|tga), ...
  und dazugehörig base_1.txt, base_2.txt. Reihenfolge bleibt stabil.
- Sauberes Newline am Ende der .txt (Ryzom ist da pingelig).

Weitere Features
- Packs arbitrary-sized images (nicht nur fixes Grid) per Shelf-Packer (schnell & robust)
- Power-of-two sheet sizing bis konfigurierbare MAX_SIZE (default 4096)
- 1‑px Border-Extrusion gegen Filtering-Seams (standardmäßig an, `--no-border` zum Abschalten)
- Subset builds aus existierender UV‑.txt (Reihenfolge bleibt erhalten)
- PNG oder TGA Output
- UV‑.txt Zeilenformat: `filename u0 v0 u1 v1` (nur Innenbereich, d.h. ohne extrudierten Rand)
- Extract-Modus: cropt Elemente via UV wieder aus der Atlas-Textur

Usage
------
Build (single/multi-sheet automatisch):
    python build_spritesheet_new.py out_sheet.png input_dir
    python build_spritesheet_new.py --format tga out_sheet.tga input_dir
    python build_spritesheet_new.py --subset existing_uv.txt out.png input_dir
    python build_spritesheet_new.py --max-size 8192 out.png input_dir

Border aus:
    python build_spritesheet_new.py --no-border out.png input_dir

Extract (UV neben Sheet oder via --uv):
    python build_spritesheet_new.py --extract out_sheet.png output_dir
    python build_spritesheet_new.py --extract --uv out_sheet.txt out_sheet.png output_dir
"""

import argparse
from pathlib import Path
from PIL import Image
import math
from typing import List, Tuple, Dict, Optional

# -------------------- Config --------------------
DEFAULT_MAX_SIZE = 4096
BORDER_PX = 1  # duplicated border on each side when enabled
VALID_EXTS = (".png", ".tga", ".jpg", ".jpeg", ".bmp", ".webp")

# -------------------- Helpers --------------------
def next_pow2(x: int) -> int:
    if x <= 1:
        return 1
    return 1 << (x - 1).bit_length()

def list_images(input_dir: Path) -> List[Path]:
    files = []
    for p in sorted(input_dir.iterdir()):
        if p.is_file() and p.suffix.lower() in VALID_EXTS:
            files.append(p)
    return files

def load_subset_list(uv_txt: Path) -> List[str]:
    order = []
    if not uv_txt.exists():
        raise FileNotFoundError(f"Subset file not found: {uv_txt}")
    with uv_txt.open("r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            tok = line.split()[0]
            order.append(Path(tok).name.lower())
    return order

def filter_and_order(files: List[Path], subset_names: List[str]) -> List[Path]:
    lookup: Dict[str, Path] = {f.name.lower(): f for f in files}
    result: List[Path] = []
    for name in subset_names:
        if name in lookup:
            result.append(lookup[name])
    return result

def ensure_rgba(img: Image.Image) -> Image.Image:
    if img.mode == "RGBA":
        return img
    if img.mode in ("RGB", "P", "L", "LA"):
        return img.convert("RGBA")
    return img.convert("RGBA")

def make_extruded(img: Image.Image, border_px: int) -> Image.Image:
    if border_px <= 0:
        return img
    w, h = img.size
    out = Image.new("RGBA", (w + 2 * border_px, h + 2 * border_px))
    out.paste(img, (border_px, border_px))

    # edges
    top = img.crop((0, 0, w, 1)).resize((w, border_px))
    bot = img.crop((0, h - 1, w, h)).resize((w, border_px))
    left = img.crop((0, 0, 1, h)).resize((border_px, h))
    right = img.crop((w - 1, 0, w, h)).resize((border_px, h))
    out.paste(top, (border_px, 0))
    out.paste(bot, (border_px, border_px + h))
    out.paste(left, (0, border_px))
    out.paste(right, (border_px + w, border_px))

    # corners
    tl = img.getpixel((0, 0))
    tr = img.getpixel((w - 1, 0))
    bl = img.getpixel((0, h - 1))
    br = img.getpixel((w - 1, h - 1))
    Image.Image.paste(out, Image.new("RGBA", (border_px, border_px), tl), (0, 0))
    Image.Image.paste(out, Image.new("RGBA", (border_px, border_px), tr), (border_px + w, 0))
    Image.Image.paste(out, Image.new("RGBA", (border_px, border_px), bl), (0, border_px + h))
    Image.Image.paste(out, Image.new("RGBA", (border_px, border_px), br), (border_px + w, border_px + h))

    return out

# -------------------- Shelf packer --------------------
class ShelfPacker:
    def __init__(self, max_size: int, border_px: int):
        self.max_size = max_size
        self.border_px = border_px
        self.items: List[Tuple[Path, Image.Image, int, int]] = []  # (path, img_with_border, w, h)

    def add(self, path: Path, img: Image.Image):
        img = ensure_rgba(img)
        if self.border_px:
            img = make_extruded(img, self.border_px)
        w, h = img.size
        self.items.append((path, img, w, h))

    def pack_subset(self, items: List[Tuple[Path, Image.Image, int, int]]):
        """Pack as many items as fit into one sheet.
        Returns: (sheet_w, sheet_h, placements, placed_indices)
        placements[path] = (x,y,w,h,inner_w,inner_h)
        placed_indices: indices of 'items' that were placed
        """
        # Sort by height DESC but keep stable within same height to preserve relative order
        ordered = sorted(list(enumerate(items)), key=lambda it: it[1][3], reverse=True)

        # Guess sheet side
        total_area = sum(w*h for _, (_, _, w, h) in ordered)
        side = next_pow2(int(math.sqrt(total_area)) or 1)
        side = min(max(64, side), self.max_size)

        for _attempt in range(18):
            W = H = side
            placements: Dict[Path, Tuple[int,int,int,int,int,int]] = {}
            placed_indices: List[int] = []
            x = y = 0
            shelf_h = 0

            for idx, (path, img, w, h) in ordered:
                if w > W or h > H:
                    # This sprite can never fit in this sheet; skip it for this round.
                    continue
                if x + w > W:
                    x = 0
                    y += shelf_h
                    shelf_h = 0
                if y + h > H:
                    # No more room on this sheet; continue trying others (we'll finalize this attempt)
                    continue
                placements[path] = (x, y, w, h, img.size[0] - 2*self.border_px, img.size[1] - 2*self.border_px)
                placed_indices.append(idx)
                x += w
                shelf_h = max(shelf_h, h)

            if placements:
                # Tighten to used area
                used_w = 0
                used_h = 0
                for (px, py, w, h, _, _) in placements.values():
                    used_w = max(used_w, px + w)
                    used_h = max(used_h, py + h)
                sheet_w = next_pow2(used_w)
                sheet_h = next_pow2(used_h)
                return sheet_w, sheet_h, placements, placed_indices

            # If nothing placed, grow side; if we've already exceeded max, abort
            side = min(self.max_size, side * 2)
            if side >= self.max_size:
                # Try once with the absolute max to see if at least one fits
                W = H = self.max_size
                # Try to place at least one largest sprite
                for idx, (path, img, w, h) in ordered:
                    if w <= W and h <= H:
                        return W, H, {path: (0,0,w,h, img.size[0] - 2*self.border_px, img.size[1] - 2*self.border_px)}, [idx]
                break

        # If still nothing placed, we cannot fit any item (all larger than max_size)
        raise RuntimeError(f"At least one sprite exceeds the maximum sheet size {self.max_size}x{self.max_size}.")

# -------------------- Build & Extract --------------------
def _save_sheet_and_uv(output_base: Path, index: int, sheet_img: Image.Image, uv_lines: List[str], fmt: str) -> None:
    # index starts at 1
    out_base = output_base.parent / f"{output_base.stem}_{index}"
    if fmt == "png":
        sheet_img.save(out_base.with_suffix(".png"))
    else:
        sheet_img.save(out_base.with_suffix(".tga"))
    uv_path = out_base.with_suffix(".txt")
    with uv_path.open("w", encoding="utf-8") as f:
        f.write("\n".join(uv_lines) + "\n")  # ensure trailing newline
    print(f"Spritesheet: {out_base.with_suffix('.' + fmt)} ({sheet_img.size[0]}x{sheet_img.size[1]}), items: {len(uv_lines)}")
    print(f"UV map:      {uv_path}")

def build_sheets_multi(output_path: Path, input_dir: Path, fmt: str, subset_uv: Optional[Path], max_size: int, border: bool):
    files = list_images(input_dir)
    if subset_uv:
        order = load_subset_list(subset_uv)
        files = filter_and_order(files, order)

    if not files:
        raise RuntimeError("No images found to pack.")

    border_px = (BORDER_PX if border else 0)

    # Preload all images once
    items: List[Tuple[Path, Image.Image, int, int]] = []
    for p in files:
        img = Image.open(p)
        img = ensure_rgba(img)
        if border_px:
            img = make_extruded(img, border_px)
        w, h = img.size
        items.append((p, img, w, h))

    packer = ShelfPacker(max_size=max_size, border_px=border_px)

    # We will greedily fill sheet after sheet until all are placed.
    remaining = items[:]
    sheet_idx = 1
    while remaining:
        sheet_w, sheet_h, placements, placed_indices = packer.pack_subset(remaining)
        sheet = Image.new("RGBA", (sheet_w, sheet_h), (0,0,0,0))

        uv_lines: List[str] = []
        # Build a map from Path to original (unpadded) size to compute inner rect
        for idx in sorted(placed_indices, reverse=True):
            path, img, w, h = remaining[idx]
            x, y, w, h, inner_w, inner_h = placements[path]
            sheet.paste(img, (x, y))

        # Write UV lines in original order of files (stable UX), but only for placed ones
        placed_paths = set(placements.keys())
        for path, img, w, h in items:
            if path not in placed_paths:
                continue
            x, y, w, h, inner_w, inner_h = placements[path]
            b = border_px
            u0 = (x + b) / sheet_w
            v0 = (y + b) / sheet_h
            u1 = (x + b + inner_w) / sheet_w
            v1 = (y + b + inner_h) / sheet_h
            uv_lines.append(f"{path.name} {u0:.12f} {v0:.12f} {u1:.12f} {v1:.12f}")

        _save_sheet_and_uv(output_path.with_suffix(""), sheet_idx, sheet, uv_lines, fmt)

        # Remove placed from remaining
        for idx in sorted(placed_indices, reverse=True):
            del remaining[idx]
        sheet_idx += 1

def extract_from_sheet(sheet_path: Path, out_dir: Path, uv_txt: Optional[Path]):
    if not uv_txt or not uv_txt.exists():
        uv_txt = sheet_path.with_suffix(".txt")
        if not uv_txt.exists():
            raise FileNotFoundError("UV .txt not found. Provide with --uv.")

    img = Image.open(sheet_path).convert("RGBA")
    W, H = img.size

    out_dir.mkdir(parents=True, exist_ok=True)

    with uv_txt.open("r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            if len(parts) != 5:
                continue
            name, u0, v0, u1, v1 = parts[0], float(parts[1]), float(parts[2]), float(parts[3]), float(parts[4])
            x0 = int(round(u0 * W))
            y0 = int(round(v0 * H))
            x1 = int(round(u1 * W))
            y1 = int(round(v1 * H))
            crop = img.crop((x0, y0, x1, y1))
            crop.save(out_dir / name)
            print(f"Extracted {name}")

# -------------------- CLI --------------------
def main():
    ap = argparse.ArgumentParser(description="Build one or more interface textures (Ryzom-style) with UV .txt output.")
    ap.add_argument("output", help="Base output image file (png or tga). If multiple sheets are needed, suffix _1, _2, ... will be used.")
    ap.add_argument("input", nargs="?", help="Input directory with sprites (required for build or when using --extract as destination).")
    ap.add_argument("-f", "--format", choices=["png", "tga"], help="Output image format (default: inferred from output extension)")
    ap.add_argument("-s", "--subset", help="Path to existing UV .txt to build a subset (order preserved).")
    ap.add_argument("-x", "--extract", action="store_true", help="Extract all interface elements from <output> into <input> directory using the UV .txt.")
    ap.add_argument("--uv", help="UV .txt to use for extraction (defaults to <output>.txt).")
    ap.add_argument("--max-size", type=int, default=DEFAULT_MAX_SIZE, help="Max sheet size (power-of-two up to this value). Default: 4096")
    ap.add_argument("--no-border", action="store_true", help="Disable 1px border duplication. Enabled by default.")
    args = ap.parse_args()

    output = Path(args.output)
    fmt = (args.format or output.suffix.lstrip(".").lower() or "png")
    if fmt not in ("png", "tga"):
        fmt = "png"

    if args.extract:
        if not args.input:
            ap.error("When using --extract, you must provide the destination directory as the second argument (input).")
        uv_txt = Path(args.uv) if args.uv else None
        extract_from_sheet(output, Path(args.input), uv_txt or output.with_suffix(".txt"))
        return

    if not args.input:
        ap.error("Missing input directory.")

    subset_uv = Path(args.subset) if args.subset else None
    build_sheets_multi(output, Path(args.input), fmt, subset_uv, args.max_size, border=(not args.no_border))

if __name__ == "__main__":
    main()
