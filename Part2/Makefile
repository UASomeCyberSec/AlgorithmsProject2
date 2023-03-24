CC = g++

# Mention default target
all:

%.o: %.cpp
	$(CC) -c -O2 -Wall -Wextra $<

program = lzw435M
program-objects = lzw435M.o
program-executable = lzw435M

$(program) : $(program-objects)
	$(CC) $^ -o $@

.PHONY: clean
clean :
	rm -f $(program-objects) $(program-executable)

all : $(program)