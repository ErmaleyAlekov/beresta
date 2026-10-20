# Beresta — the C interface and the wrappers for other languages

[Русский](bindings.md) · **English**

← [To the description and advantages](../README.en.md)

## Contents

1. [Other languages: the C interface](#other-languages-the-c-interface)
2. [Java (Project Panama)](#java-project-panama)
3. [C# (P/Invoke)](#c-pinvoke)
4. [Python (ctypes)](#python-ctypes)
5. [Node.js (Node-API)](#nodejs-node-api)
6. [WebAssembly (browsers and Workers)](#webassembly-browsers-and-workers)

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

### Conventions (the same for all 71 functions)

| Rule | Explanation |
|---|---|
| Handles | `beresta_document`, `beresta_node`, `beresta_xpath`, `beresta_nodeset`, `beresta_schema`, `beresta_diagnostics` are `void*` |
| Status | functions returning `int` give `BERESTA_OK` (0) on success or an error code |
| NULL | functions returning a handle or `char*` give NULL on an error. NULL with code 0 means "no object" (for example, there is no next sibling) |
| The last error | `beresta_last_error_code()` and `beresta_last_error_message()` — per thread, like `errno`. Every call resets it |
| Strings | always UTF-8. A `char*` from the library is freed through `beresta_string_free`, a `const char*` is borrowed |
| Sizes and indices | `int64_t` — the same type in all languages; `-1` in a result means an error |
| Ownership | the caller frees a document, a schema, an expression, a node set and a diagnostics list (`beresta_*_free`). A node belongs to its document; `beresta_node_move_to` hands it (with its descendants) over to another document, and the handles do not change |
| Threads | different documents in different threads in parallel. A schema, a compiled XPath expression and a document nobody changes can be shared between threads: a test runs 20 threads on shared handles under ThreadSanitizer |

| Group | Functions |
|---|---|
| Library | `version`, `last_error_code`, `last_error_message`, `status_name`, `string_free` |
| Message languages | `set_language`, `language`, `languages`, `message_text` |
| Documents | `document_create`, `document_parse` (bytes in any encoding), `document_parse_file`, `document_clone`, `document_root`, `document_element_by_id`, `document_save` (any output encoding), `document_free` |
| Nodes | `node_type`, `node_name`, `node_text`, `node_to_string`, `node_parent`, `node_first_child`, `node_last_child`, `node_next_sibling`, `node_previous_sibling`, `node_owner_document`, `node_child_count`, `node_child` |
| Tree changes | `node_set_text`, `node_remove`, `node_copy_to` (including between documents), `node_move_to` (a move without copying, including between documents), `node_append_element`, `node_append_text`, `node_append_comment`, `node_append_cdata`, `node_append_pi` |
| Attributes | `element_get_attribute`, `element_set_attribute`, `element_has_attribute`, `element_remove_attribute`, `element_attribute_count`, `element_attribute_name`, `element_attribute_value`, `element_local_name`, `element_namespace_uri` |
| XPath | `xpath_compile`, `xpath_bind_namespace`, `xpath_set_string`, `xpath_set_number`, `xpath_select`, `xpath_string`, `xpath_number`, `xpath_boolean`, `nodeset_size`, `nodeset_node`, `nodeset_value`, `xpath_free`, `nodeset_free` |
| XML Schema | `schema_load`, `schema_load_file`, `schema_validate` (a ready tree), `schema_validate_bytes` and `schema_validate_file` (a stream without a tree), `diagnostics_count`, `diagnostics_message`, `diagnostics_line`, `diagnostics_column`, `schema_free`, `diagnostics_free` |

The parse flags are the `BERESTA_PARSE_*` bit mask: `PRESERVE_WHITESPACE`, `DISCARD_COMMENTS`, `COALESCE_CDATA`, `VALIDATE_DTD`, `NO_NAMESPACES`, `XINCLUDE`, `FILE_RESOURCES`, `TRACK_LOCATIONS`. `FILE_RESOURCES` allows external DTDs, entities and XInclude, but only from the directory of the parsed file. `TRACK_LOCATIONS` remembers the line and column of every node: then `schema_validate` reports the places of violations (without the flag they are 0; parsing with it is about 8 % slower).

**Streaming validation without a tree.** `beresta_schema_validate_bytes` and `beresta_schema_validate_file` take the same arguments as `beresta_document_parse` and `beresta_document_parse_file`, but return a list of violations instead of a document. Inside, the SAX parser works with a `ValidatingHandler`: no tree is built, the memory is O(document depth) plus ID values and key tables, and the file is mapped into memory, not copied. Schema violations are not a call error (`BERESTA_OK` and a list), an XML syntax error is `BERESTA_PARSE_ERROR`. The places of violations are always known, the `TRACK_LOCATIONS` flag is not needed.

```c
beresta_diagnostics problems = NULL;
if (beresta_schema_validate_file("export.xml", BERESTA_PARSE_DEFAULT, schema, &problems) == BERESTA_OK)
{
    for (int64_t i = 0; i < beresta_diagnostics_count(problems); ++i)
    {
        char* text = beresta_diagnostics_message(problems, i);
        printf("%lld: %s\n", (long long)beresta_diagnostics_line(problems, i), text);
        beresta_string_free(text);
    }
    beresta_diagnostics_free(problems);
}
``` The output flags are `BERESTA_WRITE_PRETTY` and `BERESTA_WRITE_DECLARATION`. A mask was chosen over a settings structure on purpose: an integer is passed the same way from any language.

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
     XmlDocument document = Xml.parseFile(Path.of("order.xml"), ParseFlags.TRACK_LOCATIONS, null))
{
    for (Diagnostic problem : schema.validate(document))                    // a ready tree
        System.out.println(problem.line() + ": " + problem.message());
    List<Diagnostic> export = schema.validateFile(Path.of("export.xml"));   // a stream without a tree, even gigabytes
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
java --enable-native-access=ALL-UNNAMED -cp beresta-2.1.3.jar:app.jar com.example.App
```

How it works:
- `NativeLoader` detects the platform by `os.name` and `os.arch`, extracts `natives/<os>-<arch>/<file>` from the jar into a temporary directory and loads it through `SymbolLookup.libraryLookup`. The path can be set explicitly: `-Dberesta.library.path=/path/libberesta.so`.
- Call handles (`MethodHandle`) are built once from a `FunctionDescriptor`, strings are passed through `Arena.ofConfined()` and freed right after the call.
- `XmlDocument`, `XPath`, `XmlSchema` implement `AutoCloseable`. If an object was not closed, a `Cleaner` frees the native memory.
- A node keeps a reference to its document. After `close()` accessing a node gives an `IllegalStateException`, not a JVM crash.
- The node methods are split over interfaces (`NodeContent`, `NodeNavigation`, `NodeChildren`, `NodeEditing`, `NodeStructure`, `ElementAttributes`), schema validation is in `SchemaValidation`: a method can accept exactly the part of the API it needs.
- The hot walk calls (navigation, the node type, a child by index, the nodes of an XPath result) go through `invokeExact` with an exact type, the others through the common `invokeWithArguments`. The JIT inlines `invokeExact` almost like a direct call of a C function, so walking the tree from Java became 1.4–1.7 times faster.

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
    using XmlDocument document = Xml.ParseFile("order.xml", ParseFlags.TrackLocations);
    foreach (Diagnostic problem in schema.Validate(document))               // a ready tree
        Console.WriteLine(problem.Line + ": " + problem.Message);
    var export = schema.ValidateFile("export.xml");                         // a stream without a tree, even gigabytes
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
    with beresta.parse_file("order.xml", ParseFlags.TRACK_LOCATIONS) as document:
        for problem in schema.validate(document):                     # a ready tree
            print(problem.line, problem.message)
        prices = XPath("//item/@price").select_values(document)      # strings without node objects
    export = schema.validate_file("export.xml")                       # a stream without a tree, even gigabytes
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
- The wrapper remembers the node type and the wrapper classes, so a walk step makes fewer ctypes calls, and a step-by-step walk became 1.6 times faster. But the fastest way is not to walk the tree from Python at all: `select_values` assembles the strings inside the core.
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
const exported = schema.validateFile('export.xml');                         // a stream without a tree

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
- The addon (`addon/`, C++17) is 31 signature adapters for 68 C interface functions (the Adapter and Command patterns). It turns error codes into `BerestaError` exceptions with a code (`PARSE_ERROR`) and a text in the current language itself.
- Handles are `BigInt`s: an address does not have to fit into the 53 bits of a number, and equal addresses are equal by `===`. Two wrappers of one node are compared through `node.isSameNode(other)`.
- Parsing a `Buffer` or a `Uint8Array` goes without copying the bytes; a string is encoded into UTF-8.
- `XmlDocument`, `XPath` and `Schema` are closed through `close()` or `using` (`Symbol.dispose`). If you forget, `FinalizationRegistry` frees the memory.
- The addon data is kept separately for every environment (`napi_set_instance_data`), so the package works in `worker_threads` too.
- The addon is looked up in `BERESTA_NODE_ADDON`, then in `native/<os>-<arch>/beresta.node`.
- A TypeScript example runs without a compiler (Node.js 23.6+): `node bindings/node/examples/demo.mts`.
- The package from npm contains the addons of all platforms (see [Ready packages and releases](building.en.md#ready-packages-and-releases)); for browsers there is `beresta-wasm` with the same API.

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
const problems = schema.validateBytes(fileBytes); // a stream without a tree: a Uint8Array from <input type="file">
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
