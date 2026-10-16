"""override_skips.py WT APPLY_REPORT [--apply]

For every 'SKIP both-changed: A a1-a2  B b1-b2' line of a tokapply report
produced while merging core4's NL_OVERRIDE series: take the methods that
core4 annotated with NL_OVERRIDE in the B range (stage 3), find the
matching declaration in the original ours block (stage 2, around the A
range), and append NL_OVERRIDE to it the way core4 does. The ours block
must occur exactly once in the current worktree file. Everything that
cannot be matched unambiguously is listed for manual review."""
import sys, re, os, subprocess
wt, report = sys.argv[1], sys.argv[2]
apply = '--apply' in sys.argv


def stage(n, path):
    out = subprocess.run(['git', 'cat-file', '-p', ':%d:%s' % (n, path)], cwd=wt, capture_output=True).stdout
    return out.decode('utf-8', errors='surrogateescape').replace('\r\n', '\n').split('\n')


def method_name(text):
    """identifier owning the parameter list that closes right before the last NL_OVERRIDE"""
    k = text.rfind('NL_OVERRIDE')
    if k < 0:
        return None
    head = re.sub(r'\s*const\s*$', '', text[:k].rstrip())
    if not head.endswith(')'):
        return None
    depth = 0
    i = len(head) - 1
    while i >= 0:
        c = head[i]
        if c == ')':
            depth += 1
        elif c == '(':
            depth -= 1
            if depth == 0:
                break
        i -= 1
    if i < 0:
        return None
    m = re.search(r'([A-Za-z_~]\w*)\s*$', head[:i])
    return m.group(1) if m else None


def norm(s):
    return re.sub(r'\s+', '', s.split('NL_OVERRIDE')[0])


def cp(a, b):
    return len(os.path.commonprefix([a, b]))


def annotate(block, targets):
    """return (new_block, None) or (None, error)"""
    new_block = list(block)
    for name, btail in targets:
        hits = [i for i, l in enumerate(new_block) if re.search(r'\b' + re.escape(name) + r'\s*\(', l) and 'NL_OVERRIDE' not in l]
        if len(hits) > 1:
            scored = sorted(hits, key=lambda i: -cp(norm(new_block[i]), btail))
            best, second = scored[0], scored[1]
            if cp(norm(new_block[best]), btail) >= 12 and cp(norm(new_block[best]), btail) > cp(norm(new_block[second]), btail):
                hits = [best]
        if len(hits) != 1:
            return None, '%s: %d candidate declaration(s)' % (name, len(hits))
        i = hits[0]
        j = i
        end_found = False
        while j < len(new_block):
            s = new_block[j].rstrip()
            if s.endswith(';') or '{' in s:
                end_found = True
                break
            if (s.endswith(')') or s.endswith('const')) and j + 1 < len(new_block) and new_block[j + 1].strip().startswith('{'):
                end_found = True
                break
            j += 1
        if not end_found:
            return None, '%s: declaration end not found' % name
        s = new_block[j].rstrip()
        if re.search(r'=\s*0\s*;$', s):
            new_block[j] = re.sub(r'\s*=\s*0\s*;$', ' NL_OVERRIDE = 0;', s)
        elif s.endswith(';'):
            new_block[j] = s[:-1].rstrip() + ' NL_OVERRIDE;'
        elif '{' in s:
            k = s.index('{')
            new_block[j] = s[:k].rstrip() + ' NL_OVERRIDE ' + s[k:]
        else:
            new_block[j] = s + ' NL_OVERRIDE'
    return new_block, None


skips = {}
for line in open(report, encoding='utf-8', errors='replace'):
    m = re.match(r'^(\S+)\s+SKIP both-changed: A (\d+)-(\d+)\s+B (\d+)-(\d+)', line)
    if m:
        skips.setdefault(m.group(1), []).append(tuple(int(x) for x in m.groups()[1:]))

total = fixed = 0
manual = []
for path, ranges in skips.items():
    ours = stage(2, path)
    theirs = stage(3, path)
    fp = wt + '/' + path
    raw = open(fp, encoding='utf-8', errors='surrogateescape', newline='').read()
    crlf = '\r\n' in raw[:5000]
    cur = raw.replace('\r\n', '\n')
    for a1, a2, b1, b2 in ranges:
        total += 1
        targets = []
        for i in range(b1 - 1, b2):
            l = theirs[i]
            if 'NL_OVERRIDE' not in l:
                continue
            ctx = ' '.join(x.strip() for x in theirs[max(0, i - 6):i + 1])
            name = method_name(ctx)
            if name:
                targets.append((name, norm(l)))
        if not targets:
            manual.append((path, a1, a2, b1, b2, 'no NL_OVERRIDE method found in B range'))
            continue
        result = None
        last_err = 'no block'
        for up in range(0, 9):
            for down in range(0, 4):
                lo, hi = max(1, a1 - up), min(len(ours), a2 + down)
                block = ours[lo - 1:hi]
                old = '\n'.join(block)
                if not old.strip():
                    continue
                if cur.count(old) != 1:
                    last_err = 'ours block not unique in worktree'
                    continue
                new_block, err = annotate(block, targets)
                if err:
                    last_err = err
                    continue
                result = (old, '\n'.join(new_block))
                break
            if result:
                break
        if not result:
            manual.append((path, a1, a2, b1, b2, last_err))
            continue
        old, new = result
        if apply:
            cur = cur.replace(old, new)
        fixed += 1
        print('%s A %d-%d: annotated %s' % (path, a1, a2, ', '.join(n for n, _ in targets)))
    if apply and cur != raw.replace('\r\n', '\n'):
        open(fp, 'w', encoding='utf-8', errors='surrogateescape', newline='').write(cur.replace('\n', '\r\n') if crlf else cur)
print('skips=%d annotated=%d manual=%d' % (total, fixed, len(manual)))
for m in manual:
    print('  MANUAL %s A %d-%d B %d-%d: %s' % m)
