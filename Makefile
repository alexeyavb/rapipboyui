# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2025-02-09 - Several enhancements + project update mode
#   2025-02-09 - first version
#	2025-03-34 - raylib use rewrite
# ------------------------------------------------
####
# OS
####
LIBDIR ?= -L/lib

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CCFLAGS += -DLINUX
endif

ifeq ($(UNAME_S),Darwin)
	CCFLAGS += -Wc99-extensions -DOSX
endif
######################################
# target
######################################
TARGET ?= ralibpipos
######################################
# building variables
######################################
# debug build?
DEBUG ?= 1
RELEASE ?= 0
OPT ?= -O0 -Og -g3 -std=c99 -gdwarf-4
# ifeq ($(BUILD_MODE), "DEBUG")
# # optimization
# DEBUG  = 1
# RELEASE = 0
# OPT = -O0 -Og -g4 -ggdb4 -gstabs4-std=c99 --gdwarf-8
# else
# RELEASE = 1
# DEBUG = 0
# OPT = -O2 -std=c99
# endif

#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
	main.c \
	Core/Src/gltest_0.c \
	Core/Src/text_utils.c \
	Core/Src/local_timers.c \
	Core/Src/linux_evthread.c \
	Core/Src/loader_thread.c \
	Core/Src/simple_thread.c \
	Core/Src/global_variables.c \
	Core/Src/linux_i2c_thread.c

# C_SOURCES += $(call rwildcard, Core/Src, *.c)

# ASM sources
ASM_SOURCES =
# ASM sources
ASMM_SOURCES =

#######################################
# binaries
#######################################
PREFIX =
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
# CP=/usr/local/opt/binutils/bin/objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
# CP = $(PREFIX)objcopy
CP=/usr/local/opt/binutils/bin/objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
# CPU = -mcpu=cortex-m4
CPU =

# fpu
# FPU = -mfpu=fpv4-sp-d16
FPU = 

# float-abi
# FLOAT-ABI = -mfloat-abi=hard
FLOAT-ABI =

# mcu
# MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)
MCU =
# macros for gcc
# AS defines
AS_DEFS = 
# C defines
C_DEFS =  
# AS includes
AS_INCLUDES =

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT =

# C includes
C_INCLUDES =  \
	-ICore/Include \
	-I/usr/include \
	-I/usr/local/include 	

# libraries
ifeq ($(UNAME_S),Linux)
	LIBDIR += -L/usr/lib/aarch64-linux-gnu
	CFLAGS += -DEGL_NO_X11
	C_INCLUDES += -I/usr/include/libdrm -I/usr/include/GLES3 -I/usr/include/EGL

	LDLIBS += -lraylib -lGLESv2 -lEGL -lpthread -lrt -lm -lgbm -ldrm -ldl -latomic
	LDLIBS += -luuid -lm -li2c
	
	# Explicit link to libc
	ifeq ($(RAYLIB_LIBTYPE),SHARED)
		LDLIBS += -lc
	endif
endif

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

# ifdef GCC_PATH
CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) $(CCFLAGS) -Wall -fdata-sections -ffunction-sections
# else
# CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) $(CCFLAGS) -Wall -fdata-sections -ffunction-sections
# endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

LIBDIR += -L/usr/lib -L/usr/local/lib -L/usr/local/opt/
LDFLAGS = $(MCU) $(LIBDIR) $(LIBS) $(LDLIBS)

# default action: build all
all: $(BUILD_DIR)/$(TARGET)

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASMM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASMM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wall $< -o $@
$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@
$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET): $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/% | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/% | $(BUILD_DIR)
	$(BIN) $< $@
	
$(BUILD_DIR):
	mkdir $@

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
