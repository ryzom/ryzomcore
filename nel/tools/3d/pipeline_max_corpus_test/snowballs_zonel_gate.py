#!/usr/bin/env python3
"""Era-codec byte gate: era_pack(raw lumels) vs reference CompressedLumels."""
import struct, sys

F0C0=[7,0,6,5,4,3,2,1]; F1C0=[0,7,1,2,3,4,5,6]
F0C1=[5,0,4,3,2,1]; F1C1=[0,5,1,2,3,4]

def era_unpack(src):
    a0,a1=src[0],src[1]
    vals=[0]*8
    if a0>a1:
        for i in range(8): vals[i]=(F0C0[i]*a0+F1C0[i]*a1)//7
    else:
        for i in range(6): vals[i]=(F0C1[i]*a0+F1C1[i]*a1)//5
        vals[6]=0; vals[7]=255
    out=[]
    for half in range(2):
        blk=(src[2+half*3]<<16)|(src[3+half*3]<<8)|src[4+half*3]
        for n in range(8):
            out.append(vals[(blk>>21)&7]); blk=(blk<<3)&0xFFFFFF
    return out

def era_pack(block16):
    amin=min(block16); amax=max(block16)
    best=None
    for a0,a1 in ((amin,amax),(amax,amin)):
        vals=[0]*8
        if a0>a1:
            for i in range(8): vals[i]=(F0C0[i]*a0+F1C0[i]*a1)//7
        else:
            for i in range(6): vals[i]=(F0C1[i]*a0+F1C1[i]*a1)//5
            vals[6]=0; vals[7]=255
        codes=[]
        for s in block16:
            bd=10000; bc=0
            for c in range(8):
                d=abs(s-vals[c])
                if d<bd: bd=d; bc=c
                if d==0: break
            codes.append(bc)
        dest=[a0,a1,0,0,0,0,0,0]
        off=2; shift=5
        for c in codes:
            if shift>=0: dest[off]|=(c<<shift)&0xFF
            else:
                dest[off]|=(c>>(-shift))&0xFF
                dest[off+1]|=(c<<(8+shift))&0xFF
            shift-=3
            if shift<=-3:
                off+=1; shift+=8
        unp=era_unpack(bytes(dest))
        err=sum(abs(a-b) for a,b in zip(block16,unp))
        if best is None or err<best[0]: best=(err,bytes(dest))
    return best[1]

def parse_zone_lumdata(path):
    d=open(path,'rb').read()
    o=1+4+2+1+24+12+4+4
    nb,=struct.unpack_from('<I',d,o); o+=4
    o+=nb*7
    npatch,=struct.unpack_from('<I',d,o); o+=4
    out=[]
    for pi in range(npatch):
        pver=d[o]; o+=1
        o+=24+48+24
        ntiles,=struct.unpack_from('<I',d,o); o+=4
        o+=ntiles*8
        ncol,=struct.unpack_from('<I',d,o); o+=4
        o+=ncol*(2 if pver>=7 else 5)
        os_,ot_=d[o],d[o+1]; o+=2
        nlum,=struct.unpack_from('<I',d,o); o+=4
        comp=d[o:o+nlum]; o+=nlum
        if pver>=3: o+=2
        if pver>=4: o+=1
        if pver>=5:
            ntli,=struct.unpack_from('<I',d,o); o+=4
            o+=ntli*3
        out.append((os_,ot_,comp))
    return out

def read_raw(path):
    d=open(path,'rb').read(); n,=struct.unpack_from('<I',d,0); o=4; out=[]
    for i in range(n):
        os_,ot_=d[o],d[o+1]; o+=2
        nl,=struct.unpack_from('<I',d,o); o+=4
        out.append((os_,ot_,d[o:o+nl])); o+=nl
    return out

def gate(refzonel, rawpath, verbose=True):
    REF=parse_zone_lumdata(refzonel); RAW=read_raw(rawpath)
    btot=bmatch=0; vtot=vex=w3=0
    for (os_,ot_,comp),(_,_,raw) in zip(REF,RAW):
        ns,nt=os_*4,ot_*4
        if len(raw)!=ns*nt: continue
        for tt in range(ot_):
            for ts in range(os_):
                block=[raw[ts*4+u + (tt*4+v)*ns] for v in range(4) for u in range(4)]
                ours=era_pack(block)
                refb=comp[(ts+tt*os_)*8:(ts+tt*os_)*8+8]
                btot+=1
                if ours==refb: bmatch+=1
                ru=era_unpack(refb); ou=era_unpack(ours)
                for a,b in zip(ou,ru):
                    vtot+=1
                    if a==b: vex+=1
                    if abs(a-b)<=3: w3+=1
    if verbose:
        print("blocks byte-identical %d/%d (%.2f%%)  values exact %.2f%%  within+-3 %.1f%%"%(bmatch,btot,100.0*bmatch/btot,100.0*vex/vtot,100.0*w3/vtot))
    return bmatch,btot,vex,vtot

if __name__=="__main__":
    gate(sys.argv[1], sys.argv[2])
