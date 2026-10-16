#!/usr/bin/env python3
"""Syntax check of the non-C++ files in the tree: PHP, Lua, XML, HTML, CSS, JavaScript, JSON,
YAML, Python and shell. Every file must at least parse with its language's own tooling.

Usage: syntax_check.py [--root DIR] [--kinds php,lua,xml,html,css,js,json,yaml,python,shell]
                       [--jobs N] [--exclude-file FILE] [--list]

Files come from `git ls-files` (or a directory walk when DIR is not a checkout). Patterns in the
exclude file (tool/ci/syntax_check.exclude, one shell glob per line, # comments) are skipped.
Checkers are external tools, overridable through the environment:
  PHP (php -l), LUAC (luac -p), XMLLINT (xmllint --noout --nonet), TIDY (tidy -errors),
  NODE (node --check, and tool/ci/css_check.js with postcss on NODE_PATH), BASH (bash -n).
JSON and Python use this interpreter; YAML needs PyYAML and is skipped with a notice without it.
Exit status is 1 when any file fails, 2 when a requested checker's tool is missing.
"""
import os, sys, re, subprocess, shutil, fnmatch, json, argparse, py_compile
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
KINDS = {
    'php': ('.php', '.inc'),
    'lua': ('.lua',),
    'xml': ('.xml', '.xslt', '.xsl', '.xsd', '.primitive', '.svg'),
    'html': ('.html', '.htm'),
    'css': ('.css',),
    'js': ('.js', '.mjs', '.cjs'),
    'json': ('.json',),
    'yaml': ('.yml', '.yaml'),
    'python': ('.py',),
    'shell': ('.sh',),
}


def tool(env, *candidates):
    v = os.environ.get(env)
    if v:
        return v
    for c in candidates:
        if shutil.which(c):
            return c
    return None


def run(cmd, cwd=None):
    r = subprocess.run(cmd, cwd=cwd, capture_output=True)
    out = (r.stdout + r.stderr).decode('utf-8', 'replace')
    return r.returncode, out


def first_line(out, skip=()):
    for l in out.splitlines():
        l = l.strip()
        if l and not any(s in l for s in skip):
            return l[:300]
    return out.strip()[:300]


# ---- checkers: each takes (root, relpath) and returns None (ok) or a message ----
def check_php(root, f):
    rc, out = run([T['php'], '-n', '-d', 'display_errors=stderr', '-d', 'error_reporting=E_ALL', '-l', f], cwd=root)
    return None if rc == 0 else first_line(out, skip=('Errors parsing',))


def check_lua(root, f):
    rc, out = run([T['luac'], '-p', f], cwd=root)
    return None if rc == 0 else first_line(out)


def check_xml(root, f):
    if T.get('xmllint'):
        rc, out = run([T['xmllint'], '--noout', '--nonet', f], cwd=root)
        return None if rc == 0 else first_line(out)
    from lxml import etree  # local fallback
    try:
        etree.parse(os.path.join(root, f), etree.XMLParser(huge_tree=True, resolve_entities=False, no_network=True))
        return None
    except Exception as e:
        return str(e)[:300]


def check_html(root, f):
    # errors only: unknown or unbalanced markup; warnings (missing doctype, deprecated attributes) are not failures
    rc, out = run([T['tidy'], '-config', os.path.join(HERE, 'tidy.conf'), '-errors', f], cwd=root)
    if rc < 2:
        return None
    errs = [l.strip() for l in out.splitlines() if 'Error:' in l]
    return (errs[0] if errs else first_line(out))[:300]


def check_css(root, f):
    rc, out = run([T['node'], os.path.join(HERE, 'css_check.js'), f], cwd=root)
    return None if rc == 0 else first_line(out)


def check_js(root, f):
    rc, out = run([T['node'], '--check', f], cwd=root)
    if rc == 0:
        return None
    lines = [l for l in out.splitlines() if 'Error' in l]
    return (lines[0] if lines else first_line(out))[:300]


def check_json(root, f):
    try:
        with open(os.path.join(root, f), 'rb') as fh:
            json.loads(fh.read().decode('utf-8-sig'))
        return None
    except Exception as e:
        return str(e)[:300]


def check_yaml(root, f):
    import yaml
    try:
        with open(os.path.join(root, f), 'rb') as fh:
            list(yaml.safe_load_all(fh.read().decode('utf-8', 'replace')))
        return None
    except Exception as e:
        return str(e).replace('\n', ' ')[:300]


def check_python(root, f):
    import warnings
    try:
        with open(os.path.join(root, f), 'rb') as fh, warnings.catch_warnings():
            warnings.simplefilter('ignore')  # SyntaxWarning (invalid escape sequences) is not a parse failure
            compile(fh.read(), f, 'exec')
        return None
    except SyntaxError as e:
        return 'line %s: %s' % (e.lineno, e.msg)
    except Exception as e:
        return str(e)[:300]


def check_shell(root, f):
    rc, out = run([T['bash'], '-n', f], cwd=root)
    return None if rc == 0 else first_line(out)


CHECKERS = {'php': check_php, 'lua': check_lua, 'xml': check_xml, 'html': check_html, 'css': check_css,
            'js': check_js, 'json': check_json, 'yaml': check_yaml, 'python': check_python, 'shell': check_shell}
T = {}


def list_files(root):
    try:
        r = subprocess.run(['git', '-C', root, 'ls-files', '-z'], capture_output=True)
        if r.returncode == 0:
            return [p for p in r.stdout.decode('utf-8', 'surrogateescape').split('\0') if p]
    except OSError:
        pass
    files = []
    for d, dirs, names in os.walk(root):
        dirs[:] = [x for x in dirs if x not in ('.git', 'node_modules')]
        for n in names:
            files.append(os.path.relpath(os.path.join(d, n), root).replace(os.sep, '/'))
    return files


def load_excludes(path):
    pats = []
    if path and os.path.exists(path):
        for l in open(path, encoding='utf-8'):
            l = l.strip()
            if l and not l.startswith('#'):
                pats.append(l)
    return pats


def excluded(f, pats):
    return any(fnmatch.fnmatchcase(f, p) or fnmatch.fnmatchcase(f, p.rstrip('/') + '/*') for p in pats)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--root', default=None)
    ap.add_argument('--kinds', default=','.join(KINDS))
    ap.add_argument('--jobs', type=int, default=os.cpu_count() or 4)
    ap.add_argument('--exclude-file', default=None)
    ap.add_argument('--list', action='store_true', help='only list the files that would be checked')
    a = ap.parse_args()
    # default root: the checkout this script lives in (tool/ci/..), so no git is needed to find it
    root = os.path.abspath(a.root or os.path.dirname(os.path.dirname(HERE)))
    kinds = [k for k in a.kinds.split(',') if k]
    bad = [k for k in kinds if k not in KINDS]
    if bad:
        sys.exit('unknown kinds: %s' % ', '.join(bad))
    excl = load_excludes(a.exclude_file or os.path.join(root, 'tool', 'ci', 'syntax_check.exclude'))

    T.update({'php': tool('PHP', 'php'), 'luac': tool('LUAC', 'luac5.4', 'luac54', 'luac', 'luac5.1'),
              'xmllint': tool('XMLLINT', 'xmllint'), 'tidy': tool('TIDY', 'tidy'), 'node': tool('NODE', 'node'),
              'bash': tool('BASH', 'bash')})
    needs = {'php': 'php', 'lua': 'luac', 'html': 'tidy', 'css': 'node', 'js': 'node', 'shell': 'bash'}
    missing = [k for k in kinds if k in needs and not T.get(needs[k])]
    if 'xml' in kinds and not T.get('xmllint'):
        try:
            import lxml  # noqa: F401
        except ImportError:
            missing.append('xml')
    if 'yaml' in kinds:
        try:
            import yaml  # noqa: F401
        except ImportError:
            print('yaml: PyYAML not installed, skipped')
            kinds.remove('yaml')
    if missing:
        print('missing tools for: %s' % ', '.join(missing))
        sys.exit(2)

    files = list_files(root)
    work = []
    skipped = 0
    for f in files:
        low = f.lower()
        for k in kinds:
            if low.endswith(KINDS[k]):
                if excluded(f, excl):
                    skipped += 1
                else:
                    work.append((k, f))
                break
    if a.list:
        for k, f in work:
            print(k, f)
        return
    counts = {k: 0 for k in kinds}
    for k, _ in work:
        counts[k] += 1
    print('checking %d files in %s (%d excluded): %s' % (len(work), root, skipped,
          ', '.join('%s %d' % (k, counts[k]) for k in kinds if counts[k])))
    failures = {k: [] for k in kinds}
    with ThreadPoolExecutor(max_workers=a.jobs) as ex:
        for (k, f), msg in zip(work, ex.map(lambda w: CHECKERS[w[0]](root, w[1]), work)):
            if msg:
                failures[k].append((f, msg))
    total = 0
    for k in kinds:
        for f, msg in sorted(failures[k]):
            print('%s  %s: %s' % (k.upper().ljust(6), f, msg))
        total += len(failures[k])
    print('result: %d failure(s) in %d files; %s' % (total, len(work),
          ', '.join('%s %d/%d' % (k, len(failures[k]), counts[k]) for k in kinds if counts[k])))
    sys.exit(1 if total else 0)


if __name__ == '__main__':
    main()
