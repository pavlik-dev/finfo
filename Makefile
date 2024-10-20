# Makefile

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -lmagic

# Output file
OUTPUT = finfo

# Source file
SRC = main.cpp

# Rule to compile the program
$(OUTPUT): $(SRC)
	$(CXX) $(SRC) $(CXXFLAGS) -o $(OUTPUT)

#nomime: $(SRC)
#	$(CXX) $(SRC) -o $(OUTPUT) -D NO_MIME

#noemojis: $(SRC)
#	$(CXX) $(SRC) $(CXXFLAGS) -o $(OUTPUT) -D NO_EMOJIS

check: $(OUTPUT)
	$(CXX) $(SRC) $(CXXFLAGS) -o $(OUTPUT)
	rm -f $(OUTPUT)

install: $(OUTPUT)
	cp $(OUTPUT) /usr/bin/

userinstall: $(OUTPUT)
	cp $(OUTPUT) ~/.local/bin/

# Clean up generated files
clean:
	rm -f $(OUTPUT)
