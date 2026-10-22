# Changelog

[Русский](CHANGELOG.md) | English

Versions follow [semantic versioning](https://semver.org/). The entire version section
is included in the release description on GitHub and Gitverse.

## Unreleased

### XML Schema: checking the schema document

The fixes were found by debugging the W3C XML Schema Test Suite failures by hand
(`tests/xsts_failing`). The suite result is **96.8 %** (was 96.2 %): 234 cases fixed, no new failures.

- `minOccurs`/`maxOccurs` is any non-negative integer: a number longer than 9 digits is no longer an error.
- `minOccurs` and `maxOccurs` of `xs:all` can only be 0 or 1.
- `id` of XSD elements is an NCName unique within the schema document.
- `xs:annotation` occurs at most once and only as the first child (global components and particles).
- Inside `xs:group` there is one `sequence`/`choice`/`all` model. An `xs:group` reference stands only in a model or right in a type.
- `name` of a global component is an NCName (with Unicode characters).

### Tools

- `tests/xsts_failing` — the XSTS failure debugger: the failure list from a run, running cases one by one
  (a filter, `--case N`, `--stop-on-first`), a debug build together with the library.

## 2.1.4 — 2026-10-20

### License

- The copyright holder in `LICENSE` (and its copies in the packages) is Ermakov Aleksei Aleksandrovich.

### Release on Gitverse

- The Linux x86_64 and aarch64 SDKs are built on Gitverse in Astra Linux SE 1.7 (GCC 8.3, glibc 2.28):
  the library requires glibc 2.17 and runs on Astra Linux SE 1.7, RED OS 7.3, ALT p10 and newer;
  the unit tests pass on both architectures (aarch64 in qemu).
- The Windows x86_64 SDK is built on Gitverse with the MinGW-w64 compiler (`*-windows-x86_64-mingw.zip`:
  `beresta.dll`, the MinGW import and static libraries, `beresta.def` for MSVC).
- Only the macOS and Windows ARM64 SDKs and the npm, PyPI, NuGet, Maven, WebAssembly packages are taken from the GitHub release.
- Contacts and the product site: <https://ermaleyalekov.gitverse.site/beresta>.

## 2.1.3 — 2026-10-20

Building from source on Russian operating systems and a test stand for the expert.

### Build

- GCC 8 (Astra Linux SE 1.7): `std::filesystem` is linked from `libstdc++fs` — the Makefile and CMake
  add `-lstdc++fs` themselves when the GCC version is below 9.
- glibc < 2.34 (Astra Linux SE 1.7, RED OS 7.3): the unit tests are linked with `-pthread`
  (`Threads::Threads` in CMake).

### Test stand

- `sh tools/stand/run_stand.sh` builds the library from source with one command, runs the unit tests
  and the demo programs and writes the `stand_report.txt` report (OS with version, glibc, compiler,
  per-step result and the number of passed tests). See `tools/stand/README.md`.
- The stand passed (211 of 211 tests) on Astra Linux SE 1.7 and 1.8, ALT p10 and p11, RED OS 7.3 and 8.

### Documentation

- Documents for the Russian software registry (`docs/registry/`, in Russian) and the product site (`site/`).

## 2.1.2 — 2026-10-20

XML Schema validation against the W3C XML Schema Test Suite: **93.6% → 96.2%** of applicable cases.

### XSD regular expressions

- Categories `\p{..}` and blocks `\p{Is..}` / `\P{..}` — Unicode 3.1 (the version referenced
  by XSD 1.0), from own tables without ICU; generator `tools/unicode/generate_unicode_tables.py`.
- `\d` = `Nd`, `\w` — everything except `P`, `Z`, `C`; `\i` and `\c` — according to XML 1.0 Appendix B.
- Strict XSD 1.0 syntax: unknown escapes (`\x41`, `\A`, `\b`, `\1`), reverse range,
  an empty class and an unescaped `[` inside a class are schema errors, as are `\p{Cs}` and unknown names.
- XSTS 93.6% → 95.7%, no failures in the MS-Regex group (was 776).

### Values and attribute wildcards

- Partial order of `xs:duration` (3.2.6.2) in bound facets.
- `xs:anyAttribute` per 3.4.2 / 3.10.6: namespaces, intersection, union on extension,
  lax / strict on global attributes, cvc-complex-type.5; `xs:anyType` — built-in complex type.
- `fixed` on mixed content, `xsi:nil` (cvc-elt.3), ID / IDREF on default attributes,
  `xs:all` with `maxOccurs="0"`.

### Identity constraints

- `xs:key` / `xs:unique` fields see `default` / `fixed` attributes absent from the instance, both in the stream
  and in tree validation.
- Field on an element of a complex type without simple content — `KEY_FIELD_COMPLEX` error,
  on an attribute without type (skip / lax) — `KEY_FIELD_UNTYPED`.
- `QName` / `NOTATION` in keys are compared as the pair "URI + local name".
- XSTS 95.7% → 96.2%.

### Release

- The release is published simultaneously on GitHub and Gitverse; packages in registries (npm, PyPI, NuGet, Maven Central)
  are published only from GitHub. The procedure is in [docs/building.en.md](docs/building.en.md#releasing-a-version).

## 2.1.1 and earlier

See [releases on GitHub](https://github.com/ErmaleyAlekov/beresta/releases).
