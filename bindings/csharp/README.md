# Берёста для .NET

Обёртка над Си-интерфейсом «Берёсты» через P/Invoke. Зависимостей нет.

```csharp
using Beresta;

using XmlDocument document = Xml.Parse("<shop><item code='A1'>Чай</item></shop>");
XmlElement item = (XmlElement)document.Root!.FirstChild!;
Console.WriteLine(item.GetAttribute("code") + " " + item.Text);
```

Нативная библиотека собирается командой `make CSHARP=1` или CMake с опцией
`-DBERESTA_BUILD_CSHARP=ON` и попадает в `Beresta/runtimes/<rid>/native/`.
Пакет NuGet: `dotnet pack Beresta/Beresta.csproj -c Release`.
Подробности — в README.md в корне репозитория.
