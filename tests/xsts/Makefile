# ============================================================================
#  Берёста — прогонщик W3C XML Schema Test Suite (XSTS) (GCC / Clang / MinGW)
#
#  Цели:
#    make          — собрать прогонщик (библиотека собирается корневым Makefile)
#    make fetch    — скачать и распаковать набор xsts-2007-06-20 в xmlschema2006-11-06/
#    make run      — прогнать весь набор и записать отчёт xsts_report.md
#    make clean    — удалить obj/ и dep/
#    make fclean   — clean + bin/ (сам набор не трогается)
#    make re       — полная пересборка прогонщика
#
#  Набор тестов в репозиторий не кладётся (см. .gitignore рядом):
#  его скачивает make fetch (нужны curl и tar — они есть в Windows 10+,
#  Linux и macOS).
# ============================================================================
# Beresta — the W3C XML Schema Test Suite (XSTS) runner (GCC / Clang / MinGW)
# Targets:
#   make          — build the runner (the library is built by the root Makefile)
#   make fetch    — download and unpack the xsts-2007-06-20 suite into xmlschema2006-11-06/
#   make run      — run the whole suite and write the xsts_report.md report
#   make clean    — remove obj/ and dep/
#   make fclean   — clean + bin/ (the suite itself is not touched)
#   make re       — a full rebuild of the runner
# The test suite is not put into the repository (see the .gitignore next to it):
# make fetch downloads it (curl and tar are needed — they exist in Windows 10+,
# Linux and macOS).
# ============================================================================

MAKEFILE_PATH := $(firstword $(MAKEFILE_LIST))

# ---- Имена и каталоги -------------------------------------------------------
# ---- Names and directories -------------------------------------------------------
NAME        := xsts_runner
ROOT_DIR    := ../..
LIB         := $(ROOT_DIR)/bin/libberesta.a
SRC_DIR     := src
INC_DIR     := include
OBJ_DIR     := obj
DEP_DIR     := dep
BIN_DIR     := bin
SUITE_DIR   := xmlschema2006-11-06
REPORT      := xsts_report.md
LIMITS      := known_limitations.txt
SUITE_URL   := https://www.w3.org/XML/2004/xml-schema-test-suite/xmlschema2006-11-06/xsts-2007-06-20.tar.gz
SUITE_TAR   := xsts-2007-06-20.tar.gz

# ---- Компилятор и флаги -----------------------------------------------------
# EXTRA_FLAGS — для сборок с санитайзерами: make EXTRA_FLAGS="-fsanitize=address"
# ---- The compiler and flags -----------------------------------------------------
# EXTRA_FLAGS — for builds with sanitizers: make EXTRA_FLAGS="-fsanitize=address"
CXXFLAGS    := -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -O2 $(EXTRA_FLAGS)
# Путь к .d-файлу объекта (obj/x/y.o -> dep/x/y.d) и каталог файла без слеша
# The path to the .d file of an object (obj/x/y.o -> dep/x/y.d) and the file directory without a slash
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
    RMFILE       = if exist "$(call native,$(1))" del /Q "$(call native,$(1))"
else
    EXE         :=
    native       = $(1)
    MKDIR        = mkdir -p "$(1)"
    RMDIR        = rm -rf "$(1)"
    RMFILE       = rm -f "$(1)"
endif

# ---- Исходники ----------------------------------------------------------------
# ---- Sources ----------------------------------------------------------------
rwildcard   = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
SRC         := $(call rwildcard,$(SRC_DIR),*.cpp)
OBJ         := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEP         := $(call dep_of,$(OBJ))
BIN         := $(BIN_DIR)/$(NAME)$(EXE)
DIRS        := $(sort $(call dir_of,$(OBJ) $(DEP)) $(BIN_DIR))

.PHONY: all fetch run library clean fclean re

all: $(BIN)

# Библиотека собирается своим Makefile (он сам решает, что пересобрать)
# The library is built by its own Makefile (it decides itself what to rebuild)
library:
	"$(MAKE)" --no-print-directory -C $(ROOT_DIR) bin/libberesta.a

$(LIB): library

$(BIN): $(OBJ) library | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJ) $(LIB) -o $@

.SECONDEXPANSION:
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -I$(ROOT_DIR)/include $(DEPFLAGS) -c $< -o $@

$(DIRS):
	@$(call MKDIR,$@)

# ---- Набор тестов и прогон ----------------------------------------------------
# ---- The test suite and the run ----------------------------------------------------
fetch:
	curl -sSL -o $(SUITE_TAR) $(SUITE_URL)
	tar -xzf $(SUITE_TAR)
	@$(call RMFILE,$(SUITE_TAR))

run: $(BIN)
	$(call native,$(BIN)) $(SUITE_DIR) --limitations $(LIMITS) --report $(REPORT)

# ---- Очистка ------------------------------------------------------------------
# ---- Cleaning ------------------------------------------------------------------
clean:
	@$(call RMDIR,$(OBJ_DIR))
	@$(call RMDIR,$(DEP_DIR))

fclean: clean
	@$(call RMDIR,$(BIN_DIR))

re: fclean
	@"$(MAKE)" --no-print-directory all

-include $(DEP)
