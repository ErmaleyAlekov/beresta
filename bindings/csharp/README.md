# Берёста для .NET

Обёртка над Си-интерфейсом «Берёсты» через P/Invoke. Зависимостей нет.

```bash
dotnet add package Beresta
```

```csharp
using Beresta;

using XmlDocument document = Xml.Parse("<shop><item code='A1'>Чай</item></shop>");
XmlElement item = (XmlElement)document.Root!.FirstChild!;
Console.WriteLine(item.GetAttribute("code") + " " + item.Text);
```

Пакет NuGet уже содержит библиотеку для Windows (x64, arm64), Linux (x64, arm64; glibc 2.35+) и macOS (универсальная) в стандартной раскладке `runtimes/<rid>/native` — .NET выберет нужную сам. Сборка из исходников: `make CSHARP=1` или CMake с `-DBERESTA_BUILD_CSHARP=ON`, затем `dotnet pack Beresta/Beresta.csproj -c Release`.

**Как получить скорость C++.** Каждый шаг ручного обхода (`FirstChild`, `NextSibling`) — вызов через P/Invoke и объект-обёртка. Тяжёлую работу отдавайте ядру целиком:

```csharp
using XmlSchema schema = XmlSchema.FromFile("export.xsd");
var problems = schema.ValidateFile("export.xml");                      // поток без дерева, хоть гигабайты
using XPath prices = new XPath("//item/@price");
IReadOnlyList<string> values = prices.SelectValues(document);          // строки без объектов-узлов
```

- Разбор с проверкой по XSD в 2,3 раза быстрее `XmlDocument` (64 против 28 МБ/с) и в 1,5 раза быстрее `XDocument`.
- Потоковая проверка `ValidateFile` быстрее проверяющего `XmlReader` (94 против 85 МБ/с) и не строит дерево.
- `XPath` и `XmlSchema` можно делить между потоками.
- Строка и столбец нарушений в `schema.Validate(document)` — с флагом `ParseFlags.TrackLocations` при разборе.

---

# Beresta for .NET

A wrapper over the Beresta C interface through P/Invoke. No dependencies.

```bash
dotnet add package Beresta
```

The NuGet package already contains the library for Windows (x64, arm64), Linux (x64, arm64; glibc 2.35+) and macOS (universal) in the standard `runtimes/<rid>/native` layout — .NET picks the right one itself. Building from source: `make CSHARP=1` or CMake with `-DBERESTA_BUILD_CSHARP=ON`, then `dotnet pack Beresta/Beresta.csproj -c Release`.

**How to get C++ speed.** Every step of a manual walk (`FirstChild`, `NextSibling`) is a call through P/Invoke and a wrapper object. Hand heavy work over to the core entirely:

```csharp
using XmlSchema schema = XmlSchema.FromFile("export.xsd");
var problems = schema.ValidateFile("export.xml");                      // a stream without a tree, even gigabytes
using XPath prices = new XPath("//item/@price");
IReadOnlyList<string> values = prices.SelectValues(document);          // strings without node objects
```

- Parsing with XSD validation is 2.3 times faster than `XmlDocument` (64 against 28 MB/s) and 1.5 times faster than `XDocument`.
- Streaming validation with `ValidateFile` is faster than a validating `XmlReader` (94 against 85 MB/s) and builds no tree.
- `XPath` and `XmlSchema` can be shared between threads.
- The line and column of violations in `schema.Validate(document)` come with the `ParseFlags.TrackLocations` parse flag.
