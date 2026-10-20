# Changelog

[Русский](CHANGELOG.md) | English

Versions follow [semantic versioning](https://semver.org/). The entire version section
is included in the release description on GitHub and Gitverse.

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
