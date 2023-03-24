CC = g++

# Mention default target
all:

%.o: %.cpp
	$(CC) -c -O2 -Wall -Wextra $<

program = lzw435
program-objects = lzw435.o
program-executable = lzw435

$(program) : $(program-objects)
	$(CC) $^ -o $@

.PHONY: clean
clean :
	rm -f $(program-objects) $(program-executable)

all : $(program)