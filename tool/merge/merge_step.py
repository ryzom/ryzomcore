#!/usr/bin/env python3
"""One checkpoint merge of the core4 <-> atys merge branch, the way the branch was built.

Usage: merge_step.py WT THEIRS --side=atys|core4 --base=BASE [--ref=REF] [--tag=NAME]
                     [--add-clean] [--restore-ws] [--python=EXE]

WT     worktree checked out on the merge branch (clean, nothing staged)
THEIRS the checkpoint commit to merge in (an atys commit, or a core4 commit)
--side which side THEIRS belongs to; picks the applier mode and the post-processing
--base the previous checkpoint of the SAME side that is already in the branch
       (atys step: the last merged atys commit; core4 step: the last merged core4 commit).
       It is the base of the positional three-way judgement in tokapply, so conflicts
       are judged against what that side really changed, not against git's merge base.
--ref  for atys steps: the current core4 checkpoint, used to annotate kept clusters
--tag  prefix for the log files written next to the worktree (default: step)
--add-clean  stage the conflicted files tokapply resolved without a SKIP
--restore-ws atys steps only: drop pure-whitespace hunks that the auto-merge took from
             atys (ws_restore.py --lines then pure mode) and restore files whose
             change is formatting-only (token-identical to HEAD) back to HEAD

What it does, in order:
  1. git merge --no-commit --no-ff -X renormalize THEIRS  (rename limit raised)
  2. lists the conflicts, the auto-merged files per area, and the policy-sensitive
     files (web/, README, build files, protocol headers, database.xml) that must be
     checked by hand regardless of how git merged them
  3. runs tokapply.py on every two-sided conflict: ours = stage 2, theirs = stage 3,
     base = BASE:file, ref = REF:file. atys side -> --mech (fold NL_OVERRIDE/nullptr
     differences, re-tidy applied blocks); core4 side -> --verbatim (core4 text as-is).
     The result is written to the worktree file. Files with SKIP (changed on both
     sides) or UNEXPECTED residue stay unmerged in the index for hand resolution.
  4. atys side with --restore-ws: whitespace clean-up of the auto-merged files.
Nothing is committed. Review TAG.apply and TAG_conf.txt, finish the hand cases,
then `git add` and `git commit` in WT.
"""
import sys, os, re, subprocess

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import tokdiff  # noqa: E402

args = [a for a in sys.argv[1:] if not a.startswith('--')]
opts = dict(a[2:].split('=', 1) if '=' in a else (a[2:], True) for a in sys.argv[1:] if a.startswith('--'))
if len(args) != 2 or opts.get('side') not in ('atys', 'core4') or 'base' not in opts:
    print(__doc__)
    sys.exit(2)
WT, THEIRS = args
SIDE = opts['side']
BASE = opts['base']
REF = opts.get('ref')
TAG = opts.get('tag', 'step')
PY = opts.get('python', sys.executable)
ADD_CLEAN = 'add-clean' in opts
RESTORE_WS = 'restore-ws' in opts
LOGDIR = os.path.dirname(os.path.abspath(WT))
TMP = os.path.join(LOGDIR, TAG + '_tmp')
os.makedirs(TMP, exist_ok=True)

POLICY = re.compile(r'^(web/|README|\.gitlab|\.github/|azure-pipelines|CMakeLists\.txt|CMakeModules/|cmake/|CMakePresets|vcpkg|'
                    r'nel/include/nel/misc/cdb|nel/src/misc/cdb|nel/include/nel/misc/ucstring|'
                    r'ryzom/common/src/game_share/(inventories|msg_client_server|item_type|generate_|msg\.xml|database)|'
                    r'ryzom/common/data_common/(database\.xml|msg\.xml)|ryzom/client/client_default\.cfg|'
                    r'ryzom/server/src/entities_game_service/guild_manager/database_|ryzom/server/src/entities_game_service/player_manager/database_|'
                    r'ryzom/server/src/server_share/.*_itf\.(h|cpp)$|ryzom/common/src/game_share/.*_itf\.(h|cpp)$|.*/chat_unifier_itf)')


def git(*a, check=False, text=True):
    r = subprocess.run(['git', '-c', 'core.autocrlf=false'] + list(a), cwd=WT, capture_output=True)
    if check and r.returncode:
        sys.exit('git %s failed:\n%s' % (' '.join(a), r.stderr.decode('utf-8', 'replace')))
    return r.stdout.decode('utf-8', 'surrogateescape') if text else r.stdout


def blob(spec):
    r = subprocess.run(['git', 'cat-file', '-p', spec], cwd=WT, capture_output=True)
    return r.stdout if r.returncode == 0 else None


def write_tmp(name, data):
    p = os.path.join(TMP, name)
    open(p, 'wb').write(data or b'')
    return p


def lang_for(path):
    ext = os.path.splitext(path)[1].lower()
    if ext in ('.cpp', '.h', '.c', '.inl', '.hpp', '.cc'):
        return 'c'
    if ext in ('.xml', '.xslt', '.html', '.php'):
        return 'xml'
    return 'plain'


# 0. preconditions
if git('status', '--porcelain').strip():
    sys.exit('worktree %s is not clean' % WT)
head = git('rev-parse', '--short', 'HEAD').strip()
theirs = git('rev-parse', '--short', THEIRS).strip()
mb = git('merge-base', 'HEAD', THEIRS).strip()[:9]
print('branch %s  <- %s side %s  (merge-base %s; base %s%s)' % (head, SIDE, theirs, mb, BASE, ', ref ' + REF if REF else ''))
print('window %s..%s: %s commits, %s' % (BASE, theirs, git('rev-list', '--count', BASE + '..' + THEIRS).strip(),
                                           git('diff', '--shortstat', BASE, THEIRS).strip() or 'no diff'))

# 1. merge
r = subprocess.run(['git', '-c', 'core.autocrlf=false', '-c', 'merge.renameLimit=30000', 'merge', '--no-commit', '--no-ff',
                    '-X', 'renormalize', THEIRS], cwd=WT, capture_output=True)
open(os.path.join(LOGDIR, TAG + '_merge.log'), 'wb').write(r.stdout + r.stderr)
conflicts = [l for l in git('diff', '--name-only', '--diff-filter=U').split('\n') if l.strip()]
open(os.path.join(LOGDIR, TAG + '_conf.txt'), 'w', encoding='utf-8', newline='\n').write('\n'.join(conflicts) + '\n')
status = {}
for l in git('status', '--short').split('\n'):
    if l[:2] in ('UU', 'AU', 'UA', 'DU', 'UD', 'AA', 'DD'):
        status[l[3:]] = l[:2]
print('\n== merge exit %d: %d conflicts (%s)' % (r.returncode, len(conflicts),
      ', '.join('%s=%d' % (k, v) for k, v in sorted(__import__('collections').Counter(status.values()).items()))))

# 2. what the auto-merge touched
staged = [l.split('\t', 1) for l in git('diff', '--cached', '--name-status', 'HEAD').split('\n') if '\t' in l]
areas = __import__('collections').Counter('/'.join(p.split('/')[:2]) for _, p in staged)
print('== auto-merged: %d files; by area: %s' % (len(staged), ', '.join('%s %d' % (a, n) for a, n in areas.most_common(12))))
pol = [(s, p) for s, p in staged if POLICY.match(p)] + [('U', p) for p in conflicts if POLICY.match(p)]
if pol:
    print('== POLICY-SENSITIVE (check by hand, see README):')
    for s, p in pol:
        print('   %s %s' % (s, p))

# 3. trees one side owns outright: atys owns them, so atys steps take theirs, core4 steps keep ours
OWNED = ('web/', 'ryzom/server/www/', 'README.md', 'logs/', 'docs/', 'macosx-build/', 'win-build/')
owned = [f for f in conflicts if f.startswith(OWNED)]
if owned:
    which = '--theirs' if SIDE == 'atys' else '--ours'
    for f in owned:
        if blob((':3:' if SIDE == 'atys' else ':2:') + f) is None:
            git('rm', '-q', '--', f)
        else:
            git('checkout', which, '--', f)
            git('add', '--', f)
    print('== atys-owned trees: %d conflicted files taken %s and staged' % (len(owned), which))
    conflicts = [f for f in conflicts if f not in owned]

# 4. applier
MECH = re.compile(r": '([^']*)' / '([^']*)'\s*$")


def mech_only(line):
    m = MECH.search(line)
    if not m:
        return False
    a, b = m.groups()
    norm = lambda x: x.replace('NL_OVERRIDE', '').replace('NULL', 'nullptr').replace('0', 'nullptr').replace(' ', '')
    return norm(a) == norm(b)


report = open(os.path.join(LOGDIR, TAG + '.apply'), 'w', encoding='utf-8', newline='\n')
clean, hand, onesided = [], [], []
for f in conflicts:
    st = status.get(f, 'UU')
    ours = blob(':2:' + f)
    theirs_b = blob(':3:' + f)
    if ours is None or theirs_b is None:
        onesided.append((st, f))
        report.write('%s\t%s one-sided (%s), decide by hand\n' % (f, st, 'deleted on ours' if ours is None else 'deleted on theirs'))
        continue
    pa = write_tmp('ours.tmp', ours)
    pb = write_tmp('theirs.tmp', theirs_b)
    pbase = write_tmp('base.tmp', blob(BASE + ':' + f))
    pref = write_tmp('ref.tmp', blob(REF + ':' + f) if REF else None)
    out = os.path.join(WT, f)
    open(out, 'wb').write(ours)  # start from ours so a crash leaves a sane file
    cmd = [PY, '-I', os.path.join(HERE, 'tokapply.py'), pa, pb, '--base=' + pbase, '--out=' + out, '--lang=' + lang_for(f)]
    if REF:
        cmd.append('--ref=' + pref)
    cmd.append('--mech' if SIDE == 'atys' else '--verbatim')
    rr = subprocess.run(cmd, capture_output=True)
    text = (rr.stdout + rr.stderr).decode('utf-8', 'replace')
    first = text.strip().split('\n')[0] if text.strip() else '(no output)'
    report.write('%s\t%s\n' % (f, first))
    flagged = [l for l in text.split('\n') if 'SKIP' in l or 'UNEXPECTED' in l]
    for l in flagged:
        report.write('%s %s\n' % (f, l))
    # in --mech mode a residual nullptr / NL_OVERRIDE difference against the untidied atys text is expected
    bad = [l for l in flagged if not (SIDE == 'atys' and 'UNEXPECTED' in l and mech_only(l))]
    if not bad and (rr.returncode == 0 or SIDE == 'atys'):
        clean.append(f)
        if ADD_CLEAN:
            git('add', '--', f)
    else:
        hand.append((f, len(bad)))
report.close()
print('\n== applier: %d resolved cleanly%s, %d need hand work, %d one-sided' % (
    len(clean), ' (staged)' if ADD_CLEAN else ' (NOT staged, review then git add)', len(hand), len(onesided)))
for f, n in hand:
    print('   HAND %s (%d skip/residue lines)' % (f, n))
for st, f in onesided:
    print('   %s   %s' % (st, f))
print('   details: %s' % os.path.join(LOGDIR, TAG + '.apply'))

# 5. whitespace clean-up of the auto-merge (atys side only)
if SIDE == 'atys':
    fmt_only, mixed = [], []
    # files the applier resolved and staged get the same treatment: --mech re-wraps applied blocks, but
    # spacing inside a line atys changed (e.g. "nullptr );") still comes from atys
    if ADD_CLEAN:
        mixed.extend(clean)
    for s, p in staged:
        if s[0] != 'M' or p.startswith(OWNED) or lang_for(p) == 'plain' and not p.endswith(('.lua', '.txt', '.cfg', '.php', '.py', '.cmake')):
            continue
        a = blob('HEAD:' + p)
        b = blob(':0:' + p)
        if a is None or b is None or a == b:
            continue
        lang = tokdiff.lang_for(p)
        ta = [t for t, _ in tokdiff.tokenize(a.decode('utf-8', 'surrogateescape'), lang)]
        tb = [t for t, _ in tokdiff.tokenize(b.decode('utf-8', 'surrogateescape'), lang)]
        if ta == tb:
            fmt_only.append(p)
        else:
            mixed.append(p)
    print('\n== auto-merged files whose atys change is formatting only: %d' % len(fmt_only))
    for p in fmt_only:
        print('   FMT  %s' % p)
    if RESTORE_WS:
        if fmt_only:
            git('checkout', 'HEAD', '--', *fmt_only)
            print('   restored to HEAD')
        if mixed:
            ws = os.path.join(HERE, 'ws_restore.py')
            for mode in (['--lines'], []):
                rr = subprocess.run([PY, '-I', ws, WT, '--apply'] + mode + mixed, capture_output=True)
                for l in rr.stdout.decode('utf-8', 'replace').split('\n'):
                    if l.strip() and 'restored=0' not in l:
                        print('   ' + l)
    else:
        print('   (pass --restore-ws to restore them and to strip pure-whitespace hunks from the %d mixed files)' % len(mixed))

print('\nNext: resolve the HAND and one-sided files, check the policy list, git add, build, commit:')
print('  Merge %s %s (%s) into <branch>' % (SIDE, theirs, git('log', '-1', '--format=%ad', '--date=short', THEIRS).strip()))
