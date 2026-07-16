#!/usr/bin/env python3
"""Extract and decode the thumbnail of a 3ds Max .max file.

The thumbnail lives in the OLE \x05SummaryInformation property-set stream, property 17
(PIDSI_THUMBNAIL), type VT_CF (71): [cb u32][cfFormat i32; -1 = a u32 Windows clipboard
format id follows][data]. Max stores CF_DIB (8): a BITMAPINFOHEADER + palette + bottom-up
pixel rows. Output: PNG (minimal encoder, no PIL needed).

Usage: maxthumb.py <file.max> <out.png>
"""
import sys, struct, zlib, os

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__))))
# maxole.py lives next to this script
from maxole import OLEFile


def extract_dib(path):
    d = OLEFile(path).stream('\x05SummaryInformation')
    if d is None:
        raise RuntimeError('no SummaryInformation stream')
    bo, fmt = struct.unpack('<HH', d[0:4])
    if bo != 0xFFFE:
        raise RuntimeError('bad property set byte order')
    secoff, = struct.unpack('<I', d[44:48])
    size, nprops = struct.unpack('<II', d[secoff:secoff + 8])
    for i in range(nprops):
        pid, off = struct.unpack('<II', d[secoff + 8 + i * 8:secoff + 16 + i * 8])
        if pid != 17:
            continue
        p = secoff + off
        vt, = struct.unpack('<I', d[p:p + 4])
        if vt != 71:
            raise RuntimeError('thumbnail property is not VT_CF (%d)' % vt)
        cb, cf = struct.unpack('<Ii', d[p + 4:p + 12])
        q = p + 12
        if cf == -1:
            cfid, = struct.unpack('<I', d[q:q + 4])
            q += 4
        else:
            cfid = cf
        return cfid, d[q:p + 8 + cb]
    raise RuntimeError('no thumbnail property (17)')


def dib_to_rgb(dib):
    (biSize, w, h, planes, bpp, comp, sizeImage, xppm, yppm, clrUsed, clrImp) = \
        struct.unpack('<IiiHHIIiiII', dib[0:40])
    if comp != 0:
        raise RuntimeError('compressed DIB (%d) not handled' % comp)
    off = biSize
    pal = []
    if bpp <= 8:
        n = clrUsed or (1 << bpp)
        for i in range(n):
            b, g, r, _ = struct.unpack('<BBBB', dib[off + i * 4:off + i * 4 + 4])
            pal.append((r, g, b))
        off += n * 4
    stride = ((w * bpp + 31) // 32) * 4
    topdown = h < 0
    hh = abs(h)
    rows = []
    for y in range(hh):
        src = y if topdown else (hh - 1 - y)
        row = dib[off + src * stride: off + src * stride + stride]
        out = bytearray()
        if bpp == 8:
            for x in range(w):
                r, g, b = pal[row[x]]
                out += bytes((r, g, b))
        elif bpp == 24:
            for x in range(w):
                b, g, r = row[x * 3:x * 3 + 3]
                out += bytes((r, g, b))
        elif bpp == 32:
            for x in range(w):
                b, g, r = row[x * 4:x * 4 + 3]
                out += bytes((r, g, b))
        elif bpp == 16:
            for x in range(w):
                v, = struct.unpack('<H', row[x * 2:x * 2 + 2])
                r = (v >> 10 & 31) << 3; g = (v >> 5 & 31) << 3; b = (v & 31) << 3
                out += bytes((r, g, b))
        else:
            raise RuntimeError('bpp %d not handled' % bpp)
        rows.append(bytes(out))
    return w, hh, rows


def write_png(path, w, h, rows):
    def chunk(tag, data):
        c = struct.pack('>I', len(data)) + tag + data
        return c + struct.pack('>I', zlib.crc32(tag + data) & 0xffffffff)
    raw = b''.join(b'\x00' + r for r in rows)
    png = b'\x89PNG\r\n\x1a\n'
    png += chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0))
    png += chunk(b'IDAT', zlib.compress(raw, 9))
    png += chunk(b'IEND', b'')
    open(path, 'wb').write(png)


def find_dib_in_metafile(blob):
    """Max 9 stores CF_METAFILEPICT: an 8-byte METAFILEPICT header (mm, xExt, yExt as u16s),
    then a memory WMF whose META_STRETCHDIB record carries the BITMAPINFOHEADER + pixels.
    Locate the header by signature (biSize == 40, sane dims, plausible bpp)."""
    for i in range(0, len(blob) - 40, 2):
        biSize, w, h = struct.unpack('<Iii', blob[i:i + 12])
        if biSize == 40 and 8 <= w <= 4096 and 8 <= abs(h) <= 4096:
            planes, bpp, comp = struct.unpack('<HHI', blob[i + 12:i + 20])
            if planes == 1 and bpp in (1, 4, 8, 16, 24, 32) and comp == 0:
                return blob[i:]
    raise RuntimeError('no BITMAPINFOHEADER found in metafile blob')


if __name__ == '__main__':
    cfid, dib = extract_dib(sys.argv[1])
    print('clipboard format %d (%s), blob %d bytes' % (cfid, {8: 'CF_DIB', 2: 'CF_BITMAP', 3: 'CF_METAFILEPICT'}.get(cfid, '?'), len(dib)))
    if cfid == 3:
        dib = find_dib_in_metafile(dib)
    w, h, rows = dib_to_rgb(dib)
    print('thumbnail %dx%d' % (w, h))
    write_png(sys.argv[2], w, h, rows)
    print('wrote', sys.argv[2])
