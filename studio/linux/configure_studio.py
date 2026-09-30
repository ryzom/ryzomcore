#!/usr/bin/env python3
# Ryzom Core Studio - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

"""Point Ryzom Core Studio on Linux at the three repositories.

Everything the world editor needs is in ryzom-core, ryzom-data and ryzom-server-data, but
spread out and written for the Windows tools of the time: drive letters in the class
file, one zone bank split over two directories, projects that expect a "continents"
directory next to them. This script builds what studio expects out of links into the
repositories and writes the studio settings. It changes nothing inside the repositories.

Run by setup.sh; can be run on its own after the repositories moved or were updated.
Everything it creates lives in the state directory (default ~/.local/share/ryzom-studio).
"""

import argparse
import os
import re
import shutil
import struct
import sys
import time

CLASS_FILE_NAME = "world_editor_classes_linux.xml"


def log(message):
    print("  " + message)


def fail(message):
    print("error: " + message, file=sys.stderr)
    sys.exit(1)


# --------------------------------------------------------------------------------------
# Paths

def resolve_case(base, relative):
    """Walk relative below base one component at a time, matching names regardless of
    case - the data was written on Windows. Returns (path, found). A component that does
    not exist ends the walk; the rest is appended unchanged."""
    current = base
    parts = [p for p in relative.split("/") if p]
    for index, part in enumerate(parts):
        candidate = os.path.join(current, part)
        if os.path.exists(candidate):
            current = candidate
            continue
        match = None
        if os.path.isdir(current):
            lower = part.lower()
            for entry in os.listdir(current):
                if entry.lower() == lower:
                    match = entry
                    break
        if match is None:
            return os.path.join(current, *parts[index:]), False
        current = os.path.join(current, match)
    return current, True


def first_dir(*candidates):
    for candidate in candidates:
        if candidate and os.path.isdir(candidate):
            return candidate
    return None


def relink(link, target):
    """Make link point at target, replacing an older link. Never touches real files."""
    if os.path.islink(link):
        if os.readlink(link) == target:
            return
        os.unlink(link)
    elif os.path.exists(link):
        fail("%s exists and is not a link - not touching it" % link)
    os.symlink(target, link)


# --------------------------------------------------------------------------------------
# Continents: the layout studio and the .worldedit projects expect

def build_continents(data, state):
    """state/continents/<continent>/ with the .land, zoneligos/ and zonebitmaps/ linked in.

    The projects in ryzom-data/leveldesign/continents refer to ..\\continents\\<name>\\,
    and studio loads the zone bank from zoneligos/ and zonebitmaps/ next to the .land.
    In the repository the banks sit per ecosystem, and desert's is split between
    graphics2/ (bitmaps) and pipeline/export/ (ligo zones)."""
    landscape = os.path.join(data, "leveldesign", "landscape")
    if not os.path.isdir(landscape):
        fail("no leveldesign/landscape in %s - is this ryzom-data?" % data)

    root = os.path.join(state, "continents")
    os.makedirs(root, exist_ok=True)
    made = []
    for ecosystem in sorted(os.listdir(landscape)):
        eco_dir = os.path.join(landscape, ecosystem)
        if not os.path.isdir(eco_dir):
            continue
        ligos = first_dir(os.path.join(eco_dir, "zoneligos"),
                          os.path.join(data, "pipeline", "export", "ecosystems", ecosystem, "ligo_es", "zoneligos"))
        bitmaps = first_dir(os.path.join(eco_dir, "zonebitmaps"),
                            os.path.join(data, "graphics2", "landscape", "ligo", ecosystem, "zonebitmaps"))
        for land in sorted(f for f in os.listdir(eco_dir) if f.lower().endswith(".land")):
            name = os.path.splitext(land)[0]
            target = os.path.join(root, name)
            os.makedirs(target, exist_ok=True)
            relink(os.path.join(target, land), os.path.join(eco_dir, land))
            if ligos:
                relink(os.path.join(target, "zoneligos"), ligos)
            if bitmaps:
                relink(os.path.join(target, "zonebitmaps"), bitmaps)
            made.append((name, ecosystem, ligos is not None, bitmaps is not None))

    for name, ecosystem, has_ligos, has_bitmaps in made:
        if not (has_ligos and has_bitmaps):
            log("warning: %s (%s) has no complete zone bank - tiles will show as missing"
                % (name, ecosystem))
    log("%d landscapes linked below %s" % (len(made), root))
    return root


# --------------------------------------------------------------------------------------
# The class file

def translate_class_file(data, server, world_editor_files, state):
    """Copy of world_editor_classes.xml with the Windows paths made real.

    The file lists directories for the combo boxes of the property editor, written as
    "L:/leveldesign/..." or "R:\\leveldesign\\...", often in the wrong case
    (Game_elem\\Creature\\Fauna for game_element/creature/fauna). Paths that do not exist in
    the repositories stay as they are, translated but unresolved - some were broken in the
    original already."""
    source = os.path.join(data, "leveldesign", "world", "world_editor_classes.xml")
    if not os.path.isfile(source):
        source = os.path.join(world_editor_files, "world_editor_classes.xml")
    if not os.path.isfile(source):
        fail("no world_editor_classes.xml found")

    with open(source, "r", encoding="utf-8", errors="surrogateescape") as f:
        text = f.read()

    counts = {"resolved": 0, "unresolved": 0}
    unresolved = []

    def translate(match):
        original = match.group(2)
        path = original.replace("\\", "/")
        path = re.sub(r"^[A-Za-z]:/+", "", path)
        path = re.sub(r"^leveldesign/+", "", path, flags=re.IGNORECASE)
        head, _, rest = path.partition("/")
        lower = head.lower()
        if lower in ("game_elem", "game_element"):
            base, relative = os.path.join(server, "game_element"), rest
        elif lower == "world_editor_files":
            base, relative = world_editor_files, rest
        else:
            base, relative = os.path.join(data, "leveldesign"), path
        resolved, found = resolve_case(base, relative)
        counts["resolved" if found else "unresolved"] += 1
        if not found:
            unresolved.append(original)
        return match.group(1) + resolved + match.group(3)

    text = re.sub(r'(PATH=")([A-Za-z]:[\\/][^"]*)(")', translate, text)

    config_dir = os.path.join(state, "config")
    os.makedirs(config_dir, exist_ok=True)
    target = os.path.join(config_dir, CLASS_FILE_NAME)
    with open(target, "w", encoding="utf-8", errors="surrogateescape") as f:
        f.write(text)

    log("class file: %d paths resolved, %d not in the repositories (%s)"
        % (counts["resolved"], counts["unresolved"], os.path.relpath(source, data) if source.startswith(data) else source))
    return target, config_dir


# --------------------------------------------------------------------------------------
# PACS

def find_game_data(game):
    if not game:
        return None
    for candidate in (os.path.join(game, "data"), game):
        if os.path.isdir(candidate) and any(f.endswith("_pacs.bnp") for f in os.listdir(candidate)):
            return candidate
    fail("no *_pacs.bnp in %s or %s/data - is this a Ryzom installation?" % (game, game))


def extract_bnp(source, target):
    """Unpack a NeL .bnp: the file table sits at the offset stored in the last 4 bytes."""
    with open(source, "rb") as f:
        data = f.read()
    offset = struct.unpack("<I", data[-4:])[0]
    count = struct.unpack("<I", data[offset:offset + 4])[0]
    pos = offset + 4
    os.makedirs(target, exist_ok=True)
    for _ in range(count):
        length = data[pos]
        pos += 1
        name = data[pos:pos + length].decode("latin-1")
        pos += length
        size, start = struct.unpack("<II", data[pos:pos + 8])
        pos += 8
        with open(os.path.join(target, os.path.basename(name)), "wb") as out:
            out.write(data[start:start + size])
    return count


def build_pacs(data, game, state):
    """state/pacs/<continent>_pacs/ for WorldEditorPacsRoot.

    From the game installation when given: its *_pacs.bnp are what players have, so what
    the editor shows matches the live game. Continents without one fall back to the
    pipeline export in ryzom-data, which may be newer or older than live."""
    root = os.path.join(state, "pacs")
    os.makedirs(root, exist_ok=True)
    live, exported = [], []

    game_data = find_game_data(game)
    if game_data:
        for bnp in sorted(f for f in os.listdir(game_data) if f.endswith("_pacs.bnp")):
            name = bnp[:-len(".bnp")]
            target = os.path.join(root, name)
            source = os.path.join(game_data, bnp)
            stamp = os.path.join(target, ".source")
            current = "%s %d" % (source, int(os.path.getmtime(source)))
            if os.path.islink(target):
                os.unlink(target)
            if not (os.path.isfile(stamp) and open(stamp).read() == current):
                if os.path.isdir(target):
                    shutil.rmtree(target)
                extract_bnp(source, target)
                with open(stamp, "w") as f:
                    f.write(current)
            live.append(name[:-len("_pacs")])

    exports = os.path.join(data, "pipeline", "export", "continents")
    if os.path.isdir(exports):
        for continent in sorted(os.listdir(exports)):
            rbank = os.path.join(exports, continent, "rbank_output")
            target = os.path.join(root, continent + "_pacs")
            if continent in live or not os.path.isdir(rbank):
                continue
            if os.path.isdir(target) and not os.path.islink(target):
                continue
            relink(target, rbank)
            exported.append(continent)

    if live:
        log("PACS from the game (%s): %s" % (game_data, ", ".join(live)))
    if exported:
        log("PACS from the pipeline export, not necessarily live: %s" % ", ".join(exported))
    return root


# --------------------------------------------------------------------------------------
# RyzomCoreStudio.ini

def ini_value(value):
    """A value as QSettings writes it. Lists become quoted, comma separated entries."""
    if isinstance(value, list):
        return ", ".join('"%s"' % v.replace("\\", "\\\\").replace('"', '\\"') for v in value)
    value = str(value)
    if "," in value or '"' in value or value != value.strip():
        return '"%s"' % value.replace("\\", "\\\\").replace('"', '\\"')
    return value


def write_ini(path, values):
    """Set the managed keys, keep every other line as it is."""
    lines = []
    if os.path.isfile(path):
        with open(path, "r", encoding="utf-8") as f:
            lines = f.read().split("\n")
        backup = "%s.bak-%s-setup" % (path, time.strftime("%Y%m%d-%H%M%S"))
        shutil.copy2(path, backup)
        log("previous settings saved as %s" % backup)

    # Sections in order, each a list of lines.
    sections = [("", [])]
    for line in lines:
        header = re.match(r"^\[(.*)\]\s*$", line)
        if header:
            sections.append((header.group(1), []))
        else:
            sections[-1][1].append(line)

    for section, keys in values.items():
        entry = next((s for s in sections if s[0] == section), None)
        if entry is None:
            entry = (section, [])
            sections.append(entry)
        body = entry[1]
        for key, value in keys.items():
            text = "%s=%s" % (key, ini_value(value))
            for index, line in enumerate(body):
                if line.split("=", 1)[0] == key:
                    body[index] = text
                    break
            else:
                # Before trailing blank lines, so the section stays together.
                insert = len(body)
                while insert > 0 and body[insert - 1].strip() == "":
                    insert -= 1
                body.insert(insert, text)

    out = []
    for name, body in sections:
        if name:
            if out and out[-1].strip() != "":
                out.append("")
            out.append("[%s]" % name)
        out.extend(body)
    text = "\n".join(out).strip("\n") + "\n"

    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


# --------------------------------------------------------------------------------------

def check_projects(data, path_map):
    """Report .worldedit references that do not lead to a file after the path mapping."""
    projects = os.path.join(data, "leveldesign", "continents")
    if not os.path.isdir(projects):
        return
    missing = []
    for name in sorted(f for f in os.listdir(projects) if f.endswith(".worldedit")):
        with open(os.path.join(projects, name), "r", encoding="utf-8", errors="replace") as f:
            text = f.read()
        for ref in re.findall(r'<DATABASE_ELEMENT FILENAME="([^"]+\.land)"', text, flags=re.IGNORECASE):
            path = ref.replace("\\", "/")
            if not re.match(r"^[A-Za-z]:/", path):
                path = os.path.normpath(os.path.join(projects, path))
            for rule in path_map:
                source, target = rule.split("=", 1)
                if path.lower().startswith(source.lower()):
                    path = target + path[len(source):]
            if not os.path.exists(path):
                missing.append("%s -> %s" % (name, ref))
    if missing:
        log("warning: %d landscape references of the .worldedit projects lead nowhere:" % len(missing))
        for entry in missing:
            log("  " + entry)
    else:
        log("all landscapes of the .worldedit projects are found")


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--core", required=True, help="ryzom-core checkout")
    parser.add_argument("--data", required=True, help="ryzom-data checkout")
    parser.add_argument("--server-data", required=True, help="ryzom-server-data checkout")
    parser.add_argument("--game", help="Ryzom game installation, for the live PACS (optional)")
    parser.add_argument("--state", default=os.path.expanduser("~/.local/share/ryzom-studio"),
                        help="where the links and generated files go")
    parser.add_argument("--ini", default=os.path.expanduser("~/.config/RyzomCore/RyzomCoreStudio.ini"))
    args = parser.parse_args()

    core = os.path.abspath(args.core)
    data = os.path.abspath(args.data)
    server = os.path.abspath(args.server_data)
    state = os.path.abspath(args.state)
    game = os.path.abspath(os.path.expanduser(args.game)) if args.game else None

    for path, what, marker in ((data, "ryzom-data", "leveldesign"),
                               (server, "ryzom-server-data", "primitives"),
                               (core, "ryzom-core", "studio")):
        if not os.path.isdir(os.path.join(path, marker)):
            fail("%s does not look like %s (no %s/)" % (path, what, marker))

    world_editor_files = os.path.join(core, "ryzom", "common", "data_leveldesign", "leveldesign", "world_editor_files")
    icons = first_dir(os.path.join(core, "ryzom", "tools", "leveldesign", "install", "ui"),
                      os.path.join(data, "tools", "bin", "nevrax", "ui"))

    print("Configuring studio")
    continents = build_continents(data, state)
    class_file, config_dir = translate_class_file(data, server, world_editor_files, state)
    pacs = build_pacs(data, game, state)
    ai_maps = os.path.join(data, "pipeline", "export", "continents")

    leveldesign = os.path.join(data, "leveldesign")
    path_map = [
        "R:/leveldesign=" + leveldesign,
        "L:/leveldesign=" + leveldesign,
        # The projects look for ..\continents\<name>\ next to themselves.
        os.path.join(leveldesign, "continents") + "=" + continents,
    ]

    # The keys this script owns in RyzomCoreStudio.ini. Everything else - window layout,
    # show/hide switches, last directories - stays as the user has it.
    first_land = os.path.join(continents, "fyros")
    values = {
        # The paths are set here, so skip studio's own first-run path dialog.
        "General": {
            "FirstRun": "false",
        },
        "DataPath": {
            "AssetsPath": os.path.join(data, "graphics2"),
            "GameElementPath": os.path.join(server, "game_element"),
            "LevelDesignPath": leveldesign,
            "LigoConfigFile": class_file,
            "PrimitivesPath": os.path.join(server, "primitives"),
            "RecursiveSearchPathes": [world_editor_files, continents],
            "SearchPaths": [config_dir],
        },
        "LandscapeEditor": {
            "LandscapeDataDirectory": first_land if os.path.isdir(first_land) else continents,
        },
        "WorldEditor": {
            "WorldEditorCellSize": 160,
            "WorldEditorSnap": 1,
            "WorldEditorZoneSnapshotRes": 128,
            "WorldEditorPrimitiveClassFilename": CLASS_FILE_NAME,
            "WorldEditorIconPath": icons or "",
            "WorldEditorPacsRoot": pacs,
            "WorldEditorAiMapRoot": ai_maps if os.path.isdir(ai_maps) else "",
            "WorldEditorPathMap": path_map,
        },
    }
    write_ini(args.ini, values)
    log("settings written to %s" % args.ini)

    check_projects(data, path_map)

    # For the launcher. Studio writes its log.log to the working directory, which must
    # not be a repository; its own files it finds through the search paths set above.
    with open(os.path.join(state, "workdir"), "w") as f:
        f.write(state + "\n")
    print("Done.")


if __name__ == "__main__":
    main()
