VERS = 1.08

#########################################
#	Edit this variables to fit your system configuration
#########################################
MIPS = /usr/share/crossCompilerMips/bin/mipsel-linux-gcc
ATHEROS = /usr/share/crossCompilerFonera/bin/mips-linux-uclibc-gcc
CC = gcc
#CFLAGS = `libprelude-config --cflags`
#LDFLAGS = `libprelude-config --libs`
LDFLAGS = -lpthread


INCLUDES = \
	-I. \
	-I../../common 

x86:
	@echo ""
	@echo "Compiling for X86 architecture..."
	$(CC) -g -o WatchDog WatchDog.c -Wall $(CFLAGS) $(LDFLAGS) $(INCLUDES)
	@echo ""

X86: x86

clean:
		@echo ""
		@echo "Deleting generating files..."
		rm -f WatchDog_MIPS	*~ *.o
		rm -f WatchDog_Fon	*~ *.o
		rm -f WatchDog		*~ *.o
