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
LDFLAGS =
LDFLAGS += -lole32
LDFLAGS += -luser32
LDFLAGS += -lpsapi
#LDFLAGS += -lgdiplus
#LDFLAGS += -lgdi32
#LDFLAGS += -lkernel32
#LDFLAGS += -ldwmapi
#LDFLAGS += -ladvapi32
#LDFLAGS += -lShell32
#LDFLAGS += -loleaut32
#LDFLAGS += -lcomctl32
#LDFLAGS += -lOleacc
#LDFLAGS += -luuid
#LDFLAGS += -ld3d9
#LDFLAGS += -lDbghelp
LDFLAGS += -ldwmapi
#LDFLAGS += -municode
LDFLAGS += -static-libgcc
LDFLAGS += -static-libstdc++
#LDFLAGS += -Wl,--subsystem,windows


# Compiler Optimization Flags: 
# You're already using the -Os flag, which optimizes for size.
# You can also consider using the -s flag to strip the symbols from the executable, further reducing the size. 
# Additionally, consider using -fdata-sections and -ffunction-sections flags to place each function and data item in its own section. 
# This can help the linker remove unused code and data, resulting in a smaller executable.
# gcc -Os -s -fdata-sections -ffunction-sections myfile.c -o myfile.exe -lole32


# Linker Flags: 
# Use linker flags to enable linker optimizations. 
# The -Wl,--gc-sections flag tells the linker to remove unused sections. 
# This complements the use of -fdata-sections and -ffunction-sections flags during compilation.
LDFLAGS += -Wl,--gc-sections

# Alignment: 
# You mentioned alignment. While it's generally beneficial for performance, it might not significantly affect the executable size. 
# However, you can experiment with alignment flags like: -Wl,--file-alignment=16 to align sections to a smaller value. 
# Be cautious, as smaller alignment might slightly affect performance.
# gcc -Os -s -fdata-sections -ffunction-sections myfile.c -o myfile.exe -Wl,--gc-sections -Wl,--file-alignment=16 -lole32
LDFLAGS += -Wl,--file-alignment=512




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
	#$(CC) $(CFLAGS) -o $(TARGET_EXE).exe main.o $(LDFLAGS)
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
#rm -Rf $(TARGET_EXE).exe 2>/dev/null || true	


# # # # # # # # # # # # # # # # # # 
docs:
	rm -Rf Doc/html 2>/dev/null || true
	"C:\Program Files\doxygen\bin\doxygen.exe" AltTabModDoxygen
	cp Doc/overview.gif Doc/html/overview.gif 

	# Remove the annoying sync buttons in the navtree. 
	sed -i "s#\(n.html('<img src=\"'+relpath+'sync_on.png\" title=\"'+SYNCONMSG+'\"\/>');\)#//\1#" "Doc/html/navtree.js"
	sed -i "s#\(n.html('<img src=\"'+relpath+'sync_off.png\" title=\"'+SYNCOFFMSG+'\"\/>');\)#//\1#" "Doc/html/navtree.js"
	@echo ""

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