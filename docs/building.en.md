# Beresta — building, linking and ready packages

[Русский](building.md) · **English**

← [To the description and advantages](../README.en.md)

## Contents

1. [Building and linking](#building-and-linking)
2. [Ready packages and releases](#ready-packages-and-releases)

---

## Building and linking

A compiler with **C++17** support is required. There are two independent ways to build — CMake and the Makefile; both work on Windows, Linux and macOS.

What you get:

| File | For whom |
|---|---|
| `beresta.dll` / `libberesta.so` / `libberesta.dylib` | the shared library with the C interface: C, Java, C#, Python |
| `libberesta.a` (Makefile) / `beresta_static` (CMake) | the static library for C++ programs (the full C++ API) |
| `beresta.node` | the Node.js addon: the Node-API adapters + the core in one file |
| `beresta_core.wasm` + `beresta.mjs` | WebAssembly for browsers, Workers and Node.js |

**Only** the C interface functions are exported from the shared library. C++ programs link the static library.

### Makefile (GCC / Clang / MinGW)

```bash
make
```

| Target | What it does |
|---|---|
| `make` / `make all` | `bin/libberesta.a`, the shared library, the demos `bin/beresta_demo` (C++) and `bin/beresta_capi_demo` (C) |
| `make test` | builds and runs the C++ unit tests |
| `make JAVA=1` | the same + the binary into the Maven project resources |
| `make CSHARP=1` | the same + the binary into `runtimes/<rid>/native` of the .NET project |
| `make PYTHON=1` | the same + the binary into the Python package |
| `make NODE=1` | the same + the `beresta.node` addon into the Node.js package |
| `make bindings-test` | self-tests of the enabled wrappers, for example `make JAVA=1 CSHARP=1 PYTHON=1 NODE=1 bindings-test` |
| `make java-test`, `csharp-test`, `python-test`, `node-test` | the self-test of one wrapper (copies the binary itself) |
| `make java-package` | a jar through Maven (`mvn package`) |
| `make csharp-package` | a NuGet package (`dotnet pack`) |
| `make node-package` | an npm package (`npm pack`) |
| `make universal` | macOS: universal `libberesta.a`, `libberesta.dylib` and `beresta.node` (x86_64 + arm64, glued by `lipo`) laid out into the folders of both architectures |
| `make wasm` | WebAssembly: the core through `em++` (Emscripten) into `bindings/wasm/dist/` and the `beresta.mjs` bundle |
| `make wasm-test` | `make wasm` + the self-test of the WebAssembly package in Node.js |
| `make ERRORS_LANG=en` | the default error message language (`ru` if not set); see [Message languages](guide.en.md#message-languages) |
| `make platform` | show the detected OS, architecture and copy paths |
| `make clean` | removes `obj/` and `dep/` |
| `make fclean` | `clean` + `bin/`, the binary copies in the wrappers, `bindings/wasm/dist/` and `bindings/java/target/` |
| `make re` | a full rebuild |

Makefile specifics:
- `make -j` is supported, the compiler can be changed (`make CXX=clang++ CC=clang`).
- Object and `.d` files are laid out into `obj/` and `dep/` mirroring `src/`. A changed header rebuilds only the dependent files, and a changed Makefile (for example, the flags) rebuilds all objects.
- Cleaning detects the shell itself: `rmdir /S /Q` in `cmd.exe` on Windows and `rm -rf` in `sh` on Linux and macOS.
- The architecture is taken from the compiler itself (`$(CXX) -dumpmachine`), so with cross-compilation the binary goes into the folder of the target platform.
- Builds for another architecture and for WebAssembly run as a separate make invocation with their own `obj/<arch>`, `dep/<arch>` and `bin/<arch>` and do not mix with the regular one. On macOS the code targets a version not lower than `MACOS_MIN` (11.0 by default — the first one with Apple Silicon).

### CMake (MSVC / MinGW / GCC / Clang)

By default CMake builds **only the core** — the shared library:

```bash
cmake -S . -B build
```
```bash
cmake --build build --config Release
```

Everything else is enabled with flags:

| Option | Default | What it does |
|---|---|---|
| `BERESTA_BUILD_STATIC` | `OFF` | the static library `beresta_static` (`beresta::static`) for C++ |
| `BERESTA_BUILD_EXAMPLES` | `OFF` | the demos in C++ and in pure C |
| `BERESTA_BUILD_TESTS` | `OFF` | the unit tests and the self-tests of the enabled wrappers in `ctest` |
| `BERESTA_BUILD_JAVA` | `OFF` | after the build the binary is copied into the Maven project resources; the `beresta_java_package` target runs `mvn package` |
| `BERESTA_BUILD_CSHARP` | `OFF` | the binary into `runtimes/<rid>/native`; the `beresta_csharp_package` target runs `dotnet pack` |
| `BERESTA_BUILD_PYTHON` | `OFF` | the binary into the Python package |
| `BERESTA_BUILD_NODE` | `OFF` | the `beresta.node` addon into the Node.js package; the `beresta_node_package` target runs `npm pack` |
| `BERESTA_ERRORS_LANG` | `ru` | the default error message language: `ru`, `en` or any added one |
| `BERESTA_STATIC_RUNTIME` | `ON` | embed the C++ runtime into the library (`/MT` for MSVC, `-static` for MinGW, `-static-libstdc++` for GCC) |

A full build with all wrappers and tests:

```bash
cmake -S . -B build -DBERESTA_BUILD_TESTS=ON -DBERESTA_BUILD_JAVA=ON -DBERESTA_BUILD_CSHARP=ON -DBERESTA_BUILD_PYTHON=ON -DBERESTA_BUILD_NODE=ON
```
```bash
cmake --build build --config Release
```
```bash
ctest --test-dir build -C Release --output-on-failure
```

For MSVC `/utf-8 /W4 /permissive-` are enabled automatically. Without an explicit `CMAKE_BUILD_TYPE` single-config generators (Make, Ninja) build `Release`.

A universal macOS binary and WebAssembly:

```bash
cmake -S . -B build -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
```
```bash
emcmake cmake -S . -B build-wasm -DBERESTA_BUILD_TESTS=ON
```

Under Emscripten CMake builds only the WebAssembly module (`cmake/BerestaWasm.cmake`): the shared library and the wrappers are not needed in a browser.

### Where the binary is copied

CMake (the `cmake/BerestaPlatform.cmake` module) and the Makefile detect the **target** OS and architecture and normalize the names. The wrapper loaders look for the same names:

| Wrapper | Folder |
|---|---|
| Java | `bindings/java/src/main/resources/natives/<os>-<arch>/` |
| C# | `bindings/csharp/Beresta/runtimes/<rid>/native/` |
| Python | `bindings/python/beresta/_native/<os>-<arch>/` |
| Node.js | `bindings/node/native/<os>-<arch>/beresta.node` |

- `<os>-<arch>`: `windows-x86_64`, `windows-aarch64`, `linux-x86_64`, `linux-aarch64`, `macos-x86_64`, `macos-aarch64`, `*-x86`.
- `<rid>` (the .NET runtime identifier): `win-x64`, `win-arm64`, `linux-x64`, `linux-arm64`, `osx-x64`, `osx-arm64`.
- The architecture source in CMake: `MSVC_CXX_ARCHITECTURE_ID` for MSVC (correct for `-A ARM64` too), `CMAKE_OSX_ARCHITECTURES` on macOS, otherwise `CMAKE_SYSTEM_PROCESSOR`. The values can be set manually: `-DBERESTA_PLATFORM_ARCH=aarch64`.
- A universal macOS binary (`make universal` or two architectures in `CMAKE_OSX_ARCHITECTURES`) goes into both `macos-x86_64` and `macos-aarch64` (`osx-x64` and `osx-arm64` for .NET): the loaders find it on any Mac.
- One jar, NuGet package, Python wheel or npm package for all platforms is built by the release CI: every OS adds its own folder, and packaging happens once (see [Ready packages and releases](#ready-packages-and-releases)).

### Linking into your C++ project

```cmake
set(BERESTA_BUILD_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/beresta)
target_link_libraries(my_app PRIVATE beresta::static)
```

The `beresta::beresta` target is the shared library. C programs link it (through `capi/beresta_c.h`).

Or manually:

```bash
g++ -std=c++17 -Ipath/to/beresta/include main.cpp path/to/beresta/bin/libberesta.a -o app
```

One header is enough in the code: `#include "beresta.hpp"`.

---

## Ready packages and releases

The user does not need to compile the core: the packages already contain the binaries for all platforms.

| Language | Installation | What is inside |
|---|---|---|
| Node.js | `npm install beresta` | addons for Windows and Linux (x64, arm64) and a universal one for macOS |
| Browsers, Workers | `npm install beresta-wasm` | `beresta_core.wasm` + an ES module with TypeScript types |
| Python | `pip install beresta` | one `py3-none-any` wheel with the libraries of all platforms |
| .NET | `dotnet add package Beresta` | `runtimes/<rid>/native` for `win-x64`, `win-arm64`, `linux-x64`, `linux-arm64`, `osx-x64`, `osx-arm64` |
| Java | Maven Central: `io.github.ermaleyalekov:beresta` (the dependency below) or the jar from the [GitHub release](https://github.com/ErmaleyAlekov/beresta/releases) | `natives/<os>-<arch>/` of all platforms, the loader extracts the right one |
| C and C++ | the SDK archive `beresta-<version>-<platform>.zip` from the release | `include/`, the shared and static libraries, `LICENSE` |

The Linux binaries are built on Ubuntu 22.04 and work with glibc 2.35 and newer.

The Maven dependency (the Java package stays `org.beresta`):

```xml
<dependency>
    <groupId>io.github.ermaleyalekov</groupId>
    <artifactId>beresta</artifactId>
    <version>2.1.2</version>
</dependency>
```

Gradle: `implementation("io.github.ermaleyalekov:beresta:2.1.2")`.

### CI/CD in GitHub Actions

- **`.github/workflows/ci.yml`** — on every push and pull request: Linux x64 and arm64 (the Makefile with GCC, CMake with Clang, all wrappers), Windows x64 and arm64 (MSVC, `ctest`), macOS (`make universal`, a `lipo` check, the C++ tests for arm64 and for x86_64 through Rosetta), WebAssembly (the official `emscripten/emsdk` image), the ASan/UBSan/LSan sanitizers and the W3C XML Conformance Test Suite (the run fails if even one applicable case does not pass).
- **`.github/workflows/release.yml`** — on a `vX.Y.Z` tag: checks that the tag version matches the versions in `CMakeLists.txt`, `package.json` and `pyproject.toml`; builds the native binaries on five platforms; merges them into one tree; builds the npm packages (`beresta`, `beresta-wasm`), the Python wheel, NuGet, the jar (with sources, javadoc and a GPG signature for Maven Central) and the SDK archives; installs the wheel and the npm package into a clean environment and checks that they work; creates a GitHub Release with all files.

Publishing to the registries is enabled by repository secrets (Settings → Secrets and variables → Actions). While a secret is missing, the step is skipped, and the packages still lie in the GitHub Release:

| Secret | Where |
|---|---|
| `NPM_TOKEN` | npm (`beresta`, `beresta-wasm`, with provenance) |
| `PYPI_API_TOKEN` | PyPI (`beresta`) |
| `NUGET_API_KEY` | NuGet (`Beresta`) |
| `MAVEN_CENTRAL_USERNAME`, `MAVEN_CENTRAL_PASSWORD` | Maven Central (`io.github.ermaleyalekov:beresta`): a Central Portal user token |
| `MAVEN_GPG_PRIVATE_KEY`, `MAVEN_GPG_PASSPHRASE` | signing the Maven artifacts: a GPG private key (ASCII armor) and its passphrase |

### CI/CD on Gitverse

The repository is mirrored on [Gitverse](https://gitverse.ru/ermaleyalekov/beresta). Gitverse reads its workflows from `.gitverse/workflows` (in the repository settings: CI/CD → "Use the configuration from .gitverse/workflows"); GitHub does not see this folder. Gitverse has one cloud runner — `ubuntu-latest` (Linux x86_64, 30 minutes per job), so:

- **`.gitverse/workflows/ci.yml`** — on every push and merge request: `make` and `make test` on Linux x86_64. The full platform matrix is checked on GitHub.
- **`.gitverse/workflows/release.yml`** — on a `vX.Y.Z` tag as one job (artifacts are not passed between jobs on Gitverse): the build and tests on Linux x86_64, then waiting for the release of the same tag on GitHub (up to 15 minutes), downloading all its files (`tools/release/github_assets.sh`) and creating the Gitverse release through the public API (`tools/release/gitverse_release.sh`). If the GitHub release is not ready, only the Linux x86_64 SDK is attached; a manual re-run (CI/CD → Release, the `tag` parameter) replaces the files. **This workflow publishes nothing to registries.**

The Gitverse secret (Repository settings → Secrets and variables): `GV_API_TOKEN` — a token from gitverse.ru/settings/tokens with the "Repositories → Write" right. The `GITVERSE_` prefix is forbidden in Gitverse secret names.

### Releasing a version

Packages in the registries (npm, PyPI, NuGet, Maven Central) are published **only from GitHub**: the publishing steps in `.github/workflows/release.yml` additionally check `github.server_url == 'https://github.com'`. A release with files comes out on both platforms; the notes of both are the version section from [CHANGELOG.md](../CHANGELOG.md) and [CHANGELOG.en.md](../CHANGELOG.en.md) (`tools/release/release_notes.sh`).

Remotes: `origin` — GitHub (`https://github.com/ErmaleyAlekov/beresta.git`), `gitverse` — Gitverse (`git@gitverse.ru:ermaleyalekov/beresta.git`). Adding the second one:

```bash
git remote add gitverse git@gitverse.ru:ermaleyalekov/beresta.git
```

The order:

1. Bump the version in `CMakeLists.txt`, `include/capi/beresta_c.h`, `bindings/*/package.json`, `bindings/python/pyproject.toml`, `bindings/python/tests/test_xpath_schema.py`, `bindings/java/pom.xml`, `bindings/csharp/Beresta/Beresta.csproj` and in the dependency examples (`docs/building*.md`, `docs/bindings*.md`, `bindings/java/README.md`); add the version section to both `CHANGELOG` files.
2. `make test` is green; commit and push `main` to both remotes, wait for a green CI on both.
3. The tag goes to GitHub first; the packages and the release are built there (about 20–30 minutes):

```bash
git tag v2.1.2
```
```bash
git push origin main v2.1.2
```

4. When the GitHub release is ready — the same tag to Gitverse; its workflow takes the files of the GitHub release:

```bash
git push gitverse main v2.1.2
```

5. Checking: [GitHub releases](https://github.com/ErmaleyAlekov/beresta/releases) and [Actions](https://github.com/ErmaleyAlekov/beresta/actions), [Gitverse releases](https://gitverse.ru/ermaleyalekov/beresta/releases) and the CI/CD section of the repository on Gitverse; the packages — on npm, PyPI, NuGet, Maven Central (if the GitHub secrets are set).

Without a Gitverse token the release is created by hand: Releases → "Create release", the tag `vX.Y.Z`, the title "Берёста X.Y.Z", the notes — the output of `bash tools/release/release_notes.sh X.Y.Z notes.md`, the files — from the GitHub release (`bash tools/release/github_assets.sh vX.Y.Z dist 0`; on Windows — with `PYTHON=python`). With a token `GV_API_TOKEN=… bash tools/release/gitverse_release.sh vX.Y.Z notes.md dist/*` does the same.
