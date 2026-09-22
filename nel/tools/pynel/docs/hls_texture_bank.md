# `.hlsbank` / `.hlsinfo` — HLS-colorisable texture bank

Status: **implemented** (`pynel.hls_texture_bank`, `pynel.hls_bank_texture_info`,
`ryzom-hlsbank` CLI). Source of truth: `nel/include/nel/3d/hls_texture_bank.h/.cpp`,
`nel/include/nel/3d/hls_color_texture.h/.cpp` (`.hlsbank`), and
`nel/tools/3d/panoply_maker/hls_bank_texture_info.h/.cpp` (`.hlsinfo`).

## Why this exists

`characters.hlsbank` (loaded via `Driver->loadHLSBank()`, see
`init_main_loop.cpp:914`) is what lets `CAsyncTextureManager` recolor LOD
character textures at runtime by shifting DXTC5 blocks in Hue/Lum/Sat space,
instead of shipping a pre-baked texture per color variant. It's built offline
by the native `hls_bank_maker` tool from `.hlsinfo` files, which are
themselves produced by `panoply_maker` from an item's source texture + color
masks (`skin`, `user`, ...) and a `panoply_*.cfg`.

The `.hlsinfo` sources that `characters.hlsbank` was originally built from are
not all available anymore, so there was no way to tell from the source side
whether any armor was missing from the shipped bank. `hls_bank_maker` itself
is also full-rebuild-only (needs every historical item's `.hlsinfo` at once)
and is gated behind `IF(SQUISH_FOUND)` (`nel/tools/3d/CMakeLists.txt:81-90`),
so it doesn't build in a normal local checkout. This pynel module exists to:

- read a `.hlsbank` directly, to audit what's actually in it against a known
  list of panoply-managed textures;
- read a `.hlsinfo`, to inspect what a single item's colorized instances
  look like without a full bank;
- **append** new entries into an existing `.hlsbank` from a fresh `.hlsinfo`,
  without needing every other item's original `.hlsinfo` — a pynel-only
  capability `hls_bank_maker` doesn't have.

## `.hlsbank` format (`pynel.hls_texture_bank`)

Raw `COFile::serial()`, no magic/header/compression wrapper.

- `CHLSTextureBank`: `version(0)`, then `serialCont` of:
  - `color_textures` (`vector<CHLSColorTexture>`)
  - `instance_data` (`vector<uint8>`, a raw blob)
  - `instances` (`vector<CTextureInstance>`) — each entry on disk only stores
    a data offset into `instance_data` and a `color_texture_id`; the pointers
    NeL reconstructs at load time are not serialized.
- An instance's real data sits in `instance_data` at its offset: a
  0-terminated lowercase name string (a raw C-string, not NeL's usual
  length-prefixed `string()`), immediately followed by
  `color_textures[color_texture_id].num_masks()` `CHLSColorDelta` entries
  (3 bytes each: `DHue` uint8, `DLum` int8, `DSat` int8). `num_masks` comes
  from that color texture's own mask count, so instances can only be decoded
  after all color textures are parsed.
- `CHLSColorTexture`: `version(0)`, `width`/`height`/`num_mipmap`/
  `block_to_compress_index` (uint32 each), `serialCont(texture)` (raw DXTC5
  bytes, opaque to pynel), `serialCont(masks)` (`vector<CMask>`).
- `CMask`: `version(0)`, `full_block_index`/`mixt_block_index` (uint32 each),
  `serialCont(data)` (raw bytes, opaque to pynel).

Validated against the real `characters.hlsbank` (1574 color textures, 19733
instances): parses cleanly with no trailing bytes, and `dumps_hlsbank(bank)`
round-trips to a byte-identical file.

## `.hlsinfo` format (`pynel.hls_bank_texture_info`)

Produced by `panoply_maker` per source texture, one file consumed by
`hls_bank_maker` per `CHLSColorTexture` it adds to a bank.

- `DividedBy2` (bool)
- `SrcBitmap` (`CDXTCBitmap`) — a **complete raw `.dds` file**: `"DDS "` +
  `DDS_HEADER` + mip chain, same container documented in
  `nel/tools/forgery/docs/dds_export.md`. This is a downscaled
  (`>> low_def_shift`, default shift 3) copy of the source texture, already
  DXT5-compressed by `panoply_maker` — pynel does not decode or recompress
  it, only slices the mip chain out by the header's declared dimensions.
- `Masks` (`vector<CMaskBitmap>`): `Width`/`Height` + raw uint8 luminance
  pixels, uncompressed, same downscaled size as `SrcBitmap`.
- `Instances` (`vector<CTextureInstance>`): `Name` + `Mods` (one `CHLSMod`
  per mask: float `DHue` 0-360, `DLum`/`DSat` roughly -1..1, uncompressed —
  only compressed into a `.hlsbank`'s 3-byte `ColorDelta` encoding at
  append/build time).

Validated against 10 real `.hlsinfo` files (incl.
`ryw_hom_caster01_pantabottes_c{1,2,3}.hlsinfo`): parses cleanly, correct DDS
magic, sane mask dimensions and HLS mods.

## Appending to a bank (`append_texture_info`)

`hls_texture_bank.append_texture_info(bank, info)` ports `hls_bank_maker.cpp`'s
`addTextToBank()` + `CHLSColorTexture::setBitmap()`/`addMask()`:

- `build_color_texture()` reuses the `.hlsinfo`'s already-DXT5-compressed
  `SrcBitmap` mip chain byte-for-byte, no re-compression.
- `_add_mask()` classifies every 4x4 block of every mip level as
  Empty/Full/Mixt from the mask's raw luminance pixels (matching the
  engine's threshold logic), bit-packs the result, and ORs each mask's
  Mixt-block bits into the color texture's own "blocks to recompress"
  bitfield.
- `compress_hls_mod()` packs each instance's float `CHLSMod` into the
  on-disk 3-byte `ColorDelta` encoding.

Only new entries are appended — every existing color texture and instance in
`bank` is left byte-for-byte untouched. This has been cross-validated two
ways:

- **Against the real bank**: appending a real `.hlsinfo` into the real
  `characters.hlsbank` leaves all 1574 original color textures and 19733
  original instances byte-identical, and adds the expected new instances
  with correct HLS deltas.
- **Against the real native tool**: building a bank from scratch out of the
  same 10 `.hlsinfo` files via pynel produces **byte-identical output** to
  running the real `hls_bank_maker` binary on those same files (116862 bytes
  both sides) — not just structurally valid, a bit-exact drop-in
  replacement.

## CLI (`ryzom-hlsbank`)

```bash
# List every color texture and item instance in a .hlsbank
ryzom-hlsbank dump characters.hlsbank

# Append one or more .hlsinfo files into an existing .hlsbank
ryzom-hlsbank add characters.hlsbank new_item.hlsinfo -o characters.hlsbank
```

## Library usage

```python
from pynel import hls_texture_bank as hlsbank
from pynel import hls_bank_texture_info as hlsinfo

bank = hlsbank.load_hlsbank("characters.hlsbank")
print(len(bank.color_textures), len(bank.instances))

info = hlsinfo.load_hlsinfo("new_item.hlsinfo")
hlsbank.append_texture_info(bank, info)
hlsbank.save_hlsbank("characters.hlsbank", bank)
```

## Known gaps

- No writer for `.hlsinfo` yet (`dumps_hlsinfo()`/`save_hlsinfo()`) — needed
  for a Python `panoply_maker` port, not for `hls_bank_maker` replacement.
- No support for removing or modifying an existing color texture/instance in
  a bank — append-only, matching the actual need (recovering items missing
  from the shipped bank, not editing existing ones).
