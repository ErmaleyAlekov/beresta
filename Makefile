# ============================================================================
#  Берёста (beresta) — кроссплатформенный Makefile (GCC / Clang / MinGW)
#
#  Цели:
#    make               — библиотеки (динамическая + статическая) и демо-программы
#    make test          — собрать и запустить модульные тесты C++
#    make bindings-test — самопроверки включённых обёрток (JAVA/CSHARP/PYTHON/NODE=1)
#    make java-package  — jar через Maven (нужен mvn и доступ к репозиторию)
#    make csharp-package— NuGet-пакет через dotnet pack
#    make node-package  — пакет npm (.tgz) через npm pack
#    make clean         — удалить объектные файлы (obj/) и зависимости (dep/)
#    make fclean        — clean + бинарники (bin/) и копии бинарника в обёртках
#    make re            — полная пересборка
#    make platform      — показать определённую ОС, архитектуру и пути
#
#  Обёртки включаются переменными (можно несколько сразу):
#    make JAVA=1        — бинарник -> bindings/java/src/main/resources/natives/<os>-<arch>/
#    make CSHARP=1      — бинарник -> bindings/csharp/Beresta/runtimes/<rid>/native/
#    make PYTHON=1      — бинарник -> bindings/python/beresta/_native/<os>-<arch>/
#    make NODE=1        — аддон beresta.node -> bindings/node/native/<os>-<arch>/
#
#  Язык сообщений об ошибках (код каталога src/i18n/catalogs/messages_<код>.cpp):
#    make ERRORS_LANG=en  — английский (по умолчанию ru)
#
#  Интерпретатор Python для самопроверки: python на Windows, python3 в
#  Linux / macOS (там команды python часто нет); свой — make PYTHON_EXE=...
#  Node.js — make NODE_EXE=...
#
#  Структура выходных каталогов зеркально повторяет src/:
#    src/core/element/element.cpp -> obj/core/element/element.o
#                                 -> dep/core/element/element.d
# ============================================================================
#  Beresta — a cross-platform Makefile (GCC / Clang / MinGW)
#
#  Targets:
#    make               — the libraries (shared + static) and the demo programs
#    make test          — build and run the C++ unit tests
#    make bindings-test — self-tests of the enabled bindings (JAVA/CSHARP/PYTHON/NODE=1)
#    make java-package  — a jar through Maven (needs mvn and repository access)
#    make csharp-package— a NuGet package through dotnet pack
#    make node-package  — an npm package (.tgz) through npm pack
#    make clean         — remove the object files (obj/) and dependencies (dep/)
#    make fclean        — clean + the binaries (bin/) and the binary copies in the bindings
#    make re            — a full rebuild
#    make platform      — show the detected OS, architecture and paths
#
#  Bindings are enabled with variables (several at once are allowed):
#    make JAVA=1        — binary -> bindings/java/src/main/resources/natives/<os>-<arch>/
#    make CSHARP=1      — binary -> bindings/csharp/Beresta/runtimes/<rid>/native/
#    make PYTHON=1      — binary -> bindings/python/beresta/_native/<os>-<arch>/
#    make NODE=1        — the beresta.node addon -> bindings/node/native/<os>-<arch>/
#
#  The error message language (the code of a catalog src/i18n/catalogs/messages_<code>.cpp):
#    make ERRORS_LANG=en  — English (default ru)
#
#  The Python interpreter for the self-test: python on Windows, python3 on
#  Linux / macOS (the python command is often missing there); your own — make PYTHON_EXE=...
#  Node.js — make NODE_EXE=...
#
#  The output directory structure mirrors src/:
#    src/core/element/element.cpp -> obj/core/element/element.o
#                                 -> dep/core/element/element.d
# ============================================================================

# Путь к этому файлу (до include .d-файлов MAKEFILE_LIST содержит только его)
# The path to this file (before the .d files are included MAKEFILE_LIST contains only it)
MAKEFILE_PATH := $(firstword $(MAKEFILE_LIST))

# ---- Имена и каталоги -------------------------------------------------------
# ---- Names and directories -------------------------------------------------------
NAME        := libberesta.a
DEMO        := beresta_demo
CAPI_DEMO   := beresta_capi_demo
TESTS       := beresta_tests

SRC_DIR     := src
INC_DIR     := include
OBJ_DIR     := obj
DEP_DIR     := dep
BIN_DIR     := bin
TEST_DIR    := tests
EXAMPLE_DIR := examples
BIND_DIR    := bindings

# ---- Переключатели обёрток (по умолчанию выключены) ---------------------------
# ---- Wrapper switches (off by default) ---------------------------
JAVA        ?= 0
CSHARP      ?= 0
PYTHON      ?= 0
NODE        ?= 0
NODE_EXE    ?= node

# ---- Язык сообщений об ошибках / Error message language ----------------------
# Каталог обязан существовать: опечатка в коде языка останавливает сборку сразу,
# а не превращается в молчаливый переход на английский во время работы.
# The catalog must exist: a typo in the language code stops the build at once
# instead of silently falling back to English at run time.
ERRORS_LANG ?= ru
ERRORS_CATALOG := $(SRC_DIR)/i18n/catalogs/messages_$(ERRORS_LANG).cpp
ifeq ($(wildcard $(ERRORS_CATALOG)),)
    $(error Нет каталога сообщений / No message catalog: $(ERRORS_CATALOG))
endif

# ---- Компилятор и флаги (CXX можно переопределить: make CXX=clang++) -------
# ---- The compiler and flags (CXX can be overridden: make CXX=clang++) -------
CXXFLAGS    := -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -O2
CFLAGS      := -std=c99 -Wall -Wextra -Wpedantic -O2
ARFLAGS     := rcs

# Компилятор C для демо на чистом C: встроенное в make значение «cc»
# есть не везде (в MinGW его нет), поэтому по умолчанию берётся gcc
# The C compiler for the pure C demo: make's built-in value "cc"
# is not available everywhere (MinGW has none), so gcc is taken by default
ifeq ($(origin CC),default)
    CC      := gcc
endif

# Объекты библиотеки компилируются ОДИН раз для обеих библиотек:
# BERESTA_BUILDING_SHARED помечает функции Си-интерфейса как экспортируемые
# BERESTA_ERRORS_LANG — язык сообщений по умолчанию (без кавычек: код превращается
# в строку в самом коде, поэтому одинаково работает в cmd.exe и sh).
#
# Library objects are compiled ONCE for both libraries:
# BERESTA_BUILDING_SHARED marks the C interface functions as exported;
# BERESTA_ERRORS_LANG is the default message language (unquoted: the code turns it
# into a string itself, so it works the same in cmd.exe and sh).
LIB_FLAGS   := -DBERESTA_BUILDING_SHARED -DBERESTA_ERRORS_LANG=$(ERRORS_LANG)

# ---- Определение ОС ----------------------------------------------------------
# На Windows явно используем cmd.exe, чтобы поведение не зависело от того,
# есть ли в PATH sh.exe из Git/MSYS. Команды файловой системы выбираются
# под конкретную оболочку: cmd (Windows) или POSIX sh (Linux/macOS).
# ---- OS detection ----------------------------------------------------------
# On Windows cmd.exe is used explicitly so that the behavior does not depend on
# whether sh.exe from Git/MSYS is in PATH. The file system commands are chosen
# for the specific shell: cmd (Windows) or POSIX sh (Linux/macOS).
ifeq ($(OS),Windows_NT)
    PLATFORM_OS := windows
else
    UNAME_S     := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        PLATFORM_OS := macos
    else ifeq ($(UNAME_S),Linux)
        PLATFORM_OS := linux
    else
        PLATFORM_OS := $(shell uname -s | tr A-Z a-z)
    endif
endif

# ---- Определение архитектуры -------------------------------------------------
# Источник — сам компилятор (-dumpmachine: x86_64-w64-mingw32,
# aarch64-linux-gnu, arm64-apple-darwin23...): так при кросс-компиляции
# бинарник попадёт в папку ЦЕЛЕВОЙ архитектуры, а не архитектуры хоста.
# Имена приводятся к единым: x86_64, aarch64, x86 (как в CMake и Java).
# ---- Architecture detection -------------------------------------------------
# The source is the compiler itself (-dumpmachine: x86_64-w64-mingw32,
# aarch64-linux-gnu, arm64-apple-darwin23...): so with cross-compilation
# the binary goes into the folder of the TARGET architecture, not the host one.
# The names are normalized: x86_64, aarch64, x86 (as in CMake and Java).
MACHINE     := $(shell $(CXX) -dumpmachine)
RAW_ARCH    := $(firstword $(subst -, ,$(MACHINE)))
PLATFORM_ARCH := $(RAW_ARCH)
ifneq ($(filter x86_64 amd64 x64,$(RAW_ARCH)),)
    PLATFORM_ARCH := x86_64
endif
ifneq ($(filter aarch64 arm64,$(RAW_ARCH)),)
    PLATFORM_ARCH := aarch64
endif
ifneq ($(filter i386 i486 i586 i686 x86,$(RAW_ARCH)),)
    PLATFORM_ARCH := x86
endif
PLATFORM    := $(PLATFORM_OS)-$(PLATFORM_ARCH)

# Идентификатор среды .NET (RID): win-x64, linux-arm64, osx-arm64...
# The .NET runtime identifier (RID): win-x64, linux-arm64, osx-arm64...
RID_OS      := $(PLATFORM_OS)
RID_OS      := $(patsubst windows,win,$(RID_OS))
RID_OS      := $(patsubst macos,osx,$(RID_OS))
RID_ARCH    := $(patsubst x86_64,x64,$(patsubst aarch64,arm64,$(PLATFORM_ARCH)))
DOTNET_RID  := $(RID_OS)-$(RID_ARCH)

# ---- Платформенные особенности -------------------------------------------------
# SHARED_NAME — каноническое имя бинарника, его же ищут загрузчики обёрток.
# SHARED_LDFLAGS встраивают C++ runtime в библиотеку: jar / NuGet / пакет
# Python переносят ОДИН файл, и он не должен требовать libstdc++ на машине.
# --exclude-libs,ALL скрывает символы встроенной libstdc++: иначе .so
# экспортировала бы сотни её функций, и в процессе JVM или Python они
# могли бы подменить функции системной libstdc++ (или наоборот).
# ---- Platform specifics -------------------------------------------------
# SHARED_NAME is the canonical binary name, the wrapper loaders look for it too.
# SHARED_LDFLAGS embed the C++ runtime into the library: the jar / NuGet / Python
# package carry ONE file, and it must not require libstdc++ on the machine.
# --exclude-libs,ALL hides the symbols of the embedded libstdc++: otherwise the .so
# would export hundreds of its functions, and in a JVM or Python process they
# could replace the functions of the system libstdc++ (or vice versa).
# -s убирает отладочные символы: без него beresta.dll от MinGW весит ~18 МБ
# вместо ~2 МБ. ADDON_LDFLAGS — то же для аддона Node.js (на Linux ему нужен
# -ldl ради dlsym в старых glibc).
# -s strips the debug symbols: without it the MinGW beresta.dll weighs ~18 MB
# instead of ~2 MB. ADDON_LDFLAGS are the same for the Node.js addon (on Linux it
# needs -ldl for dlsym in old glibc versions).
ifeq ($(PLATFORM_OS),windows)
    SHELL          := cmd.exe
    .SHELLFLAGS    := /C
    EXE            := .exe
    SHARED_NAME    := beresta.dll
    SHARED_LDFLAGS := -shared -static -s -Wl,--out-implib,$(BIN_DIR)/libberesta.dll.a
    ADDON_LDFLAGS  := -shared -static -s
    CAPI_LDFLAGS   :=
    PYTHON_EXE     ?= python
    native          = $(subst /,\,$(1))
    MKDIR           = if not exist "$(call native,$(1))" mkdir "$(call native,$(1))"
    RMDIR           = if exist "$(call native,$(1))" rmdir /S /Q "$(call native,$(1))"
    COPY            = copy /Y "$(call native,$(1))" "$(call native,$(2))" >NUL
    RUN_IN          = cd /D "$(call native,$(1))" && $(2)
    TOUCH           = type nul > "$(call native,$(1))"
    REMOVE_STAMPS   = if exist "$(call native,$(1))" del /Q "$(call native,$(1))\errors_lang_*.stamp" 2>NUL
else ifeq ($(PLATFORM_OS),macos)
    EXE            :=
    SHARED_NAME    := libberesta.dylib
    SHARED_LDFLAGS := -dynamiclib -install_name @rpath/$(SHARED_NAME)
    ADDON_LDFLAGS  := -dynamiclib
    CAPI_LDFLAGS   := -Wl,-rpath,@loader_path
    PIC_FLAGS      := -fPIC -fvisibility=hidden -fvisibility-inlines-hidden
else
    EXE            :=
    SHARED_NAME    := libberesta.so
    SHARED_LDFLAGS := -shared -s -static-libstdc++ -static-libgcc -Wl,--exclude-libs,ALL -Wl,-soname,$(SHARED_NAME)
    ADDON_LDFLAGS  := -shared -s -static-libstdc++ -static-libgcc -Wl,--exclude-libs,ALL -ldl
    CAPI_LDFLAGS   := -Wl,-rpath,'$$ORIGIN'
    PIC_FLAGS      := -fPIC -fvisibility=hidden -fvisibility-inlines-hidden
endif

ifneq ($(PLATFORM_OS),windows)
    PYTHON_EXE     ?= python3
    native          = $(1)
    MKDIR           = mkdir -p "$(1)"
    RMDIR           = rm -rf "$(1)"
    COPY            = cp -f "$(1)" "$(2)"
    RUN_IN          = cd "$(1)" && $(2)
    TOUCH           = touch "$(1)"
    REMOVE_STAMPS   = rm -f "$(1)"/errors_lang_*.stamp
endif

# ---- Вспомогательные функции ------------------------------------------------
# Рекурсивный поиск файлов средствами самого make (без внешней утилиты find)
# ---- Helper functions ------------------------------------------------
# A recursive file search with make itself (without the external find utility)
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Путь к .d-файлу для объектного файла: obj/x/y.o -> dep/x/y.d
# The path to the .d file for an object file: obj/x/y.o -> dep/x/y.d
dep_of    = $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$(1))

# Каталог файла без завершающего слеша: obj/x/y.o -> obj/x
# The file directory without the trailing slash: obj/x/y.o -> obj/x
dir_of    = $(patsubst %/,%,$(dir $(1)))

# ---- Наборы исходников и объектов -------------------------------------------
# ---- Source and object sets -------------------------------------------
LIB_SRC     := $(call rwildcard,$(SRC_DIR),*.cpp)
LIB_OBJ     := $(LIB_SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

DEMO_SRC    := $(call rwildcard,$(EXAMPLE_DIR)/src,*.cpp)
DEMO_OBJ    := $(DEMO_SRC:$(EXAMPLE_DIR)/src/%.cpp=$(OBJ_DIR)/$(EXAMPLE_DIR)/%.o)

CAPI_SRC    := $(EXAMPLE_DIR)/capi/capi_demo.c
CAPI_OBJ    := $(OBJ_DIR)/$(EXAMPLE_DIR)/capi/capi_demo.o

TEST_SRC    := $(call rwildcard,$(TEST_DIR)/src,*.cpp)
TEST_OBJ    := $(TEST_SRC:$(TEST_DIR)/src/%.cpp=$(OBJ_DIR)/$(TEST_DIR)/%.o)

# Аддон Node.js: свои исходники + все объекты ядра (один самодостаточный файл)
# The Node.js addon: its own sources + all core objects (one self-contained file)
NODE_DIR    := $(BIND_DIR)/node
ADDON_DIR   := $(NODE_DIR)/addon
ADDON_SRC   := $(call rwildcard,$(ADDON_DIR)/src,*.cpp)
ADDON_OBJ   := $(ADDON_SRC:$(ADDON_DIR)/src/%.cpp=$(OBJ_DIR)/node/%.o)

ALL_OBJ     := $(LIB_OBJ) $(DEMO_OBJ) $(CAPI_OBJ) $(TEST_OBJ) $(ADDON_OBJ)
ALL_DEP     := $(call dep_of,$(ALL_OBJ))

LIB_BIN     := $(BIN_DIR)/$(NAME)
SHARED_BIN  := $(BIN_DIR)/$(SHARED_NAME)
DEMO_BIN    := $(BIN_DIR)/$(DEMO)$(EXE)
CAPI_BIN    := $(BIN_DIR)/$(CAPI_DEMO)$(EXE)
TEST_BIN    := $(BIN_DIR)/$(TESTS)$(EXE)
ADDON_BIN   := $(BIN_DIR)/beresta.node

# ---- Папки обёрток, куда копируется динамическая библиотека ------------------
# ---- Wrapper folders the shared library is copied to ------------------
JAVA_DIR    := $(BIND_DIR)/java
JAVA_NATIVE := $(JAVA_DIR)/src/main/resources/natives/$(PLATFORM)
CSHARP_DIR  := $(BIND_DIR)/csharp
CSHARP_NATIVE := $(CSHARP_DIR)/Beresta/runtimes/$(DOTNET_RID)/native
PYTHON_DIR  := $(BIND_DIR)/python
PYTHON_NATIVE := $(PYTHON_DIR)/beresta/_native/$(PLATFORM)
NODE_NATIVE := $(NODE_DIR)/native/$(PLATFORM)
NODE_COPY   := $(NODE_NATIVE)/beresta.node

ALL_COPIES     := $(JAVA_NATIVE)/$(SHARED_NAME) $(CSHARP_NATIVE)/$(SHARED_NAME) $(PYTHON_NATIVE)/$(SHARED_NAME)
BINDING_COPIES :=
BINDING_TESTS  :=
ifeq ($(JAVA),1)
    BINDING_COPIES += $(JAVA_NATIVE)/$(SHARED_NAME)
    BINDING_TESTS  += java-test
endif
ifeq ($(CSHARP),1)
    BINDING_COPIES += $(CSHARP_NATIVE)/$(SHARED_NAME)
    BINDING_TESTS  += csharp-test
endif
ifeq ($(PYTHON),1)
    BINDING_COPIES += $(PYTHON_NATIVE)/$(SHARED_NAME)
    BINDING_TESTS  += python-test
endif
ifeq ($(NODE),1)
    BINDING_COPIES += $(NODE_COPY)
    BINDING_TESTS  += node-test
endif

ALL_DIRS    := $(sort $(call dir_of,$(ALL_OBJ) $(ALL_DEP) $(ALL_COPIES) $(NODE_COPY)) $(BIN_DIR))

# ---- Флаги автоматической генерации зависимостей ----------------------------
# -MMD : записать зависимости от пользовательских заголовков в .d-файл
# -MP  : добавить пустые цели для заголовков (удалённый .hpp не ломает сборку)
# -MF  : путь к .d-файлу в каталоге dep/, -MT : цель внутри .d-файла
# ---- Automatic dependency generation flags ----------------------------
# -MMD : write the dependencies on user headers into the .d file
# -MP  : add empty targets for headers (a removed .hpp does not break the build)
# -MF  : the path to the .d file in the dep/ directory, -MT : the target inside the .d file
DEPFLAGS     = -MMD -MP -MF $(call dep_of,$@) -MT $@

# ---- Основные цели ----------------------------------------------------------
# ---- Main targets ----------------------------------------------------------
.PHONY: all test bindings-test java-test csharp-test python-test node-test \
        java-package csharp-package node-package platform clean fclean re

all: $(LIB_BIN) $(SHARED_BIN) $(DEMO_BIN) $(CAPI_BIN) $(BINDING_COPIES)

test: $(TEST_BIN)
	$(call native,$(TEST_BIN))

# Статическая библиотека из всех объектов src/ (для программ на C++)
# The static library from all objects of src/ (for C++ programs)
$(LIB_BIN): $(LIB_OBJ) | $(BIN_DIR)
	$(AR) $(ARFLAGS) $@ $(LIB_OBJ)

# Динамическая библиотека: наружу экспортируется только Си-интерфейс
# The shared library: only the C interface is exported
$(SHARED_BIN): $(LIB_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(LIB_OBJ) $(SHARED_LDFLAGS) -o $@

# Демонстрационная программа на C++ (статическая библиотека)
# The C++ demo program (static library)
$(DEMO_BIN): $(DEMO_OBJ) $(LIB_BIN) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(DEMO_OBJ) $(LIB_BIN) -o $@

# Демонстрационная программа на чистом C (динамическая библиотека)
# The pure C demo program (shared library)
$(CAPI_BIN): $(CAPI_OBJ) $(SHARED_BIN) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CAPI_OBJ) -L$(BIN_DIR) -lberesta $(CAPI_LDFLAGS) -o $@

# Модульные тесты
# Unit tests
$(TEST_BIN): $(TEST_OBJ) $(LIB_BIN) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_OBJ) $(LIB_BIN) -o $@

# Аддон Node.js: переходники Node-API + объекты ядра в одном файле beresta.node
# The Node.js addon: the Node-API adapters + the core objects in one beresta.node file
$(ADDON_BIN): $(ADDON_OBJ) $(LIB_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(ADDON_OBJ) $(LIB_OBJ) $(ADDON_LDFLAGS) -o $@

# ---- Копирование бинарника в обёртки (пересоздаётся при изменении) ------------
# Правило есть для всех трёх обёрток (make java-test работает и без JAVA=1),
# а переменные JAVA/CSHARP/PYTHON решают, какие копии входят в make all.
# ---- Copying the binary into the wrappers (recreated on change) ------------
# The rule exists for all three wrappers (make java-test works even without JAVA=1),
# and the JAVA/CSHARP/PYTHON variables decide which copies are part of make all.
.SECONDEXPANSION:

$(ALL_COPIES): $(SHARED_BIN) | $$(call dir_of,$$@)
	$(call COPY,$<,$@)

$(NODE_COPY): $(ADDON_BIN) | $$(call dir_of,$$@)
	$(call COPY,$<,$@)

# ---- Правила компиляции -----------------------------------------------------
# Вторичное раскрытие ($$) позволяет вычислить каталоги-предпосылки для
# каждого объекта. Каталоги — order-only (после |): их создание не вызывает
# пересборку, а make создаёт каждый каталог ровно один раз даже при make -j.
# Сам Makefile — тоже предпосылка: изменение флагов компиляции пересобирает
# объекты (файлы .d отслеживают только заголовки, но не флаги).
# ---- Compilation rules -----------------------------------------------------
# Secondary expansion ($$) allows computing the prerequisite directories for
# each object. The directories are order-only (after |): creating them does not
# trigger a rebuild, and make creates each directory exactly once even with make -j.
# The Makefile itself is a prerequisite too: changing the compile flags rebuilds
# the objects (the .d files track only headers, not flags).
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CXX) $(CXXFLAGS) $(PIC_FLAGS) $(LIB_FLAGS) -I$(INC_DIR) $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR)/$(EXAMPLE_DIR)/%.o: $(EXAMPLE_DIR)/src/%.cpp $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -I$(EXAMPLE_DIR)/include $(DEPFLAGS) -c $< -o $@

$(CAPI_OBJ): $(CAPI_SRC) $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CC) $(CFLAGS) -DBERESTA_USING_SHARED -I$(INC_DIR) $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/src/%.cpp $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -I$(TEST_DIR)/include $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR)/node/%.o: $(ADDON_DIR)/src/%.cpp $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CXX) $(CXXFLAGS) $(PIC_FLAGS) -I$(INC_DIR) -I$(ADDON_DIR)/include $(DEPFLAGS) -c $< -o $@

# ---- Смена языка пересобирает ровно один объект / A language change rebuilds one object ----
# Макрос языка читает только i18n_language.cpp. Метка с кодом языка в имени
# появляется заново при каждой смене ERRORS_LANG (старые метки удаляются),
# поэтому make пересобирает этот объект и библиотеки — и только их.
# Only i18n_language.cpp reads the language macro. A stamp named after the
# language code reappears on every ERRORS_LANG change (old stamps are removed),
# so make rebuilds this object and the libraries — and nothing else.
ERRORS_STAMP := $(OBJ_DIR)/i18n/errors_lang_$(ERRORS_LANG).stamp

$(OBJ_DIR)/i18n/i18n_language.o: $(ERRORS_STAMP)

$(ERRORS_STAMP): | $(OBJ_DIR)/i18n
	@$(call REMOVE_STAMPS,$(OBJ_DIR)/i18n)
	@$(call TOUCH,$@)

# Создание любого каталога из списка
# Creating any directory from the list
$(ALL_DIRS):
	@$(call MKDIR,$@)

# ---- Самопроверки обёрток ----------------------------------------------------
# Java проверяется без Maven: javac + java из JDK 22+. Ресурсы с бинарником
# копируются в каталог классов, поэтому classpath состоит из одного пути.
# ---- Wrapper self-tests ----------------------------------------------------
# Java is checked without Maven: javac + java from JDK 22+. The resources with the binary
# are copied into the classes directory, so the classpath consists of one path.
JAVA_CLASSES := $(JAVA_DIR)/target/selftest-classes
JAVA_SOURCES := $(call rwildcard,$(JAVA_DIR)/src/main/java,*.java) $(call rwildcard,$(JAVA_DIR)/src/test/java,*.java)

bindings-test: $(BINDING_TESTS)

java-test: $(JAVA_NATIVE)/$(SHARED_NAME)
	@$(call RMDIR,$(JAVA_CLASSES))
	javac --release 22 -encoding UTF-8 -d $(call native,$(JAVA_CLASSES)) $(call native,$(JAVA_SOURCES))
	@$(call MKDIR,$(JAVA_CLASSES)/natives/$(PLATFORM))
	@$(call COPY,$(JAVA_NATIVE)/$(SHARED_NAME),$(JAVA_CLASSES)/natives/$(PLATFORM)/$(SHARED_NAME))
	java --enable-native-access=ALL-UNNAMED -Dstdout.encoding=UTF-8 -cp $(call native,$(JAVA_CLASSES)) org.beresta.selftest.SelfTest

csharp-test: $(CSHARP_NATIVE)/$(SHARED_NAME)
	dotnet run --project $(call native,$(CSHARP_DIR)/Beresta.SelfTest/Beresta.SelfTest.csproj) -c Release

python-test: $(PYTHON_NATIVE)/$(SHARED_NAME)
	$(call RUN_IN,$(PYTHON_DIR),$(PYTHON_EXE) -m unittest discover -s tests)

# Node.js проверяется встроенным node --test (без Jest, Mocha и npm install)
# Node.js is checked with the built-in node --test (without Jest, Mocha and npm install)
node-test: $(NODE_COPY)
	$(call RUN_IN,$(NODE_DIR),$(NODE_EXE) --test)

java-package: $(JAVA_NATIVE)/$(SHARED_NAME)
	mvn -B -q -f $(call native,$(JAVA_DIR)/pom.xml) package

csharp-package: $(CSHARP_NATIVE)/$(SHARED_NAME)
	dotnet pack $(call native,$(CSHARP_DIR)/Beresta/Beresta.csproj) -c Release

node-package: $(NODE_COPY)
	$(call RUN_IN,$(NODE_DIR),npm pack)

platform:
	@echo OS=$(PLATFORM_OS) ARCH=$(PLATFORM_ARCH) PLATFORM=$(PLATFORM) RID=$(DOTNET_RID) ERRORS_LANG=$(ERRORS_LANG)
	@echo SHARED=$(SHARED_BIN)
	@echo JAVA=$(JAVA_NATIVE)
	@echo CSHARP=$(CSHARP_NATIVE)
	@echo PYTHON=$(PYTHON_NATIVE)
	@echo NODE=$(NODE_NATIVE)

# ---- Очистка ----------------------------------------------------------------
# fclean удаляет и копии бинарника в обёртках текущей платформы: в
# репозиторий они не попадают (.gitignore), а устаревшая копия опаснее,
# чем отсутствующая — обёртка загрузила бы старую версию библиотеки.
# ---- Cleaning ----------------------------------------------------------------
# fclean also removes the binary copies in the wrappers of the current platform:
# they do not go into the repository (.gitignore), and a stale copy is more dangerous
# than a missing one — the wrapper would load an old version of the library.
clean:
	@$(call RMDIR,$(OBJ_DIR))
	@$(call RMDIR,$(DEP_DIR))

fclean: clean
	@$(call RMDIR,$(BIN_DIR))
	@$(call RMDIR,$(JAVA_NATIVE))
	@$(call RMDIR,$(JAVA_DIR)/target)
	@$(call RMDIR,$(CSHARP_NATIVE))
	@$(call RMDIR,$(PYTHON_NATIVE))
	@$(call RMDIR,$(NODE_NATIVE))

re: fclean
	@$(MAKE) --no-print-directory all

# ---- Подключение сгенерированных зависимостей -------------------------------
# ---- Including the generated dependencies -------------------------------
-include $(ALL_DEP)
