# This Makefile controls the build process of the project.
#
# Note to self..
# $@ (full target name of the current target)
# $? (returns the dependencies that are newer than the current target)
# $* (returns the text that corresponds to % in the target)
# $< (name of the first dependency)
# $^ (name of all the dependencies with space as the delimiter)


# Program name.
TARGET_EXE = AltTabMod
CC = gcc
CFLAGS = -Wall -Os
CFLAGS += -mtune=native
CFLAGS += -s -fdata-sections -ffunction-sections
LDFLAGS = -lole32
LDFLAGS += -luser32
LDFLAGS += -lpsapi
LDFLAGS += -ldwmapi
LDFLAGS += -static-libgcc
LDFLAGS += -static-libstdc++
LDFLAGS += -Wl,--subsystem,windows

# Linker Flags: 
# Use linker flags to enable linker optimizations. 
# The -Wl,--gc-sections flag tells the linker to remove unused sections. 
# This complements the use of -fdata-sections and -ffunction-sections flags during compilation.
LDFLAGS += -Wl,--gc-sections


# # # # # # # # # # # # # # # # # # 
.DEFAULT_GOAL := all
.PHONY: all
all: pre res build compile clean done


# # # # # # # # # # # # # # # # # # 
# General info
pre:
	@echo ""
	@echo "For more info use: make info"
	@echo ""
	@echo "Starting build process..!!!"    
    
# # # # # # # # # # # # # # # # # # 
# Build resource file. 
res:
	@echo "Creating resource file"
	windres.exe -I ..\AltTabMod -J rc -O coff -i resource.rc -o resource.res
	@echo ""

# # # # # # # # # # # # # # # # # # 
# build .object files
build:
	@echo "Makeing objects"
	$(CC) $(CFLAGS) -c main.c -o main.o $(LDFLAGS)
	@echo ""

# # # # # # # # # # # # # # # # # # 
# Compile object files.
compile:
	@echo "Makeing binary file"
	$(CC) $(CFLAGS) -o $(TARGET_EXE).exe resource.res main.o $(LDFLAGS)
	@echo ""

# # # # # # # # # # # # # # # # # # 
# Done message..
done:
	@echo ""
	@echo "Done!..." 
	@echo ""

# # # # # # # # # # # # # # # # # # 
# clean up the *.o files. the .res file and the .exe file
clean:
	rm -Rf *.o 2>/dev/null || true
	rm -Rf *.res 2>/dev/null || true	


# # # # # # # # # # # # # # # # # # 
# print out info for make usage.
info:
	@echo ""
	@echo "make ,builds the project."
	@echo "make clean, clean up build files."
	@echo "make res, creates the resource file"
	@echo "make build, creates object files"
	@echo "make compile, links it all together."
	@echo ""
