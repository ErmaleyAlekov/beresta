# ============================================================================
#  Берёста (beresta) — кроссплатформенный Makefile (GCC / Clang / MinGW)
#
#  Цели:
#    make               — библиотеки (динамическая + статическая) и демо-программы
#    make test          — собрать и запустить модульные тесты C++
#    make bindings-test — самопроверки включённых обёрток (JAVA/CSHARP/PYTHON=1)
#    make java-package  — jar через Maven (нужен mvn и доступ к репозиторию)
#    make csharp-package— NuGet-пакет через dotnet pack
#    make clean         — удалить объектные файлы (obj/) и зависимости (dep/)
#    make fclean        — clean + бинарники (bin/) и копии бинарника в обёртках
#    make re            — полная пересборка
#    make platform      — показать определённую ОС, архитектуру и пути
#
#  Обёртки включаются переменными (можно несколько сразу):
#    make JAVA=1        — бинарник -> bindings/java/src/main/resources/natives/<os>-<arch>/
#    make CSHARP=1      — бинарник -> bindings/csharp/Beresta/runtimes/<rid>/native/
#    make PYTHON=1      — бинарник -> bindings/python/beresta/_native/<os>-<arch>/
#
#  Интерпретатор Python для самопроверки: python на Windows, python3 в
#  Linux / macOS (там команды python часто нет); свой — make PYTHON_EXE=...
#
#  Структура выходных каталогов зеркально повторяет src/:
#    src/core/element/element.cpp -> obj/core/element/element.o
#                                 -> dep/core/element/element.d
# ============================================================================

# Путь к этому файлу (до include .d-файлов MAKEFILE_LIST содержит только его)
MAKEFILE_PATH := $(firstword $(MAKEFILE_LIST))

# ---- Имена и каталоги -------------------------------------------------------
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
JAVA        ?= 0
CSHARP      ?= 0
PYTHON      ?= 0

# ---- Компилятор и флаги (CXX можно переопределить: make CXX=clang++) -------
CXXFLAGS    := -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -O2
CFLAGS      := -std=c99 -Wall -Wextra -Wpedantic -O2
ARFLAGS     := rcs

# Компилятор C для демо на чистом C: встроенное в make значение «cc»
# есть не везде (в MinGW его нет), поэтому по умолчанию берётся gcc
ifeq ($(origin CC),default)
    CC      := gcc
endif

# Объекты библиотеки компилируются ОДИН раз для обеих библиотек:
# BERESTA_BUILDING_SHARED помечает функции Си-интерфейса как экспортируемые
LIB_FLAGS   := -DBERESTA_BUILDING_SHARED

# ---- Определение ОС ----------------------------------------------------------
# На Windows явно используем cmd.exe, чтобы поведение не зависело от того,
# есть ли в PATH sh.exe из Git/MSYS. Команды файловой системы выбираются
# под конкретную оболочку: cmd (Windows) или POSIX sh (Linux/macOS).
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
ifeq ($(PLATFORM_OS),windows)
    SHELL          := cmd.exe
    .SHELLFLAGS    := /C
    EXE            := .exe
    SHARED_NAME    := beresta.dll
    SHARED_LDFLAGS := -shared -static -Wl,--out-implib,$(BIN_DIR)/libberesta.dll.a
    CAPI_LDFLAGS   :=
    PYTHON_EXE     ?= python
    native          = $(subst /,\,$(1))
    MKDIR           = if not exist "$(call native,$(1))" mkdir "$(call native,$(1))"
    RMDIR           = if exist "$(call native,$(1))" rmdir /S /Q "$(call native,$(1))"
    COPY            = copy /Y "$(call native,$(1))" "$(call native,$(2))" >NUL
    RUN_IN          = cd /D "$(call native,$(1))" && $(2)
else ifeq ($(PLATFORM_OS),macos)
    EXE            :=
    SHARED_NAME    := libberesta.dylib
    SHARED_LDFLAGS := -dynamiclib -install_name @rpath/$(SHARED_NAME)
    CAPI_LDFLAGS   := -Wl,-rpath,@loader_path
    PIC_FLAGS      := -fPIC -fvisibility=hidden -fvisibility-inlines-hidden
else
    EXE            :=
    SHARED_NAME    := libberesta.so
    SHARED_LDFLAGS := -shared -static-libstdc++ -static-libgcc -Wl,--exclude-libs,ALL -Wl,-soname,$(SHARED_NAME)
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
endif

# ---- Вспомогательные функции ------------------------------------------------
# Рекурсивный поиск файлов средствами самого make (без внешней утилиты find)
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Путь к .d-файлу для объектного файла: obj/x/y.o -> dep/x/y.d
dep_of    = $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$(1))

# Каталог файла без завершающего слеша: obj/x/y.o -> obj/x
dir_of    = $(patsubst %/,%,$(dir $(1)))

# ---- Наборы исходников и объектов -------------------------------------------
LIB_SRC     := $(call rwildcard,$(SRC_DIR),*.cpp)
LIB_OBJ     := $(LIB_SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

DEMO_SRC    := $(call rwildcard,$(EXAMPLE_DIR)/src,*.cpp)
DEMO_OBJ    := $(DEMO_SRC:$(EXAMPLE_DIR)/src/%.cpp=$(OBJ_DIR)/$(EXAMPLE_DIR)/%.o)

CAPI_SRC    := $(EXAMPLE_DIR)/capi/capi_demo.c
CAPI_OBJ    := $(OBJ_DIR)/$(EXAMPLE_DIR)/capi/capi_demo.o

TEST_SRC    := $(call rwildcard,$(TEST_DIR)/src,*.cpp)
TEST_OBJ    := $(TEST_SRC:$(TEST_DIR)/src/%.cpp=$(OBJ_DIR)/$(TEST_DIR)/%.o)

ALL_OBJ     := $(LIB_OBJ) $(DEMO_OBJ) $(CAPI_OBJ) $(TEST_OBJ)
ALL_DEP     := $(call dep_of,$(ALL_OBJ))

LIB_BIN     := $(BIN_DIR)/$(NAME)
SHARED_BIN  := $(BIN_DIR)/$(SHARED_NAME)
DEMO_BIN    := $(BIN_DIR)/$(DEMO)$(EXE)
CAPI_BIN    := $(BIN_DIR)/$(CAPI_DEMO)$(EXE)
TEST_BIN    := $(BIN_DIR)/$(TESTS)$(EXE)

# ---- Папки обёрток, куда копируется динамическая библиотека ------------------
JAVA_DIR    := $(BIND_DIR)/java
JAVA_NATIVE := $(JAVA_DIR)/src/main/resources/natives/$(PLATFORM)
CSHARP_DIR  := $(BIND_DIR)/csharp
CSHARP_NATIVE := $(CSHARP_DIR)/Beresta/runtimes/$(DOTNET_RID)/native
PYTHON_DIR  := $(BIND_DIR)/python
PYTHON_NATIVE := $(PYTHON_DIR)/beresta/_native/$(PLATFORM)

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

ALL_DIRS    := $(sort $(call dir_of,$(ALL_OBJ) $(ALL_DEP) $(ALL_COPIES)) $(BIN_DIR))

# ---- Флаги автоматической генерации зависимостей ----------------------------
# -MMD : записать зависимости от пользовательских заголовков в .d-файл
# -MP  : добавить пустые цели для заголовков (удалённый .hpp не ломает сборку)
# -MF  : путь к .d-файлу в каталоге dep/, -MT : цель внутри .d-файла
DEPFLAGS     = -MMD -MP -MF $(call dep_of,$@) -MT $@

# ---- Основные цели ----------------------------------------------------------
.PHONY: all test bindings-test java-test csharp-test python-test \
        java-package csharp-package platform clean fclean re

all: $(LIB_BIN) $(SHARED_BIN) $(DEMO_BIN) $(CAPI_BIN) $(BINDING_COPIES)

test: $(TEST_BIN)
	$(call native,$(TEST_BIN))

# Статическая библиотека из всех объектов src/ (для программ на C++)
$(LIB_BIN): $(LIB_OBJ) | $(BIN_DIR)
	$(AR) $(ARFLAGS) $@ $(LIB_OBJ)

# Динамическая библиотека: наружу экспортируется только Си-интерфейс
$(SHARED_BIN): $(LIB_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(LIB_OBJ) $(SHARED_LDFLAGS) -o $@

# Демонстрационная программа на C++ (статическая библиотека)
$(DEMO_BIN): $(DEMO_OBJ) $(LIB_BIN) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(DEMO_OBJ) $(LIB_BIN) -o $@

# Демонстрационная программа на чистом C (динамическая библиотека)
$(CAPI_BIN): $(CAPI_OBJ) $(SHARED_BIN) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CAPI_OBJ) -L$(BIN_DIR) -lberesta $(CAPI_LDFLAGS) -o $@

# Модульные тесты
$(TEST_BIN): $(TEST_OBJ) $(LIB_BIN) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_OBJ) $(LIB_BIN) -o $@

# ---- Копирование бинарника в обёртки (пересоздаётся при изменении) ------------
# Правило есть для всех трёх обёрток (make java-test работает и без JAVA=1),
# а переменные JAVA/CSHARP/PYTHON решают, какие копии входят в make all.
.SECONDEXPANSION:

$(ALL_COPIES): $(SHARED_BIN) | $$(call dir_of,$$@)
	$(call COPY,$<,$@)

# ---- Правила компиляции -----------------------------------------------------
# Вторичное раскрытие ($$) позволяет вычислить каталоги-предпосылки для
# каждого объекта. Каталоги — order-only (после |): их создание не вызывает
# пересборку, а make создаёт каждый каталог ровно один раз даже при make -j.
# Сам Makefile — тоже предпосылка: изменение флагов компиляции пересобирает
# объекты (файлы .d отслеживают только заголовки, но не флаги).
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CXX) $(CXXFLAGS) $(PIC_FLAGS) $(LIB_FLAGS) -I$(INC_DIR) $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR)/$(EXAMPLE_DIR)/%.o: $(EXAMPLE_DIR)/src/%.cpp $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -I$(EXAMPLE_DIR)/include $(DEPFLAGS) -c $< -o $@

$(CAPI_OBJ): $(CAPI_SRC) $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CC) $(CFLAGS) -DBERESTA_USING_SHARED -I$(INC_DIR) $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/src/%.cpp $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -I$(TEST_DIR)/include $(DEPFLAGS) -c $< -o $@

# Создание любого каталога из списка
$(ALL_DIRS):
	@$(call MKDIR,$@)

# ---- Самопроверки обёрток ----------------------------------------------------
# Java проверяется без Maven: javac + java из JDK 22+. Ресурсы с бинарником
# копируются в каталог классов, поэтому classpath состоит из одного пути.
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

java-package: $(JAVA_NATIVE)/$(SHARED_NAME)
	mvn -B -q -f $(call native,$(JAVA_DIR)/pom.xml) package

csharp-package: $(CSHARP_NATIVE)/$(SHARED_NAME)
	dotnet pack $(call native,$(CSHARP_DIR)/Beresta/Beresta.csproj) -c Release

platform:
	@echo OS=$(PLATFORM_OS) ARCH=$(PLATFORM_ARCH) PLATFORM=$(PLATFORM) RID=$(DOTNET_RID)
	@echo SHARED=$(SHARED_BIN)
	@echo JAVA=$(JAVA_NATIVE)
	@echo CSHARP=$(CSHARP_NATIVE)
	@echo PYTHON=$(PYTHON_NATIVE)

# ---- Очистка ----------------------------------------------------------------
# fclean удаляет и копии бинарника в обёртках текущей платформы: в
# репозиторий они не попадают (.gitignore), а устаревшая копия опаснее,
# чем отсутствующая — обёртка загрузила бы старую версию библиотеки.
clean:
	@$(call RMDIR,$(OBJ_DIR))
	@$(call RMDIR,$(DEP_DIR))

fclean: clean
	@$(call RMDIR,$(BIN_DIR))
	@$(call RMDIR,$(JAVA_NATIVE))
	@$(call RMDIR,$(JAVA_DIR)/target)
	@$(call RMDIR,$(CSHARP_NATIVE))
	@$(call RMDIR,$(PYTHON_NATIVE))

re: fclean
	@$(MAKE) --no-print-directory all

# ---- Подключение сгенерированных зависимостей -------------------------------
-include $(ALL_DEP)
