"""gen_db.py REPO OUTDIR : regenerate the server-side CDB bank accessors from
ryzom/common/data_common/database.xml with generate_client_db.xslt (lxml),
the way tool/quick_start/win32/install/regenerate_xslt.bat does with msxsl."""
import sys, os
from lxml import etree
repo, outdir = sys.argv[1], sys.argv[2]
xml = etree.parse(os.path.join(repo, 'ryzom/common/data_common/database.xml'))
xslt = etree.XSLT(etree.parse(os.path.join(repo, 'ryzom/common/src/game_share/generate_client_db.xslt')))
os.makedirs(outdir, exist_ok=True)
for bank in ('PLR', 'GUILD', 'OUTPOST'):
    for output, ext in (('header', '.h'), ('cpp', '.cpp')):
        res = xslt(xml, output=etree.XSLT.strparam(output), side=etree.XSLT.strparam('server'), bank=etree.XSLT.strparam(bank))
        text = str(res)
        name = 'database_' + bank.lower() + ext
        with open(os.path.join(outdir, name), 'w', encoding='utf-8', newline='') as f:
            f.write(text)
        print(name, len(text.split('\n')), 'lines')
