# Updating the vendored libwebp

`lib/libwebp/libwebp/` is **third-party source**. It is a pruned copy of
upstream libwebp and must never be reformatted, re-styled or `clang-format`ed.
Upstream ships its own `.clang-format`, which is kept in the tree for exactly
that reason.

## Current version

| Item | Value |
| ---- | ----- |
| Upstream | <https://github.com/webmproject/libwebp> |
| Tag | `v1.6.0` |
| Version string | `1.6.0` (see `configure.ac`, `AC_INIT([libwebp], [1.6.0], ...)`) |
| License | BSD-3-Clause, plus the additional patent grant in `PATENTS` |

## Prune rule

Pixelix needs a **decoder only**. Everything which is not needed is removed
from the tree, not just excluded from the build, so that a reader does not have
to guess what is live. The rule is mechanical:

Keep, relative to the upstream root:

- `COPYING`
- `PATENTS`
- `AUTHORS`
- `README.md`
- `.clang-format`
- `src/dec/` (whole directory)
- `src/demux/` (whole directory)
- `src/dsp/` (whole directory)
- `src/utils/` (whole directory)
- `src/webp/` (whole directory, the public headers)

Delete everything else. In particular `src/enc/`, `src/mux/`, `src/Makefile.am`
and the other loose files under `src/`, plus `tests/`, `doc/`, `examples/`,
`extras/`, `cmake/`, `gradle/`, `imageio/`, `infra/`, `m4/`, `man/`,
`sharpyuv/`, `swig/`, `webp_js/`, the Android, autotools, gradle and MSVC build
files, and the repository dotfiles other than `.clang-format`.

`sharpyuv/` is encoder-only; no file under the five kept directories references
it (`grep -rn sharpyuv lib/libwebp/libwebp/src/` is empty).

The five kept directories are taken **wholesale**, including their
`Makefile.am`, `.pc.in` and `.rc` files. Keeping whole directories is what makes
the rule mechanical; those few files are inert.

That leaves 142 files. Upstream has 346 (excluding Pixelix' own
`library.json`).

## Update procedure

1. Check out the new upstream tag into `lib/libwebp/libwebp/`, replacing the
   whole directory.
2. Apply the prune rule above.
3. Bump `version` in `lib/libwebp/library.json` and the table in this file.
4. Re-check the `srcFilter` (see below) against the new file list: new encoder
   files inside the kept directories need a new exclusion.
5. Re-apply any patch recorded under *Local patches* below.

## `srcFilter` rationale

The prune keeps whole directories, so some encoder-side translation units
survive inside `src/dsp/` and `src/utils/`. They are excluded from the build by
the `srcFilter` in `library.json` instead of being deleted, because the
directory-wholesale prune rule is easier to re-apply than a per-file one:

| Pattern | Why |
| ------- | --- |
| `-<src/dsp/*enc*.c>` | The encoder DSP kernels (`enc.c`, `enc_sse2.c`, `lossless_enc*.c`, ...). |
| `-<src/dsp/cost*.c>` | Rate-distortion cost tables, encoder only. |
| `-<src/dsp/ssim*.c>` | SSIM metric, encoder/tool only. |
| `-<src/utils/bit_writer_utils.c>` | Bit *writer*; the decoder uses `bit_reader_utils.c`. |
| `-<src/utils/huffman_encode_utils.c>` | Huffman *encoder*; the decoder uses `huffman_utils.c`. |
| `-<src/utils/quant_levels_utils.c>` | Alpha *quantizer*, encoder only. |

`src/utils/palette.c` **stays**, although it is encoder-side analysis code.
`src/utils/utils.c` — which the decoder needs for `WebPSafeMalloc()` — exports
the public `WebPGetColorPalette()`, which forwards to `GetColorPalette()` in
`palette.c`. Excluding `palette.c` compiles fine and then fails at link time
with `undefined reference to 'GetColorPalette'`, because the whole `utils.o` is
pulled in. Upstream builds both in the same `libwebputils` library.

`src/utils/quant_levels_dec_utils.c` **stays**. Despite the similar name it is
the decoder's alpha *dequantizer* (smoothing of a quantized alpha plane) and is
referenced from `src/dec/alpha_dec.c`. Do not let the `quant_levels` prefix
tempt a future update into excluding both.

`unflags: ["-DHAVE_CONFIG_H"]` keeps a global `-DHAVE_CONFIG_H` — should one
ever be introduced — from making the sources look for the autotools `config.h`,
which the prune removed. The build is configured purely through `-D` flags from
`config/imageFormats.ini` (`WEBP_REDUCE_SIZE`, `WEBP_REDUCE_CSP`,
`WEBP_DISABLE_STATS`).

## Local patches

**None at the time of writing.** The tree is byte-for-byte upstream.

There is one known open point which may one day require a patch. libwebp
allocates through `WebPSafeMalloc()` / `WebPSafeCalloc()` in
`src/utils/utils.c`, which call the plain C `malloc()`. Whether those
allocations land in PSRAM or in the internal heap depends on the Arduino ESP32
sdkconfig — `CONFIG_SPIRAM_USE_MALLOC` and the "always internal" size
threshold. PNG and WebP are only enabled on boards which have PSRAM precisely
because the buffers are large, so if a measurement on a real board shows the
large libwebp allocations staying in internal RAM, the fix is a minimal,
clearly marked patch to `src/utils/utils.c` routing them to
`heap_caps_malloc(..., MALLOC_CAP_SPIRAM)`. If that ever happens, the patch is
recorded **here**, with its diff and its reason, and re-applied on every
update. An un-noted edit of vendored source is not acceptable: the next update
would silently drop it.
