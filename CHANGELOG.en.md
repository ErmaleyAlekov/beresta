# Changelog

[Русский](CHANGELOG.md) | English

Versions follow [semantic versioning](https://semver.org/). The entire version section
is included in the release description on GitHub and Gitverse.

## Unreleased

### XML Schema: recursive models and derivation

- Fixed a false "cyclic type derivation" for Composite-style schemas: a container type holds elements whose
  type extends the container itself (`BoxType` holds `Panel`, `PanelType` extends `BoxType`). The result used
  to depend on the alphabetical order of element names. A derived type now waits until its base is complete
  and gets its full model; real cycles (`A` → `A`, `A` → `B` → `A`) are still a schema error.

### PHP binding through FFI

- A new Composer package `bindings/php` (`ermaleyalekov/beresta`, PHP 8.2+, the `ffi` extension): all 71 functions
  of the C interface and the `Beresta\Parser`, `Document`, `Node`, `Element`, `XPath`, `Schema`, `Diagnostic` classes.
  Nodes are iterated with `foreach`, attributes are read as `$element['code']`, `clone $document` is a deep copy.
- Loading for PHP-FPM / Apache in `opcache.preload` (`Beresta\Beresta::preload()`).
- Build: `make PHP=1`, `cmake -DBERESTA_BUILD_PHP=ON`; the self-test `make PHP=1 php-test` is an own runner without PHPUnit.

### XML Schema: checking the schema document

The fixes were found by debugging the W3C XML Schema Test Suite failures by hand
(`tests/xsts_failing`). The suite result is **98.2 %** (was 96.2 %): 792 cases fixed, no new failures
(234 by the fixes below, 558 by the facet and attribute rules and the choice of the declaration, the next section).

- `minOccurs`/`maxOccurs` is any non-negative integer: a number longer than 9 digits is no longer an error.
- `minOccurs` and `maxOccurs` of `xs:all` can only be 0 or 1.
- `id` of XSD elements is an NCName unique within the schema document.
- `xs:annotation` occurs at most once and only as the first child (global components and particles).
- Inside `xs:group` there is one `sequence`/`choice`/`all` model. An `xs:group` reference stands only in a model or right in a type.
- `name` of a global component is an NCName (with Unicode characters).
- Schema building depth is limited: more than 256 levels of declarations, types and models is the
  `SCHEMA_TOO_DEEP` schema error. Before, the particle limit started anew at every anonymous type, and with a
  raised `maxDepth` a schema of about a thousand nested elements crashed the process with a stack overflow.
- A schema document deeper than 1024 element levels is rejected before building (`SCHEMA_DOCUMENT_TOO_DEEP`).
- README and the comparison: "no recursion" is made precise — there is no recursion over the document tree.
  Schema building and expression parsing are recursive, their limits are described.

### XML Schema: facets, attributes and the declaration by particle

- The facets of simple types are checked when the schema is loaded (part 2, §4.3): a facet is applicable to the base type
  (`totalDigits` on a string, `length` on a number, `enumeration` on boolean are errors), the values of bounds and of the
  enumeration are values of the base type, `totalDigits` > 0, `length` is not in one step with `minLength`/`maxLength`, `min*` is not
  greater than `max*`, two lower or two upper bounds in one step are not allowed, a restriction does not weaken the base (bounds,
  lengths, digits, `fractionDigits` = 0 for integers, `whiteSpace`), a facet is not repeated in a step. New messages
  `SCHEMA_FACET_*`.
- `xs:attribute`: a global declaration has no `use`, `form`, `ref`; `use` is only optional/required/prohibited;
  with `default` only `use="optional"` is allowed (`fixed` with `required`/`prohibited` is still allowed); exactly one
  of `name` and `ref`, a reference has no `type`, `form` and nested type; `type` and a nested `xs:simpleType` together are not allowed;
  the name is an NCName, not `xmlns`, the namespace is not xsi.
- `xs:all` contains no `sequence`/`choice`/`all` and itself does not stand inside `sequence`/`choice` (including through
  a group reference). `xs:any` and a group reference inside `xs:all` are allowed, as in XSD 1.1.
- A child element is validated against the declaration of the model particle that accepted it, not against the first
  declaration of the same name: same-named elements with different `fixed` or types and an element accepted by `xs:any`
  are validated correctly; every `xs:any` has its own `processContents` (previously the last one applied to the whole type).
- An attribute with `use="prohibited"` is admitted by the type's `xs:anyAttribute` if it allows its namespace.
- `fixed` and the enumeration of union and list are compared by value: a union value belongs to the first matching
  member (`fixed="1"` equals `true` for union(boolean, int)), for a list the items are compared.
- The `xsd_advanced_tests` test relied on a schema with an `xs:all` group inside `xs:sequence` — it is not allowed
  (cos-all-limited), the test was rewritten. In `tests/xsts/known_limitations.txt` the `disputed` key was extended with cases where
  the suite expects the 1st edition gMonth notation in facets and NMTOKENS / positiveInteger restrictions prohibited by §4.3.

### XPath

- The nesting depth of an expression is limited: more than 128 levels of parentheses, function arguments and
  predicates (and in XSD assertions also `if` branches and quantifiers) is the `XPATH_ERROR` parse error. Before,
  an expression of about a thousand nested parentheses crashed the process with a stack overflow.
- A long operator chain (`1+1+…+1`, `a | b | …`, `x or y or …`) is evaluated and freed by a loop:
  before, a chain of several thousand operators crashed the process with a stack overflow.

### Tools

- `tests/xsts_failing` — the XSTS failure debugger: the failure list from a run, running cases one by one
  (a filter, `--case N`, `--stop-on-first`), a debug build together with the library.

### XML Schema: values by the type, annotations and notations

The W3C XML Schema Test Suite result is **98.6 %**: failures 671 → 515, no new ones.

- `default` and `fixed` of elements and attributes are checked by the type when the schema is loaded. For an element
  of a complex type the value is allowed with simple content (checked by it) or mixed content; with empty or element-only
  content it is an error. New messages `SCHEMA_VALUE_CONSTRAINT_INVALID`, `SCHEMA_VALUE_CONSTRAINT_CONTENT`.
- On `xs:element` the `type` attribute, a nested `xs:simpleType` and a nested `xs:complexType` exclude each other
  (`SCHEMA_ELEMENT_PARTS_CONFLICT`).
- The order of children is checked for every element of the schema document, not only for global components and particles:
  `xs:annotation` occurs once and first also in `xs:key`, `xs:selector`, `xs:field`, `xs:list`, `xs:union`, `xs:extension`,
  `xs:anyAttribute` and unused definitions. Inside `xs:annotation` only `xs:appinfo` / `xs:documentation` are allowed.
- `xs:notation`: only right in `xs:schema`, only an annotation inside; the name is an NCName without repeats; `public`
  or `system` is required (`SCHEMA_NOTATION_NO_IDENTIFIER`).
- The XSD 1.0 rules removed in 1.1 (`xs:any` inside `xs:all`, `default`/`fixed` on the ID type) are not applied:
  Beresta is a 1.1 processor. `tests/xsts/known_limitations.txt` has a new key `xsd11-rules` (7 cases); 3 groups with a
  gMonth `fixed` in the 1st edition notation are added to `disputed`.

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
