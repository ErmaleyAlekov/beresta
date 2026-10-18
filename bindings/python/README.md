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

---

# Beresta for Python

A wrapper over the Beresta C interface through the standard `ctypes` module: DOM, XPath 1.0, XML Schema with `xs:key`/`xs:keyref`, DTD, eight encodings. No dependencies.

```bash
pip install beresta
```

The wheel from PyPI is one for all platforms and already contains the library for Windows (x64, arm64), Linux (x64, arm64; glibc 2.35+) and macOS (universal: Intel and Apple Silicon) — no compiler is needed. Building from source: `make PYTHON=1` or `cmake -DBERESTA_BUILD_PYTHON=ON` in the repository root — the binary lands in `beresta/_native/<os>-<arch>/`.
