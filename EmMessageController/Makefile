PROJECT=EmMessageController

# Two additional CFLAGS must be used for Angstrom
# They must not be used for Debian or Ubuntu. I couldn't find out why. 
# The hint came from C:\gcc-linaro\share\doc\gcc-linaro-arm-linux-gnueabihf\README.txt 
#
# Uncomment the following line if you use Angstrom on your BeagleBone
#TARGET=angstrom

# Directory for C-Source
vpath %.c $(CURDIR)/source

# Directory for includes
CINCLUDE = $(CURDIR)/include  

# Directory for object files
OBJDIR = $(CURDIR)/object

# Other dependencies
DEPS = \
 Makefile \
 include/$(PROJECT).h \
# include/tools.h

# Compiler object files 
COBJ = \
 $(OBJDIR)/$(PROJECT).o \
# $(OBJDIR)/tools.o

# gcc binaries to use
##CC = "C:\GCC\gcc-linaro\bin\arm-linux-gnueabihf-gcc.exe"
##LD = "C:\GCC\gcc-linaro\bin\arm-linux-gnueabihf-gcc.exe"

CC = "C:\SysGCC\Beaglebone\bin\arm-linux-gnueabihf-gcc.exe"
LD = "C:\SysGCC\Beaglebone\bin\arm-linux-gnueabihf-gcc.exe"


# rm is part of yagarto-tools
SHELL = cmd
REMOVE = rm -f

# Compiler options
# Two additional flags neccessary for Angstrom Linux. Don't use them with Ubuntu or Debian  
CFLAGS = -marm
ifeq ($(TARGET),angstrom)
CFLAGS += -march=armv4t
CFLAGS += -mfloat-abi=soft
endif
CFLAGS += -O0 
CFLAGS += -g 
CFLAGS += -I.
CFLAGS += -I$(CINCLUDE)
CFLAGS += $(CDEFINE)


# for a better output
MSG_EMPTYLINE = . 
MSG_COMPILING = ---COMPILE--- 
MSG_LINKING = ---LINK--- 
MSG_SUCCESS = ---SUCCESS--- 

# Our favourite
all: $(PROJECT)

# Linker call
$(PROJECT): $(COBJ)
	@echo $(MSG_EMPTYLINE)
	@echo $(MSG_LINKING)
	$(LD) -lm -o $@ $^ $(CFLAGS)
	@echo $(MSG_EMPTYLINE)
	@echo $(MSG_SUCCESS) $(PROJECT)

# Compiler call
$(COBJ): $(OBJDIR)/%.o: %.c $(DEPS)
	@echo $(MSG_EMPTYLINE)
	@echo $(MSG_COMPILING) $<
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(REMOVE) $(OBJDIR)/*.o
	$(REMOVE) $(PROJECT)

