#!/usr/bin/env python3
# Runs ryzom_r2_scenario_chain over every .r2 file below the given directories, one process per file,
# and prints a classification of the results. The per-file output of the tool goes to <out>/<n>.txt and,
# with --keep-rt, the run-time trees to <out>/<n>.rt.txt, where n is the position of the file in the
# sorted list (<out>/index.txt). The tool runs in <out>/cwd; nothing is written next to the scenarios.
#
#   python3 run_corpus.py --tool <ryzom_r2_scenario_chain> --out <dir> --env-file <env_ryz.txt>
#                         [--leveldesign <dir>] <scenario dir or file>...
#
# The tool needs the DLL directories of the build's externals on the PATH: give them with --env-file (a
# file with a line "PATH=<dirs>") or with --path. On Windows a program started without them stops in a
# system dialog.
#
# Returns 0 when every file was run, whatever the results; 2 for a wrong argument (a directory that does
# not exist, outputs of another run with --reuse).

import argparse
import collections
import concurrent.futures
import json
import os
import re
import subprocess
import sys

# exit codes of the tool (TExitCode in r2_scenario_chain.cpp) and the class of result each one is
EXIT_CLASSES = {
    0: "loads, translates, check passes",
    2: "does not parse",
    3: "host: a script of the editor did not load",
    4: "update fails",
    5: "saved scenario does not read back",
    6: "loads but translation fails",
    8: "translates, the service's check refuses the run-time scenario",
    9: "update repeats without end",
    10: "host: exception",
    11: "host: Lua error outside a protected call",
    12: "file missing",
    13: "expectation of the test not met",
}


def parse_output(text):
    """The tool prints "key: value" lines and "key <<<" ... ">>>" blocks, whose lines start with "| "."""
    values = collections.OrderedDict()
    blocks = []
    lines = text.splitlines()
    i = 0
    while i < len(lines):
        line = lines[i]
        if line.endswith(" <<<"):
            key = line[:-4]
            body = []
            i += 1
            while i < len(lines) and lines[i] != ">>>":
                body.append(lines[i][2:] if lines[i].startswith("| ") else lines[i])
                i += 1
            blocks.append((key, "\n".join(body)))
        else:
            m = re.match(r"^([a-z_0-9]+): (.*)$", line)
            if m:
                values.setdefault(m.group(1), []).append(m.group(2))
        i += 1
    return values, blocks


def error_kind(traceback):
    """A stable name for a Lua error: the message without ids, and the innermost script line."""
    lines = traceback.splitlines()
    message = lines[0] if lines else ""
    message = re.sub(r"^\S+\.lua:\d+: ", "", message)
    message = re.sub(r"^\[string [^\]]*\]:\d+: ", "", message)
    message = re.sub(r"Client\d+_\d+", "<id>", message)
    message = re.sub(r"'[^']*'", "'..'", message) if len(message) > 120 else message
    # the innermost script line that is not the editor's own assert or its replacement of next
    where = ""
    for line in lines[1:]:
        m = re.search(r"(r2_[a-z0-9_]+\.lua:\d+)", line)
        if m and not re.search(r"in global '(assert|next|error)'", line):
            where = m.group(1)
            break
    if not where:
        m = re.search(r"(r2_[a-z0-9_]+\.lua:\d+)", lines[0] if lines else "")
        if m:
            where = m.group(1)
    return (message.strip() or "(empty message, assert)") + " @ " + (where or "?")


def check_kind(verdict):
    """A stable name for a refusal of the run-time check: the description without numbers and quoted text."""
    verdict = re.sub(r"'[^']*'", "'..'", verdict)
    verdict = re.sub(r"\"[^\"]*\"", "\"..\"", verdict)
    return re.sub(r"\d+", "N", verdict)


def fields(text):
    return dict((k, v) for k, v in re.findall(r"([a-z_]+)=(\S+)", text))


def split_uses(text):
    """"name@place, name@place" as the tool joins them; a name can hold a comma inside parentheses."""
    return [item for item in re.split(r", (?![^()]*\))", text) if item]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--tool", required=True)
    parser.add_argument("--out", required=True)
    parser.add_argument("--leveldesign")
    parser.add_argument("--env-file", help="file with a line PATH=<dirs>, as env_ryz.txt of the externals")
    parser.add_argument("--path", action="append", default=[], help="directory to add to the PATH")
    parser.add_argument("--timeout", type=int, default=300)
    parser.add_argument("--jobs", type=int, default=4, help="scenarios run at the same time")
    parser.add_argument("--tool-arg", action="append", default=[], help="option passed on to the tool, as --tool-arg=--option")
    parser.add_argument("--keep-rt", action="store_true", help="keep the run-time trees")
    parser.add_argument("--reuse", action="store_true", help="read the outputs of an earlier run instead of running")
    parser.add_argument("dirs", nargs="+")
    args = parser.parse_args()

    files = []
    for d in args.dirs:
        if os.path.isfile(d):
            files.append(d)
        elif os.path.isdir(d):
            for root, _, names in os.walk(d):
                for name in names:
                    if name.lower().endswith(".r2"):
                        files.append(os.path.join(root, name))
        else:
            sys.stderr.write("%s: no such file or directory\n" % d)
            return 2
    files.sort()
    if not os.path.isfile(args.tool):
        sys.stderr.write("%s: the tool is not there\n" % args.tool)
        return 2

    # the outputs of a directory belong to one list of files and one set of tool arguments
    run_description = {"files": files, "leveldesign": args.leveldesign or "", "tool_args": args.tool_arg}
    description_path = os.path.join(args.out, "run.json")
    if args.reuse:
        if not os.path.exists(description_path):
            sys.stderr.write("--reuse: %s has no run to reuse\n" % args.out)
            return 2
        with open(description_path, encoding="utf-8") as f:
            if json.load(f) != run_description:
                sys.stderr.write("--reuse: the outputs in %s were made from other files or with other tool arguments\n" % args.out)
                return 2
    else:
        os.makedirs(args.out, exist_ok=True)
        for name in os.listdir(args.out):
            if re.match(r"^\d+(\.rt)?\.txt(\.serial\.txt)?$", name):
                os.remove(os.path.join(args.out, name))
        with open(description_path, "w", encoding="utf-8") as f:
            json.dump(run_description, f, indent=1)
        with open(os.path.join(args.out, "index.txt"), "w", encoding="utf-8") as f:
            for n, path in enumerate(files):
                f.write("%d\t%s\n" % (n, path))
    work_dir = os.path.join(args.out, "cwd")
    os.makedirs(work_dir, exist_ok=True)

    env = dict(os.environ)
    path_dirs = list(args.path)
    if args.env_file:
        with open(args.env_file, encoding="utf-8") as f:
            for line in f:
                if line.startswith("PATH="):
                    path_dirs += [d for d in line[5:].strip().split(os.pathsep) if d and d != "%PATH%"]
    if path_dirs:
        env["PATH"] = os.pathsep.join(path_dirs) + os.pathsep + env.get("PATH", "")
    if os.name == "nt" and not path_dirs and not args.reuse:
        sys.stderr.write("no --env-file and no --path: the tool is not started without its DLL directories\n")
        return 2

    def run_one(item):
        n, path = item
        out_path = os.path.join(args.out, "%d.txt" % n)
        rt_path = os.path.join(args.out, "%d.rt.txt" % n)
        if args.reuse:
            if not os.path.exists(out_path):
                return "", None
            with open(out_path, encoding="utf-8", errors="replace", newline="") as f:
                text = f.read().replace("\r\n", "\n")
            # the return code is the last line the runner added
            code = None
            tail = text.rstrip("\n").rsplit("\n", 1)[-1]
            if tail.startswith("runner_process_exit: ") and tail[21:].lstrip("-").isdigit():
                code = int(tail[21:])
            return text, code
        command = [os.path.abspath(args.tool), os.path.abspath(path), "--rt-out", os.path.abspath(rt_path)]
        if args.leveldesign:
            command += ["--leveldesign", args.leveldesign]
        command += args.tool_arg
        try:
            proc = subprocess.run(command, env=env, cwd=work_dir, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=args.timeout)
            text = proc.stdout.decode("utf-8", errors="replace")
            code = proc.returncode
            text += "\nrunner_process_exit: %d\n" % code
        except subprocess.TimeoutExpired as e:
            text = (e.stdout or b"").decode("utf-8", errors="replace") + "\nrunner_process_exit: timeout\n"
            code = None
        text = text.replace("\r\n", "\n")
        with open(out_path, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
        # the tool writes the serial copy of a run-time tree next to it when the two differ
        for leftover in ([] if args.keep_rt else [rt_path, rt_path + ".serial.txt"]):
            if os.path.exists(leftover):
                os.remove(leftover)
        return text, code

    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, args.jobs)) as pool:
        outputs = pool.map(run_one, list(enumerate(files)))
        texts = []
        codes = []
        for text, code in outputs:
            texts.append(text)
            codes.append(code)
            sys.stderr.write("\r%d/%d" % (len(texts), len(files)))
            sys.stderr.flush()
    sys.stderr.write("\n")

    results = collections.Counter()
    steps = collections.Counter()
    upgrade_failures = collections.Counter()
    upgrade_errors = collections.Counter()
    translate_errors = collections.Counter()
    translate_contexts = collections.Counter()
    parse_errors = collections.Counter()
    check_refusals = collections.Counter()
    upgraded_classes = collections.Counter()
    update_outcomes = collections.Counter()
    update_notes = collections.Counter()
    unchanged_saves = collections.Counter()
    untouched_failures = collections.Counter()
    returns = collections.Counter()
    stubs =collections.defaultdict(collections.Counter)
    undefined = collections.defaultdict(collections.Counter)
    host_calls = collections.defaultdict(collections.Counter)
    dropped = collections.defaultdict(collections.Counter)
    late = collections.defaultdict(collections.Counter)
    swallowed = collections.Counter()
    by_result = collections.defaultdict(list)
    maxima = collections.Counter()
    scenario_maxima = collections.Counter()

    for n, path in enumerate(files):
        values, blocks = parse_output(texts[n])

        def first(key, default=""):
            return values.get(key, [default])[0]

        # the class of the result is the return code of the process
        if codes[n] is None:
            result = "host: the tool did not end (timeout or no output)"
        else:
            result = EXIT_CLASSES.get(codes[n], "host: the tool ended with return code %d" % codes[n])
        results[result] += 1
        by_result[result].append(n)

        # how far each file got
        if first("parse") == "ok":
            steps["1 parsed"] += 1
        if first("upgrade") in ("ok", "unchanged", "not_needed"):
            steps["2 through the version update"] += 1
            update_outcomes[{"ok": "the update changed the scenario",
                             "unchanged": "earlier versions, nothing to change",
                             "not_needed": "at the versions of the editor"}[first("upgrade")]] += 1
            if first("upgrade") == "unchanged":
                unchanged_saves["saved scenario against the file, apart from the versions: " + first("scenario_vs_file", "not saved")] += 1
        for value in values.get("version_update_untouched", []) + values.get("reload_version_update_untouched", []):
            if value != "yes":
                untouched_failures[value] += 1
        for note in set(re.sub(r"\(.*$", "", value) for value in values.get("version_update_note", [])):
            update_notes[note] += 1
        # the scenario as it comes back from the server when the test ends: it is to be up to date
        if "return_version_before" in values:
            if "return_upload" in values:
                returns["the update sent a scenario to take the place of the server's"] += 1
            elif "return_version_update" in values:
                returns["the update ran again: " + first("return_version_update")] += 1
            elif first("return_version_before").startswith("current=yes"):
                returns["up to date"] += 1
            else:
                returns["not up to date, and no update ran"] += 1
        if first("roundtrip").startswith("ok"):
            steps["3 saved and read back"] += 1
        if first("translate") == "ok":
            steps["4 translated"] += 1
        if first("rt_check") == "ok" and first("rt_check_serial") == "ok":
            steps["5 run-time scenario passes the service's check"] += 1

        if first("parse").startswith("fail"):
            reason = re.sub(r"line \d+,? column \d+", "line N column N", first("parse_error"))
            parse_errors[re.sub(r"'[^']*'", "'..'", reason)] += 1
        upgrade = first("upgrade")
        if upgrade in ("failed", "error", "unknown_class", "repeats"):
            # the reason names the class, or the step and the error: the classes of the scenario and the
            # place of the error are kept, the rest of the message is not
            reason = first("version_update_failure")
            detail = ""
            m = re.search(r"failed in the step (\S+?)(?:: (.*))?$", reason)
            if m:
                detail = "step %s" % m.group(1)
                if m.group(2):
                    detail += ": " + error_kind(m.group(2))
            if upgrade == "unknown_class":
                undefined_classes = set(re.sub(r":[^,]*", "", first("version_before_undefined")).split(","))
                m = re.search(r"of the class '([^']*)'", reason)
                if m:
                    undefined_classes.add(m.group(1))
                detail = "classes the editor does not define: " + ",".join(sorted(c for c in undefined_classes if c))
            if upgrade == "repeats":
                detail = "the scenario came back in need of an update %s times" % first("update_rounds")
            upgrade_failures["%s: %s" % (upgrade, detail)] += 1
            for key, block in blocks:
                if key.endswith("upgrade_error") or key.endswith("version_update_error"):
                    upgrade_errors[error_kind(block)] += 1
        if first("upgrade") == "ok":
            for item in first("version_update_steps").split(","):
                if item:
                    upgraded_classes[item.split("=")[0]] += 1
        if first("translate") == "fail":
            kind = "?"
            for key, block in blocks:
                if key == "translate_error":
                    kind = error_kind(block)
                    for line in block.splitlines():
                        if line.startswith("context: "):
                            # the entity and the logic step in hand; the other locals only tell the way there
                            found = dict(re.findall(r"(\w+)=(\[[^\]]*\]|\S+)", line[9:]))
                            names = ("target", "eventHandler", "conditionEntity", "condition", "actionEntity", "action")
                            short = " ".join("%s=%s" % (name, found[name]) for name in names if name in found)
                            translate_contexts[kind + "  |  " + (short or line[9:])] += 1
            if kind == "?":
                kind = first("translate_error", "?")
            translate_errors[kind] += 1
        for key in ("rt_check", "rt_check_serial"):
            verdict = first(key)
            if verdict and verdict != "ok":
                check_refusals[key + ": " + check_kind(verdict)] += 1

        for key in values:
            if key.startswith("stubs_"):
                for item in split_uses(values[key][0]):
                    stubs[key[6:]][item] += 1
            elif key.startswith("dropped_"):
                for item in split_uses(values[key][0]):
                    dropped[key[8:]][item] += 1
            elif key.startswith("late_"):
                for item in split_uses(values[key][0]):
                    late[key[5:]][item] += 1
            elif key.startswith("host_") and key != "host_load":
                for item in values[key][0].split(", "):
                    if "=" in item:
                        host_calls[key[5:]][item.split("=")[0]] += 1
            elif key.startswith("undefined_") and key != "undefined_load":
                for item in split_uses(values[key][0]):
                    undefined[key[10:]][item] += 1
        for key, block in blocks:
            if key.endswith("_lua_error"):
                swallowed[key[:-10] + ": " + error_kind(block)] += 1

        if first("translate") == "ok":
            rt = fields(first("rt"))
            for key in ("acts", "texts", "plot_items", "locations", "nodes", "bytes", "longest_string"):
                scenario_maxima[key] = max(scenario_maxima[key], int(rt.get(key, 0)))
            totals = collections.Counter()
            for line in values.get("rt_act", []):
                act = fields(line)
                for key, value in act.items():
                    if key == "index":
                        continue
                    maxima[key] = max(maxima[key], int(value))
                    totals[key] += int(value)
            for key, value in totals.items():
                scenario_maxima["sum over acts: " + key] = max(scenario_maxima["sum over acts: " + key], value)

    def dump(title, counter, by_name=False):
        print(title)
        items = sorted(counter.items()) if by_name else counter.most_common()
        for key, count in items:
            print("  %5d  %s" % (count, key))
        print()

    print("files: %d" % len(files))
    print("tool arguments: %s\n" % (" ".join(args.tool_arg) or "(none)"))
    dump("results", results)
    dump("steps passed (files)", steps, by_name=True)
    dump("what the version update came to (scenarios)", update_outcomes)
    dump("scenarios the update left as they were", unchanged_saves)
    dump("scenarios that did not come through the update as they were read, though it changed nothing", untouched_failures)
    dump("update steps that ran in the scenarios the update changed (class:from>to, scenarios)", upgraded_classes)
    dump("what the update told the author (note, scenarios)", update_notes)
    dump("the scenario as it comes back from the server when the test ends (scenarios)", returns)
    dump("update failures", upgrade_failures)
    dump("Lua errors raised by the update", upgrade_errors)
    for phase in sorted(dropped):
        dump("node requests of the scripts that were recorded and dropped with the historic, phase %s (scenarios)" % phase, dropped[phase])
    for phase in sorted(late):
        dump("node requests of the scripts that the client applies a frame later, phase %s (scenarios)" % phase, late[phase])
    dump("translation errors", translate_errors)
    dump("translation errors with the context the translator was in (classes and logic type names)", translate_contexts)
    dump("refusals of the run-time check", check_refusals)
    dump("parse errors", parse_errors)
    dump("errors the scripts caught and went on from, by phase (occurrences)", swallowed)
    for phase in sorted(stubs):
        dump("stubs called and values substituted, phase %s (scenarios)" % phase, stubs[phase])
    for phase in sorted(host_calls):
        dump("host functions called, phase %s (scenarios)" % phase, host_calls[phase])
    for phase in sorted(undefined):
        dump("undefined names read, phase %s (scenarios)" % phase, undefined[phase])
    print("maxima per act over the scenarios that translated")
    for key in sorted(maxima):
        print("  %-16s %d" % (key, maxima[key]))
    print()
    print("maxima per scenario over the scenarios that translated")
    for key in sorted(scenario_maxima):
        print("  %-30s %d" % (key, scenario_maxima[key]))
    print()
    with open(os.path.join(args.out, "by_result.txt"), "w", encoding="utf-8") as f:
        for result, numbers in by_result.items():
            f.write("%s: %s\n" % (result, " ".join(str(n) for n in numbers)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
