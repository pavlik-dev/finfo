# Define variables
CXX := g++
CXXFLAGS := -Wall -O2
LDFLAGS := -lmagic
MAIN_SRC := main.cpp
OUTPUT := finfo

all: $(OUTPUT)

# Check for libmagic.so.1
check_magic:
	@if [ $(ldconfig -p | grep libmagic.so.1) ]; then \
		echo "Error: libmagic.so.1 (magic.h) not found."; \
		exit 1; \
	fi

# Rule to compile main.cpp into the finfo executable, linking with libmagic
finfo: check_magic $(MAIN_SRC)
	$(CXX) $(CXXFLAGS) -o $(OUTPUT) $(MAIN_SRC) $(LDFLAGS)

# Clean target to remove compiled files
clean:
	rm -f $(OUTPUT)

install:
	sudo cp -r $(OUTPUT) /usr/bin/finfo

