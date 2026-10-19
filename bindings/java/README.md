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

Подключение из Maven Central (Java-пакет — `org.beresta`):

```xml
<dependency>
    <groupId>io.github.ermaleyalekov</groupId>
    <artifactId>beresta</artifactId>
    <version>2.1.1</version>
</dependency>
```

Gradle: `implementation("io.github.ermaleyalekov:beresta:2.1.1")`.

Тот же jar прикладывается к каждому [релизу на GitHub](https://github.com/ErmaleyAlekov/beresta/releases) и уже содержит библиотеку для Windows (x64, arm64), Linux (x64, arm64; glibc 2.35+) и macOS (универсальная); загрузчик сам извлечёт нужную. Запускайте приложение с ключом `--enable-native-access=ALL-UNNAMED`.
Сборка из исходников: `make JAVA=1` или CMake с `-DBERESTA_BUILD_JAVA=ON` (библиотека попадёт в `src/main/resources/natives/<os>-<arch>/`), затем `mvn package`.

**Как получить скорость C++.** Каждый шаг ручного обхода (`firstChild`, `nextSibling`) — вызов через Panama и объект-обёртка. Тяжёлую работу отдавайте ядру целиком:

```java
try (XmlSchema schema = XmlSchema.fromFile(Path.of("export.xsd"));
     XPath prices = new XPath("//item/@price"))
{
    List<Diagnostic> problems = schema.validateFile(Path.of("export.xml"));   // поток без дерева, хоть гигабайты
    List<String> values = prices.selectValues(document);                      // строки без объектов-узлов
}
```

- XPath в 1,4–1,8 раза быстрее, чем в JDK, а `XPath` и `XmlSchema` можно делить между потоками.
- Потоковая проверка `validateFile` быстрее `javax.xml.validation.Validator` (97 против 90 МБ/с) и не строит дерево.
- Дерево на 28 % компактнее, чем JDK DOM (88 против 123 МБ на каталоге 13 МБ).
- Безопасность по умолчанию: стандартный `DocumentBuilder` загружает внешние сущности (XXE), пока его явно не настроить, а здесь ничего внешнего без разрешения не грузится.
- Строка и столбец нарушений в `schema.validate(document)` — с флагом `ParseFlags.TRACK_LOCATIONS` при разборе.

---

# Beresta for Java

A wrapper over the Beresta C interface through Project Panama (the Foreign Function & Memory API, Java 22+).
Neither JNI nor third-party dependencies.

Getting it from Maven Central (the Java package is `org.beresta`):

```xml
<dependency>
    <groupId>io.github.ermaleyalekov</groupId>
    <artifactId>beresta</artifactId>
    <version>2.1.1</version>
</dependency>
```

Gradle: `implementation("io.github.ermaleyalekov:beresta:2.1.1")`.

The same jar is attached to every [GitHub release](https://github.com/ErmaleyAlekov/beresta/releases) and already contains the library for Windows (x64, arm64), Linux (x64, arm64; glibc 2.35+) and macOS (universal); the loader extracts the right one itself. Run the application with the `--enable-native-access=ALL-UNNAMED` flag.
Building from source: `make JAVA=1` or CMake with `-DBERESTA_BUILD_JAVA=ON` (the library lands in `src/main/resources/natives/<os>-<arch>/`), then `mvn package`.

**How to get C++ speed.** Every step of a manual walk (`firstChild`, `nextSibling`) is a call through Panama and a wrapper object. Hand heavy work over to the core entirely:

```java
try (XmlSchema schema = XmlSchema.fromFile(Path.of("export.xsd"));
     XPath prices = new XPath("//item/@price"))
{
    List<Diagnostic> problems = schema.validateFile(Path.of("export.xml"));   // a stream without a tree, even gigabytes
    List<String> values = prices.selectValues(document);                      // strings without node objects
}
```

- XPath is 1.4–1.8 times faster than in the JDK, and `XPath` and `XmlSchema` can be shared between threads.
- Streaming validation with `validateFile` is faster than `javax.xml.validation.Validator` (97 against 90 MB/s) and builds no tree.
- The tree is 28 % more compact than the JDK DOM (88 against 123 MB on a 13 MB catalog).
- Security by default: the standard `DocumentBuilder` loads external entities (XXE) until it is explicitly configured not to, while here nothing external is loaded without permission.
- The line and column of violations in `schema.validate(document)` come with the `ParseFlags.TRACK_LOCATIONS` parse flag.
