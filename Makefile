# Define variables
CXX := g++
CXXFLAGS := -Wall -O2
EXTS_DIR := exts
MAIN_SRC := main.cpp
EXT_SRC := mime_extension.cpp
EXT_OUTPUT := $(EXTS_DIR)/extension.ext
#CURDIR := $(shell realpath ./)
#BASENAME := $(shell basename $(CURDIR))

# Rule to create the exts/ folder
$(EXTS_DIR):
	mkdir -p $(EXTS_DIR)

# Check for magic.h (using pkg-config for simplicity)
check_magic:
	@if ! pkg-config --exists libmagic; then \
		echo "Error: libmagic (magic.h) not found."; \
		exit 1; \
	fi

# Rule to compile main.cpp into the finfo executable, linking with libmagic
finfo: check_magic $(MAIN_SRC)
	$(CXX) $(CXXFLAGS) -o finfo $(MAIN_SRC) $(LDFLAGS) -rdynamic -g

# Rule to compile ext.cpp into a shared object with a custom extension (.ext)
$(EXT_OUTPUT): $(EXT_SRC) | $(EXTS_DIR)
	$(CXX) $(CXXFLAGS) -fPIC -shared -o $(EXT_OUTPUT) $(EXT_SRC)

# Clean target to remove compiled files
clean:
	rm -f finfo $(EXT_OUTPUT)
	rm -rf $(EXTS_DIR)

install:
	sudo cp -r ./ /opt/finfo
	sudo ln -s /opt/finfo/finfo /usr/bin/finfo
	@echo -e "\n\033[1mInstallation done!\033[0m"
	@echo "You're all set now!"

# Default target
all: finfo $(EXT_OUTPUT)
