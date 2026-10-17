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

Нативная библиотека собирается командой `make JAVA=1` или CMake с опцией `-DBERESTA_BUILD_JAVA=ON`
и попадает в `src/main/resources/natives/<os>-<arch>/`; затем `mvn package`.
Запускайте приложение с ключом `--enable-native-access=ALL-UNNAMED`.
Подробности — в README.md в корне репозитория.
