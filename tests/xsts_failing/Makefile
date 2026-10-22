# ============================================================================
#  Берёста — отладчик провалов W3C XML Schema Test Suite (GCC / Clang / MinGW)
#
#  Собирает библиотеку, общий код прогонщика tests/xsts (кроме его main.cpp)
#  и свои исходники в ОДНУ программу с отладочной информацией: точки
#  останова ставятся и в отладчике, и внутри самой библиотеки.
#
#  Цели:
#    make                 — отладочная сборка (-g -O0) в bin/debug/
#    make run ARGS="..."  — прогнать случаи списка failing_cases.tsv (ARGS — фильтры)
#    make regen           — оптимизированная сборка (bin/release/) и пересборка
#                           failing_cases.tsv полным прогоном набора
#    make BUILD=release   — оптимизированная сборка без отладки
#    make clean / fclean / re
#
#  Набор лежит в ../xsts/xmlschema2006-11-06 (make fetch в tests/xsts).
# ============================================================================
# Beresta — the W3C XML Schema Test Suite failure debugger (GCC / Clang / MinGW)
# Builds the library, the shared code of the tests/xsts runner (except its main.cpp)
# and its own sources into ONE program with debug information: breakpoints
# can be set both in the debugger and inside the library itself.
# Targets:
#   make                 — a debug build (-g -O0) into bin/debug/
#   make run ARGS="..."  — run the cases of the failing_cases.tsv list (ARGS — filters)
#   make regen           — an optimized build (bin/release/) and a rebuild of
#                          failing_cases.tsv by a full suite run
#   make BUILD=release   — an optimized build without debugging
#   make clean / fclean / re
# The suite is in ../xsts/xmlschema2006-11-06 (make fetch in tests/xsts).
# ============================================================================

MAKEFILE_PATH := $(firstword $(MAKEFILE_LIST))

# ---- Имена и каталоги -------------------------------------------------------
# ---- Names and directories -------------------------------------------------------
NAME        := xsts_failing
BUILD       ?= debug
ROOT_DIR    := ../..
XSTS_DIR    := ../xsts
SRC_DIR     := src
INC_DIR     := include
OBJ_DIR     := obj/$(BUILD)
DEP_DIR     := dep/$(BUILD)
BIN_DIR     := bin/$(BUILD)
LIST        := failing_cases.tsv
ARGS        ?=

# ---- Компилятор и флаги -----------------------------------------------------
# Отладка по умолчанию: -O0 сохраняет все переменные и порядок строк для
# пошагового прохода. BUILD=release нужен для пересборки списка: полный
# прогон набора без оптимизации идёт в разы дольше.
# ---- The compiler and flags -----------------------------------------------------
# Debugging by default: -O0 keeps all variables and the line order for
# stepping. BUILD=release is needed to rebuild the list: a full
# suite run without optimization takes several times longer.
ifeq ($(BUILD),release)
    OPT_FLAGS := -O2
else
    OPT_FLAGS := -g -O0
endif
CXXFLAGS    := -std=c++17 -Wall -Wextra -Wpedantic -Wshadow $(OPT_FLAGS) $(EXTRA_FLAGS)
# Флаги библиотеки — как в корневом Makefile (язык сообщений — русский)
# The library flags — as in the root Makefile (the message language is Russian)
LIB_FLAGS   := -DBERESTA_BUILDING_SHARED -DBERESTA_ERRORS_LANG=ru
INCLUDES    := -I$(INC_DIR) -I$(XSTS_DIR)/include -I$(ROOT_DIR)/include
# Путь к .d-файлу объекта (obj/debug/x/y.o -> dep/debug/x/y.d) и каталог файла без слеша
# The path to the .d file of an object (obj/debug/x/y.o -> dep/debug/x/y.d) and the file directory without a slash
dep_of       = $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$(1))
dir_of       = $(patsubst %/,%,$(dir $(1)))
DEPFLAGS     = -MMD -MP -MF $(call dep_of,$@) -MT $@

# ---- Команды оболочки под ОС --------------------------------------------------
# ---- Shell commands for the OS --------------------------------------------------
ifeq ($(OS),Windows_NT)
    SHELL       := cmd.exe
    .SHELLFLAGS := /C
    EXE         := .exe
    native       = $(subst /,\,$(1))
    MKDIR        = if not exist "$(call native,$(1))" mkdir "$(call native,$(1))"
    RMDIR        = if exist "$(call native,$(1))" rmdir /S /Q "$(call native,$(1))"
else
    EXE         :=
    native       = $(1)
    MKDIR        = mkdir -p "$(1)"
    RMDIR        = rm -rf "$(1)"
endif

# ---- Исходники: три источника, у каждого своя подпапка obj/ ------------------
#   obj/<сборка>/library/  — библиотека (../../src)
#   obj/<сборка>/xsts/     — прогонщик tests/xsts без его main.cpp
#   obj/<сборка>/failing/  — исходники этой папки
# ---- Sources: three origins, each with its own obj/ subfolder ------------------
#   obj/<build>/library/  — the library (../../src)
#   obj/<build>/xsts/     — the tests/xsts runner without its main.cpp
#   obj/<build>/failing/  — the sources of this folder
rwildcard   = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
LIB_SRC     := $(call rwildcard,$(ROOT_DIR)/src,*.cpp)
XSTS_SRC    := $(filter-out $(XSTS_DIR)/src/main.cpp,$(call rwildcard,$(XSTS_DIR)/src,*.cpp))
OWN_SRC     := $(call rwildcard,$(SRC_DIR),*.cpp)
LIB_OBJ     := $(LIB_SRC:$(ROOT_DIR)/src/%.cpp=$(OBJ_DIR)/library/%.o)
XSTS_OBJ    := $(XSTS_SRC:$(XSTS_DIR)/src/%.cpp=$(OBJ_DIR)/xsts/%.o)
OWN_OBJ     := $(OWN_SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/failing/%.o)
OBJ         := $(LIB_OBJ) $(XSTS_OBJ) $(OWN_OBJ)
DEP         := $(call dep_of,$(OBJ))
BIN         := $(BIN_DIR)/$(NAME)$(EXE)
DIRS        := $(sort $(call dir_of,$(OBJ) $(DEP)) $(BIN_DIR))

.PHONY: all run regen clean fclean re

all: $(BIN)

$(BIN): $(OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

.SECONDEXPANSION:
$(OBJ_DIR)/library/%.o: $(ROOT_DIR)/src/%.cpp $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CXX) $(CXXFLAGS) $(LIB_FLAGS) -I$(ROOT_DIR)/include $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR)/xsts/%.o: $(XSTS_DIR)/src/%.cpp $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR)/failing/%.o: $(SRC_DIR)/%.cpp $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEPFLAGS) -c $< -o $@

$(DIRS):
	@$(call MKDIR,$@)

# ---- Прогон списка и его пересборка -------------------------------------------
# ---- Running the list and rebuilding it -------------------------------------------
run: $(BIN)
	$(call native,$(BIN)) $(ARGS)

# Пересборка списка — оптимизированной программой (отдельный вызов make,
# чтобы объекты release не смешивались с отладочными)
# Rebuilding the list — by the optimized program (a separate make call,
# so that the release objects do not mix with the debug ones)
regen:
	@"$(MAKE)" --no-print-directory BUILD=release all
	$(call native,bin/release/$(NAME)$(EXE)) --regenerate --list $(LIST)

# ---- Очистка ------------------------------------------------------------------
# ---- Cleaning ------------------------------------------------------------------
clean:
	@$(call RMDIR,obj)
	@$(call RMDIR,dep)

fclean: clean
	@$(call RMDIR,bin)

re: fclean
	@"$(MAKE)" --no-print-directory all

-include $(DEP)
