# Beresta — comparison, use cases, performance and limitations

[Русский](comparison.md) · **English**

← [To the description and advantages](../README.en.md)

## Contents

1. [Comparison with other libraries](#comparison-with-other-libraries)
2. [Where to use it](#where-to-use-it)
3. [Performance](#performance)
4. [Limitations](#limitations)

---

## Comparison with other libraries

What popular XML libraries for C and C++ can do "out of the box" (versions as in the [measurements](#performance)):

| Feature | Beresta | libxml2 | Xerces-C++ | pugixml | tinyxml2 | RapidXML |
|---|---|---|---|---|---|---|
| Strings in the API | `std::string` (UTF-8) | `xmlChar*` (UTF-8), manual `xmlFree` | `XMLCh*` (UTF-16), `transcode`/`release` | `char*` or `wchar_t*` | `const char*` | `char*` |
| Namespaces | yes, "live" URIs | yes | yes | no (the prefix is part of the name) | no | no |
| DTD validation | yes | yes | yes | no | no | no |
| XML Schema | 1.0: keys, substitution groups, `redefine`; from 1.1 — `xs:assert` | 1.0, partial | 1.0, the most complete | no | no | no |
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

**What to choose.** If you need only fast parsing of trusted XML without namespaces and checks, pugixml or RapidXML are 10–15 times faster. If you need strict checking of the schema itself against all the XSD 1.0 constraints (the validity of `restriction` for content models, an unambiguous model — UPA), take Xerces-C++. Large industry schemas with `xs:redefine` and substitution groups are loaded and validated by Beresta itself. Beresta is for everything in between: full-featured XML with namespaces, DTD, XSD and XPath, secure by default, without dependencies, with a convenient C++ API and wrappers for five languages and WebAssembly.

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
- **Embedded systems and C and C++ tools.** No dependencies and no recursion over the document tree, and memory use is predictable: a deep or huge document will not crash the process.

When another library is a better choice is said in [Comparison with other libraries](#comparison-with-other-libraries) ("What to choose").

---

## Performance

The measurements are made by the [`benchmarks/`](../benchmarks/) program on the same data for all libraries: a product catalog (26 MB: 100,000 elements with attributes, entities and Cyrillic) and an article (14 MB: a long Russian text with mixed content). The median of 5 runs, AMD Ryzen 5 5600X, Ubuntu 26.04, GCC 15 `-O2`, the libraries from the distribution packages. The walk checksums, the output size and the XPath node count match for all libraries — so they built the same tree.

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
- **Streaming validation without a tree** (`SaxParser` with a schema, and `validateFile` from the wrappers) — 93 MB/s, 1.5 times faster than with building a tree; libxml2 (`xmlSchemaValidateStream`) — 112 MB/s. The file is mapped into memory, so the process needs O(document depth) of its own memory: a 135 MB export is validated in 37 MB, while a tree would take 857 MB.

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

**Wrappers against the native XML libraries of their languages** (a 13 MB catalog, 50,000 products; parsing and validation — MB/s, walk and XPath — ms; "Stream + XSD" — validation without a tree; memory — the growth of the process memory after parsing; all libraries agree on the element count, the XPath node count and the schema error count):

| Language | Library | Parse | Walk | XPath | Parse + XSD | Stream + XSD | Memory |
|---|---|---|---|---|---|---|---|
| Java 25 | **Beresta (Panama)** | 105 | 305 | **69** | 64 | **97** | **88 MB (×6.6)** |
| | JDK (`javax.xml`, Xerces-J) | **149** | **4** | 127 | **67** | 90 (`Validator`) | 123 MB (×9.3) |
| .NET 10 | **Beresta (P/Invoke)** | 102 | 240 | 91 | **64** | **94** | 86 MB (×6.5) |
| | `XmlDocument` | 74 | 32 | **20** | 28 | 85 (`XmlReader`) | 72 MB (×5.5) |
| | `XDocument` (LINQ to XML) | **109** | **4** | 29 | 42 | 85 (`XmlReader`) | **54 MB (×4.1)** |
| Python 3.14 | **Beresta (ctypes)** | 103 | 1545 | 100 | 63 | **93** | 86 MB (×6.5) |
| | lxml 6.1 (libxml2 2.14) | **121** | 27 | **59** | **77** | 49 (`iterparse`) | 119 MB (×8.9) |
| | `xml.etree.ElementTree` | 56 | **5** | — | — | — | **82 MB (×6.2)** |

"Stream + XSD" for the native libraries is their standard way to validate a document without a DOM: `javax.xml.validation.Validator` over a `StreamSource`, a validating `XmlReader` read to the end, and `lxml.etree.iterparse` with a schema and clearing of elements. Streaming validation needs almost no memory: a 135 MB file (500,000 products) validated through `schema.validate_file` from Python takes 37 MB of the process's own memory at the peak, while `parse_file` + `validate` takes 857 MB.

> 💡 **How to get C++ speed from a wrapper.** The slow spot of any wrapper is the boundary between languages. Every step of a manual walk (`firstChild`, `nextSibling`) is a call through Panama, P/Invoke or ctypes and a new wrapper object. A million small steps from Python measure the speed of ctypes, not of the parser. So hand heavy work over to the core entirely:
> - **XPath:** an expression (`XPath`) is compiled once and evaluated any number of times, including from different threads. Take node values through `selectValues` / `SelectValues` / `select_values`: the strings are assembled inside the core, and no node objects are created.
> - **XSD validation:** `schema.validate(document)` for a ready tree. In .NET it is 2.3 times faster than `XmlDocument` and 1.5 times faster than `XDocument`.
> - **Streaming validation without a tree:** `validateFile` / `ValidateFile` / `validate_file`. It suits multi-gigabyte exports: no tree is built, and the file is mapped into memory.
>
> In these scenarios Beresta runs at the speed of compiled code, and its tree takes about 30 % less memory than the JDK DOM and lxml.

Where the wrappers make sense:
- **The core is the same everywhere**, so parsing and schema validation run at C++ speed (63–105 MB/s) in any language. A step-by-step walk from a wrapper is still tens of times slower than in the native libraries, although it became 1.4 times faster in Java (`invokeExact` on the hot calls) and 1.6 times faster in Python (the node type and the wrapper classes are remembered).
- **Streaming schema validation** wins in all three languages: faster than the JDK `Validator` (97 against 90 MB/s), faster than the validating .NET `XmlReader` (94 against 85) and almost twice as fast as lxml (93 against 49).
- **Node.js and the browser** are the clearest win: among libraries with a real DOM, XPath and XSD Beresta is the fastest (see the table above), and a browser has no built-in XSD validation at all.
- **.NET** — parsing with XSD validation is 2.3 times faster than `XmlDocument` (64 against 28 MB/s) and 1.5 times faster than `XDocument`, parsing is on par with `XDocument`. If no schema is needed, `XDocument` is faster and leaner.
- **Java** — XPath is 1.4–1.8 times faster than in the JDK (in different runs the JDK shows 97 to 127 ms), the tree is 28 % more compact (88 against 123 MB), XSD validation is on par. Parsing is slower than the built-in Xerces-J. An important difference is security by default: the standard `DocumentBuilder` loads external entities (XXE) until it is explicitly configured not to.
- **Python** — lxml is faster at parsing into a tree and in XPath, but Beresta's streaming validation is almost twice as fast, and its tree is 28 % more compact (86 against 119 MB). XPath 1.0 and XSD work without third-party packages: the built-in `xml.etree` can do neither, and the documentation of the `xml` module explicitly warns that it is not secure against hostile data.
- **Common reasons to choose the wrapper:** the same error messages (in Russian or English), `/order/item[2]`-style paths, XPath and XSD rules in all languages — microservices in Java, .NET, Python and Node.js validate the same files the same way. The protection against entity bombs and XXE is on in the core and does not need to be configured in every language. The windows-1251, KOI8-R and IBM866 encodings work without external libraries.

To repeat: `node benchmarks/wrappers/make_data.js`, then the programs in `benchmarks/wrappers/` — `WrapperBench` and `StreamBench` (Java), `WrapperBench` with the `--stream` switch (.NET), `bench.py` and `stream_bench.py` (Python); see [`benchmarks/README.md`](../benchmarks/README.md).

---

## Limitations

Deliberate limitations (KISS / YAGNI):

- **XML Schema** is a subset of XSD 1.0:
  - when the schema is loaded, the validity of a content model restriction under `restriction` (cos-particle-restrict) and the unambiguity of a model (UPA) are not checked: an ambiguous model is accepted and checked exactly, without picking the first branch. For the same reason a group (an attribute group) in `xs:redefine` without a self-reference is not checked as a restriction of the former one (src-redefine.6.2, 7.2);
  - a schema document is not checked against the schema for schemas in full: `block`/`final`, the boolean attributes, the attributes of `xs:element` by its place and of named groups are checked, but not, for example, the uniqueness of `id` and the validity of `default`/`fixed` by the type. The score on the official W3C XML Schema Test Suite (96.2 % of the applicable cases) and the list of deviations are in [`tests/xsts`](../tests/xsts/README.md);
  - an unavailable `schemaLocation` in `xs:include`/`xs:import`/`xs:redefine` is a loading error, although the specification allows skipping such a document: a typo in a path does not turn into a silently incomplete schema;
  - a schema with the `targetNamespace` of XML Schema itself (the schema for schemas) is not loaded — the names of this namespace are reserved for the built-in types; cycles through `xs:redefine` (A redefines B, and B redefines or includes A) are not supported; a type derived from a complex type inside the model of that same type is taken as circular derivation;
  - from XSD 1.1 there are `xs:override`, the `xs:assert`/`xs:assertion` assertions and `vc:minVersion`/`vc:maxVersion`; `xs:alternative`, open content, `vc:typeAvailable` and the other version attributes are not supported;
  - the assertion language is XPath 1.0 and a subset of XPath 2.0 (see "XSD 1.1 assertions" above), the other XPath 2.0 constructs are a schema loading error. Dates are compared, but there is no date and duration arithmetic; an assertion sees the values of `xs:time`, `xs:duration`, the g-types, the binary types, `QName` and lists as strings; the `xs:assertion` facet of a list type is a loading error; assertions are not checked for an element with `xsi:nil="true"`; comments and processing instructions do not get into the assertion tree;
  - default values from the schema are not added to the DOM; the ID/IDREF of default attributes are counted, but the fields of `xs:key`/`xs:unique`/`xs:keyref` do not see such attributes;
  - `pattern` regular expressions use the Unicode 3.1 tables — the version XSD 1.0 refers to: all general categories (`\p{L}`, `\p{Mn}`, `\p{Cn}`…) and blocks (`\p{IsCyrillic}`, `\p{IsOldItalic}`…) come from the library's own tables, without ICU; `\i` and `\c` follow XML 1.0 appendix B. Characters added to Unicode after 3.1 count as unassigned (`\p{Cn}`), and there are no blocks newer than 3.1. Perl/.NET extensions (`\x41`, `A`, `\b`, `\A`, the back references `\1`) are a schema error, as the XSD grammar requires, not the letter after the slash.
- **XPath:** XPath 1.0 is fully supported; from XPath 2.0 — only the subset for XSD 1.1 assertions (with typed values and exact xs:decimal arithmetic). General XPath 2.0/3.1 (sequences, FLWOR, the full type system) is a separate language and is not part of Beresta. Xerces, Xalan, libxml2 and System.Xml do not have it either.
- **XInclude:** the `xml:base` attribute is not added to included content; XPointer is only `element()` and ID.
- **Encodings:** the eight listed above. Others have to be converted in advance or added with your own table.
- **No** XSLT, DOM Range, XML 1.1 specifics (`version="1.1"` is parsed by the 1.0 rules) or network loading.
- **An external entity with `version="1.1"`** in the text declaration is rejected. The 5th edition of XML 1.0 does not require this directly; it is done this way after the W3C case `rmt-e2e-38` (an erratum of the 2nd edition: an XML 1.0 document cannot include an XML 1.1 entity).
- **Processing instructions inside the DTD** are passed to the SAX handler (`dtdProcessingInstruction`, by default a plain `processingInstruction`), but do not become DOM nodes: `DocumentType` has no children. PIs of the internal subset are kept in the `getInternalSubset()` text and in serialization.
- **Extreme depth.** The document tree is never walked recursively, so a deep document will not overflow the stack. Schema building and expression parsing are recursive, and their depth is limited: schema building by 256 levels of declarations, types and models, the schema document by 1024 element levels, an XSD regular expression by 64 group levels, an XPath expression by 128 levels of parentheses, function arguments and predicates. A long chain of XPath operators (`1+1+…+1`) is not nested: it is parsed, evaluated and freed by a loop, so its length is not limited. But some results themselves grow faster than the depth: the total indentation of pretty output and the document order keys in XPath are proportional to the depth for every node (so is the path in an XSD validator message, but it is built only on an error). On trees tens of thousands of levels deep these operations are noticeably slower. By default the parser limits the depth to `maxDepth` = 1024.
- **The C interface and the wrappers** cover DOM, XPath and XML Schema, including streaming validation without a tree. A custom SAX handler, `TreeWalker`, a custom `ResourceResolver` and the fine settings of `ParserOptions` (limits, a forced encoding) are available only from C++. From the wrappers the depth limit is always 1024: a deeper document gives `LIMIT_EXCEEDED`, not a stack overflow. DTD validation and XInclude are enabled with parse flags in other languages. Nodes in the wrappers live inside their document, but they can be both copied (`copyTo` / `CopyTo` / `copy_to`) and moved without copying (`moveTo` / `MoveTo` / `move_to`), including between documents. Creating a node without a document, as in C++, is not possible from other languages, and the root element of a document is not moved — only copied. Walking the tree step by step from the wrappers is tens to hundreds of times slower than from C++: every step is a call across the language boundary; for bulk processing use XPath (`selectValues`) or SAX in C++.
- **A node after removal.** After `remove()` the wrapper of the removed node and the wrappers of its descendants are invalid and must not be used. A document checks only its own closing.
- **32-bit platforms** are supported by the core, but the wrappers were checked on 64-bit ones.
- **WebAssembly:** there is no file system, so `parseFile`, `save`, `Schema.fromFile`, `validateFile` and loading external DTDs, entities and XInclude from files are unavailable: the file functions give `IO_ERROR` with an explanation at once. Parse and validate strings and bytes, output to a string.
- **Streaming XSD validation** (`SaxParser` with a schema) checks the `xs:key`/`xs:unique`/`xs:keyref` keys only with selectors and fields from the XSD grammar; expressions beyond it (the check over a tree accepts them) are not evaluated in a stream, and a message reports that. Schema errors are reported after the end of the document (the sibling numbers in paths are known only then). A file (`parseFile`, `validateFile`) is mapped into memory: the process's own memory stays O(depth), and the OS loads and drops the file pages by itself. A string, a buffer and an `std::istream` are taken by the parser entirely — without a tree, but not as a byte stream.
- **An attribute lookup by name** (`getAttribute`, `hasAttribute`) is a scan, as in libxml2, Xerces and pugixml. During parsing, repetitions are searched by a hash table, so a tag with 50,000 attributes is parsed in linear time; many attributes are better read through `getAttributes()` or XPath.
