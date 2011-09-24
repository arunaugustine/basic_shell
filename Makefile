CC = gcc
CCFLAGS = -g
INCLUDES = basic_shell.h
LIBRARIES = 
EXECUTABLES = basic_shell


# Command for creating an executable file
.c:
	$(CC) $(CCFLAGS) $(INCLUDES) -o $@ $@.c $(LIBRARIES)


# All files to be generated
all: $(EXECUTABLES)


# Clean the directory
clean: 
	rm -f $(EXECUTABLES)

