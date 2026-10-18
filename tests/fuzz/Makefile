# ============================================================================
#  Берёста — цели libFuzzer (только Clang: -fsanitize=fuzzer есть только в нём)
#
#  Цели:
#    make             — собрать fuzz_parser, fuzz_xpath, fuzz_schema в bin/
#    make corpus      — собрать начальный корпус из тестов и набора W3C
#    make fuzz-<имя>  — запустить одну цель (FUZZ_TIME секунд, по умолчанию 600)
#    make fuzz        — запустить все цели ПАРАЛЛЕЛЬНО (make -j3 fuzz)
#    make clean       — удалить obj/ и dep/
#    make fclean      — clean + bin/ (корпус и находки не удаляются)
#    make re          — полная пересборка
#
#  Библиотека компилируется заново с санитайзерами (свои obj/lib/), чтобы
#  инструментирован был весь код, а не только обёртки целей.
#  Падения libFuzzer кладёт в artifacts/<цель>/ (crash-*, leak-*, timeout-*).
# ============================================================================
# Beresta — libFuzzer targets (Clang only: only it has -fsanitize=fuzzer)
# Targets:
#   make             — build fuzz_parser, fuzz_xpath, fuzz_schema into bin/
#   make corpus      — build the initial corpus from the tests and the W3C suite
#   make fuzz-<name> — run one target (FUZZ_TIME seconds, 600 by default)
#   make fuzz        — run all targets IN PARALLEL (make -j3 fuzz)
#   make clean       — remove obj/ and dep/
#   make fclean      — clean + bin/ (the corpus and the findings are not removed)
#   make re          — a full rebuild
# The library is compiled anew with sanitizers (its own obj/lib/) so that
# all the code is instrumented, not only the target wrappers.
# libFuzzer puts crashes into artifacts/<target>/ (crash-*, leak-*, timeout-*).
# ============================================================================

MAKEFILE_PATH := $(firstword $(MAKEFILE_LIST))

# ---- Имена и каталоги -------------------------------------------------------
# ---- Names and directories -------------------------------------------------------
ROOT_DIR    := ../..
LIB_SRC_DIR := $(ROOT_DIR)/src
SRC_DIR     := src
INC_DIR     := include
OBJ_DIR     := obj
DEP_DIR     := dep
BIN_DIR     := bin
CORPUS_DIR  := corpus
ARTIFACTS   := artifacts
DICT        := dict/xml.dict
TARGETS     := fuzz_parser fuzz_xpath fuzz_schema

# ---- Компилятор и флаги -----------------------------------------------------
# ---- The compiler and flags -----------------------------------------------------
CXX         := clang++
PYTHON      ?= python3
FUZZ_TIME   ?= 600
FUZZ_JOBS   ?= 1
# 1 — после падения продолжать поиск (режим разведки), 0 — остановиться на первом
# 1 — continue the search after a crash (exploration mode), 0 — stop at the first one
FUZZ_KEEP_GOING ?= 0
SANITIZE    := -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer
CXXFLAGS    := -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -O1 -g $(SANITIZE)
LIB_FLAGS   := -fsanitize=fuzzer-no-link
LINK_FLAGS  := -fsanitize=fuzzer $(SANITIZE)

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

# ---- Исходники ----------------------------------------------------------------
# ---- Sources ----------------------------------------------------------------
rwildcard   = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
dep_of      = $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$(1))
dir_of      = $(patsubst %/,%,$(dir $(1)))
DEPFLAGS     = -MMD -MP -MF $(call dep_of,$@) -MT $@

LIB_SRC     := $(call rwildcard,$(LIB_SRC_DIR),*.cpp)
LIB_OBJ     := $(LIB_SRC:$(LIB_SRC_DIR)/%.cpp=$(OBJ_DIR)/lib/%.o)
COMMON_SRC  := $(call rwildcard,$(SRC_DIR)/common,*.cpp)
COMMON_OBJ  := $(COMMON_SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TARGET_OBJ  := $(TARGETS:%=$(OBJ_DIR)/targets/%.o)
BINS        := $(TARGETS:%=$(BIN_DIR)/%$(EXE))
ALL_OBJ     := $(LIB_OBJ) $(COMMON_OBJ) $(TARGET_OBJ)
DIRS        := $(sort $(call dir_of,$(ALL_OBJ) $(call dep_of,$(ALL_OBJ))) $(BIN_DIR))

.PHONY: all corpus fuzz $(TARGETS:%=fuzz-%) clean fclean re

all: $(BINS)

# Каждая цель = своя точка входа + общие помощники + инструментированная библиотека
# Each target = its own entry point + common helpers + the instrumented library
$(BIN_DIR)/%$(EXE): $(OBJ_DIR)/targets/%.o $(COMMON_OBJ) $(LIB_OBJ) | $(BIN_DIR)
	$(CXX) $(LINK_FLAGS) $< $(COMMON_OBJ) $(LIB_OBJ) -o $@

.SECONDEXPANSION:
$(OBJ_DIR)/lib/%.o: $(LIB_SRC_DIR)/%.cpp $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CXX) $(CXXFLAGS) $(LIB_FLAGS) -I$(ROOT_DIR)/include $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(MAKEFILE_PATH) | $$(call dir_of,$$@) $$(call dir_of,$$(call dep_of,$$@))
	$(CXX) $(CXXFLAGS) $(LIB_FLAGS) -I$(INC_DIR) -I$(ROOT_DIR)/include $(DEPFLAGS) -c $< -o $@

$(DIRS):
	@$(call MKDIR,$@)

# ---- Корпус и запуск -----------------------------------------------------------
# ---- The corpus and running -----------------------------------------------------------
corpus:
	$(PYTHON) -I tools/make_corpus.py $(ROOT_DIR) $(CORPUS_DIR)

fuzz: $(TARGETS:%=fuzz-%)

# -rss_limit_mb и -timeout ловят взрывной рост памяти и зависания;
# -max_len ограничивает вход, -fork занимает FUZZ_JOBS ядер одной целью
# (новые входы сливаются обратно в корпус), FUZZ_KEEP_GOING=1 не даёт
# первому же падению остановить разведку — все находки ложатся в artifacts/
# -rss_limit_mb and -timeout catch explosive memory growth and hangs;
# -max_len limits the input, -fork occupies FUZZ_JOBS cores with one target
# (new inputs are merged back into the corpus), FUZZ_KEEP_GOING=1 does not let
# the very first crash stop the exploration — all findings go into artifacts/
$(TARGETS:%=fuzz-%): fuzz-%: $(BIN_DIR)/%$(EXE)
	@$(call MKDIR,$(CORPUS_DIR)/$(patsubst fuzz_%,%,$*))
	@$(call MKDIR,$(ARTIFACTS)/$*)
	$(call native,$(BIN_DIR)/$*$(EXE)) -max_total_time=$(FUZZ_TIME) -max_len=16384 -timeout=10 \
		-rss_limit_mb=2048 -dict=$(DICT) -artifact_prefix=$(ARTIFACTS)/$*/ \
		-fork=$(FUZZ_JOBS) -ignore_crashes=$(FUZZ_KEEP_GOING) -ignore_timeouts=$(FUZZ_KEEP_GOING) \
		-ignore_ooms=$(FUZZ_KEEP_GOING) -print_final_stats=1 \
		$(CORPUS_DIR)/$(patsubst fuzz_%,%,$*)

# ---- Очистка ------------------------------------------------------------------
# ---- Cleaning ------------------------------------------------------------------
clean:
	@$(call RMDIR,$(OBJ_DIR))
	@$(call RMDIR,$(DEP_DIR))

fclean: clean
	@$(call RMDIR,$(BIN_DIR))

re: fclean
	@"$(MAKE)" --no-print-directory all

-include $(call dep_of,$(ALL_OBJ))
