"""remerge_files.py WT BASE THEIRS file...
Reset each file in the worktree+index to git's plain three-way auto-merge
result (ours=HEAD, base=BASE, theirs=THEIRS), preserving CRLF blobs."""
import sys, subprocess, os, tempfile
wt, base, theirs = sys.argv[1:4]
files = sys.argv[4:]
def show(spec):
    return subprocess.run(['git', 'show', spec], cwd=wt, capture_output=True).stdout
for f in files:
    parts = {}
    for name, spec in (('ours', 'HEAD:' + f), ('base', base + ':' + f), ('theirs', theirs + ':' + f)):
        raw = show(spec)
        parts[name] = raw
    crlf = b'\r\n' in parts['ours']
    tmp = {}
    for name, raw in parts.items():
        path = os.path.join(tempfile.gettempdir(), 'rm_' + name + '.tmp')
        open(path, 'wb').write(raw.replace(b'\r\n', b'\n'))
        tmp[name] = path
    r = subprocess.run(['git', 'merge-file', '-p', tmp['ours'], tmp['base'], tmp['theirs']], cwd=wt, capture_output=True)
    if r.returncode < 0 or r.returncode > 0 and b'<<<<<<<' in r.stdout:
        print(f'{f}: CONFLICT in plain merge (rc={r.returncode}), left untouched')
        continue
    out = r.stdout
    if crlf:
        out = out.replace(b'\n', b'\r\n')
    open(os.path.join(wt, f), 'wb').write(out)
    subprocess.run(['git', 'add', f], cwd=wt, capture_output=True)
    print(f'{f}: reset to auto-merge result (crlf={crlf})')
