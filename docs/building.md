# Берёста — сборка, подключение и готовые пакеты

**Русский** · [English](building.en.md)

← [К описанию и преимуществам](../README.md)

## Содержание

1. [Сборка и подключение](#сборка-и-подключение)
2. [Готовые пакеты и релизы](#готовые-пакеты-и-релизы)

---

## Сборка и подключение

Нужен компилятор с поддержкой **C++17**. Есть два независимых способа сборки — CMake и Makefile; оба работают на Windows, Linux и macOS.

Что получается в результате:

| Файл | Для кого |
|---|---|
| `beresta.dll` / `libberesta.so` / `libberesta.dylib` | динамическая библиотека с Си-интерфейсом: C, Java, C#, Python |
| `libberesta.a` (Makefile) / `beresta_static` (CMake) | статическая библиотека для программ на C++ (полный C++ API) |
| `beresta.node` | аддон Node.js: переходники Node-API + ядро в одном файле |
| `beresta_core.wasm` + `beresta.mjs` | WebAssembly для браузеров, Workers и Node.js |

Из динамической библиотеки экспортируются **только** функции Си-интерфейса. Программы на C++ подключают статическую библиотеку.

### Makefile (GCC / Clang / MinGW)

```bash
make
```

| Цель | Что делает |
|---|---|
| `make` / `make all` | `bin/libberesta.a`, динамическая библиотека, демо `bin/beresta_demo` (C++) и `bin/beresta_capi_demo` (C) |
| `make test` | собирает и запускает модульные тесты C++ |
| `make JAVA=1` | то же + бинарник в ресурсы Maven-проекта |
| `make CSHARP=1` | то же + бинарник в `runtimes/<rid>/native` проекта .NET |
| `make PYTHON=1` | то же + бинарник в пакет Python |
| `make NODE=1` | то же + аддон `beresta.node` в пакет Node.js |
| `make bindings-test` | самопроверки включённых обёрток, например `make JAVA=1 CSHARP=1 PYTHON=1 NODE=1 bindings-test` |
| `make java-test`, `csharp-test`, `python-test`, `node-test` | самопроверка одной обёртки (копирует бинарник сама) |
| `make java-package` | jar через Maven (`mvn package`) |
| `make csharp-package` | пакет NuGet (`dotnet pack`) |
| `make node-package` | пакет npm (`npm pack`) |
| `make ERRORS_LANG=en` | язык сообщений об ошибках по умолчанию (`ru`, если не задан); см. [Языки сообщений](guide.md#языки-сообщений) |
| `make universal` | macOS: универсальные `libberesta.a`, `libberesta.dylib` и `beresta.node` (x86_64 + arm64, склейка `lipo`) с раскладкой по папкам обеих архитектур |
| `make wasm` | WebAssembly: ядро через `em++` (Emscripten) в `bindings/wasm/dist/` и сборка `beresta.mjs` |
| `make wasm-test` | `make wasm` + самопроверка пакета WebAssembly в Node.js |
| `make platform` | показать определённые ОС, архитектуру и пути копирования |
| `make clean` | удаляет `obj/` и `dep/` |
| `make fclean` | `clean` + `bin/`, копии бинарника в обёртках, `bindings/wasm/dist/` и `bindings/java/target/` |
| `make re` | полная пересборка |

Особенности Makefile:
- `make -j` поддерживается, компилятор можно сменить (`make CXX=clang++ CC=clang`).
- Объектные и `.d`-файлы раскладываются по `obj/` и `dep/` зеркально `src/`. Изменённый заголовок пересобирает только зависимые файлы, а изменённый Makefile (например, флаги) — все объекты.
- Очистка сама определяет оболочку: `rmdir /S /Q` в `cmd.exe` на Windows и `rm -rf` в `sh` на Linux и macOS.
- Архитектура берётся у самого компилятора (`$(CXX) -dumpmachine`), поэтому при кросс-компиляции бинарник попадает в папку целевой платформы.
- Сборки для другой архитектуры и для WebAssembly идут отдельным запуском make со своими `obj/<arch>`, `dep/<arch>` и `bin/<arch>` и не смешиваются с обычной. На macOS код собирается для версии не ниже `MACOS_MIN` (по умолчанию 11.0 — первая с Apple Silicon).

### CMake (MSVC / MinGW / GCC / Clang)

По умолчанию CMake собирает **только ядро** — динамическую библиотеку:

```bash
cmake -S . -B build
```
```bash
cmake --build build --config Release
```

Всё остальное включается флагами:

| Опция | По умолчанию | Что делает |
|---|---|---|
| `BERESTA_BUILD_STATIC` | `OFF` | статическая библиотека `beresta_static` (`beresta::static`) для C++ |
| `BERESTA_BUILD_EXAMPLES` | `OFF` | демо на C++ и на чистом C |
| `BERESTA_BUILD_TESTS` | `OFF` | модульные тесты и самопроверки включённых обёрток в `ctest` |
| `BERESTA_BUILD_JAVA` | `OFF` | после сборки бинарник копируется в ресурсы Maven-проекта; цель `beresta_java_package` вызывает `mvn package` |
| `BERESTA_BUILD_CSHARP` | `OFF` | бинарник в `runtimes/<rid>/native`; цель `beresta_csharp_package` вызывает `dotnet pack` |
| `BERESTA_BUILD_PYTHON` | `OFF` | бинарник в пакет Python |
| `BERESTA_BUILD_NODE` | `OFF` | аддон `beresta.node` в пакет Node.js; цель `beresta_node_package` вызывает `npm pack` |
| `BERESTA_ERRORS_LANG` | `ru` | язык сообщений об ошибках по умолчанию: `ru`, `en` или любой добавленный |
| `BERESTA_STATIC_RUNTIME` | `ON` | встроить C++ runtime в библиотеку (`/MT` для MSVC, `-static` для MinGW, `-static-libstdc++` для GCC) |

Полная сборка со всеми обёртками и тестами:

```bash
cmake -S . -B build -DBERESTA_BUILD_TESTS=ON -DBERESTA_BUILD_JAVA=ON -DBERESTA_BUILD_CSHARP=ON -DBERESTA_BUILD_PYTHON=ON -DBERESTA_BUILD_NODE=ON
```
```bash
cmake --build build --config Release
```
```bash
ctest --test-dir build -C Release --output-on-failure
```

Для MSVC автоматически включаются `/utf-8 /W4 /permissive-`. Без явного `CMAKE_BUILD_TYPE` однотипные генераторы (Make, Ninja) собирают `Release`.

Универсальный бинарник macOS и WebAssembly:

```bash
cmake -S . -B build -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
```
```bash
emcmake cmake -S . -B build-wasm -DBERESTA_BUILD_TESTS=ON
```

Под Emscripten CMake собирает только модуль WebAssembly (`cmake/BerestaWasm.cmake`): динамическая библиотека и обёртки в браузере не нужны.

### Куда копируется бинарник

CMake (модуль `cmake/BerestaPlatform.cmake`) и Makefile определяют **целевую** ОС и архитектуру и приводят имена к единому виду. Те же имена ищут загрузчики обёрток:

| Обёртка | Папка |
|---|---|
| Java | `bindings/java/src/main/resources/natives/<os>-<arch>/` |
| C# | `bindings/csharp/Beresta/runtimes/<rid>/native/` |
| Python | `bindings/python/beresta/_native/<os>-<arch>/` |
| Node.js | `bindings/node/native/<os>-<arch>/beresta.node` |

- `<os>-<arch>`: `windows-x86_64`, `windows-aarch64`, `linux-x86_64`, `linux-aarch64`, `macos-x86_64`, `macos-aarch64`, `*-x86`.
- `<rid>` (идентификатор среды .NET): `win-x64`, `win-arm64`, `linux-x64`, `linux-arm64`, `osx-x64`, `osx-arm64`.
- Источник архитектуры в CMake: `MSVC_CXX_ARCHITECTURE_ID` для MSVC (верно и для `-A ARM64`), `CMAKE_OSX_ARCHITECTURES` на macOS, иначе `CMAKE_SYSTEM_PROCESSOR`. Значения можно задать вручную: `-DBERESTA_PLATFORM_ARCH=aarch64`.
- Универсальный бинарник macOS (`make universal` или две архитектуры в `CMAKE_OSX_ARCHITECTURES`) кладётся сразу в папки `macos-x86_64` и `macos-aarch64` (`osx-x64` и `osx-arm64` для .NET): загрузчики находят его на любом Mac.
- Один jar, пакет NuGet, колесо Python или пакет npm для всех платформ собирает релизный CI: каждая ОС добавляет свою папку, а упаковка идёт один раз (см. [Готовые пакеты и релизы](#готовые-пакеты-и-релизы)).

### Подключение к своему проекту на C++

```cmake
set(BERESTA_BUILD_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/beresta)
target_link_libraries(my_app PRIVATE beresta::static)
```

Цель `beresta::beresta` — динамическая библиотека. Её подключают программы на C (через `capi/beresta_c.h`).

Или вручную:

```bash
g++ -std=c++17 -Ipath/to/beresta/include main.cpp path/to/beresta/bin/libberesta.a -o app
```

В коде достаточно одного заголовка: `#include "beresta.hpp"`.

---

## Готовые пакеты и релизы

Пользователю не нужно компилировать ядро: пакеты уже содержат бинарники для всех платформ.

| Язык | Установка | Что внутри |
|---|---|---|
| Node.js | `npm install beresta` | аддоны для Windows и Linux (x64, arm64) и универсальный для macOS |
| Браузер, Workers | `npm install beresta-wasm` | `beresta_core.wasm` + ES-модуль с типами TypeScript |
| Python | `pip install beresta` | одно колесо `py3-none-any` с библиотеками всех платформ |
| .NET | `dotnet add package Beresta` | `runtimes/<rid>/native` для `win-x64`, `win-arm64`, `linux-x64`, `linux-arm64`, `osx-x64`, `osx-arm64` |
| Java | Maven Central: `io.github.ermaleyalekov:beresta` (зависимость ниже) или jar из [релиза на GitHub](https://github.com/ErmaleyAlekov/beresta/releases) | `natives/<os>-<arch>/` всех платформ, загрузчик извлекает нужную |
| C и C++ | архив SDK `beresta-<версия>-<платформа>.zip` из релиза | `include/`, динамическая и статическая библиотеки, `LICENSE` |

Linux-бинарники собираются на Ubuntu 22.04 и работают с glibc 2.35 и новее.

Зависимость Maven (Java-пакет остаётся `org.beresta`):

```xml
<dependency>
    <groupId>io.github.ermaleyalekov</groupId>
    <artifactId>beresta</artifactId>
    <version>2.1.1</version>
</dependency>
```

Gradle: `implementation("io.github.ermaleyalekov:beresta:2.1.1")`.

### CI/CD в GitHub Actions

- **`.github/workflows/ci.yml`** — на каждый push и pull request: Linux x64 и arm64 (Makefile с GCC, CMake с Clang, все обёртки), Windows x64 и arm64 (MSVC, `ctest`), macOS (`make universal`, проверка `lipo`, тесты C++ для arm64 и для x86_64 через Rosetta), WebAssembly (официальный образ `emscripten/emsdk`), санитайзеры ASan/UBSan/LSan и W3C XML Conformance Test Suite (прогон падает, если хотя бы один применимый случай не прошёл).
- **`.github/workflows/release.yml`** — по тегу `vX.Y.Z`: проверяет, что версия тега совпадает с версиями в `CMakeLists.txt`, `package.json` и `pyproject.toml`; собирает нативные бинарники на пяти платформах; сводит их в одно дерево; собирает пакеты npm (`beresta`, `beresta-wasm`), колесо Python, NuGet, jar (с исходниками, javadoc и подписью GPG для Maven Central) и архивы SDK; ставит колесо и пакет npm в чистое окружение и проверяет, что они работают; создаёт GitHub Release со всеми файлами.

Публикация в реестры включается секретами репозитория (Settings → Secrets and variables → Actions). Пока секрета нет, шаг пропускается, а пакеты всё равно лежат в GitHub Release:

| Секрет | Куда |
|---|---|
| `NPM_TOKEN` | npm (`beresta`, `beresta-wasm`, с provenance) |
| `PYPI_API_TOKEN` | PyPI (`beresta`) |
| `NUGET_API_KEY` | NuGet (`Beresta`) |
| `MAVEN_CENTRAL_USERNAME`, `MAVEN_CENTRAL_PASSWORD` | Maven Central (`io.github.ermaleyalekov:beresta`): токен пользователя Central Portal |
| `MAVEN_GPG_PRIVATE_KEY`, `MAVEN_GPG_PASSPHRASE` | подпись артефактов Maven: закрытый ключ GPG (ASCII armor) и его пароль |

Выпуск версии: поднимите версию в `CMakeLists.txt`, `include/capi/beresta_c.h`, `bindings/*/package.json`, `bindings/python/pyproject.toml`, `bindings/java/pom.xml` и `bindings/csharp/Beresta/Beresta.csproj`, затем:

```bash
git tag v2.1.1
```
```bash
git push origin v2.1.1
```
