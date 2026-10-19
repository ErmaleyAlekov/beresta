# Beresta

[Русский](README.md) · **English**

**Beresta** (`beresta`) is a **C++17** XML library without a single third-party dependency. It fully replaces Xerces-C++: DOM, SAX, namespaces, encodings, DTD, XML Schema, XPath 1.0, XInclude, tree traversal and serialization.

The core is built into a shared library with a flat C interface. That is why Beresta can be used not only from C++, but also from **C**, **Java 22+** (Project Panama, no JNI), **C#** (P/Invoke), **Python** (ctypes) and **Node.js** (Node-API, no node-gyp, with TypeScript types), and the **WebAssembly** build works in browsers, Web Workers and Cloudflare Workers. The ready wrappers live in `bindings/`, and the CI builds the npm, PyPI and NuGet packages with all platforms inside (see [Ready packages and releases](docs/building.en.md#ready-packages-and-releases)).

The name comes from birch bark letters: in medieval Rus documents were written on birch bark (*beresta*). The library works with documents too, only XML ones.

```cpp
#include "beresta.hpp"

beresta::Document doc = beresta::XmlParser().parse("<greeting lang=\"ru\">Привет</greeting>");
std::cout << doc.getRoot()->getTextContent();                    // Привет
std::cout << beresta::xpath::evaluateString(doc, "/*/@lang");     // ru
std::cout << doc.toString();                                      // <greeting lang="ru">Привет</greeting>
```

---

## Documentation

- [Comparison, use cases, performance and limitations](docs/comparison.en.md)
- [Building, linking and ready packages](docs/building.en.md)
- [The C++ guide and examples](docs/guide.en.md)
- [The C interface and the wrappers for other languages](docs/bindings.en.md)
- [Project structure, tests and extending](docs/development.en.md)

---

## Advantages

### In short
1. **The full set of standards without dependencies.** XML 1.0, namespaces, eight encodings, DTD, XML Schema 1.0 with keys and XSD 1.1 `xs:assert` assertions, all of XPath 1.0, XInclude, SAX, DOM Traversal — and not a single third-party library, not even iconv or ICU.
2. **The most compact tree among full-featured libraries.** A 26 MB catalog takes 172 MB in memory — against 248 MB for libxml2 and 348 MB for Xerces-C++ (see [Performance](docs/comparison.en.md#performance)).
3. **The speed of a full-featured library.** Parsing is twice as fast as Xerces-C++, and almost on par with libxml2 on text. XPath is faster than libxml2, XSD validation is 1.8 times faster than Xerces-C++ — right during parsing or as a stream without a tree.
4. **Standalone nodes.** A node is created without a document, moved between documents without `importNode`/`adoptNode` and outlives the document it was taken from. Almost no C++ library has this (see the [comparison](docs/comparison.en.md#comparison-with-other-libraries)).
5. **Secure by default.** Entity bombs are always limited, external resources are not loaded without explicit permission, XSD regular expressions run in linear time, and hash tables keyed by document data are protected with a secret key against collision attacks (hash flooding).
6. **No recursion at all.** Parsing, copying, comparison, output, XSD validation and the destructor walk the tree with an explicit stack — a deep document will not crash the process.
7. **Well tested.** 159 unit tests, including load tests (20 threads on shared handles, 50,000 node moves, 50,000 attributes in a tag, a recursive schema 10,000 levels deep), 100 % of the applicable cases of the W3C XML Conformance Test Suite, the ASan/UBSan/LSan/TSan sanitizers, libFuzzer fuzzing.
8. **Six languages from one binary, plus WebAssembly.** C++, C, Java (Panama), C# (P/Invoke), Python (ctypes) and Node.js (Node-API) — all included, without JNI, node-gyp or packages from the network. The same API for browsers and Cloudflare Workers is the `beresta-wasm` package.
9. **Wrappers that do not slow down on heavy work.** Parsing, XPath and schema validation run inside the core at C++ speed. Streaming validation from Java, .NET and Python is faster than the native tools of these languages, and a multi-gigabyte export is validated in tens of megabytes of memory. Parsing with XSD validation in .NET is 2.3 times faster than `XmlDocument`, XPath in Java is 1.4–1.8 times faster than the JDK (see [Performance](docs/comparison.en.md#performance)).
10. **Ready packages for all platforms.** `npm install beresta`, `pip install beresta`, `dotnet add package Beresta`: the binaries for Windows and Linux (x64, arm64) and a universal one for macOS (Intel and Apple Silicon) are already inside — nothing needs to be compiled.
11. **Messages in Russian and English.** The language is chosen at build time and changed at run time; a new language is added with one file.
12. **Readable code.** Short functions, small files, bilingual comments everywhere — the library can be read, audited and improved on your own.

### Modern C++
- **Only `std::string` in UTF-8.** No `XMLCh`, `XMLString::transcode()` or manual `release()`. Values from the tree fit any C++ code right away.
- **RAII everywhere.** A parent owns its children through `std::unique_ptr`, so `delete`, `release()` and `XMLPlatformUtils::Initialize/Terminate` are not needed, and leaks are impossible by construction.
- **Nodes are standalone.** There is no global document manager. A node is created with a plain constructor, lives anywhere and is moved into any tree without `importNode`/`adoptNode`. The namespace of a node is computed from its current place in the tree, so after a move it is always correct for the new document.
- **A deep copy in one line:** `Element copy = original;`. For copying through the base class there is a virtual `clone()`.
- **Typed settings.** Instead of the string "features" of `DOMConfiguration` there is a plain `ParserOptions` structure, and the compiler catches a typo in a name.
- **Strong exception guarantees.** If an insertion, a replacement or an assignment fails, the tree stays as it was, and the passed node stays with the caller.

### Parsing
- **One parser — two modes.** The SAX core produces events, and the DOM is built by one of the handlers (the Builder pattern). SAX is as strict as DOM.
- **The input formatting does not matter.** Formatted and "solid" XML give the same tree.
- **Parsing on `std::string_view`.** For UTF-8 the input text is not copied: the scanner returns slices of the source string.
- **Iterative parsing without recursion.** Open elements and entity texts are kept in explicit stacks, so deep input will not overflow the call stack.
- **No recursion over the tree anywhere.** Copying, comparison, serialization (plain and indented), text collection, `normalize`, XSD validation and even the destructor walk the tree with an explicit stack. The destructor takes the tree apart without allocating memory at all. The document depth is limited only by memory: the tests build, copy, compare, print and parse a tree 100,000 levels deep.
- **Error positions in UTF-8 characters.** Coordinates are counted incrementally and only on request. The position of any node can be recorded (`trackLocations`).

### The full set of standards
- **XML 1.0 (5th edition):** passes **100 %** of the applicable cases of the official W3C XML Conformance Test Suite (2001 of 2585; the rest are XML 1.1 and cases for editions 1–4), see [`tests/conformance`](tests/conformance/README.md). Name characters follow the tables of the 5th edition.
- **Namespaces in XML:** checked while parsing, with "live" URIs in the DOM.
- **Eight encodings:** UTF-8, UTF-16 LE/BE, ISO-8859-1, US-ASCII, windows-1251, windows-1252, KOI8-R, IBM866. The encoding is detected by the BOM and the declaration. All tables are written from scratch, without iconv or ICU.
- **DTD:** internal and external subsets, parameter entities, conditional sections, default attributes, full validity checking, `getElementById`.
- **XML Schema 1.0:** 45 built-in types, facets, `restriction`/`extension`, `list`/`union`, `sequence`/`choice`/`all`, groups, `xs:any`, substitution groups, `xsi:type` with derivation checks, `block`/`final`, abstract elements and types, `xsi:nil`, ID/IDREF, the identity constraints `xs:key`/`xs:unique`/`xs:keyref`, `include`/`import`/`redefine`/`override`. The official W3C XML Schema Test Suite — **93.6 %** of the applicable cases; the deviations and deliberate limitations are in [`tests/xsts`](tests/xsts/README.md).
- **From XML Schema 1.1:** the `xs:assert` and `xs:assertion` assertions in XPath 1.0 with the common XPath 2.0 constructs (`if`, `some`/`every`, `eq`/`lt`…, `xs:decimal(…)`, `xs:date(…)`) and **exact decimal arithmetic** (`0.10 + 0.20 = 0.30` is true), `xs:override`, `vc:minVersion`/`vc:maxVersion`. Neither Xerces-C++, nor libxml2, nor .NET has assertions.
- **All of XPath 1.0:** all 13 axes, including `namespace`, all 27 functions of the core library, variables and prefixes.
- **XInclude 1.0:** `parse="xml|text"`, `xi:fallback`, XPointer `element()` and by ID, cycle protection.
- **DOM Traversal:** `TreeWalker` and `NodeIterator` with filters — the algorithms strictly follow the W3C specification.

### Reliability and security
- **Protection from "billion laughs".** Limits on the number, size and nesting of entity expansions are always on, not a separate setting.
- **No XXE or SSRF by default.** Without an explicit `ResourceResolver` nothing external is loaded. `FileResourceResolver` does not leave its root directory and rejects network URIs.
- **Protection from hash flooding.** Hash tables whose keys come from the document (ID values, element and attribute names) use SipHash-1-3 with a random per-process key: strings with the same hash cannot be prepared in advance.
- **No quadratic checks on hostile input.** Repeated attributes are searched by a loop only in tags with a few attributes, beyond that — by a hash table. A tag with 100,000 attributes (1.3 MB) is parsed in 0.07 s.
- **Our own XSD regular expressions on a Thompson NFA.** The check time is linear, "catastrophic backtracking" is impossible in principle.
- **Exact facet arithmetic.** Decimal numbers are compared as digit strings of any length, without the precision loss of `double`.
- **Checked by sanitizers:** all tests pass under AddressSanitizer, LeakSanitizer, UBSan and `_GLIBCXX_ASSERTIONS` — without memory leaks, undefined behavior or out-of-bounds container access, and under ThreadSanitizer — without races (20 threads on a shared schema, XPath and document).
- **Load tests for the wrapper scenarios:** 50,000 node moves between documents with a check of every previously given handle after the source document is freed; the parse time of a tag with 25,000 and 200,000 attributes grows linearly; a recursive schema 10,000 levels deep is validated the same way in SAX, DOM and over a ready tree.
- **The child index in O(1).** A node stores its number among its siblings, so `nextSibling`, `removeChild` of the last child and node moves from the wrappers do not search for the node by a scan: 150,000 moves through the C interface take 13 ms.
- **Checked by fuzzing:** three libFuzzer targets (parsing with any `ParserOptions`, XPath, XML Schema) under ASan + UBSan; the latest round has no findings, see [`tests/fuzz`](tests/fuzz/README.md).

### Output
- **Compact by default.** Not a single extra space, the attribute order is preserved, re-parsing the output gives the same text.
- **Pretty output on request.** Indentation is added only where it does not change the data: mixed content is written verbatim.
- **Any output encoding.** The declaration gets the right `encoding` itself, a BOM is written when needed, and characters outside the encoding become `&#N;`.

### One binary — six languages
- **A flat C interface** (`extern "C"`, 71 functions): a stable ABI, undecorated names, objects go out as opaque `void*` handles. C++ exceptions never cross the library boundary.
- **Java 22+ through Project Panama** (the Foreign Function & Memory API): not a line of JNI, Java works with native memory directly. The binary travels inside the jar and extracts itself for the right OS and architecture.
- **C# through P/Invoke** with `SafeHandle`: memory is freed deterministically (`using`) and reliably (on finalization). A NuGet package with the standard `runtimes/<rid>/native` layout.
- **Python through ctypes:** not a single package from PyPI and no compiler on the user's machine.
- **Node.js through an own Node-API addon:** without `ffi-napi`, node-gyp or npm dependencies; the ready binary travels in the package, TypeScript types are included.
- **WebAssembly** (`bindings/wasm`, the `beresta-wasm` package): the same core built by Emscripten and the same JS layer as for Node.js — DOM, XPath and XSD in browsers, Web Workers and Cloudflare Workers. The module weighs about 650 KB (about 250 KB gzipped).
- **A universal binary for macOS:** `make universal` glues the x86_64 and arm64 builds with `lipo`, CMake does the same with `CMAKE_OSX_ARCHITECTURES="x86_64;arm64"`.
- **Heavy work stays inside the core.** XPath (`selectValues` without node objects), schema validation and streaming validation of files of any size are called from any language with one call, and only the result crosses the language boundary.
- **The build lays out the binary itself.** CMake and the Makefile detect the OS and the processor architecture and copy the library into the Maven project resources, into `runtimes/<rid>/native` for .NET, into the Python package and (as an addon) into the npm package.
- **Only the C interface functions are visible from outside** — exactly 71 symbols. C++ classes are hidden (`-fvisibility=hidden`, on Windows only `BERESTA_API` is exported). The C++ runtime is embedded into the library, so a clean machine needs neither `libstdc++` nor `vcruntime`. On Linux the symbols of the embedded `libstdc++` are hidden too (`--exclude-libs,ALL`): in a JVM or Python process they do not conflict with the system `libstdc++`.

### Engineering qualities
- **Zero dependencies:** only the C++17 standard library. The wrappers need nothing either except the JDK, .NET, Python or Node.js.
- **Cross-platform:** Windows, Linux, macOS, WebAssembly. The compilers MSVC, MinGW, GCC, Clang, Emscripten. There are CMake and a Makefile with automatic dependencies, and the GitHub Actions CI builds and checks all of it on every push.
- **The node type without `dynamic_cast` in the core:** the type is found through `getType()`, the cast is a `static_cast`. Only the C interface checks handles through `dynamic_cast`: a text node instead of an element gives the `INVALID_ARGUMENT` error, not undefined behavior.
- **Messages in two languages:** all error texts live in the message catalog, Russian and English are included. The default language is chosen at build time, one function changes it at run time, and a new language is added with one file.
- **Readable code:** functions no longer than 25 lines, no more than 5 functions per file, detailed bilingual comments (Russian and English), classic patterns (Composite, Template Method, Builder, Facade, Strategy, Interpreter, Observer, Factory Method, Adapter).

---

## License

Beresta is distributed under the **MIT** license — the text is in the [LICENSE](LICENSE) file, details about the license in the packages are in [docs/development.en.md](docs/development.en.md#license).
