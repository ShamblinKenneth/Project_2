# Makefile for YouTube Tag Correlation Analyzer
# Place at repo root. Expects sources in src/*.cpp and data/*.csv
#
# Usage:
#   make            # build (default)
#   make run        # run binary (interactive)
#   make count-rows # show rows per CSV and total (excludes header)
#   make format     # run clang-format on source files (if installed)
#   make clean      # remove build artifacts
#   make rebuild    # clean then build

# Compiler + flags
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pipe

# Paths
SRCDIR   := src
BUILDDIR := build
BINDIR   := bin
TARGET   := $(BINDIR)/yt_analyzer

SRC      := $(wildcard $(SRCDIR)/*.cpp)
OBJ      := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC))

# Default target
.PHONY: all
all: $(TARGET)

# Build directories
$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

$(BINDIR):
	@mkdir -p $(BINDIR)

# Compile each source file to object
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	@echo "Compiling: $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Link
$(TARGET): $(OBJ) | $(BINDIR)
	@echo "Linking: $@"
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

# Run (interactive)
.PHONY: run
run: $(TARGET)
	@echo "Running $(TARGET) — press Ctrl+C to quit"
	@./$(TARGET)

# Count rows across CSVs in data/ (excludes one header per file)
# Requires bash + awk; prints each file's number of data rows and a total.
.PHONY: count-rows
count-rows:
	@echo "Counting rows (excluding header) for CSVs in data/:"
	@if [ -d data ] ; then \
	  total=0; \
	  for f in data/*.csv; do \
	    if [ -f "$$f" ]; then \
	      cnt=$$(awk 'FNR>1{c++} END{print c+0}' "$$f"); \
	      echo "$$f: $$cnt"; \
	      total=$$(expr $$total + $$cnt); \
	    fi; \
	  done; \
	  echo "Total rows across CSVs (excl. headers): $$total"; \
	else \
	  echo "No data/ folder found."; exit 1; \
	fi

# Format source files with clang-format (if installed)
.PHONY: format
format:
	@if command -v clang-format >/dev/null 2>&1; then \
	  echo "Running clang-format on $(SRC)"; \
	  clang-format -i $(SRC); \
	else \
	  echo "clang-format not found — install it to use this target"; \
	fi

# Remove build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build and bin directories..."
	@rm -rf $(BUILDDIR) $(BINDIR) results.csv

.PHONY: rebuild
rebuild: clean all

# Developer helper: run the program non-interactively using a here-doc.
# Example: make run-auto TAGS="music,gaming"
# NOTE: this assumes program prompts match the interactive flow.
.PHONY: run-auto
run-auto: $(TARGET)
ifdef TAGS
	@echo "Running non-interactive using TAGS='$(TAGS)'"
	@# Adjust the here-doc below to match the binary's prompt sequence if needed.
	@printf "1\n$(TAGS)\n2\n1\n3\n" | ./$(TARGET)
else
	@echo "Usage: make run-auto TAGS=\"music,gaming\""
endif
