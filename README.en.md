# Beresta

[Русский](README.md) · **English**

**Beresta** (`beresta`) is a **C++17** XML library without a single third-party dependency. It fully replaces Xerces-C++: DOM, SAX, namespaces, encodings, DTD, XML Schema, XPath 1.0, XInclude, tree traversal and serialization.

The core is built into a shared library with a flat C interface. That is why Beresta can be used not only from C++, but also from **C**, **Java 22+** (Project Panama, no JNI), **C#** (P/Invoke), **Python** (ctypes) and **Node.js** (Node-API, no node-gyp, with TypeScript types), and the **WebAssembly** build works in browsers, Web Workers and Cloudflare Workers. The ready wrappers live in `bindings/`, and the CI builds the npm, PyPI and NuGet packages with all platforms inside (see [Ready packages and releases](#ready-packages-and-releases)).

The name comes from birch bark letters: in medieval Rus documents were written on birch bark (*beresta*). The library works with documents too, only XML ones.

```cpp
#include "beresta.hpp"

beresta::Document doc = beresta::XmlParser().parse("<greeting lang=\"ru\">Привет</greeting>");
std::cout << doc.getRoot()->getTextContent();                    // Привет
std::cout << beresta::xpath::evaluateString(doc, "/*/@lang");     // ru
std::cout << doc.toString();                                      // <greeting lang="ru">Привет</greeting>
```

---

## Contents

1. [Advantages](#advantages)
2. [Comparison with other libraries](#comparison-with-other-libraries)
3. [Where to use it](#where-to-use-it)
4. [Performance](#performance)
5. [Building and linking](#building-and-linking)
6. [Ready packages and releases](#ready-packages-and-releases)
7. [Quick start](#quick-start)
8. [Data model and memory](#data-model-and-memory)
9. [Working with the tree](#working-with-the-tree)
10. [Copying and moving nodes](#copying-and-moving-nodes)
11. [Parsing: DOM](#parsing-dom)
12. [Parsing: SAX](#parsing-sax)
13. [Namespaces](#namespaces)
14. [Encodings, files and streams](#encodings-files-and-streams)
15. [Serialization](#serialization)
16. [DTD and entities](#dtd-and-entities)
17. [XML Schema](#xml-schema)
18. [XPath 1.0](#xpath-10)
19. [Tree traversal: TreeWalker and NodeIterator](#tree-traversal-treewalker-and-nodeiterator)
20. [XInclude](#xinclude)
21. [External resources and security](#external-resources-and-security)
22. [Error handling](#error-handling)
23. [Message languages](#message-languages)
24. [Guarantees and thread safety](#guarantees-and-thread-safety)
25. [Other languages: the C interface](#other-languages-the-c-interface)
26. [Java (Project Panama)](#java-project-panama)
27. [C# (P/Invoke)](#c-pinvoke)
28. [Python (ctypes)](#python-ctypes)
29. [Node.js (Node-API)](#nodejs-node-api)
30. [WebAssembly (browsers and Workers)](#webassembly-browsers-and-workers)
31. [Limitations](#limitations)
32. [Project structure](#project-structure)
33. [Tests](#tests)
34. [Extending the library](#extending-the-library)
35. [License](#license)

---

## Advantages

### In short
1. **The full set of standards without dependencies.** XML 1.0, namespaces, eight encodings, DTD, XML Schema 1.0 with keys, all of XPath 1.0, XInclude, SAX, DOM Traversal — and not a single third-party library, not even iconv or ICU.
2. **The most compact tree among full-featured libraries.** A 26 MB catalog takes 172 MB in memory — against 248 MB for libxml2 and 348 MB for Xerces-C++ (see [Performance](#performance)).
3. **The speed of a full-featured library.** Parsing is twice as fast as Xerces-C++, and almost on par with libxml2 on text. XPath is faster than libxml2, XSD validation is 1.8 times faster than Xerces-C++ — right during parsing or as a stream without a tree.
4. **Standalone nodes.** A node is created without a document, moved between documents without `importNode`/`adoptNode` and outlives the document it was taken from. Almost no C++ library has this (see the [comparison](#comparison-with-other-libraries)).
5. **Secure by default.** Entity bombs are always limited, external resources are not loaded without explicit permission, XSD regular expressions run in linear time, and hash tables keyed by document data are protected with a secret key against collision attacks (hash flooding).
6. **No recursion at all.** Parsing, copying, comparison, output, XSD validation and the destructor walk the tree with an explicit stack — a deep document will not crash the process.
7. **Well tested.** 136 unit tests, 100 % of the applicable cases of the W3C XML Conformance Test Suite, the ASan/UBSan/LSan sanitizers, libFuzzer fuzzing.
8. **Six languages from one binary, plus WebAssembly.** C++, C, Java (Panama), C# (P/Invoke), Python (ctypes) and Node.js (Node-API) — all included, without JNI, node-gyp or packages from the network. The same API for browsers and Cloudflare Workers is the `beresta-wasm` package.
9. **Ready packages for all platforms.** `npm install beresta`, `pip install beresta`, `dotnet add package Beresta`: the binaries for Windows and Linux (x64, arm64) and a universal one for macOS (Intel and Apple Silicon) are already inside — nothing needs to be compiled.
10. **Messages in Russian and English.** The language is chosen at build time and changed at run time; a new language is added with one file.
11. **Readable code.** Short functions, small files, bilingual comments everywhere — the library can be read, audited and improved on your own.

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
- **XML Schema 1.0:** 45 built-in types, facets, `restriction`/`extension`, `list`/`union`, `sequence`/`choice`/`all`, groups, `xs:any`, `xsi:type`, `xsi:nil`, ID/IDREF, the identity constraints `xs:key`/`xs:unique`/`xs:keyref`, `include`/`import`.
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
- **Checked by sanitizers:** all tests pass under AddressSanitizer, LeakSanitizer, UBSan and `_GLIBCXX_ASSERTIONS` — without memory leaks, undefined behavior or out-of-bounds container access.
- **Checked by fuzzing:** three libFuzzer targets (parsing with any `ParserOptions`, XPath, XML Schema) under ASan + UBSan; the latest round has no findings, see [`tests/fuzz`](tests/fuzz/README.md).

### Output
- **Compact by default.** Not a single extra space, the attribute order is preserved, re-parsing the output gives the same text.
- **Pretty output on request.** Indentation is added only where it does not change the data: mixed content is written verbatim.
- **Any output encoding.** The declaration gets the right `encoding` itself, a BOM is written when needed, and characters outside the encoding become `&#N;`.

### One binary — six languages
- **A flat C interface** (`extern "C"`, 69 functions): a stable ABI, undecorated names, objects go out as opaque `void*` handles. C++ exceptions never cross the library boundary.
- **Java 22+ through Project Panama** (the Foreign Function & Memory API): not a line of JNI, Java works with native memory directly. The binary travels inside the jar and extracts itself for the right OS and architecture.
- **C# through P/Invoke** with `SafeHandle`: memory is freed deterministically (`using`) and reliably (on finalization). A NuGet package with the standard `runtimes/<rid>/native` layout.
- **Python through ctypes:** not a single package from PyPI and no compiler on the user's machine.
- **Node.js through an own Node-API addon:** without `ffi-napi`, node-gyp or npm dependencies; the ready binary travels in the package, TypeScript types are included.
- **WebAssembly** (`bindings/wasm`, the `beresta-wasm` package): the same core built by Emscripten and the same JS layer as for Node.js — DOM, XPath and XSD in browsers, Web Workers and Cloudflare Workers. The module weighs about 650 KB (about 250 KB gzipped).
- **A universal binary for macOS:** `make universal` glues the x86_64 and arm64 builds with `lipo`, CMake does the same with `CMAKE_OSX_ARCHITECTURES="x86_64;arm64"`.
- **The build lays out the binary itself.** CMake and the Makefile detect the OS and the processor architecture and copy the library into the Maven project resources, into `runtimes/<rid>/native` for .NET, into the Python package and (as an addon) into the npm package.
- **Only the C interface functions are visible from outside** — exactly 69 symbols. C++ classes are hidden (`-fvisibility=hidden`, on Windows only `BERESTA_API` is exported). The C++ runtime is embedded into the library, so a clean machine needs neither `libstdc++` nor `vcruntime`. On Linux the symbols of the embedded `libstdc++` are hidden too (`--exclude-libs,ALL`): in a JVM or Python process they do not conflict with the system `libstdc++`.

### Engineering qualities
- **Zero dependencies:** only the C++17 standard library. The wrappers need nothing either except the JDK, .NET, Python or Node.js.
- **Cross-platform:** Windows, Linux, macOS, WebAssembly. The compilers MSVC, MinGW, GCC, Clang, Emscripten. There are CMake and a Makefile with automatic dependencies, and the GitHub Actions CI builds and checks all of it on every push.
- **The node type without `dynamic_cast` in the core:** the type is found through `getType()`, the cast is a `static_cast`. Only the C interface checks handles through `dynamic_cast`: a text node instead of an element gives the `INVALID_ARGUMENT` error, not undefined behavior.
- **Messages in two languages:** all error texts live in the message catalog, Russian and English are included. The default language is chosen at build time, one function changes it at run time, and a new language is added with one file.
- **Readable code:** functions no longer than 25 lines, no more than 5 functions per file, detailed bilingual comments (Russian and English), classic patterns (Composite, Template Method, Builder, Facade, Strategy, Interpreter, Observer, Factory Method, Adapter).

---

## Comparison with other libraries

What popular XML libraries for C and C++ can do "out of the box" (versions as in the [measurements](#performance)):

| Feature | Beresta | libxml2 | Xerces-C++ | pugixml | tinyxml2 | RapidXML |
|---|---|---|---|---|---|---|
| Strings in the API | `std::string` (UTF-8) | `xmlChar*` (UTF-8), manual `xmlFree` | `XMLCh*` (UTF-16), `transcode`/`release` | `char*` or `wchar_t*` | `const char*` | `char*` |
| Namespaces | yes, "live" URIs | yes | yes | no (the prefix is part of the name) | no | no |
| DTD validation | yes | yes | yes | no | no | no |
| XML Schema | 1.0, with keys | 1.0, partial | 1.0, the most complete | no | no | no |
| XPath 1.0 | all of it, with the `namespace` axis | yes | only for XSD keys | yes, prefixes are part of the name | no | no |
| XInclude | yes | yes | yes | no | no | no |
| Encodings without external libraries | 8, including windows-1251, KOI8-R, IBM866 | UTF-8/16, Latin-1; the rest through iconv | many built in, the rest through ICU | UTF-8/16/32, Latin-1 | UTF-8 | UTF-8 |
| Protection from entity bombs | always | yes (limits) | as a separate setting | entities are not expanded | entities are not expanded | entities are not expanded |
| External resources by default | not loaded | only with parse flags | loaded | not supported | not supported | not supported |
| A node without a document | yes | partially, see below | no | no | no | no |
| Moving a node to another document | `removeChild` + `appendChild` | manually: `xmlDOMWrapAdoptNode` | `importNode` (a copy) | only a copy | only a copy | unsafe (pool memory) |
| Memory model | `unique_ptr`, RAII | manual (`xmlFree*`) | manual (`release()`) | RAII, document pool | document pool | document pool, modifies the input |
| Wrappers included | C, Java, C#, Python, Node.js | no (third-party: lxml, libxmljs) | no | no | no | no |
| WebAssembly included | yes, `beresta-wasm` | no (third-party builds) | no | no | no | no |
| Hash table protection from collisions | SipHash with a random key | yes (a random dictionary seed) | no | no hash tables for input | no hash tables for input | no hash tables for input |
| Third-party dependencies | none | zlib, iconv, ICU (optional) | ICU, curl (optional) | none | none | none |
| License | MIT | MIT | Apache 2.0 | MIT | zlib | Boost / MIT |

**Standalone nodes** in other libraries:
- **Xerces-C++** and all W3C DOM implementations (JAXP in Java, `XmlDocument` in .NET, the browser DOM) bind a node to its owner document: it can be created only through `document.createElement`, and put into another document only through `importNode` (a copy) or `adoptNode`. The memory of Xerces nodes is freed together with the document.
- **libxml2** allows creating a node without a document (`xmlNewNode(NULL, ...)`), but when moving between documents you have to fix the references to the document string dictionary and to the namespace declarations yourself (`xmlDOMWrapAdoptNode`, `xmlReconciliateNs`). A mistake here is a dangling pointer.
- **pugixml, tinyxml2, RapidXML** allocate nodes from the pool of their document: a node does not exist without a document, and it can be moved into another document only by copying.
- A similar model exists in other languages: **LINQ to XML** (`XElement` in .NET), **JDOM** in Java and **ElementTree** in Python. In C++ Beresta gives the same, but with ownership through `std::unique_ptr`: whoever holds the pointer owns the node.

**What to choose.** If you need only fast parsing of trusted XML without namespaces and checks, pugixml or RapidXML are 10–15 times faster. If you need complete XML Schema 1.0 with `xs:redefine` and substitution groups (large industry schemas), take Xerces-C++ or libxml2. Beresta is for everything in between: full-featured XML with namespaces, DTD, XSD and XPath, secure by default, without dependencies, with a convenient C++ API and wrappers for five languages and WebAssembly.

---

## Where to use it

Beresta is designed for tasks where XML is an exchange format with strict rules: the document must match a schema, the check cannot be skipped, and the data comes from outside and may be hostile.

### For developers in Russia and the CIS

There XML is the main format of legally significant exchange, and it is almost always described by XSD schemas. A few typical tasks:

- **Reporting to government agencies.** Electronic reports to the Federal Tax Service (FNS), the Social Fund (SFR) and other agencies are transmitted in XML formats published together with their XSD schemas. A file can be validated against the schema before it is sent: every error comes with the path to the element (`/Файл/Документ/СвНП[1]`), a position and a clear text in Russian or English.
- **Interagency exchange (SMEV 3).** SOAP envelopes and data types are described by XSD with namespaces. Keys and references between records (`xs:key`, `xs:keyref`, `xs:unique`) are checked by the schema itself, without application code.
- **Electronic document flow.** Universal transfer documents (UPD), invoices and acts in FNS formats, documents with marking codes, exchange with EGAIS are XML with strict schemas and Cyrillic text. Eight encodings, including windows-1251, KOI8-R and IBM866, work without iconv or ICU.
- **Procurement and open data.** Exports of the Unified Procurement Information System (EIS) and open data portals are large XML files. For them there is streaming SAX — including XSD validation without building a tree — and the most compact tree among full-featured libraries: 1.4 times less memory than libxml2 and half of what Xerces-C++ needs.
- **Banks and finance.** ISO 20022 payment messages and the electronic formats of the Bank of Russia are XML with schemas. Security by default (no XXE, entity bombs or hash collision attacks) closes a whole class of vulnerabilities before an audit finds them.
- **Accounting systems and trade.** Exchange with 1C (EnterpriseData, CommerceML), marketplaces and online shops goes in XML. The same schema can be checked in a Java or .NET service, a Python script, a Node.js server and right in the browser — with the same result.
- **Isolated environments and import substitution.** There are no third-party dependencies, the license is MIT, the sources are open and readable (short functions, bilingual comments): the code can be audited, built from source without internet access and included into your own product. The core is portable C++17 without assembly or system-specific extensions, so it should build with a regular GCC or Clang on domestic Linux distributions. Builds on Astra Linux, RED OS, ALT and Elbrus processors have not been checked in this repository yet.

### In any project

- **XML validation in the browser and on the edge.** `beresta-wasm` validates a file against XSD and runs XPath right on the page, before anything is sent to the server, and in Web Workers too.
- **The same behavior in every language.** Services in Java, .NET, Python and Node.js use one core, hence the same error messages, the same whitespace rules and the same value comparison.
- **SOAP and WSDL integrations, configurations, RSS and Atom, SVG, XML inside office formats** (OOXML and ODF are XML in a ZIP archive; the application does the unpacking).
- **Embedded systems and C and C++ tools.** No dependencies and no recursion, and memory use is predictable: a deep or huge document will not crash the process.

When another library is a better choice is said in [Comparison with other libraries](#comparison-with-other-libraries) ("What to choose").

---

## Performance

The measurements are made by the [`benchmarks/`](benchmarks/) program on the same data for all libraries: a product catalog (26 MB: 100,000 elements with attributes, entities and Cyrillic) and an article (14 MB: a long Russian text with mixed content). The median of 5 runs, AMD Ryzen 5 5600X, Ubuntu 26.04, GCC 15 `-O2`, the libraries from the distribution packages. The walk checksums, the output size and the XPath node count match for all libraries — so they built the same tree.

**Catalog, 26 MB** (parsing and validation — MB/s, more is better; walk and XPath — ms, less is better; "Parse + XSD" — a tree and validation, "SAX + XSD" — streaming validation without a tree):

| Library | Parse | Walk | Output | XPath | Parse + XSD | SAX + XSD | DOM memory |
|---|---|---|---|---|---|---|---|
| **Beresta 2.1** | 86 | 47 | 421 | **108** | 62 | 93 | **172 MB (×6.5)** |
| libxml2 2.15 | 120 | 27 | 523 | 127 | 75 | 112 | 248 MB (×9.3) |
| Xerces-C++ 3.2 | 46 | 65 | 156 | — | 35 | — | 348 MB (×13.0) |
| pugixml 1.14 | 1563 | 39 | 1333 | 17 | — | — | 107 MB (×4.0) |
| tinyxml2 11 | 185 | 15 | 416 | — | — | — | 176 MB (×6.6) |
| RapidXML 1.13 | 1166 | 26 | 593 | — | — | — | 145 MB (×5.4) |

**Article, 14 MB:**

| Library | Parse | Walk | Output | XPath | DOM memory |
|---|---|---|---|---|---|
| **Beresta 2.1** | 186 | 11 | 438 | 35 | **39 MB (×2.8)** |
| libxml2 2.15 | 202 | 6 | 1020 | 11 | 57 MB (×4.0) |
| Xerces-C++ 3.2 | 8 | 9 | 238 | — | 64 MB (×4.5) |
| pugixml 1.14 | 2309 | 4 | 2067 | 3 | 28 MB (×2.0) |

How to read these numbers:
- **Among full-featured libraries** (DOM + namespaces + DTD + XSD + encoding checks) Beresta parses twice as fast as Xerces-C++, text almost on par with libxml2 (the catalog 1.4 times slower), and runs XPath on the catalog faster than libxml2. It needs less memory for the tree than both.
- **pugixml and RapidXML** are 10–15 times faster, but they are a different class: in-place parsing without checks — no namespaces, DTD, XSD, character or encoding checks, and RapidXML also modifies the input buffer. If you need only fast parsing of trusted XML, they are the right choice.
- **Parsing with XSD validation** is 1.8 times faster than with Xerces-C++ and 1.2 times slower than with libxml2. Beresta validates the document right while parsing, in one pass; libxml2 in this measurement builds a tree and validates it in a second pass (`xmlSchemaValidateDoc`). The gap to libxml2 is mostly the difference in the speed of parsing itself (86 against 120 MB/s): validation costs Beresta no more.
- **Streaming validation without a tree** (`SaxParser` with a schema) — 93 MB/s, 1.5 times faster than with building a tree; libxml2 (`xmlSchemaValidateStream`) — 112 MB/s. The validation needs O(document depth) memory — for multi-gigabyte exports that do not fit into a tree.

Repeating the measurement (Linux; the libraries for comparison are needed only by the benchmark program):

```bash
sudo apt install libxml2-dev libpugixml-dev libxerces-c-dev libtinyxml2-dev librapidxml-dev
```
```bash
cd benchmarks && make run
```

**Node.js 22, a 5 MB catalog** (20,000 products; parsing — MB/s, walk and XPath — ms):

| Library | Parse | Walk | XPath | Nodes walked |
|---|---|---|---|---|
| **Beresta 2.1 (Node-API)** | **55** | 116 | **33** | 280,001 |
| libxmljs2 0.37 (a libxml2 wrapper) | 53 | 229 | 38 | 280,001 |
| @xmldom/xmldom 0.9 + xpath | 8 | 15 | 237,658 | 280,001 |
| fast-xml-parser 5.11 | 14 | 9 | — | 220,006 |

- Parsing is measured on a JavaScript string, so for Beresta it includes encoding into UTF-8 (`Buffer.from`). Among libraries with a real DOM and XPath Beresta is the fastest: parsing on par with libxmljs2, the walk twice as fast, XPath slightly faster.
- Pure JavaScript libraries parse 4–7 times slower. Their walk is faster because the tree already lies in the JavaScript heap and no step crosses the Node-API boundary. XPath over xmldom took almost 4 minutes on this query (a single measurement).
- fast-xml-parser builds not a DOM but plain objects: it has no XPath, namespaces or checks, hence fewer nodes walked.
- To repeat: `npm install fast-xml-parser @xmldom/xmldom xpath libxmljs2` in a separate directory, then `BENCH_NODE_MODULES=<directory> node benchmarks/node/compare.js 20000`.

**The native addon and WebAssembly** (the same 5 MB catalog, Node.js 22, one run for both):

| Build | Parse, MB/s | XPath, ms |
|---|---|---|
| Node-API addon (`beresta`) | 66 | 35 |
| WebAssembly (`beresta-wasm`) | 43 | 29 |

WebAssembly parses about a third slower than native code (the usual price of Wasm), while its XPath is even faster: the result is assembled inside the module, without crossing the Node-API boundary at every node.

**Wrappers against the native XML libraries of their languages** (a 13 MB catalog, 50,000 products; parsing and validation — MB/s, walk and XPath — ms; memory — the growth of the process memory after parsing; all libraries agree on the element count, the XPath node count and the schema error count):

| Language | Library | Parse | Walk | XPath | Parse + XSD | Memory |
|---|---|---|---|---|---|---|
| Java 25 | **Beresta (Panama)** | 103 | 438 | **69** | 63 | **88 MB (×6.6)** |
| | JDK DOM (`javax.xml`, Xerces-J) | **146** | **4** | 97 | **68** | 123 MB (×9.3) |
| .NET 10 | **Beresta (P/Invoke)** | 105 | 228 | 88 | **65** | 86 MB (×6.5) |
| | `XmlDocument` | 70 | 30 | **23** | 28 | 72 MB (×5.5) |
| | `XDocument` (LINQ to XML) | **116** | **4** | 28 | 42 | **54 MB (×4.1)** |
| Python 3.14 | **Beresta (ctypes)** | 103 | 2420 | 103 | 66 | 86 MB (×6.5) |
| | lxml 6.1 (libxml2 2.14) | **126** | 25 | **57** | **78** | 119 MB (×8.9) |
| | `xml.etree.ElementTree` | 61 | **6** | — | — | **82 MB (×6.2)** |

Where the wrappers make sense:
- **The core is the same everywhere**, so parsing and schema validation run at C++ speed (63–105 MB/s) in any language. Walking the tree from a wrapper is slow: every step (`firstChild`, `nextSibling`) is a call across the language boundary and a wrapper object. The fewer tree steps on the language side, the better the wrapper pays off: XPath, `selectValues`, schema validation and output run entirely inside the core.
- **Node.js and the browser** are the clearest win: among libraries with a real DOM, XPath and XSD Beresta is the fastest (see the table above), and a browser has no built-in XSD validation at all.
- **.NET** — XSD validation is 1.5–2.3 times faster than in `System.Xml`, parsing is on par with `XDocument`. If no schema is needed, `XDocument` is faster and leaner.
- **Java** — XPath is 1.4 times faster, the tree is 30 % more compact; parsing is slower than in the built-in Xerces-J, XSD validation is on par. Another difference is security by default: the standard `DocumentBuilder` loads external entities until it is explicitly configured not to.
- **Python** — lxml is faster in everything except memory: the ctypes overhead is large. Here the wrapper is justified if the same behavior as in the other languages, no dependencies or memory matter.
- Common reasons to choose the wrapper: the same error messages, paths and rules in all languages, security by default, the windows-1251/KOI8-R/IBM866 encodings without external libraries.

To repeat: `node benchmarks/wrappers/make_data.js`, then the programs in `benchmarks/wrappers/` (see [`benchmarks/README.md`](benchmarks/README.md)).

---

## Building and linking

A compiler with **C++17** support is required. There are two independent ways to build — CMake and the Makefile; both work on Windows, Linux and macOS.

What you get:

| File | For whom |
|---|---|
| `beresta.dll` / `libberesta.so` / `libberesta.dylib` | the shared library with the C interface: C, Java, C#, Python |
| `libberesta.a` (Makefile) / `beresta_static` (CMake) | the static library for C++ programs (the full C++ API) |
| `beresta.node` | the Node.js addon: the Node-API adapters + the core in one file |
| `beresta_core.wasm` + `beresta.mjs` | WebAssembly for browsers, Workers and Node.js |

**Only** the C interface functions are exported from the shared library. C++ programs link the static library.

### Makefile (GCC / Clang / MinGW)

```bash
make
```

| Target | What it does |
|---|---|
| `make` / `make all` | `bin/libberesta.a`, the shared library, the demos `bin/beresta_demo` (C++) and `bin/beresta_capi_demo` (C) |
| `make test` | builds and runs the C++ unit tests |
| `make JAVA=1` | the same + the binary into the Maven project resources |
| `make CSHARP=1` | the same + the binary into `runtimes/<rid>/native` of the .NET project |
| `make PYTHON=1` | the same + the binary into the Python package |
| `make NODE=1` | the same + the `beresta.node` addon into the Node.js package |
| `make bindings-test` | self-tests of the enabled wrappers, for example `make JAVA=1 CSHARP=1 PYTHON=1 NODE=1 bindings-test` |
| `make java-test`, `csharp-test`, `python-test`, `node-test` | the self-test of one wrapper (copies the binary itself) |
| `make java-package` | a jar through Maven (`mvn package`) |
| `make csharp-package` | a NuGet package (`dotnet pack`) |
| `make node-package` | an npm package (`npm pack`) |
| `make universal` | macOS: universal `libberesta.a`, `libberesta.dylib` and `beresta.node` (x86_64 + arm64, glued by `lipo`) laid out into the folders of both architectures |
| `make wasm` | WebAssembly: the core through `em++` (Emscripten) into `bindings/wasm/dist/` and the `beresta.mjs` bundle |
| `make wasm-test` | `make wasm` + the self-test of the WebAssembly package in Node.js |
| `make ERRORS_LANG=en` | the default error message language (`ru` if not set); see [Message languages](#message-languages) |
| `make platform` | show the detected OS, architecture and copy paths |
| `make clean` | removes `obj/` and `dep/` |
| `make fclean` | `clean` + `bin/`, the binary copies in the wrappers, `bindings/wasm/dist/` and `bindings/java/target/` |
| `make re` | a full rebuild |

Makefile specifics:
- `make -j` is supported, the compiler can be changed (`make CXX=clang++ CC=clang`).
- Object and `.d` files are laid out into `obj/` and `dep/` mirroring `src/`. A changed header rebuilds only the dependent files, and a changed Makefile (for example, the flags) rebuilds all objects.
- Cleaning detects the shell itself: `rmdir /S /Q` in `cmd.exe` on Windows and `rm -rf` in `sh` on Linux and macOS.
- The architecture is taken from the compiler itself (`$(CXX) -dumpmachine`), so with cross-compilation the binary goes into the folder of the target platform.
- Builds for another architecture and for WebAssembly run as a separate make invocation with their own `obj/<arch>`, `dep/<arch>` and `bin/<arch>` and do not mix with the regular one. On macOS the code targets a version not lower than `MACOS_MIN` (11.0 by default — the first one with Apple Silicon).

### CMake (MSVC / MinGW / GCC / Clang)

By default CMake builds **only the core** — the shared library:

```bash
cmake -S . -B build
```
```bash
cmake --build build --config Release
```

Everything else is enabled with flags:

| Option | Default | What it does |
|---|---|---|
| `BERESTA_BUILD_STATIC` | `OFF` | the static library `beresta_static` (`beresta::static`) for C++ |
| `BERESTA_BUILD_EXAMPLES` | `OFF` | the demos in C++ and in pure C |
| `BERESTA_BUILD_TESTS` | `OFF` | the unit tests and the self-tests of the enabled wrappers in `ctest` |
| `BERESTA_BUILD_JAVA` | `OFF` | after the build the binary is copied into the Maven project resources; the `beresta_java_package` target runs `mvn package` |
| `BERESTA_BUILD_CSHARP` | `OFF` | the binary into `runtimes/<rid>/native`; the `beresta_csharp_package` target runs `dotnet pack` |
| `BERESTA_BUILD_PYTHON` | `OFF` | the binary into the Python package |
| `BERESTA_BUILD_NODE` | `OFF` | the `beresta.node` addon into the Node.js package; the `beresta_node_package` target runs `npm pack` |
| `BERESTA_ERRORS_LANG` | `ru` | the default error message language: `ru`, `en` or any added one |
| `BERESTA_STATIC_RUNTIME` | `ON` | embed the C++ runtime into the library (`/MT` for MSVC, `-static` for MinGW, `-static-libstdc++` for GCC) |

A full build with all wrappers and tests:

```bash
cmake -S . -B build -DBERESTA_BUILD_TESTS=ON -DBERESTA_BUILD_JAVA=ON -DBERESTA_BUILD_CSHARP=ON -DBERESTA_BUILD_PYTHON=ON -DBERESTA_BUILD_NODE=ON
```
```bash
cmake --build build --config Release
```
```bash
ctest --test-dir build -C Release --output-on-failure
```

For MSVC `/utf-8 /W4 /permissive-` are enabled automatically. Without an explicit `CMAKE_BUILD_TYPE` single-config generators (Make, Ninja) build `Release`.

A universal macOS binary and WebAssembly:

```bash
cmake -S . -B build -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
```
```bash
emcmake cmake -S . -B build-wasm -DBERESTA_BUILD_TESTS=ON
```

Under Emscripten CMake builds only the WebAssembly module (`cmake/BerestaWasm.cmake`): the shared library and the wrappers are not needed in a browser.

### Where the binary is copied

CMake (the `cmake/BerestaPlatform.cmake` module) and the Makefile detect the **target** OS and architecture and normalize the names. The wrapper loaders look for the same names:

| Wrapper | Folder |
|---|---|
| Java | `bindings/java/src/main/resources/natives/<os>-<arch>/` |
| C# | `bindings/csharp/Beresta/runtimes/<rid>/native/` |
| Python | `bindings/python/beresta/_native/<os>-<arch>/` |
| Node.js | `bindings/node/native/<os>-<arch>/beresta.node` |

- `<os>-<arch>`: `windows-x86_64`, `windows-aarch64`, `linux-x86_64`, `linux-aarch64`, `macos-x86_64`, `macos-aarch64`, `*-x86`.
- `<rid>` (the .NET runtime identifier): `win-x64`, `win-arm64`, `linux-x64`, `linux-arm64`, `osx-x64`, `osx-arm64`.
- The architecture source in CMake: `MSVC_CXX_ARCHITECTURE_ID` for MSVC (correct for `-A ARM64` too), `CMAKE_OSX_ARCHITECTURES` on macOS, otherwise `CMAKE_SYSTEM_PROCESSOR`. The values can be set manually: `-DBERESTA_PLATFORM_ARCH=aarch64`.
- A universal macOS binary (`make universal` or two architectures in `CMAKE_OSX_ARCHITECTURES`) goes into both `macos-x86_64` and `macos-aarch64` (`osx-x64` and `osx-arm64` for .NET): the loaders find it on any Mac.
- One jar, NuGet package, Python wheel or npm package for all platforms is built by the release CI: every OS adds its own folder, and packaging happens once (see [Ready packages and releases](#ready-packages-and-releases)).

### Linking into your C++ project

```cmake
set(BERESTA_BUILD_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/beresta)
target_link_libraries(my_app PRIVATE beresta::static)
```

The `beresta::beresta` target is the shared library. C programs link it (through `capi/beresta_c.h`).

Or manually:

```bash
g++ -std=c++17 -Ipath/to/beresta/include main.cpp path/to/beresta/bin/libberesta.a -o app
```

One header is enough in the code: `#include "beresta.hpp"`.

---

## Ready packages and releases

The user does not need to compile the core: the packages already contain the binaries for all platforms.

| Language | Installation | What is inside |
|---|---|---|
| Node.js | `npm install beresta` | addons for Windows and Linux (x64, arm64) and a universal one for macOS |
| Browsers, Workers | `npm install beresta-wasm` | `beresta_core.wasm` + an ES module with TypeScript types |
| Python | `pip install beresta` | one `py3-none-any` wheel with the libraries of all platforms |
| .NET | `dotnet add package Beresta` | `runtimes/<rid>/native` for `win-x64`, `win-arm64`, `linux-x64`, `linux-arm64`, `osx-x64`, `osx-arm64` |
| Java | the jar from the [GitHub release](https://github.com/ErmaleyAlekov/beresta/releases) | `natives/<os>-<arch>/` of all platforms, the loader extracts the right one |
| C and C++ | the SDK archive `beresta-<version>-<platform>.zip` from the release | `include/`, the shared and static libraries, `LICENSE` |

The Linux binaries are built on Ubuntu 22.04 and work with glibc 2.35 and newer.

### CI/CD in GitHub Actions

- **`.github/workflows/ci.yml`** — on every push and pull request: Linux x64 and arm64 (the Makefile with GCC, CMake with Clang, all wrappers), Windows x64 and arm64 (MSVC, `ctest`), macOS (`make universal`, a `lipo` check, the C++ tests for arm64 and for x86_64 through Rosetta), WebAssembly (the official `emscripten/emsdk` image), the ASan/UBSan/LSan sanitizers and the W3C XML Conformance Test Suite (the run fails if even one applicable case does not pass).
- **`.github/workflows/release.yml`** — on a `vX.Y.Z` tag: checks that the tag version matches the versions in `CMakeLists.txt`, `package.json` and `pyproject.toml`; builds the native binaries on five platforms; merges them into one tree; builds the npm packages (`beresta`, `beresta-wasm`), the Python wheel, NuGet, the jar and the SDK archives; installs the wheel and the npm package into a clean environment and checks that they work; creates a GitHub Release with all files.

Publishing to the registries is enabled by repository secrets (Settings → Secrets and variables → Actions). While a secret is missing, the step is skipped, and the packages still lie in the GitHub Release:

| Secret | Where |
|---|---|
| `NPM_TOKEN` | npm (`beresta`, `beresta-wasm`, with provenance) |
| `PYPI_API_TOKEN` | PyPI (`beresta`) |
| `NUGET_API_KEY` | NuGet (`Beresta`) |

Releasing a version: bump the version in `CMakeLists.txt`, `include/capi/beresta_c.h`, `bindings/*/package.json`, `bindings/python/pyproject.toml`, `bindings/java/pom.xml` and `bindings/csharp/Beresta/Beresta.csproj`, then:

```bash
git tag v2.1.0
```
```bash
git push origin v2.1.0
```

---

## Quick start

### Parse, find, change, save

```cpp
#include "beresta.hpp"
#include <iostream>

int main()
{
    try
    {
        beresta::Document doc = beresta::XmlParser().parseFile("library.xml");

        for (beresta::Element* book : beresta::xpath::selectElements(doc, "//book[@lang='ru']"))
            book->setAttribute("checked", "yes");

        beresta::SerializerOptions options;
        options.prettyPrint = true;
        beresta::XmlSerializer(options).writeFile(doc, "library.out.xml");
    }
    catch (const beresta::DomException& error)
    {
        std::cerr << error.what() << '\n';     // [PARSE_ERROR] line 3, column 7: ...
        return 1;
    }
    return 0;
}
```

### Build a document from scratch

```cpp
beresta::Document doc;
doc.setDeclaration(beresta::XmlDeclaration{"1.0", "UTF-8", std::nullopt});

beresta::Element* root = doc.setRoot(std::make_unique<beresta::Element>("note"));
root->setAttribute("title", "Q&A");
root->appendChild(std::make_unique<beresta::Comment>(" draft "));

auto body = std::make_unique<beresta::Element>("body");
body->setTextContent("1 < 2");
root->appendChild(std::move(body));

std::cout << doc.toString();
// <?xml version="1.0" encoding="UTF-8"?><note title="Q&amp;A"><!-- draft --><body>1 &lt; 2</body></note>
```

> If the Windows console shows garbage instead of Cyrillic, switch the console to UTF-8: `SetConsoleOutputCP(CP_UTF8)` from `<windows.h>` (this is done in `examples/` and `tests/`).

---

## Data model and memory

### The class hierarchy (the Composite pattern)

```
Node                          an abstract node
├── ContainerNode             a node with children (abstract)
│   ├── Element               a tag, attributes, children
│   └── Document              the declaration, DOCTYPE, prolog, one root
├── CharacterNode             a node with a string value (abstract)
│   ├── Text
│   ├── Comment
│   └── CDataSection
├── ProcessingInstruction     <?target data?>
└── DocumentType              <!DOCTYPE ...> + the DTD model
```

`NodeType`: `DOCUMENT`, `DOCUMENT_TYPE`, `ELEMENT`, `TEXT`, `CDATA_SECTION`, `COMMENT`, `PROCESSING_INSTRUCTION`.

The XML declaration is not a node but a property of the document (`Document::getDeclaration()`). The PI target `xml` is reserved: `ProcessingInstruction("xml", ...)` throws `INVALID_NAME`.

### Ownership

- A parent **owns** its children (`std::vector<std::unique_ptr<Node>>`).
- A child **knows** its parent through a non-owning pointer and keeps its own index among the siblings. That is why `getNextSibling()` works in O(1).
- Ownership goes only top-down: there are no cycles, and deleting the root frees the whole subtree.

**Why not `shared_ptr` + `weak_ptr`:** then every node would have to be created in a `shared_ptr`, and a plain `Element copy = original;` on the stack could not bind the children to the copy.

| Where the pointer comes from | How long it lives |
|---|---|
| `appendChild`, `getChild`, `findChildElement`, `getRoot`, XPath, `TreeWalker` | while the node is in the tree and the tree is alive |
| `removeChild`, `detachChild`, `replaceChild`, `releaseRoot`, `clone`, `parseElement` | you own the node (`unique_ptr`) |

---

## Working with the tree

### Navigation

```cpp
beresta::Element* root = doc.getRoot();
beresta::Node* first = root->getFirstChild();
beresta::Node* next = first->getNextSibling();          // O(1)
beresta::Node* last = root->getLastChild();
std::size_t index = next->getIndexInParent();
beresta::Document* owner = next->getOwnerDocument();
bool inside = next->isDescendantOf(*root);
```

### Changing

```cpp
root->appendChild(std::make_unique<beresta::Element>("item"));
root->insertChild(0, std::make_unique<beresta::Comment>("start"));
std::unique_ptr<beresta::Node> old = root->replaceChild(1, std::make_unique<beresta::Text>("new"));
std::unique_ptr<beresta::Node> taken = root->detachChild(*root->getLastChild());
root->normalize();                 // merge adjacent Text nodes, remove empty ones
root->setTextContent("text only");
root->clearChildren();
```

### Attributes

```cpp
item.setAttribute("id", "42");                       // add or replace (the position is kept)
std::optional<std::string> id = item.getAttribute("id");
bool has = item.hasAttribute("id");
bool removed = item.removeAttribute("id");
for (const beresta::Attribute& a : item.getAttributes()) { /* a.name, a.value */ }
item.setAttributes({{"a", "1"}, {"b", "2"}});        // a batch in O(n); names are checked before any change
```

### Search

```cpp
const beresta::Element* first = root->findChildElement("item");                 // among the children
std::vector<const beresta::Element*> direct = root->findChildElements("item");
std::vector<const beresta::Element*> deep = doc.getElementsByTagName("item");    // the whole subtree, "*" — any
const beresta::Element* byId = doc.getElementById("b7");                         // an ID from the DTD or xml:id
```

Use [XPath](#xpath-10) for complex queries.

### Comparison and text

```cpp
bool same = a.isEqualNode(b);               // structural equality; the attribute order does not matter
std::string text = element.getTextContent(); // the text of the descendants (Text and CDATA, without comments)
```

---

## Copying and moving nodes

```cpp
beresta::Element copy = *book;                                  // a deep copy on the stack
auto heap = std::make_unique<beresta::Element>(*book);          // in the heap
std::unique_ptr<beresta::Node> any = node->clone();             // polymorphic, the type is kept
std::unique_ptr<beresta::Element> shell = book->cloneShallow(); // the name and attributes, without children

*target = source;     // the content is replaced with a deep copy, the place of target in the tree does not change
```

A copy is **standalone** (`getParent() == nullptr`) and shares no node with the original. Moving without copying:

```cpp
targetParent->appendChild(sourceParent->removeChild(index));
```

A node taken out of a document outlives the document itself.

From other languages the same is done by `beresta_node_move_to` and the wrapper methods next to copying:

| Language | Copy | Move without copying |
|---|---|---|
| C | `beresta_node_copy_to(node, parent, &copy)` | `beresta_node_move_to(node, parent)` |
| Java | `node.copyTo(parent)` | `node.moveTo(parent)` |
| C# | `node.CopyTo(parent)` | `node.MoveTo(parent)` |
| Python | `node.copy_to(parent)` | `node.move_to(parent)` |
| Node.js, WebAssembly | `node.copyTo(parent)` | `node.moveTo(parent)` |

```python
source = beresta.parse("<a><item><name>Мёд</name></item></a>")
target = beresta.parse("<b/>")
item = source.root.first_child
name = item.first_child          # a descendant wrapper obtained before the move
item.move_to(target.root)        # the node (with its descendants) now belongs to target
source.close()                   # closing the former document does not touch the node
print(name.text, item.document is target)   # Мёд True
```

A move puts the node at the end of the children of `parent` — an element or a document (in a document without a root the element becomes the root). The handle of the node and the handles of its descendants do not change: the object is not copied, the new document just starts owning it. In the wrappers every node has one wrapper (the document registry), so the owner document changes for all already issued wrappers of the subtree: after a move their lifetime is defined by the new document, not the former one. You cannot move the document itself (`INVALID_ARGUMENT`), the root element of a document or a node into itself or its descendant (`HIERARCHY_VIOLATION`). If the new parent does not accept the node (for example, text at the top level of a document), `HIERARCHY_VIOLATION` is returned, and the tree stays as it was.

---

## Parsing: DOM

```cpp
beresta::XmlParser parser(options);
beresta::Document a = parser.parse(text);                   // from memory
beresta::Document b = parser.parseFile("data.xml");         // from a file (baseUri = the path)
beresta::Document c = parser.parse(stream);                 // std::istream
std::unique_ptr<beresta::Element> e = parser.parseElement("<x a='1'/>");   // a standalone fragment
```

The input is **bytes** in any supported encoding: it is detected by the BOM and the XML declaration.

### `ParserOptions`

| Field | Default | Purpose |
|---|---|---|
| `preserveWhitespaceText` | `false` | keep text consisting only of whitespace (indentation) |
| `discardComments` | `false` | do not add comments to the DOM |
| `coalesceCData` | `false` | turn CDATA into plain text |
| `trackLocations` | `false` | record the line and column of every node |
| `namespaceAware` | `true` | check the Namespaces in XML rules |
| `validateDtd` | `false` | validate the document against the DTD |
| `loadExternalDtd` | `false` | load the external DTD (needs a `resolver`) |
| `applyDtdDefaults` | `true` | add default attributes from `<!ATTLIST>` |
| `processXInclude` | `false` | process `xi:include` (needs a `resolver`) |
| `maxDepth` | `1024` | the maximum element nesting |
| `maxEntityExpansions` | `10000` | the maximum number of entity expansions |
| `maxEntityDepth` | `16` | the nesting of entities (and XInclude) |
| `maxExpandedSize` | 10 MB | the total size of text from entities |
| `forcedEncoding` | `""` | a forced input encoding |
| `baseUri` | `""` | the base for relative references |
| `resolver` | `nullptr` | the loader of external resources |
| `errorHandler` | `nullptr` | the receiver of errors and warnings |
| `schema` | `nullptr` | an XML Schema to validate against during parsing (DOM and SAX) |

`resolver` and `errorHandler` are **non-owning** pointers: the objects must live while parsing goes on.

### Whitespace

Text **consisting only of whitespace** between tags is dropped by default: it is formatting, and thanks to this "pretty" and "solid" XML give the same tree. Text with at least one non-whitespace character or a character reference is kept verbatim. For mixed markup (`<b>a</b> <i>b</i>`) enable `preserveWhitespaceText`.

### Node positions

```cpp
beresta::ParserOptions options;
options.trackLocations = true;
beresta::Document doc = beresta::XmlParser(options).parse(text);
std::size_t line = doc.getRoot()->getSourceLocation().line;
```

A node from an expanded entity gets the position of the reference to that entity in the source file.

---

## Parsing: SAX

Streaming parsing: no tree is built, memory use does not depend on the document size.

```cpp
class PriceCounter : public beresta::ContentHandler
{
public:
    void startElement(const beresta::SaxElement& e) override { inside = e.localName == "price"; }
    void characters(std::string_view text) override { if (inside) total += beresta::xpath::stringToNumber(text); }
    void endElement(const beresta::SaxElement&) override { inside = false; }

    bool inside = false;
    double total = 0;
};

PriceCounter counter;
beresta::SaxParser(options).parseFile("big.xml", counter);
```

| Event | When |
|---|---|
| `setDocumentLocator(const Locator&)` | before the start; `locator.getLocation()` gives the position of the current event |
| `startDocument()` / `endDocument()` | the start and the end |
| `xmlDeclaration(const XmlDeclaration&)` | the declaration |
| `documentType(const DocumentType&)` | DOCTYPE together with the DTD model |
| `startElement(const SaxElement&)` / `endElement(...)` | tags: `name`, `namespaceUri`, `localName`, `attributes` |
| `characters(text)` / `cdata(text)` | text (entities already expanded) / CDATA |
| `ignorableWhitespace(text)` | whitespace between tags |
| `comment(text)` / `processingInstruction(target, data)` | comments and PIs |
| `dtdProcessingInstruction(target, data)` | PIs from the internal and external DTD, before `documentType`; by default it calls `processingInstruction` (section 2.6: PIs are passed to the application). `DomBuilder` skips them |

All `string_view`s in events are valid **only during the call**. `DomBuilder` is a public handler that builds the DOM; it can be inherited.

With `options.schema` the document is validated against an XML Schema along the parse by the streaming validator, and your handler gets the same events; no tree is built (see [XML Schema](#xml-schema)).

---

## Namespaces

While parsing (`namespaceAware = true`) the following is checked: prefixes are declared, `xml`/`xmlns` are not redefined, names are correct, there are no two attributes with one expanded name. A violation is the fatal error `NAMESPACE_ERROR`.

In the DOM URIs are computed **"live"** from the `xmlns` declarations in the ancestors. That is why they always match what will be written to a file, even after an element is moved.

```cpp
const beresta::Element* body = envelope->findChildElementNS("http://schemas.xmlsoap.org/soap/envelope/", "Body");
body->getPrefix();                  // "soap"
body->getLocalName();               // "Body"
body->getNamespaceURI();            // optional<string>
body->lookupNamespaceURI("soap");
body->lookupPrefix("urn:x");
doc.getElementsByTagNameNS("urn:x", "*");

element.declareNamespace("x", "urn:x");             // xmlns:x="urn:x"
element.setAttributeNS("urn:x", "x:flag", "1");     // declares the prefix itself if needed
element.getAttributeNS("urn:x", "flag");
element.removeAttributeNS("urn:x", "flag");
```

---

## Encodings, files and streams

Detecting the input encoding:
1. The BOM (UTF-8, UTF-16LE, UTF-16BE).
2. The `<?` signature in UTF-16 without a BOM.
3. The `encoding` attribute in the declaration (`<?xml` followed by whitespace; `<?xml-stylesheet ...?>` is a plain PI).
4. Otherwise UTF-8.

If the BOM or the UTF-16 signature determined the encoding and the declaration names another one (a UTF-8 BOM and `encoding="iso-8859-1"`), it is the `ENCODING_ERROR` error (section 4.3.3). Line ends (`\r\n`, `\r`) are normalized once — when the document, an external DTD or an entity is decoded; a `\r` character from the `&#13;` reference is kept.

`ParserOptions::forcedEncoding` overrides the detection (and the agreement check with the declaration). Bytes invalid in the encoding and characters forbidden in XML (control codes and the like) give the `ENCODING_ERROR` error with a position.

| Encoding | Names |
|---|---|
| UTF-8 | `UTF-8`, `utf8` |
| UTF-16 | `UTF-16` (with a BOM), `UTF-16LE`, `UTF-16BE` |
| Latin | `ISO-8859-1` (`latin1`), `US-ASCII`, `windows-1252` (`cp1252`) |
| Cyrillic | `windows-1251` (`cp1251`), `KOI8-R`, `IBM866` (`cp866`) |

```cpp
const beresta::encoding::Charset* cp1251 = beresta::encoding::findCharset("cp1251");

std::string bytes = beresta::io::readFile("in.xml");    // binary reading, the path is std::filesystem::path
beresta::io::writeFile("out.xml", bytes);
beresta::io::readStream(std::cin);
```

---

## Serialization

```cpp
std::string utf8 = node.toString();     // compact, always UTF-8
node.serialize(buffer);                 // append to an existing buffer

beresta::SerializerOptions options;
options.prettyPrint = true;              // indentation
options.indent = "  ";
options.newline = "\n";
options.encoding = "windows-1251";       // the output encoding
options.writeDeclaration = true;         // write the declaration (if there was one or the encoding is not UTF-8)
options.forceDeclaration = false;        // write it even for a document without a declaration

beresta::XmlSerializer serializer(options);
std::string bytes = serializer.toString(doc);
serializer.write(doc, stream);
serializer.writeFile(doc, "out.xml");
```

Rules:
- The compact output does not contain a single extra space, and the attribute order is kept.
- With `prettyPrint` every child of an element that contains only elements goes on a new line. Elements with text are written verbatim, so re-parsing gives the same tree.
- `encoding` in the declaration always matches the actual output encoding. A BOM is written for `UTF-16`. Characters outside the encoding become `&#N;`.
- DOCTYPE is written in the minimal form: `<!DOCTYPE r SYSTEM "r.dtd"[...]>`.

---

## DTD and entities

```cpp
const char* xml =
    "<!DOCTYPE r [<!ENTITY who \"world\"><!ENTITY bold \"<b>&who;</b>\">"
    "<!ATTLIST r lang CDATA \"en\">]><r>Hello, &bold;</r>";
beresta::Document doc = beresta::XmlParser().parse(xml);
doc.getRoot()->getAttribute("lang");      // "en" — the default value from ATTLIST
doc.toString();                            // DOCTYPE is kept, entities are expanded
```

- **Entities** — internal (with markup inside), external (through a `resolver`), parameter entities `%pe;`, including inside declarations in the external subset. The conditional sections `INCLUDE`/`IGNORE` work.
- **Default attributes** and the normalization of non-CDATA attribute values are done as in Xerces.
- **The DTD model** is available through `doc.getDocumentType()->getModel()`: the declarations of entities, elements, attributes and notations.
- **An external PE in the internal subset** is read by the rules of the external subset (`%pe;` references inside declarations, conditional sections); a relative `SYSTEM` of an entity is resolved from the text where it is declared.
- **An undeclared entity.** In a document without an external DTD and without `%pe;` references (or with `standalone="yes"`) it is a fatal error; otherwise it is a validity error (if validation is on), and the reference is skipped (section 4.1). With `standalone="yes"` referring to an entity declared in the external DTD is not allowed.
- **Validity checking** (`validateDtd = true`) checks:
  - the root and that elements and attributes are declared;
  - `#REQUIRED` and `#FIXED`;
  - the types `ID`, `IDREF(S)`, `NMTOKEN(S)`, `ENTITY(IES)`, enumerations, ID uniqueness and IDREF targets; with namespaces ID/IDREF/ENTITY values contain no `:`;
  - the models `EMPTY` (not even a comment, PI or a reference to an empty entity inside), `ANY`, `MIXED` and `CHILDREN`;
  - the declarations themselves: a repeated element declaration, a repeated name in mixed content, one ID and one NOTATION attribute per element, the default of an ID attribute, declared notations, repetitions in enumerations, the lexical validity of default values;
  - the nesting of group brackets, declarations and conditional sections in parameter entities;
  - `standalone="yes"`: the result must not depend on the external DTD (defaults, normalization, whitespace in element content).

```cpp
beresta::CollectingErrorHandler handler;
beresta::ParserOptions options;
options.validateDtd = true;
options.errorHandler = &handler;
beresta::XmlParser(options).parse(text);
for (const beresta::Diagnostic& d : handler.getDiagnostics())
    std::cout << d.message << '\n';       // all errors in one pass
```

Without an `errorHandler` the first validity error throws a `ParseException` with the `VALIDATION_ERROR` code.

---

## XML Schema

```cpp
// Loading a schema (once; a schema is immutable and shared between threads)
std::shared_ptr<const beresta::xsd::Schema> schema = beresta::xsd::Schema::fromFile("catalog.xsd", loadOptions);

// Option 1: a separate validation of a ready DOM
beresta::xsd::SchemaValidator validator(schema);
std::vector<beresta::Diagnostic> errors = validator.validate(doc);
bool ok = validator.isValid(doc);

// Option 2: validation right while parsing into a DOM — one pass, without a second tree walk
beresta::ParserOptions options;
options.schema = schema;
options.errorHandler = &handler;          // without it — an exception on the first error
beresta::XmlParser(options).parseFile("catalog.xml");

// Option 3: streaming validation without a tree — for multi-gigabyte exports
MyHandler events;                         // any ContentHandler (or an empty one)
beresta::SaxParser(options).parseFile("export.xml", events);
```

Every error contains the path to the element and a position: `line 12, column 5: /catalog/book[2]/price: the value "-5" is less than the minimum 0`. Error positions are always there: the validator takes them from the parser, and they are recorded into the tree nodes only if `trackLocations` is on.

**Validation during parsing.** The validator is a decorator over the SAX event stream (`xsd::ValidatingHandler`): the events pass through it and go further — into `DomBuilder`, into your handler or nowhere. The content model is matched step by step, one child at a time (a greedy pass for sequences of different elements, an automaton over particle configurations for the other models), simple values — when the element is closed, by the accumulated text. All three options are the same validation logic: `SchemaValidator::validate` turns a ready tree into an event stream by a walk without recursion. So the errors, their order, paths and positions are the same in all three options.

- **DOM with a schema** (`XmlParser`): the document is validated in the same pass that builds it; the `xs:key`/`xs:unique`/`xs:keyref` keys — over the ready tree, with the same XPath engine. With `processXInclude` the document is validated after the inclusions are substituted, over the tree.
- **SAX with a schema** (`SaxParser`): no tree is built, the validation memory is O(document depth) plus the ID values and the key tables. Keys are evaluated in a stream (see below). The errors arrive in `errorHandler` after `endDocument`: the numbers in paths like `item[5]` are known only when all the siblings have been read.
- **Your own pipeline:** `ValidatingHandler` can be put before any handler by hand.

```cpp
beresta::DomBuilder builder(options);
beresta::xsd::ValidatingHandler validator(schema, &builder,
    beresta::xsd::validationSettingsFor(options, beresta::xsd::IdentityEvaluation::ON_TREE));
beresta::SaxParser(parserOptionsWithoutSchema).parse(text, validator);
beresta::Document doc = builder.takeDocument();
validator.checkIdentity(doc);                                // the keys — over the built tree
std::vector<beresta::Diagnostic> errors = validator.takeDiagnostics();
```

| Loading | Description |
|---|---|
| `Schema::fromString(text, options)` | from XSD text |
| `Schema::fromFile(path, options)` | from a file (`baseUri` = the path) |
| `Schema::fromDocuments({&a, &b}, options)` | several schemas of different namespaces |
| `SchemaLoadOptions::resolver` | the loader for `xs:include` / `xs:import` |

**Supported:**
- `targetNamespace`, `elementFormDefault`/`attributeFormDefault`, `form`;
- global and local elements and attributes, `ref`, anonymous types;
- `complexType` with `sequence`/`choice`/`all`, nested groups, `minOccurs`/`maxOccurs`, `mixed`;
- `complexContent` `extension`/`restriction`, `simpleContent` `extension`/`restriction`;
- `xs:group`, `xs:attributeGroup`, `xs:any` (`##any`, `##other`, `##local`, `##targetNamespace`, a list; `strict`/`lax`/`skip`), `xs:anyAttribute`;
- attributes: `use="required|optional|prohibited"`, `default`, `fixed`;
- elements: `default`, `fixed`, `nillable` + `xsi:nil`, `abstract`, `xsi:type`;
- `simpleType` `restriction`/`list`/`union`;
- the facets `length`, `minLength`, `maxLength`, `pattern`, `enumeration`, `whiteSpace`, `minInclusive`, `maxInclusive`, `minExclusive`, `maxExclusive`, `totalDigits`, `fractionDigits`;
- all **45 built-in types** of XSD 1.0 (strings, integers with exact bounds, `decimal`, `float`/`double`, dates and times with time zones, `duration`, `hexBinary`, `base64Binary`, `QName`, ID/IDREF(S), NMTOKEN(S), ENTITY(IES) and so on);
- `ID` uniqueness and the existence of `IDREF` targets;
- the identity constraints `xs:unique`, `xs:key`, `xs:keyref` (details below);
- `xs:include` (including "chameleon" schemas) and `xs:import`.

### Identity constraints: xs:key, xs:unique, xs:keyref

Relational checks inside a document are done by the schema itself, without application code:

```xml
<xs:element name="shop">
    <xs:complexType>...</xs:complexType>
    <!-- the product SKU is required and unique within the shop -->
    <xs:key name="productKey">
        <xs:selector xpath="s:product"/>
        <xs:field xpath="s:sku"/>
    </xs:key>
    <!-- the barcode is optional, but unique if present -->
    <xs:unique name="eanUnique">
        <xs:selector xpath="s:product"/>
        <xs:field xpath="@ean"/>
    </xs:unique>
    <!-- every order refers to an existing product -->
    <xs:keyref name="orderRef" refer="s:productKey">
        <xs:selector xpath="s:order"/>
        <xs:field xpath="@sku"/>
    </xs:keyref>
</xs:element>
```

```
/shop/product[3]: the value (1) is repeated: the constraint xs:key "productKey" is violated
/shop/order[2]: keyref "orderRef": the value (7) is not among the values of the key "productKey"
```

- **Comparison by the type value**, not by the string: for `xs:decimal` the values `1`, `1.0` and `01` are one key, for `xs:boolean` `1` and `true` match, dates are compared taking the time zone into account. The field type comes from the document validation: the attribute type or the simple type of the element. The canonical form of a value is also available directly: `beresta::xsd::canonicalValue(type, value)`.
- **Composite keys** of several `xs:field`s, attribute fields and element fields, selectors at any depth (`.//item`) and unions (`a | b`).
- **The specification rules:** for `xs:key` every field must be present, for `xs:unique` rows without values are skipped, a field cannot select more than one node or an element with complex content.
- **Scopes:** a key table is built for every element with a declared constraint, and `xs:keyref` finds a key declared on the same element or on its descendants.
- **Schema errors are caught at load time:** an undeclared `refer`, a `keyref` referring to another `keyref`, different numbers of fields in a `keyref` and its key, a repeated constraint name, a syntax error in `xpath`.
- Prefixes in `xpath` are resolved by the `xmlns` declarations of the schema document. A name without a prefix is, as XSD 1.0 requires, outside any namespace.
- Selectors and fields are compiled once at schema load time by the same XPath 1.0 engine. It also accepts expressions beyond the XSD grammar, which real schemas follow anyway.
- **In a stream (SAX without a tree)** keys are checked as in Xerces: selectors and fields from the XSD grammar (the `child` and `attribute` axes, a leading `.//`, `|`, `*`, `p:*`) are matched against the stack of names of the open elements, a key row is collected by the closing of the target element, tables and references — by the closing of the scope. The messages are the same as with the check over a tree. A constraint with an expression outside this grammar (for example, `a//b` or with a predicate) cannot be checked without a tree — one message per constraint reports it.

Simple types are available without a schema too:

```cpp
const beresta::xsd::SimpleType* type = beresta::xsd::builtinType("dateTime");
std::optional<std::string> error = beresta::xsd::validateSimpleValue(*type, "2026-10-17T12:00:00+03:00", nullptr);
bool equal = beresta::xsd::valuesEqual(*beresta::xsd::builtinType("decimal"), "1.0", "001");   // true
```

---

## XPath 1.0

```cpp
// Quick wrappers
std::vector<beresta::Element*> books = beresta::xpath::selectElements(doc, "//book[price > 300]");
std::string title = beresta::xpath::evaluateString(doc, "string(//book[1]/title)");

// A compiled expression: many times and from different threads
beresta::xpath::XPathExpression expression("//s:item[@code = $code]/s:name");
beresta::xpath::XPathContext context;
context.namespaces.emplace("s", "urn:shop");
context.variables.emplace("code", beresta::xpath::XPathValue::fromString("B2"));

beresta::xpath::XPathValue value = expression.evaluate(doc, context);
value.getType();          // NODE_SET, BOOLEAN, NUMBER, STRING
value.getNodes();         // XPathItem{node, attribute, owner, isNamespace} in document order
value.toString(); value.toNumber(); value.toBoolean();
expression.selectElements(doc, context);
```

- **Axes (all 13):** `child`, `descendant`, `descendant-or-self`, `parent`, `ancestor`, `ancestor-or-self`, `following-sibling`, `preceding-sibling`, `following`, `preceding`, `attribute`, `namespace`, `self`, plus the abbreviations `//`, `.`, `..`, `@`.
- **The `namespace` axis** returns a node for every prefix in effect on the element: the declarations of the element itself and its ancestors (the nearest hides the farther ones), the `xmlns=""` undeclaration and the implicit `xml` prefix. The node name is the prefix, the string value is the URI, `namespace-uri()` is empty, the parent is the element. In document order namespace nodes go after the element and before its attributes. In `XPathItem` such a node has `isNamespace == true`, `attribute` points to the `xmlns:p` declaration, `owner` to the element. `xmlns` declarations, as XPath requires, are not considered attributes.
- **Node tests:** names, `*`, `p:*`, `node()`, `text()`, `comment()`, `processing-instruction('t')`.
- **Expressions:** predicates (a number = a position), `|`, `or`, `and`, `=`, `!=`, `<`, `<=`, `>`, `>=`, `+`, `-`, `*`, `div`, `mod`, the unary minus. Set comparisons follow the specification rules.
- **Functions (all 27):** `last`, `position`, `count`, `id`, `local-name`, `namespace-uri`, `name`, `string`, `concat`, `starts-with`, `contains`, `substring-before`, `substring-after`, `substring`, `string-length`, `normalize-space`, `translate`, `boolean`, `not`, `true`, `false`, `lang`, `number`, `sum`, `floor`, `ceiling`, `round`.
- String functions count **Unicode characters**, not bytes. Numbers are printed by the XPath rules: `0.1 + 0.2` = `0.30000000000000004`, `1 div 0` = `Infinity`.
- As in XPath 1.0, a name **without a prefix** means "outside any namespace". For a document with `xmlns="..."` bind a prefix in `XPathContext`.

---

## Tree traversal: TreeWalker and NodeIterator

```cpp
class SkipDrafts : public beresta::NodeFilter
{
public:
    beresta::FilterResult acceptNode(const beresta::Node& node) const override
    {
        const auto& element = static_cast<const beresta::Element&>(node);
        if (element.getAttribute("draft") == "yes")
            return beresta::FilterResult::REJECT;     // hide together with the subtree
        return beresta::FilterResult::ACCEPT;          // SKIP — hide the node but show its children
    }
};

SkipDrafts filter;
beresta::TreeWalker walker(*doc.getRoot(), beresta::show::ELEMENTS, &filter);
for (beresta::Node* n = walker.nextNode(); n != nullptr; n = walker.nextNode()) { /* ... */ }
walker.parentNode(); walker.firstChild(); walker.lastChild();
walker.nextSibling(); walker.previousSibling(); walker.previousNode();

beresta::NodeIterator texts(doc, beresta::show::TEXTS);
for (beresta::Node* t = texts.nextNode(); t != nullptr; t = texts.nextNode()) { /* ... */ }
```

The `show::` masks: `ALL_NODES`, `DOCUMENTS`, `DOCUMENT_TYPES`, `ELEMENTS`, `TEXTS`, `CDATA_SECTIONS`, `COMMENTS`, `PROCESSING_INSTRUCTIONS` (combined with `|`). The algorithms follow DOM Level 2 Traversal, including the difference between `SKIP` and `REJECT`.

---

## XInclude

```xml
<book xmlns:xi="http://www.w3.org/2001/XInclude">
  <xi:include href="chapter1.xml"/>
  <xi:include href="licence.txt" parse="text" encoding="windows-1251"/>
  <xi:include href="parts.xml" xpointer="element(/1/2)"/>
  <xi:include href="maybe.xml"><xi:fallback><p>The section is unavailable</p></xi:fallback></xi:include>
</book>
```

```cpp
beresta::FileResourceResolver resolver("docs/");
beresta::ParserOptions options;
options.resolver = &resolver;
options.processXInclude = true;
beresta::Document book = beresta::XmlParser(options).parseFile("docs/book.xml");
// or manually for a ready tree:
std::size_t replaced = beresta::xinclude::processXIncludes(doc, options);
```

Supported: `parse="xml"` and `parse="text"` (with `encoding`), nested inclusions (cycles are detected), `xi:fallback`, XPointer `element(/1/2)`, `element(id/1)` and the short form by ID.

---

## External resources and security

By default **nothing external is loaded**: neither the external DTD, nor external entities, nor XInclude, nor `xs:include`. This closes XXE and SSRF. To allow loading, pass a resolver:

```cpp
beresta::FileResourceResolver resolver("C:/data/xml");   // only local files inside the root
options.resolver = &resolver;
options.loadExternalDtd = true;
```

`FileResourceResolver` resolves relative paths from `baseUri`, rejects network URIs (`http:`, `https:`, `ftp:` and the like) and paths that leave the root through `..`.

Your own loader (a cache, an archive, a database) is a `ResourceResolver` subclass with the `resolve(const ResourceRequest&)` method. The request contains `kind` (`EXTERNAL_DTD`, `EXTERNAL_ENTITY`, `XINCLUDE`, `SCHEMA`), `publicId`, `systemId` and `baseUri`.

The limits `maxEntityExpansions`, `maxEntityDepth`, `maxExpandedSize` and `maxDepth` protect from "entity bombs" and excessively deep documents. Exceeding a limit is the `LIMIT_EXCEEDED` error.

Protection from algorithmic complexity attacks is built in: hash tables keyed by document data (ID values, names) use SipHash-1-3 with a random per-process key (`utils::SeededHash`), and repeated attributes and expanded names are found in O(1) even in a tag with hundreds of thousands of attributes. The batch attribute setter `Element::setAttributes` is linear too. The iteration order of such tables changes from run to run, so they are used only for lookup and never for output: the result of parsing and serialization is deterministic.

---

## Error handling

```
std::exception
└── beresta::DomException          a code + a message
    └── beresta::ParseException    + a position (offset, line, column)
```

| `DomErrorCode` | When |
|---|---|
| `PARSE_ERROR` | XML syntax |
| `NAMESPACE_ERROR` | the namespace rules |
| `ENCODING_ERROR` | an unknown encoding, invalid bytes or characters |
| `VALIDATION_ERROR` | DTD or XML Schema |
| `RESOURCE_ERROR` | an external resource was not loaded |
| `LIMIT_EXCEEDED` | a protective limit is exceeded |
| `SCHEMA_ERROR` | an error in the schema itself or in a `pattern` |
| `XPATH_ERROR` | XPath syntax or evaluation |
| `INVALID_NAME`, `INVALID_CONTENT` | an invalid name or node content |
| `HIERARCHY_VIOLATION`, `INDEX_OUT_OF_RANGE`, `NULL_NODE`, `NOT_FOUND` | tree operations |
| `IO_ERROR` | files and streams |

```cpp
try { /* ... */ }
catch (const beresta::ParseException& e) { e.getLine(); e.getColumn(); e.getCode(); }
catch (const beresta::DomException& e)   { e.getCode(); e.getMessage(); beresta::errorCodeToString(e.getCode()); }
```

### ErrorHandler

| Severity | Example | Behavior |
|---|---|---|
| `WARNING` | — | ignored by default |
| `RECOVERABLE_ERROR` | DTD and XSD errors | if `error()` does not throw, parsing continues |
| `FATAL_ERROR` | syntax, encoding | after `fatalError()` the parser always throws |

`CollectingErrorHandler` collects everything into `getDiagnostics()`, there are also `hasErrors()` and `clear()`.

---

## Message languages

Error texts are not scattered over the code as string literals: every message is a `beresta::i18n::Message` constant (`include/i18n/message.hpp`, 302 messages), and the texts live in the language catalogs `src/i18n/catalogs/`. Two languages are included: Russian (`ru`) and English (`en`).

```
[PARSE_ERROR] строка 1, столбец 7: ожидался </b>, найден </a>     ← ru
[PARSE_ERROR] line 1, column 7: expected </b>, found </a>          ← en
```

The error code name (`PARSE_ERROR`, `VALIDATION_ERROR`...) is the same in all languages, so programmatic error handling does not depend on the language.

**The default language** is chosen at build time. An unknown code stops the build with a clear error:

```bash
make ERRORS_LANG=en
```
```bash
cmake -S . -B build -DBERESTA_ERRORS_LANG=en
```

**At run time** the language is changed with one call — at once for all threads. An unknown code is rejected, and the language stays as it was:

| Language | Change | Current | All languages |
|---|---|---|---|
| C++ | `beresta::i18n::setLanguage("en")` (→ `bool`) | `beresta::i18n::language()` | `beresta::i18n::availableLanguages()` |
| C | `beresta_set_language("en")` (→ `BERESTA_INVALID_ARGUMENT` if there is no such language) | `beresta_language()` | `beresta_languages()` → `"ru,en"` |
| Java | `Languages.use("en")` | `Languages.current()` | `Languages.available()` |
| C# | `BerestaLibrary.Language = "en"` | `BerestaLibrary.Language` | `BerestaLibrary.Languages` |
| Python | `beresta.set_language("en")` | `beresta.language()` | `beresta.languages()` |
| Node.js, WebAssembly | `beresta.setLanguage('en')` | `beresta.language()` | `beresta.languages()` |

The wrappers' own messages ("the document is already closed" and the like) are also taken from the common catalog through `beresta_message_text`: a language added to the library is picked up by the wrappers without changes.

Message templates contain the markers `{0}`, `{1}`... — their order in a translation may be any; `{{0}}` prints the argument in curly braces (names like `{urn:shop}item` are written this way). If a language catalog lacks some text, English is used.

```cpp
std::string text = beresta::i18n::format(beresta::i18n::Message::LIMIT_DEPTH, {"1024"});
```

### How to add a language

1. Copy `src/i18n/catalogs/messages_en.cpp` to `messages_<code>.cpp` (for example, `messages_de.cpp`) and translate the texts keeping the `{N}` markers.
2. At the end of the file rename the catalog: `const Catalog GERMAN_CATALOG{"de", "Deutsch", TEXTS, ...};`.
3. Declare it in `include/i18n/i18n.hpp` (`extern const Catalog GERMAN_CATALOG;`) and add the `&GERMAN_CATALOG` line to the `catalogs()` registry in `src/i18n/i18n_registry.cpp`.

Nothing else needs to change: the Makefile and CMake find the catalogs themselves, so `make ERRORS_LANG=de` and `-DBERESTA_ERRORS_LANG=de` work right away, and so do `Languages.use("de")` and its analogs in all wrappers.

---

## Guarantees and thread safety

- **The strong exception guarantee** for `appendChild`, `insertChild`, `replaceChild`, `setRoot`, the `set*` methods, `setAttributes` (names are checked before any change) and the copy assignment: on an error nothing changes.
- **`XmlParser`, `SaxParser`, `XmlSerializer`, `XPathExpression`, `Schema`, `SchemaValidator`** are immutable: one object can be used from many threads at once (if your `resolver` and `errorHandler` are thread-safe too).
- **Different trees** can be processed in parallel without synchronization. **One tree** can be read from many threads through the `const` methods. If at least one thread changes it, external synchronization is needed.

---

## Other languages: the C interface

The `include/capi/beresta_c.h` header is written in C99 and can be included in both `.c` and `.cpp`. All the wrappers are built on it, and it can be used directly from C, Go (cgo), Rust (FFI), Delphi and any language that can call C functions.

```c
#include "capi/beresta_c.h"

const char* xml = "<shop><item code=\"A1\">Tea</item><item code=\"B2\">Coffee</item></shop>";
beresta_document document = NULL;
beresta_xpath xpath = NULL;

if (beresta_document_parse(xml, (int64_t)strlen(xml), BERESTA_PARSE_DEFAULT, NULL, &document) != BERESTA_OK)
{
    fprintf(stderr, "[%s] %s\n", beresta_status_name(beresta_last_error_code()), beresta_last_error_message());
    return 1;
}
beresta_xpath_compile("//item[@code = $code]", &xpath);
beresta_xpath_set_string(xpath, "code", "B2");

char* name = beresta_xpath_string(xpath, document);   /* "Coffee" */
printf("%s\n", name);
beresta_string_free(name);                             /* the library frees the string */

beresta_xpath_free(xpath);
beresta_document_free(document);                       /* the document together with all nodes */
```

A complete example is `examples/capi/capi_demo.c` (built together with the demos). Linking into your program:

```bash
gcc -std=c99 -Ipath/to/beresta/include app.c -Lpath/to/beresta/bin -lberesta -o app
```

### Conventions (the same for all 69 functions)

| Rule | Explanation |
|---|---|
| Handles | `beresta_document`, `beresta_node`, `beresta_xpath`, `beresta_nodeset`, `beresta_schema`, `beresta_diagnostics` are `void*` |
| Status | functions returning `int` give `BERESTA_OK` (0) on success or an error code |
| NULL | functions returning a handle or `char*` give NULL on an error. NULL with code 0 means "no object" (for example, there is no next sibling) |
| The last error | `beresta_last_error_code()` and `beresta_last_error_message()` — per thread, like `errno`. Every call resets it |
| Strings | always UTF-8. A `char*` from the library is freed through `beresta_string_free`, a `const char*` is borrowed |
| Sizes and indices | `int64_t` — the same type in all languages; `-1` in a result means an error |
| Ownership | the caller frees a document, a schema, an expression, a node set and a diagnostics list (`beresta_*_free`). A node belongs to its document; `beresta_node_move_to` hands it (with its descendants) over to another document, and the handles do not change |
| Threads | different documents in different threads in parallel; a schema is thread-safe after loading |

| Group | Functions |
|---|---|
| Library | `version`, `last_error_code`, `last_error_message`, `status_name`, `string_free` |
| Message languages | `set_language`, `language`, `languages`, `message_text` |
| Documents | `document_create`, `document_parse` (bytes in any encoding), `document_parse_file`, `document_clone`, `document_root`, `document_element_by_id`, `document_save` (any output encoding), `document_free` |
| Nodes | `node_type`, `node_name`, `node_text`, `node_to_string`, `node_parent`, `node_first_child`, `node_last_child`, `node_next_sibling`, `node_previous_sibling`, `node_owner_document`, `node_child_count`, `node_child` |
| Tree changes | `node_set_text`, `node_remove`, `node_copy_to` (including between documents), `node_move_to` (a move without copying, including between documents), `node_append_element`, `node_append_text`, `node_append_comment`, `node_append_cdata`, `node_append_pi` |
| Attributes | `element_get_attribute`, `element_set_attribute`, `element_has_attribute`, `element_remove_attribute`, `element_attribute_count`, `element_attribute_name`, `element_attribute_value`, `element_local_name`, `element_namespace_uri` |
| XPath | `xpath_compile`, `xpath_bind_namespace`, `xpath_set_string`, `xpath_set_number`, `xpath_select`, `xpath_string`, `xpath_number`, `xpath_boolean`, `nodeset_size`, `nodeset_node`, `nodeset_value`, `xpath_free`, `nodeset_free` |
| XML Schema | `schema_load`, `schema_load_file`, `schema_validate`, `diagnostics_count`, `diagnostics_message`, `diagnostics_line`, `diagnostics_column`, `schema_free`, `diagnostics_free` |

The parse flags are the `BERESTA_PARSE_*` bit mask: `PRESERVE_WHITESPACE`, `DISCARD_COMMENTS`, `COALESCE_CDATA`, `VALIDATE_DTD`, `NO_NAMESPACES`, `XINCLUDE`, `FILE_RESOURCES`. The last one allows external DTDs, entities and XInclude, but only from the directory of the parsed file. The output flags are `BERESTA_WRITE_PRETTY` and `BERESTA_WRITE_DECLARATION`. A mask was chosen over a settings structure on purpose: an integer is passed the same way from any language.

Codes 1–15 equal `DomErrorCode` + 1 (`BERESTA_PARSE_ERROR`, `BERESTA_VALIDATION_ERROR`...). Three more codes exist only in the C interface: `BERESTA_INVALID_ARGUMENT` (NULL or a handle of the wrong type), `BERESTA_OUT_OF_MEMORY`, `BERESTA_UNKNOWN_ERROR`.

---

## Java (Project Panama)

The wrapper is the `bindings/java` Maven project (the `org.beresta` package, Java 22+). It uses the Foreign Function & Memory API: there is no JNI, no `jni.h` headers and no glue code in C. There are no dependencies either.

```java
import org.beresta.*;

try (XmlDocument document = Xml.parse("<shop><item code='A1'>Tea</item></shop>"))
{
    XmlElement item = (XmlElement) document.root().firstChild();
    System.out.println(item.getAttribute("code") + " " + item.text());       // A1 Tea

    try (XPath query = new XPath("//item[@code = $c]"))
    {
        System.out.println(query.set("c", "A1").string(document));            // Tea
    }
}

try (XmlDocument order = XmlDocument.create("order"))
{
    XmlElement item = order.root().appendElement("item");
    item.setAttribute("qty", "2");
    item.appendText("Honey & nuts");
    System.out.println(order.toXml());   // <order><item qty="2">Honey &amp; nuts</item></order>
    order.save(Path.of("order.xml"), WriteFlags.PRETTY, "windows-1251");
}

try (XmlSchema schema = XmlSchema.fromFile(Path.of("order.xsd"));
     XmlDocument document = Xml.parseFile(Path.of("order.xml"), ParseFlags.DEFAULT, null))
{
    for (Diagnostic problem : schema.validate(document))
        System.out.println(problem.line() + ": " + problem.message());
}
catch (BerestaException error)
{
    System.out.println(error.codeName() + " " + error.detail());           // PARSE_ERROR line 1, ...
}
```

Building:

```bash
make JAVA=1
```
```bash
mvn -f bindings/java/pom.xml package
```

Running an application:

```bash
java --enable-native-access=ALL-UNNAMED -cp beresta-2.1.0.jar:app.jar com.example.App
```

How it works:
- `NativeLoader` detects the platform by `os.name` and `os.arch`, extracts `natives/<os>-<arch>/<file>` from the jar into a temporary directory and loads it through `SymbolLookup.libraryLookup`. The path can be set explicitly: `-Dberesta.library.path=/path/libberesta.so`.
- Call handles (`MethodHandle`) are built once from a `FunctionDescriptor`, strings are passed through `Arena.ofConfined()` and freed right after the call.
- `XmlDocument`, `XPath`, `XmlSchema` implement `AutoCloseable`. If an object was not closed, a `Cleaner` frees the native memory.
- A node keeps a reference to its document. After `close()` accessing a node gives an `IllegalStateException`, not a JVM crash.
- The node methods are split over interfaces (`NodeContent`, `NodeNavigation`, `NodeChildren`, `NodeEditing`, `NodeStructure`, `ElementAttributes`): a method can accept exactly the part of the API it needs.

A self-test without Maven or JUnit (only JDK 22+ is needed): `make JAVA=1 java-test`.

---

## C# (P/Invoke)

The wrapper is the `bindings/csharp/Beresta` project (the `Beresta` namespace, the `Beresta.NET.dll` assembly, the target frameworks `netstandard2.1` and `net9.0`).

```csharp
using Beresta;

using (XmlDocument document = Xml.Parse("<shop><item code='A1'>Tea</item></shop>"))
{
    XmlElement item = (XmlElement)document.Root!.FirstChild!;
    Console.WriteLine(item.GetAttribute("code") + " " + item.Text);            // A1 Tea

    using XPath query = new XPath("//item[@code = $c]").Set("c", "A1");
    Console.WriteLine(query.EvaluateString(document));                        // Tea
}

using (XmlDocument order = Xml.NewDocument("order"))
{
    XmlElement item = order.Root!.AppendElement("item");
    item.SetAttribute("qty", "2");
    item.AppendText("Honey & nuts");
    order.Save("order.xml", WriteFlags.Pretty, "windows-1251");
}

try
{
    using XmlSchema schema = XmlSchema.FromFile("order.xsd");
    using XmlDocument document = Xml.ParseFile("order.xml");
    foreach (Diagnostic problem in schema.Validate(document))
        Console.WriteLine(problem.Line + ": " + problem.Message);
}
catch (BerestaException error)
{
    Console.WriteLine(error.CodeName + " " + error.Detail);
}
```

Building the NuGet package:

```bash
make CSHARP=1 csharp-package
```

How it works:
- The functions are declared through `[DllImport]`; input strings are passed as `UnmanagedType.LPUTF8Str`, output `char*`s are read and freed right away.
- A document, an expression and a schema own native memory through a `SafeHandle`: freeing is guaranteed both on `Dispose` and on finalization, and the reference counter does not let a resource be freed during a call.
- In the NuGet package the binaries lie in the standard `runtimes/<rid>/native/` layout, and .NET picks the right one itself. With a project reference (`ProjectReference`) they are found by an own resolver (`NativeLibrary.SetDllImportResolver`). The path can be set explicitly with the `BERESTA_LIBRARY` environment variable.
- The assembly is named `Beresta.NET.dll`, not `Beresta.dll`: Windows does not distinguish the case of file names, and the managed assembly would clash with the native `beresta.dll`.

The self-test is a console program without xUnit: `make CSHARP=1 csharp-test`. It is built for the newest framework of the installed SDK (`net9.0` with SDK 9, `net10.0` with SDK 10), so it runs without installing an extra runtime.

---

## Python (ctypes)

The `bindings/python/beresta` package (Python 3.8+). It uses only the standard `ctypes` module: nothing needs to be installed and nothing is compiled on the user's machine.

```python
import beresta
from beresta import BerestaError, ParseFlags, Schema, WriteFlags, XPath

with beresta.parse("<shop><item code='A1'>Tea</item></shop>") as document:
    item = document.root.first_child
    print(item.get_attribute("code"), item.text)                      # A1 Tea
    print(XPath("//item[@code = $c]", variables={"c": "A1"}).string(document))
    print([child.name for child in document.root])                    # ['item']

with beresta.new_document("order") as order:
    item = order.root.append_element("item")
    item.set_attribute("qty", "2")
    item.append_text("Honey & nuts")
    order.save("order.xml", WriteFlags.PRETTY, encoding="windows-1251")

try:
    schema = Schema.from_file("order.xsd")
    with beresta.parse_file("order.xml", ParseFlags.DISCARD_COMMENTS) as document:
        for problem in schema.validate(document):
            print(problem.line, problem.message)
except BerestaError as error:
    print(error.name, error.message)
```

Preparing the package:

```bash
make PYTHON=1
```
```bash
python -m pip wheel bindings/python -w dist
```

How it works:
- `restype` and `argtypes` are set for **every** function. Without them ctypes would truncate 64-bit pointers to `int`.
- Returned `char*`s are declared as `c_void_p`, not `c_char_p`: this way the pointer is not lost, and the string can be freed.
- A document works as a context manager (`with`). If it was not closed, `weakref.finalize` frees the memory.
- Nodes are plain Python objects: `len(node)`, `node[i]`, `for child in node`, the `text`, `name`, `parent` properties.
- The library is looked up in `BERESTA_LIBRARY`, then in `beresta/_native/<os>-<arch>/`, then in the system.

A self-test on the standard `unittest`: `make PYTHON=1 python-test`. The Makefile runs `python` on Windows and `python3` on Linux and macOS; another interpreter is set like this: `make PYTHON_EXE=/path/python3.12 python-test`.

---

## Node.js (Node-API)

The `bindings/node` package (Node.js 18+, `using` works in Node.js 24+). The calls go through an own Node-API addon, not through `ffi-napi` or node-gyp: the declarations of the stable Node-API ABI are written by hand, and the addon finds the functions it needs in the Node process at load time. That is why installing it needs neither Node headers, nor a compiler, nor npm dependencies — the build itself puts the ready `beresta.node` (about 1–2 MB, the core is already inside) into the package. TypeScript types are included (`lib/index.d.ts`).

```js
const beresta = require('beresta');

using document = beresta.parse("<shop><item code='A1'>Tea</item><item code='B2'>Coffee</item></shop>");
const item = document.root.firstChild;
console.log(item.getAttribute('code'), item.text);                          // A1 Tea

using query = new beresta.XPath('//item[@code = $c]', { variables: { c: 'B2' } });
console.log(query.string(document));                                        // Coffee

using order = beresta.createDocument('order');
order.root.appendElement('item').setAttribute('qty', 2).appendText('Honey & nuts');
console.log(order.toXml(beresta.WriteFlags.PRETTY));
order.save('order.xml', { encoding: 'windows-1251' });

using schema = beresta.Schema.fromFile('order.xsd');
for (const problem of schema.validate(order))
    console.log(problem.line, problem.message);

try { beresta.parse('<a><b></a>'); }
catch (error) { console.log(error.code, error.message); }                   // PARSE_ERROR line 1, ...
```

Building and the self-test (the built-in `node --test`, without Jest or Mocha):

```bash
make NODE=1 node-test
```
```bash
make NODE=1 node-package
```

How it works:
- The addon (`addon/`, C++17) is 31 signature adapters for 66 C interface functions (the Adapter and Command patterns). It turns error codes into `BerestaError` exceptions with a code (`PARSE_ERROR`) and a text in the current language itself.
- Handles are `BigInt`s: an address does not have to fit into the 53 bits of a number, and equal addresses are equal by `===`. Two wrappers of one node are compared through `node.isSameNode(other)`.
- Parsing a `Buffer` or a `Uint8Array` goes without copying the bytes; a string is encoded into UTF-8.
- `XmlDocument`, `XPath` and `Schema` are closed through `close()` or `using` (`Symbol.dispose`). If you forget, `FinalizationRegistry` frees the memory.
- The addon data is kept separately for every environment (`napi_set_instance_data`), so the package works in `worker_threads` too.
- The addon is looked up in `BERESTA_NODE_ADDON`, then in `native/<os>-<arch>/beresta.node`.
- A TypeScript example runs without a compiler (Node.js 23.6+): `node bindings/node/examples/demo.mts`.
- The package from npm contains the addons of all platforms (see [Ready packages and releases](#ready-packages-and-releases)); for browsers there is `beresta-wasm` with the same API.

---

## WebAssembly (browsers and Workers)

The `bindings/wasm` package (`beresta-wasm`): the core built by Emscripten and **the same JS layer as for Node.js** — the classes, methods and errors are the same, the API just becomes available after `await init()`.

```js
import init from 'beresta-wasm';

const beresta = await init();
const order = beresta.parse(xmlText);
const schema = beresta.Schema.fromString(xsdText);
for (const problem of schema.validate(order))
    console.log(problem.message);                 // /order/item[3]: the attribute "price": the value "-5" is less than the minimum 0
console.log(new beresta.XPath('sum(//@price)').number(order));
order.close();
schema.close();
```

Building and the self-test (Emscripten 3.1+ is required):

```bash
make wasm-test
```

A ready example page is `bindings/wasm/examples/browser.html`: XML, XSD and XPath checked right in the browser. Open it through any static server (`python -m http.server --directory bindings/wasm`, then `http://localhost:8000/examples/browser.html`).

How it works:
- The core is compiled by `em++` with `-fwasm-exceptions` (C++ exceptions by means of Wasm itself) and exports the same C interface functions as the shared library (`bindings/wasm/exports.json`).
- `lib/native_wasm.js` is a backend for the common JS layer (the Adapter pattern): the same function table as in the Node.js addon, while the 31 signatures are described as data (`wasm_signatures.js`), and one common call puts strings and bytes into the Wasm memory, parses the result and turns errors into `BerestaError`.
- The layer is written in CommonJS, while a browser needs an ES module, so `tools/bundle.mjs` is an own bundler without dependencies: it walks `require`, substitutes the Wasm backend for the addon loader and writes one `dist/beresta.mjs` file. The classes are not duplicated: a change in `bindings/node/lib` goes into the browser package at once.
- The module weighs about 650 KB (about 250 KB gzipped). Parsing runs at about two thirds of the native addon speed, XPath is not slower.
- In Cloudflare Workers the module is passed through `init({ instantiateWasm })` (an example is in `bindings/wasm/README.md`); the CI checks a browser and Node.js, but not Workers.

---

## Limitations

Deliberate limitations (KISS / YAGNI):

- **XML Schema** is a subset of XSD 1.0:
  - `xs:redefine` and `xs:override` are not supported;
  - `substitutionGroup` sets the type, but membership in a substitution group is not taken into account when the model is checked;
  - the derivation validity of `xsi:type` and `restriction` is not checked;
  - default values from the schema are not added to the DOM;
  - `duration` has no order for the bound facets;
  - Unicode categories in `pattern` (`\p{L}` and the like) cover the main scripts (Latin, Cyrillic, Greek, Hebrew, Arabic, CJK), not the whole Unicode table.
- **XPath:** XPath 1.0 is fully supported; XPath 2.0/3.1 (the XSD type system, sequences, FLWOR) is a separate language and is not part of Beresta. Xerces, Xalan, libxml2 and System.Xml do not have it either.
- **XInclude:** the `xml:base` attribute is not added to included content; XPointer is only `element()` and ID.
- **Encodings:** the eight listed above. Others have to be converted in advance or added with your own table.
- **No** XSLT, DOM Range, XML 1.1 specifics (`version="1.1"` is parsed by the 1.0 rules) or network loading.
- **An external entity with `version="1.1"`** in the text declaration is rejected. The 5th edition of XML 1.0 does not require this directly; it is done this way after the W3C case `rmt-e2e-38` (an erratum of the 2nd edition: an XML 1.0 document cannot include an XML 1.1 entity).
- **Processing instructions inside the DTD** are passed to the SAX handler (`dtdProcessingInstruction`, by default a plain `processingInstruction`), but do not become DOM nodes: `DocumentType` has no children. PIs of the internal subset are kept in the `getInternalSubset()` text and in serialization.
- **Extreme depth.** There is no recursion anywhere, so the stack will not overflow. But some results themselves grow faster than the depth: the total indentation of pretty output and the document order keys in XPath are proportional to the depth for every node (so is the path in an XSD validator message, but it is built only on an error). On trees tens of thousands of levels deep these operations are noticeably slower. By default the parser limits the depth to `maxDepth` = 1024.
- **The C interface and the wrappers** cover DOM, XPath and XML Schema. SAX, `TreeWalker`, a custom `ResourceResolver` and the fine settings of `ParserOptions` (limits, a forced encoding) are available only from C++. DTD validation and XInclude are enabled with parse flags in other languages. Nodes in the wrappers live inside their document, but they can be both copied (`copyTo` / `CopyTo` / `copy_to`) and moved without copying (`moveTo` / `MoveTo` / `move_to`), including between documents. Creating a node without a document, as in C++, is not possible from other languages, and the root element of a document is not moved — only copied. Walking the tree step by step from the wrappers is tens to hundreds of times slower than from C++: every step is a call across the language boundary; for bulk processing use XPath (`selectValues`) or SAX in C++.
- **A node after removal.** After `remove()` the wrapper of the removed node and the wrappers of its descendants are invalid and must not be used. A document checks only its own closing.
- **32-bit platforms** are supported by the core, but the wrappers were checked on 64-bit ones.
- **WebAssembly:** there is no file system, so `parseFile`, `save`, `Schema.fromFile` and loading external DTDs, entities and XInclude from files are unavailable — parse strings and bytes, output to a string.
- **Streaming XSD validation** (`SaxParser` with a schema) checks the `xs:key`/`xs:unique`/`xs:keyref` keys only with selectors and fields from the XSD grammar; expressions beyond it (the check over a tree accepts them) are not evaluated in a stream, and a message reports that. Schema errors are reported after the end of the document (the sibling numbers in paths are known only then), and the SAX parser, like the DOM one, reads the input into memory entirely — without a tree, but not as a byte stream.

---

## Project structure

```
beresta/
├── CMakeLists.txt  Makefile  README.md  README.en.md
├── .github/workflows/            ci.yml (build and tests), release.yml (packages and release)
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
└── tests/                        C++ unit tests (an own mini framework)
    ├── conformance/              the W3C XML Conformance Test Suite runner and report
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

**136 C++ tests in 43 suites:**
- the core: copying, hierarchy, elements, navigation, namespaces, serialization, trees 100,000 levels deep;
- the parser: formats, content, errors, limits, SAX (including PIs from the DTD), encodings, protection from complexity attacks (repeated attributes beyond the loop threshold, the keyed hash);
- DTD: entities, validity checking, external resources;
- the serializer, tree traversal, XPath (paths, axes, functions, the `namespace` axis, names), XSD (types, schemas, advanced features, `xs:key`/`xs:unique`/`xs:keyref`, paths in error messages, whitespace rules; streaming validation: SAX, DOM and a ready tree give the same errors, streaming keys, a deep document), the content model (the step-by-step cursor against the general algorithm on thousands of random models), XInclude;
- the C interface: error codes, building and changing the tree, moving nodes between documents (with freeing the source document and rolling back a rejected move), XPath, XML Schema;
- message languages: catalog completeness (every message has a translation with the same `{N}` markers), changing the language at run time, argument substitution;
- regressions: differences from the W3C suite (syntax, entities, validity, encodings) and fuzzing findings.

Standard conformance and resistance to hostile input are checked separately:

```bash
cd tests/conformance && make fetch && make run      # W3C XML Conformance Test Suite -> conformance_report.md
cd tests/fuzz && make -j && make corpus && make -j3 fuzz FUZZ_TIME=600 FUZZ_JOBS=3   # libFuzzer, Clang only
```

The tests pass both in a regular build and under `-fsanitize=undefined -D_GLIBCXX_ASSERTIONS`.

The wrapper self-tests work without test frameworks (JUnit, xUnit and pytest are not needed):

```bash
make JAVA=1 CSHARP=1 PYTHON=1 NODE=1 bindings-test
```

They check parsing, navigation, building, copying and moving between documents (including "move from A to B, close A, use the node in B"), errors, writing in another encoding, XPath, XML Schema and message languages: Java — 13 checks, C# — 12, Python — 15, Node.js — 16 (the built-in `node --test`), WebAssembly — 5 (`make wasm-test`). In CMake all of this (and the C++ tests) is run by `ctest` if `BERESTA_BUILD_TESTS` and the wrapper flags are on.

Where it was checked (without a single compiler warning):

| Platform | Compilers | What was checked |
|---|---|---|
| Windows 11 x64 | MSVC (Visual Studio 2026), MinGW GCC 15 | CMake and the Makefile, the C++ tests, `ctest`, the self-tests of Java 25 / .NET 9 / Python 3.12 / Node.js 24, `dotnet pack` |
| Ubuntu 26.04 x64 | GCC 15, Clang 21 | CMake and the Makefile, the C++ tests, `ctest`, the self-tests of Java 25 / .NET 10 / Python 3.14 / Node.js 22, `mvn package` (running with the binary from the jar), `dotnet pack`, a wheel in a clean venv |
| Ubuntu 26.04 x64 | GCC 15 + ASan, UBSan, LeakSanitizer | the C++ tests, the C and C++ demos: no memory leaks, no undefined behavior |
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

**A new message language:** see [How to add a language](#how-to-add-a-language).

**Comments** are written in two languages: first the Russian text, then its English translation in the same comment (`// русский` and on the next line `// English`, or `// русский / English` for a short trailing comment).

**A new C interface function:**
1. A declaration with `BERESTA_API` in `include/capi/beresta_c.h` (C types only: `void*`, `int`, `int64_t`, `uint32_t`, `double`, `const char*`).
2. An implementation in `src/capi/<group>/` wrapped in `BERESTA_CAPI_TRY` … `BERESTA_CAPI_CATCH(...)`: no exception may get out.
3. A signature line in every wrapper: `_signatures.py` (Python), `*Functions.java` (Java), `NativeMethods.*.cs` (C#), a line in `bindings/node/addon/src/module/binding_table.cpp` (Node.js; the adapter for a new signature goes into `adapters/`), a line in `BINDINGS` in `bindings/wasm/lib/wasm_signatures.js` and a name in `bindings/wasm/exports.json` (WebAssembly; a new signature is described there as data). Then a method in the object API of the wrapper.

---

## License

Beresta is distributed under the **MIT** license — the text is in the [LICENSE](LICENSE) file.

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
