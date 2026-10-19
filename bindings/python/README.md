# Берёста для Python

Обёртка над Си-интерфейсом «Берёсты» через стандартный модуль `ctypes`: DOM, XPath 1.0, XML Schema с `xs:key`/`xs:keyref`, DTD, восемь кодировок. Зависимостей нет.

```bash
pip install beresta
```

```python
import beresta

with beresta.parse("<shop><item code='A1'>Чай</item></shop>") as document:
    item = document.root.first_child
    print(item.get_attribute("code"), item.text)
```

Колесо из PyPI одно на все платформы и уже содержит библиотеку для Windows (x64, arm64), Linux (x64, arm64; glibc 2.35+) и macOS (универсальная: Intel и Apple Silicon) — компилятор не нужен. Сборка из исходников: `make PYTHON=1` или `cmake -DBERESTA_BUILD_PYTHON=ON` в корне репозитория — бинарник окажется в `beresta/_native/<os>-<arch>/`.

**Как получить скорость C++.** Каждый шаг ручного обхода (`first_child`, `next_sibling`) — вызов через ctypes и новый объект-обёртка: миллион таких шагов измеряет скорость ctypes, а не парсера. Тяжёлую работу отдавайте ядру целиком:

```python
from beresta import Schema, XPath

schema = Schema.from_file("export.xsd")
problems = schema.validate_file("export.xml")               # поток без дерева, хоть гигабайты
prices = XPath("//item/@price").select_values(document)     # строки без объектов-узлов
```

- `XPath` компилируется один раз и вычисляется сколько угодно раз, в том числе из разных потоков: ctypes отпускает GIL на время вызова.
- Потоковая проверка (`validate_file`, `validate_bytes`) почти вдвое быстрее lxml `iterparse` со схемой (93 против 49 МБ/с), а файл 135 МБ проверяется в 37 МБ собственной памяти процесса.
- Дерево на 28 % компактнее, чем у lxml (86 против 119 МБ на каталоге 13 МБ).
- Защита от сущностных бомб и XXE включена в ядре: настраивать ничего не нужно.
- Строка и столбец нарушений в `schema.validate(document)` — с флагом `ParseFlags.TRACK_LOCATIONS` при разборе.

---

# Beresta for Python

A wrapper over the Beresta C interface through the standard `ctypes` module: DOM, XPath 1.0, XML Schema with `xs:key`/`xs:keyref`, DTD, eight encodings. No dependencies.

```bash
pip install beresta
```

The wheel from PyPI is one for all platforms and already contains the library for Windows (x64, arm64), Linux (x64, arm64; glibc 2.35+) and macOS (universal: Intel and Apple Silicon) — no compiler is needed. Building from source: `make PYTHON=1` or `cmake -DBERESTA_BUILD_PYTHON=ON` in the repository root — the binary lands in `beresta/_native/<os>-<arch>/`.

**How to get C++ speed.** Every step of a manual walk (`first_child`, `next_sibling`) is a call through ctypes and a new wrapper object: a million such steps measure the speed of ctypes, not of the parser. Hand heavy work over to the core entirely:

```python
from beresta import Schema, XPath

schema = Schema.from_file("export.xsd")
problems = schema.validate_file("export.xml")               # a stream without a tree, even gigabytes
prices = XPath("//item/@price").select_values(document)     # strings without node objects
```

- An `XPath` is compiled once and evaluated any number of times, including from different threads: ctypes releases the GIL for the call time.
- Streaming validation (`validate_file`, `validate_bytes`) is almost twice as fast as lxml `iterparse` with a schema (93 against 49 MB/s), and a 135 MB file is validated in 37 MB of the process's own memory.
- The tree is 28 % more compact than lxml's (86 against 119 MB on a 13 MB catalog).
- The protection against entity bombs and XXE is on in the core: nothing needs to be configured.
- The line and column of violations in `schema.validate(document)` come with the `ParseFlags.TRACK_LOCATIONS` parse flag.
