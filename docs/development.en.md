# Beresta — project structure, tests and extending

[Русский](development.md) · **English**

← [To the description and advantages](../README.en.md)

## Contents

1. [Project structure](#project-structure)
2. [Tests](#tests)
3. [Extending the library](#extending-the-library)
4. [License](#license)

---

## Project structure

```
beresta/
├── CMakeLists.txt  Makefile  README.md  README.en.md  CHANGELOG.md  CHANGELOG.en.md
├── .github/workflows/            ci.yml (build and tests), release.yml (packages, registries and the GitHub release)
├── .gitverse/workflows/          ci.yml and release.yml for Gitverse (Linux, a release without registries)
├── cmake/                        platform detection, wrappers, WebAssembly, the Java self-test
├── include/                      declarations only
│   ├── beresta.hpp               the common C++ header
│   ├── capi/                     beresta_c.h — the flat C interface; internal/ — the exception barrier
│   ├── core/                     Node, ContainerNode, Element, Document, DocumentType, Text, ...
│   ├── parser/                   XmlParser, ParserOptions, ErrorHandler, ResourceResolver, DomBuilder
│   │   └── internal/             the input stack, the entity budget, namespaces, decoders
│   ├── sax/                      SaxParser, ContentHandler, Locator
│   ├── encoding/                 Charset, tables, encoding detection
│   ├── dtd/                      the model, reading, parsing and validating DTD
│   ├── content/                  the content model (common for DTD and XSD), the step-by-step cursor
│   ├── xsd/                      Schema, SchemaValidator, ValidatingHandler, data types; regex/ — the pattern engine
│   ├── xpath/                    XPathExpression, XPathValue; internal/ — the lexer, parser, AST
│   ├── traversal/                TreeWalker, NodeIterator, NodeFilter
│   ├── serializer/               XmlSerializer
│   ├── xinclude/                 XInclude
│   ├── io/                       files and streams
│   ├── exception/                DomException, ParseException
│   ├── i18n/                     the message catalog: Message, format, setLanguage
│   └── utils/                    XML characters, UTF-8, escaping, names, the keyed hash
├── src/                          the implementation, mirroring include/ (≤ 5 functions per file)
│   ├── i18n/catalogs/            messages_ru.cpp, messages_en.cpp — the message texts
│   └── capi/                     the C interface: errors, handles, library, document, node, element, xpath, schema
├── bindings/
│   ├── java/                     the Maven project: org.beresta (API), org.beresta.internal (Panama), selftest
│   ├── csharp/                   Beresta (the library, NuGet) and Beresta.SelfTest
│   ├── python/                   the beresta package, tests/, pyproject.toml
│   ├── node/                     the npm package: addon/ (Node-API, C++), lib/ (JS + index.d.ts), test/, examples/
│   └── wasm/                     the beresta-wasm package: lib/ (the Wasm backend), tools/ (the bundler), test/, examples/
├── benchmarks/                   measurements against libxml2, Xerces, pugixml, tinyxml2, RapidXML; node/ — against JS libraries
├── examples/                     demos in C++ (src/) and in C (capi/)
├── tools/                        release/ — release notes and mirroring releases (GitHub → Gitverse); unicode/ — the table generator
└── tests/                        C++ unit tests (an own mini framework)
    ├── conformance/              the W3C XML Conformance Test Suite runner and report
    ├── xsts/                     the W3C XML Schema Test Suite runner, the limitation list and report
    └── fuzz/                     libFuzzer targets, a dictionary, a corpus builder
```

The core and the C interface are about **19.8 thousand lines of code** without comments and blank lines (300 implementation files, 97 headers; together with the bilingual comments — about 32 thousand lines). The wrappers are about 6.8 thousand more lines of code in Java, C#, Python, JavaScript and C++ (the Node.js addon).

### Parser design

```
bytes ──► encoding::decodeDocument ──► UTF-8 ──► ParseSession ──► [ValidatingHandler] ──► ContentHandler
              (BOM, declaration,               (an iterative loop,  (XSD along the      ├── DomBuilder ──► Document ──► XInclude
               character checks)                the entity stack,     parse, if a         └── your SAX handler
                                                namespaces,           schema is set)
                                                DTD + validator)
```

### Wrapper design

```
Java (FFM API) ──┐
C# (P/Invoke) ───┤
Python (ctypes) ─┼──► beresta_c.h ──► the exception barrier ──► the C++ core (DOM, XPath, XSD, ...)
Node.js (Node-   ┤    (extern "C",     (a code + a message
  API addon) ────┤     void* handles)   in thread_local)
WebAssembly ─────┤
C, Go, Rust ─────┘
```

A node wrapper is a native handle plus a reference to the owner document: while the wrapper is alive, the garbage collector will not collect the document, and after the document is closed accessing the node gives an exception instead of a process crash. Every document keeps a registry of its wrappers with weak references (the Identity Map pattern): one node has one wrapper. Thanks to the registry a move (`moveTo`) changes the owner of all already issued wrappers of the subtree at once.

---

## Tests

```bash
make test
```

**173 C++ tests in 59 suites:**
- the core: copying, hierarchy, elements, navigation, namespaces, serialization, trees 100,000 levels deep;
- the parser: formats, content, errors, limits, SAX (including PIs from the DTD), encodings, protection from complexity attacks (repeated attributes beyond the loop threshold, the keyed hash, a linear growth of the parse time from 25,000 to 200,000 attributes), mapping files into memory (Cyrillic names, an empty file, CRLF and windows-1251);
- DTD: entities, validity checking, external resources;
- the serializer, tree traversal, XPath (paths, axes, functions, the `namespace` axis, names), XSD (types, schemas, advanced features, `xs:key`/`xs:unique`/`xs:keyref`, paths in error messages, whitespace rules, substitution groups, `xsi:type` with `block`/`final`/`abstract`, `xs:redefine`/`xs:override`; the XSD 1.1 assertions `xs:assert`/`xs:assertion` — the bank statement example, negative cases, exact arithmetic of cents, inheritance, XPath 2.0 constructs, unsupported XPath 2.0 as a loading error, `vc:minVersion`, a shared schema from 8 threads; streaming validation: SAX, DOM and a ready tree give the same errors, streaming keys, a deep document), a recursive schema 10,000 levels deep with `xs:unique` in all three modes), the content model (the step-by-step cursor against the general algorithm on thousands of random models), exact decimal arithmetic, XInclude;
- the C interface: error codes, building and changing the tree, moving nodes between documents (with freeing the source document and rolling back a rejected move), XPath, XML Schema, streaming validation (matches the tree validation, files, errors); load like the wrappers': 20 threads on a shared schema, XPath and document (every thread has its own errors), 50,000 moves between documents with a check of every previously given handle;
- message languages: catalog completeness (every message has a translation with the same `{N}` markers), changing the language at run time, argument substitution;
- regressions: differences from the W3C suite (syntax, entities, validity, encodings) and fuzzing findings.

Standard conformance and resistance to hostile input are checked separately:

```bash
cd tests/conformance && make fetch && make run      # W3C XML Conformance Test Suite -> conformance_report.md
cd tests/xsts && make fetch && make run             # W3C XML Schema Test Suite -> xsts_report.md
cd tests/fuzz && make -j && make corpus && make -j3 fuzz FUZZ_TIME=600 FUZZ_JOBS=3   # libFuzzer, Clang only
```

The tests pass in a regular build, under AddressSanitizer + UBSan + LeakSanitizer with `-D_GLIBCXX_ASSERTIONS` and under ThreadSanitizer.

The wrapper self-tests work without test frameworks (JUnit, xUnit and pytest are not needed):

```bash
make JAVA=1 CSHARP=1 PYTHON=1 NODE=1 bindings-test
```

They check parsing, navigation, building, copying and moving between documents (including "move from A to B, close A, use the node in B"), errors, writing in another encoding, XPath, XML Schema, streaming validation (matches the tree, files with Cyrillic names), load (50,000 moves keeping the wrapper registry, 8 threads on a shared schema and XPath) and message languages: Java — 17 checks, C# — 16, Python — 21, Node.js — 19 (the built-in `node --test`), WebAssembly — 6 (`make wasm-test`). In CMake all of this (and the C++ tests) is run by `ctest` if `BERESTA_BUILD_TESTS` and the wrapper flags are on.

Where it was checked (without a single compiler warning):

| Platform | Compilers | What was checked |
|---|---|---|
| Windows 11 x64 | MSVC (Visual Studio 2026), MinGW GCC 15 | CMake and the Makefile, the C++ tests, `ctest`, the self-tests of Java 25 / .NET 9 / Python 3.12 / Node.js 24, `dotnet pack` |
| Ubuntu 26.04 x64 | GCC 15, Clang 21 | CMake and the Makefile, the C++ tests, `ctest`, the self-tests of Java 25 / .NET 10 / Python 3.14 / Node.js 22, `mvn package` (running with the binary from the jar), `dotnet pack`, a wheel in a clean venv |
| Ubuntu 26.04 x64 | GCC 15 + ASan, UBSan, LeakSanitizer | the C++ tests, the C and C++ demos: no memory leaks, no undefined behavior |
| Ubuntu 26.04 x64 | GCC 15 + ThreadSanitizer | the C++ tests, including 20 threads on shared C interface handles: not a single race |
| Ubuntu 26.04 x64 | Clang 21 + ASan, UBSan, LeakSanitizer, libFuzzer | the C++ tests, the W3C XML Conformance Test Suite run (100 %), fuzzing of three targets |
| WebAssembly | Emscripten 3.1.69 | `make wasm` and `emcmake cmake` + `ctest`, the self-test in Node.js 22, the example page in Chromium |

macOS (the universal binary), Linux arm64 and Windows arm64 are built and tested in the GitHub Actions CI (`.github/workflows/ci.yml`); these platforms were not checked on the author's own hardware.

How to add a C++ suite:
1. Test functions in `tests/src/<module>/..._tests.cpp`.
2. `registerXxxTests(test::TestRunner&)` in the same file and a declaration in `tests/include/suites/test_suites.hpp`.
3. A line in the `SUITES` table in `tests/src/main.cpp`.

Checks: `TEST_ASSERT(condition)`, `TEST_ASSERT_EQ(string, "expected")`, `TEST_ASSERT_THROWS(statement, ExceptionType)`.

---

## Extending the library

Beresta uses no third-party libraries — neither in the core, nor in the wrappers, nor in the tests. When extending it, keep this property: an algorithm, a table or a parser is better written on your own.

**A new node type:**
1. A value in `NodeType` (and in `beresta_node_kind` of the C interface).
2. A `CharacterNode` or `ContainerNode` subclass with `getType`, `clone`, `serialize`, `isEqualNode` (and `validateChild` for a container).
3. Handling in `Document::validateChild`, `DomBuilder`, `showMaskFor` and, if needed, in the XPath axes.

**A new encoding:** a table of 128 codes in `src/encoding/tables/`, a declaration in `single_byte_tables.hpp`, an instance and aliases in `charset_registry.cpp`.

**A new XPath function:** an implementation with the `XPathValue f(const FunctionCall&)` signature and a line in the `FUNCTIONS` table (`xpath_function_table.cpp`).

**A custom resource loader:** a `ResourceResolver` subclass.

**A new error message:** a constant in `include/i18n/message.hpp` and a text in every catalog of `src/i18n/catalogs/` with the same `{N}` markers (the "catalog completeness" test checks this). In code — `i18n::format(i18n::Message::NAME, {arguments})` instead of a string literal.

**A new message language:** see [How to add a language](guide.en.md#how-to-add-a-language).

**Comments** are written in two languages: first the Russian text, then its English translation in the same comment (`// русский` and on the next line `// English`, or `// русский / English` for a short trailing comment).

**A new C interface function:**
1. A declaration with `BERESTA_API` in `include/capi/beresta_c.h` (C types only: `void*`, `int`, `int64_t`, `uint32_t`, `double`, `const char*`).
2. An implementation in `src/capi/<group>/` wrapped in `BERESTA_CAPI_TRY` … `BERESTA_CAPI_CATCH(...)`: no exception may get out.
3. A signature line in every wrapper: `_signatures.py` (Python), `*Functions.java` (Java), `NativeMethods.*.cs` (C#), a line in `bindings/node/addon/src/module/binding_table.cpp` (Node.js; the adapter for a new signature goes into `adapters/`), a line in `BINDINGS` in `bindings/wasm/lib/wasm_signatures.js` and a name in `bindings/wasm/exports.json` (WebAssembly; a new signature is described there as data). Then a method in the object API of the wrapper.

---

## License

Beresta is distributed under the **MIT** license — the text is in the [LICENSE](../LICENSE) file.

In short: the library can be used, modified and distributed for free, including in closed and commercial products. The only condition is to keep the copyright notice and the license text in copies of the program. There are no warranties: the program is provided "as is".

The license is already included in all packages, so nothing has to be added manually when distributing:

| Package | Where the license is |
|---|---|
| jar (Maven) | `META-INF/LICENSE` and the `<licenses>` element in `pom.xml` |
| NuGet | `LICENSE` in the package root and `PackageLicenseExpression` = `MIT` |
| wheel (Python) | `*.dist-info/licenses/LICENSE` and the `license = "MIT"` field in the metadata |
| npm (`beresta`, `beresta-wasm`) | `LICENSE` in the package root and the `"license": "MIT"` field in `package.json` |

`bindings/python`, `bindings/node` and `bindings/wasm` have their own copies of `LICENSE`, because setuptools and npm do not take files outside the package directory. If you change the root `LICENSE`, update them too.

No third-party code is part of the project: the core, the wrappers and the tests are written from scratch, so there are no other licenses or notices (NOTICE, THIRD-PARTY).
