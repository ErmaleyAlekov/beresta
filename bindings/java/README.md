# Берёста для Java

Обёртка над Си-интерфейсом «Берёсты» через Project Panama (Foreign Function & Memory API, Java 22+).
Ни JNI, ни сторонних зависимостей.

```java
import org.beresta.*;

try (XmlDocument document = Xml.parse("<shop><item code='A1'>Чай</item></shop>"))
{
    XmlElement item = (XmlElement) document.root().firstChild();
    System.out.println(item.getAttribute("code") + " " + item.text());
}
```

Готовый jar прикладывается к каждому [релизу на GitHub](https://github.com/ErmaleyAlekov/beresta/releases) и уже содержит библиотеку для Windows (x64, arm64), Linux (x64, arm64; glibc 2.35+) и macOS (универсальная); загрузчик сам извлечёт нужную. Запускайте приложение с ключом `--enable-native-access=ALL-UNNAMED`.
Сборка из исходников: `make JAVA=1` или CMake с `-DBERESTA_BUILD_JAVA=ON` (библиотека попадёт в `src/main/resources/natives/<os>-<arch>/`), затем `mvn package`.

---

# Beresta for Java

A wrapper over the Beresta C interface through Project Panama (the Foreign Function & Memory API, Java 22+).
Neither JNI nor third-party dependencies.

A ready jar is attached to every [GitHub release](https://github.com/ErmaleyAlekov/beresta/releases) and already contains the library for Windows (x64, arm64), Linux (x64, arm64; glibc 2.35+) and macOS (universal); the loader extracts the right one itself. Run the application with the `--enable-native-access=ALL-UNNAMED` flag.
Building from source: `make JAVA=1` or CMake with `-DBERESTA_BUILD_JAVA=ON` (the library lands in `src/main/resources/natives/<os>-<arch>/`), then `mvn package`.
