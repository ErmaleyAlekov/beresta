# ============================================================================
#  Берёста — замеры скорости и памяти против других XML-библиотек
#  (GCC / Clang / MinGW)
#
#  Цели:
#    make          — собрать программу замеров (библиотеку собирает корневой Makefile)
#    make run      — замерить и записать таблицы в benchmark_report.md
#    make clean    — удалить obj/ и dep/
#    make fclean   — clean + bin/
#    make re       — полная пересборка
#
#  Сравниваемые библиотеки подключаются, если их нашёл pkg-config (Linux,
#  macOS): libxml2, pugixml, Xerces-C, tinyxml2; RapidXML — по наличию
#  заголовка. Отключить любую: make WITH_XERCES=0. Без них программа меряет
#  одну «Берёсту». Сама «Берёста» от них не зависит: это только инструмент
#  сравнения, в библиотеку и её пакеты он не входит.
#  Ubuntu: sudo apt install libxml2-dev libpugixml-dev libxerces-c-dev libtinyxml2-dev librapidxml-dev
# ============================================================================
#  Beresta — speed and memory measurements against other XML libraries
#  (GCC / Clang / MinGW)
#
#  Targets:
#    make          — build the benchmark (the root Makefile builds the library)
#    make run      — measure and write the tables into benchmark_report.md
#    make clean    — remove obj/ and dep/
#    make fclean   — clean + bin/
#    make re       — a full rebuild
#
#  The compared libraries are included if pkg-config found them (Linux,
#  macOS): libxml2, pugixml, Xerces-C, tinyxml2; RapidXML — if its header
#  exists. Disable any of them: make WITH_XERCES=0. Without them the program
#  measures Beresta alone. Beresta itself does not depend on them: this is only
#  a comparison tool, it is not part of the library and its packages.
#  Ubuntu: sudo apt install libxml2-dev libpugixml-dev libxerces-c-dev libtinyxml2-dev librapidxml-dev
# ============================================================================

MAKEFILE_PATH := $(firstword $(MAKEFILE_LIST))

# ---- Имена и каталоги / Names and directories -------------------------------
NAME        := beresta_bench
ROOT_DIR    := ..
LIB         := $(ROOT_DIR)/bin/libberesta.a
SRC_DIR     := src
INC_DIR     := include
OBJ_DIR     := obj
DEP_DIR     := dep
BIN_DIR     := bin
REPORT      := benchmark_report.md

# ---- Компилятор и флаги / The compiler and flags ----------------------------
# Тот же уровень оптимизации, что у библиотеки и у пакетов дистрибутива (-O2)
# The same optimization level as the library and the distribution packages (-O2)
CXXFLAGS    := -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -O2 $(EXTRA_FLAGS)
dep_of       = $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$(1))
dir_of       = $(patsubst %/,%,$(dir $(1)))
DEPFLAGS     = -MMD -MP -MF $(call dep_of,$@) -MT $@

# ---- Команды оболочки под ОС / Shell commands for the OS --------------------
ifeq ($(OS),Windows_NT)
    SHELL       := cmd.exe
    .SHELLFLAGS := /C
    EXE         := .exe
    SYSTEM_LIBS := -lpsapi
    native       = $(subst /,\,$(1))
    MKDIR        = if not exist "$(call native,$(1))" mkdir "$(call native,$(1))"
    RMDIR        = if exist "$(call native,$(1))" rmdir /S /Q "$(call native,$(1))"
    HAS_PACKAGE  =
else
    EXE         :=
    SYSTEM_LIBS :=
    native       = $(1)
    MKDIR        = mkdir -p "$(1)"
    RMDIR        = rm -rf "$(1)"
    HAS_PACKAGE  = $(shell pkg-config --exists $(1) 2>/dev/null && echo 1)
endif

# ---- Поиск сравниваемых библиотек / Finding the compared libraries ----------
WITH_LIBXML2  ?= $(call HAS_PACKAGE,libxml-2.0)
WITH_PUGIXML  ?= $(call HAS_PACKAGE,pugixml)
WITH_XERCES   ?= $(call HAS_PACKAGE,xerces-c)
WITH_TINYXML2 ?= $(call HAS_PACKAGE,tinyxml2)
WITH_RAPIDXML ?= $(if $(wildcard /usr/include/rapidxml/rapidxml.hpp /usr/local/include/rapidxml/rapidxml.hpp),1)

rwildcard   = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
SRC         := $(SRC_DIR)/main.cpp $(call rwildcard,$(SRC_DIR)/core,*.cpp) $(call rwildcard,$(SRC_DIR)/engines/beresta,*.cpp)
DEFINES     :=
EXTRA_CFLAGS:=
EXTRA_LIBS  :=

# Каждая найденная библиотека добавляет свои исходники, макрос, флаги и компоновку
# Each found library adds its sources, a macro, flags and linking
ifeq ($(WITH_LIBXML2),1)
    SRC          += $(call rwildcard,$(SRC_DIR)/engines/libxml2,*.cpp)
    DEFINES      += -DBENCH_WITH_LIBXML2
    EXTRA_CFLAGS += $(shell pkg-config --cflags libxml-2.0)
    EXTRA_LIBS   += $(shell pkg-config --libs libxml-2.0)
endif
ifeq ($(WITH_PUGIXML),1)
    SRC          += $(call rwildcard,$(SRC_DIR)/engines/pugixml,*.cpp)
    DEFINES      += -DBENCH_WITH_PUGIXML
    EXTRA_CFLAGS += $(shell pkg-config --cflags pugixml)
    EXTRA_LIBS   += $(shell pkg-config --libs pugixml)
endif
ifeq ($(WITH_XERCES),1)
    SRC          += $(call rwildcard,$(SRC_DIR)/engines/xerces,*.cpp)
    DEFINES      += -DBENCH_WITH_XERCES
    EXTRA_CFLAGS += $(shell pkg-config --cflags xerces-c)
    EXTRA_LIBS   += $(shell pkg-config --libs xerces-c)
endif
ifeq ($(WITH_TINYXML2),1)
    SRC          += $(call rwildcard,$(SRC_DIR)/engines/tinyxml2,*.cpp)
    DEFINES      += -DBENCH_WITH_TINYXML2
    EXTRA_CFLAGS += $(shell pkg-config --cflags tinyxml2)
    EXTRA_LIBS   += $(shell pkg-config --libs tinyxml2)
endif
ifeq ($(WITH_RAPIDXML),1)
    SRC          += $(call rwildcard,$(SRC_DIR)/engines/rapidxml,*.cpp)
    DEFINES      += -DBENCH_WITH_RAPIDXML
endif

OBJ         := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEP         := $(call dep_of,$(OBJ))
BIN         := $(BIN_DIR)/$(NAME)$(EXE)
DIRS        := $(sort $(call dir_of,$(OBJ) $(DEP)) $(BIN_DIR))

.PHONY: all run library clean fclean re

all: $(BIN)

# Библиотека собирается своим Makefile (он сам решает, что пересобрать)
# The library is built by its own Makefile (it decides itself what to rebuild)
library:
	"$(MAKE)" --no-print-directory -C $(ROOT_DIR) bin/libberesta.a

$(LIB): library

$(BIN): $(OBJ) library | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJ) $(LIB) $(EXTRA_LIBS) $(SYSTEM_LIBS) -o $@

.SECONDEXPANSION:
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CXX) $(CXXFLAGS) $(DEFINES) $(EXTRA_CFLAGS) -I$(INC_DIR) -I$(ROOT_DIR)/include $(DEPFLAGS) -c $< -o $@

$(DIRS):
	@$(call MKDIR,$@)

# ---- Замер / The measurement ------------------------------------------------
run: $(BIN)
	$(call native,$(BIN)) --report $(REPORT)

# ---- Очистка / Cleaning -----------------------------------------------------
clean:
	@$(call RMDIR,$(OBJ_DIR))
	@$(call RMDIR,$(DEP_DIR))

fclean: clean
	@$(call RMDIR,$(BIN_DIR))

re: fclean
	@"$(MAKE)" --no-print-directory all

-include $(DEP)
