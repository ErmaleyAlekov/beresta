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

---

# Beresta for .NET

A wrapper over the Beresta C interface through P/Invoke. No dependencies.

```bash
dotnet add package Beresta
```

The NuGet package already contains the library for Windows (x64, arm64), Linux (x64, arm64; glibc 2.35+) and macOS (universal) in the standard `runtimes/<rid>/native` layout — .NET picks the right one itself. Building from source: `make CSHARP=1` or CMake with `-DBERESTA_BUILD_CSHARP=ON`, then `dotnet pack Beresta/Beresta.csproj -c Release`.
