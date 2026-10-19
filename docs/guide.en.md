# Beresta — the C++ guide and examples

[Русский](guide.md) · **English**

← [To the description and advantages](../README.en.md)

## Contents

1. [Quick start](#quick-start)
2. [Data model and memory](#data-model-and-memory)
3. [Working with the tree](#working-with-the-tree)
4. [Copying and moving nodes](#copying-and-moving-nodes)
5. [Parsing: DOM](#parsing-dom)
6. [Parsing: SAX](#parsing-sax)
7. [Namespaces](#namespaces)
8. [Encodings, files and streams](#encodings-files-and-streams)
9. [Serialization](#serialization)
10. [DTD and entities](#dtd-and-entities)
11. [XML Schema](#xml-schema)
12. [XPath 1.0](#xpath-10)
13. [Tree traversal: TreeWalker and NodeIterator](#tree-traversal-treewalker-and-nodeiterator)
14. [XInclude](#xinclude)
15. [External resources and security](#external-resources-and-security)
16. [Error handling](#error-handling)
17. [Message languages](#message-languages)
18. [Guarantees and thread safety](#guarantees-and-thread-safety)

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
- substitution groups `substitutionGroup`: members (including transitive ones) stand in place of the head and are checked by their own declaration; abstract members and the ones blocked by `block` are not in the group;
- derivation: the `xsi:type` type must be derived from the declared one (built-in type chains, `extension`/`restriction`, `union` members), `block`/`blockDefault` on the element and the type prohibit substitution methods, `final`/`finalDefault` prohibit derivation when the schema is loaded; an abstract type without a derived `xsi:type` is an error;
- `simpleType` `restriction`/`list`/`union`;
- the facets `length`, `minLength`, `maxLength`, `pattern`, `enumeration`, `whiteSpace`, `minInclusive`, `maxInclusive`, `minExclusive`, `maxExclusive`, `totalDigits`, `fractionDigits`;
- all **45 built-in types** of XSD 1.0 (strings, integers with exact bounds, `decimal`, `float`/`double`, dates and times with time zones, `duration`, `hexBinary`, `base64Binary`, `QName`, ID/IDREF(S), NMTOKEN(S), ENTITY(IES) and so on);
- `ID` uniqueness and the existence of `IDREF` targets;
- the identity constraints `xs:unique`, `xs:key`, `xs:keyref` (details below);
- `xs:include` (including "chameleon" schemas), `xs:import`, `xs:redefine` and `xs:override` (from XSD 1.1: replacing a component entirely);
- XSD 1.1 assertions: `xs:assert` of complex types (the ones inherited through `extension`/`restriction` are checked too) and the `xs:assertion` facet of simple types with `$value` (details below);
- XSD 1.1 conditional inclusion: Beresta is a version 1.1 processor, components with `vc:minVersion` > 1.1 or `vc:maxVersion` <= 1.1 are skipped at loading.

**Substitution groups and `xs:redefine` do not slow validation down.** Replacing components through `xs:redefine` / `xs:override` happens in the definition registry before the models are built — the validated document does not know about it. The set of admissible substitutes of every head is computed once when the schema is loaded (transitively, taking `abstract`, `block` and `final` into account) and kept ordered: a child's name is looked up by a binary search only if it did not match the particle itself. The group flag sits on the model particle, so models without substitution groups still take the fast greedy path.

```xml
<!-- v2.xsd: a new version of an industry schema without editing the original -->
<xs:redefine schemaLocation="v1.xsd">
    <xs:complexType name="Party">
        <xs:complexContent>
            <xs:extension base="Party">          <!-- base is the former Party from v1.xsd -->
                <xs:sequence><xs:element name="inn" type="xs:string"/></xs:sequence>
            </xs:extension>
        </xs:complexContent>
    </xs:complexType>
</xs:redefine>
```

### XSD 1.1 assertions: xs:assert and xs:assertion

Rules that a content model cannot express — reconciling sums, dependencies between fields, conditions on dates — are written right in the schema as an XPath expression. An example is a bank statement whose closing balance must match the transactions:

```xml
<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"
           xmlns:vc="http://www.w3.org/2007/XMLSchema-versioning" vc:minVersion="1.1">
  <xs:element name="Statement">
    <xs:complexType>
      <xs:sequence>
        <!-- AccountInfo: saldoAwal, saldoAkhir (xs:decimal); Transactions/Tx: type, amount (xs:decimal) -->
      </xs:sequence>
      <!-- without transactions the balances are equal -->
      <xs:assert test="if (not(Transactions/Tx)) then AccountInfo/saldoAwal = AccountInfo/saldoAkhir else true()"/>
      <!-- opening balance + credit - debit = closing balance -->
      <xs:assert test="if (Transactions/Tx) then (AccountInfo/saldoAwal + sum(Transactions/Tx[type='cr']/amount)
                       - sum(Transactions/Tx[type='dr']/amount) = AccountInfo/saldoAkhir) else true()"/>
    </xs:complexType>
  </xs:element>

  <!-- a facet: a non-negative amount with at most two digits after the point -->
  <xs:simpleType name="Amount">
    <xs:restriction base="xs:decimal">
      <xs:assertion test="$value ge 0 and $value * 100 = floor($value * 100)"/>
    </xs:restriction>
  </xs:simpleType>
</xs:schema>
```

```
line 2, column 1: /Statement: the assertion if (Transactions/Tx) then (AccountInfo/saldoAwal + … is not satisfied
/payment/amount: the value "10.005" does not satisfy the assertion $value ge 0 and $value * 100 = floor($value * 100)
```

- **Exact decimal arithmetic.** Values of `xs:decimal` and its derivatives (`xs:integer`, `xs:long` …) and the numbers in the expression itself are exact decimal numbers of any length: `+ - *`, `div` (18 digits after the point), `mod`, `sum()`, `floor`/`ceiling`/`round` and comparisons are computed without `double`. So `0.10 + 0.20 = 0.30` is true, ten credits of `0.10` give exactly `1.00`, and sums beyond the precision of `double` match to the cent. An untyped text that looks like a decimal number is added exactly too; `xs:double`/`xs:float` are computed in `double`, as XPath 2.0 requires.
- **Typed values.** A node is atomized by its type from validation (an analog of the PSVI): `xs:decimal` is a number, `xs:boolean` is a boolean, `xs:date`/`xs:dateTime` are moments in time with the time zone taken into account, everything else is a string. So `1000.00 = 1000` is true. An empty element with a `default` value sees that value. An element with element-only content has no typed value: comparing with it is an error.
- **The assertion language** is the whole of XPath 1.0 plus the common XPath 2.0 constructs: `if (…) then … else …`, `some`/`every $x in … satisfies …` (several bindings separated by commas), the value comparisons `eq ne lt le gt ge`, the constructors `xs:string`, `xs:boolean`, `xs:decimal`, `xs:integer`, `xs:double`, `xs:date`, `xs:dateTime`, the functions `exists()` and `empty()`. A name without a prefix is in no namespace unless `xpathDefaultNamespace` is given (on `xs:assert`, `xs:assertion` or `xs:schema`: a URI, `##targetNamespace`, `##defaultNamespace`, `##local`).
- **Everything else from XPath 2.0 is a schema loading error**, not a "silent" truth during validation: `for`, `instance of`, `cast as`, `castable as`, `to`, `idiv`, `intersect`/`except`, `is`, sequences `(1, 2)`, numbers with an exponent, functions outside the list (`matches`, `tokenize`, `string-join`, `fn:*` …), constructors of other types (`xs:time`, durations) and absolute paths (the root of the assertion tree is the element itself, not a document). The message gives the schema line and the assertion text: `schema, line 12: the assertion test="…" is invalid or uses an unsupported XPath 2.0 construct: …`.
- **An evaluation error is a violation**, as XSD 1.1 requires: a string that cannot be cast to a number, division by zero, date arithmetic, several nodes where one value is expected — the assertion is not satisfied, the reason is in the message.
- **Inheritance.** The assertions of the base type are checked for derived types too (`extension`, `restriction`, `xsi:type`); `xs:assertion` facets accumulate over the restriction steps, and for a union `$value` is typed by the matching member.
- **Three modes — the same violations** (path, line, column): validation of a ready tree, parsing into a DOM with a schema and the streaming `SaxParser`. An assertion needs the whole element, so the stream copies into a mini-tree the subtree of only those elements whose type has assertions, and evaluates them when the element closes; the rest of the validation still takes O(depth). The mini-tree of a nested element moves into the outer one after its check without copying, and the traversal uses an explicit stack.

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
