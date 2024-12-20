
# 0 = disable
# 1 = enable

ENABLE_REMOTE_CONTROL			?= 0
ENABLE_UART_DEBUG			  	?= 1

#------------------------------------------------------------------------------
AUTHOR_STRING ?= JOAQUIM.ORG
VERSION_STRING ?= V0.0.1
PROJECT_NAME := uv-kx_$(VERSION_STRING)

BUILD := _build
BIN := firmware

EXTERNAL_LIB := external
LINKER := linker
BSP := bsp
SRC := src

LD_FILE := $(LINKER)/firmware.ld

#------------------------------------------------------------------------------
# Tool Configure
#------------------------------------------------------------------------------

PYTHON = python

# Toolchain commands
# Should be added to your PATH
CROSS_COMPILE ?= arm-none-eabi-
CC      = $(CROSS_COMPILE)gcc
CXX     = $(CROSS_COMPILE)g++
AS      = $(CROSS_COMPILE)as
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE    = $(CROSS_COMPILE)size

# Set make directory command, Windows tries to create a directory named "-p" if that flag is there.
ifeq ($(OS), Windows_NT) # windows
#	MKDIR = mkdir $(subst /,\,$(1)) > nul 2>&1 || (exit 0)
	MKDIR = powershell -Command "New-Item -ItemType Directory -Force -Path"
	RM = powershell -Command "Remove-Item -Recurse -Force"
#	RM = rmdir /s /q
	FixPath = $(subst /,\,$1)
	WHERE = where
	DEL = del /q
	K5PROG = utils/k5prog/k5prog.exe -D -F -YYYYY -p /dev/$(COMPORT) -b
else
	MKDIR = mkdir -p $(1)
	RM = rm -rf
	FixPath = $1
	WHERE = which
	DEL = del
	K5PROG = utils/k5prog/k5prog -D -F -YYY -p /dev/$(COMPORT) -b
endif

ifneq (, $(shell $(WHERE) python))
	MY_PYTHON := python
else ifneq (, $(shell $(WHERE) python3))
	MY_PYTHON := python3
endif

ifdef MY_PYTHON
	HAS_CRCMOD := $(shell $(MY_PYTHON) -c "import crcmod" 2>&1)
endif

ifndef MY_PYTHON
$(info )
$(info !!!!!!!! PYTHON NOT FOUND, *.PACKED.BIN WON'T BE BUILT)
$(info )
else ifneq (,$(HAS_CRCMOD))
$(info )
$(info !!!!!!!! CRCMOD NOT INSTALLED, *.PACKED.BIN WON'T BE BUILT)
$(info !!!!!!!! run: pip install crcmod)
$(info )
endif

#------------------------------------------------------------------------------
# ASM flags
ASMFLAGS =
ASMFLAGS += -mcpu=cortex-m0

# C flags
CCFLAGS = 
CCFLAGS += -Wall -Werror -mcpu=cortex-m0 -fno-builtin -fshort-enums -fno-delete-null-pointer-checks -MMD
#-g
#CCFLAGS += -flto
#CCFLAGS += -ftree-vectorize -funroll-loops
CCFLAGS += -Wextra -Wno-unused-function -Wno-unused-variable -Wno-unknown-pragmas
#-Wunused-parameter -Wconversion
CCFLAGS += -fno-math-errno -pipe -ffunction-sections -fdata-sections -ffast-math
#CCFLAGS += -fsingle-precision-constant -finline-functions-called-once
CCFLAGS += -Os -g3 -fno-exceptions -fno-non-call-exceptions -fno-delete-null-pointer-checks
CCFLAGS += -DARMCM0

# C++ flags
CXXFLAGS =
CXXFLAGS += -Wall -Werror -mcpu=cortex-m0 -fno-builtin -fshort-enums -fno-delete-null-pointer-checks -MMD
#-g
#CXXFLAGS += -flto
#CXXFLAGS += -ftree-vectorize -funroll-loops
CXXFLAGS += -Wextra -Wunused-parameter -Wconversion -Wno-unknown-pragmas
CXXFLAGS += -fno-math-errno -pipe -ffunction-sections -fdata-sections -ffast-math
#CXXFLAGS += -fsingle-precision-constant -finline-functions-called-once
CXXFLAGS += -std=c++17 -pedantic -Wno-expansion-to-defined -fno-rtti
CXXFLAGS += -Os -g3 -fno-exceptions -fno-non-call-exceptions -fno-delete-null-pointer-checks
CXXFLAGS += -DARMCM0
#-O3

# Linker flags
LDFLAGS =
LDFLAGS += -z noseparate-code -z noexecstack -mcpu=cortex-m0 -nostartfiles -Wl,-L,linker -Wl,-T,$(LD_FILE) -Wl,--gc-sections
LDFLAGS += -Wl,--build-id=none

# Use newlib-nano instead of newlib
LDFLAGS += --specs=nano.specs -lc -lnosys -mthumb -mabi=aapcs -lm -fno-rtti -fno-exceptions

#show size
LDFLAGS += -Wl,--print-memory-usage

#------------------------------------------------------------------------------

CCFLAGS += -DPRINTF_INCLUDE_CONFIG_H
CXXFLAGS += -DAUTHOR_STRING=\"$(AUTHOR_STRING)\" -DVERSION_STRING=\"$(VERSION_STRING)\"

ifeq ($(ENABLE_UART_DEBUG),1)
	CXXFLAGS += -DENABLE_UART_DEBUG
endif
ifeq ($(ENABLE_REMOTE_CONTROL),1)
	CXXFLAGS += -DENABLE_REMOTE_CONTROL
endif


#------------------------------------------------------------------------------
ASM_SRC = $(LINKER)/start.S
ASM_OBJS = $(addprefix $(BUILD)/, $(ASM_SRC:.S=.o))

#------------------------------------------------------------------------------
# FreeRTOS Library source and object files
FREERTOS_SRCS += $(EXTERNAL_LIB)/FreeRTOS/list.c
FREERTOS_SRCS += $(EXTERNAL_LIB)/FreeRTOS/queue.c
FREERTOS_SRCS += $(EXTERNAL_LIB)/FreeRTOS/tasks.c
FREERTOS_SRCS += $(EXTERNAL_LIB)/FreeRTOS/timers.c
FREERTOS_SRCS += $(EXTERNAL_LIB)/FreeRTOS/portable/GCC/ARM_CM0/port.c

FREERTOS_OBJS = $(addprefix $(BUILD)/, $(FREERTOS_SRCS:.c=.o))
	
INCLUDE_PATH += $(EXTERNAL_LIB)/FreeRTOS/include/.
INCLUDE_PATH += $(EXTERNAL_LIB)/FreeRTOS/portable/GCC/ARM_CM0/.

#------------------------------------------------------------------------------
# CMSIS Library source and object files
#CMSIS_SRCS = $(wildcard $(EXTERNAL_LIB)/CMSIS_5/*.c)
#CMSIS_OBJS = $(addprefix $(BUILD)/, $(CMSIS_SRCS:.c=.o))

INCLUDE_PATH += $(EXTERNAL_LIB)/CMSIS_5/CMSIS/Core/Include/.
INCLUDE_PATH += $(EXTERNAL_LIB)/CMSIS_5/Device/ARM/ARMCM0/Include/.

#------------------------------------------------------------------------------
# printf Library source and object files
PRINTF_SRCS = $(wildcard $(EXTERNAL_LIB)/printf/*.c)
PRINTF_OBJS = $(addprefix $(BUILD)/, $(PRINTF_SRCS:.c=.o))

INCLUDE_PATH += $(EXTERNAL_LIB)/printf/.

#------------------------------------------------------------------------------
# u8g2 Library source and object files

U8G2_SRCS = $(wildcard $(EXTERNAL_LIB)/U8G2/csrc/*.c)
U8G2_OBJS = $(addprefix $(BUILD)/, $(U8G2_SRCS:.c=.o))

U8G2_SRCSXX = $(wildcard $(EXTERNAL_LIB)/U8G2/cppsrc/*.cpp)
U8G2_OBJSXX = $(addprefix $(BUILD)/, $(U8G2_SRCSXX:.cpp=.o))

INCLUDE_PATH += $(EXTERNAL_LIB)/U8G2/csrc/.
INCLUDE_PATH += $(EXTERNAL_LIB)/U8G2/cppsrc/.
#------------------------------------------------------------------------------

# Main firmware source and object files
APP_SRCS += $(wildcard $(SRC)/*.cpp)
APP_SRCS += $(wildcard $(SRC)/driver/*.cpp)
APP_SRCS += $(wildcard $(SRC)/system/*.cpp)
APP_SRCS += $(wildcard $(SRC)/apps/*.cpp)
APP_SRCS += $(wildcard $(SRC)/radio/*.cpp)
APP_SRCS += $(wildcard $(SRC)/ui/*.cpp)
APP_OBJS = $(addprefix $(BUILD)/, $(APP_SRCS:.cpp=.o))

# Find all include directories recursively
INCLUDE_PATH += $(BSP)/dp32g030/.
INCLUDE_PATH += $(EXTERNAL_LIB)/.
INCLUDE_PATH += $(SRC)/.
INCLUDE_PATH += $(shell powershell -Command "Get-ChildItem -Path $(SRC) -Directory -Recurse | Resolve-Path -Relative")

#------------------------------------------------------------------------------

INC_PATHS = $(addprefix -I,$(INCLUDE_PATH))

#------------------------------------------------------------------------------
# Phony targets
.PHONY: all app directories clean prog

# Default target
#all: $(BUILD) $(BUILD)/$(PROJECT_NAME).out $(BIN)

all: directories app

# Create necessary directories
directories:
	@if not exist $(BUILD) $(MKDIR) $(BUILD)
	@if not exist $(BIN) $(MKDIR) $(BIN)

#------------------------------------------------------------------------------

OBJECTS = $(ASM_OBJS) $(FREERTOS_OBJS) $(CMSIS_OBJS) $(PRINTF_OBJS) $(U8G2_OBJS) $(U8G2_OBJSXX) $(APP_OBJS)

-include $(OBJECTS:.o=.d)

$(BUILD)/%.o: %.c
	@echo CC $<
	@if not exist $(@D) $(MKDIR) $(@D)
	@$(CC) $(CCFLAGS) $(INC_PATHS) -c $< -o $@

$(BUILD)/%.o: %.cpp
	@echo GCC $<
	@if not exist $(@D) $(MKDIR) $(@D)
	@$(CXX) $(CXXFLAGS) $(INC_PATHS) -c $< -o $@

# Assemble files
$(BUILD)/%.o: %.S
	@echo AS $<
	@if not exist $(@D) $(MKDIR) $(@D)
	@$(CXX) -x assembler-with-cpp $(ASMFLAGS) $(INC_PATHS) -c $< -o $@
#------------------------------------------------------------------------------

# Main firmware
app: $(BUILD)/$(PROJECT_NAME).out

$(BUILD)/$(PROJECT_NAME).out: $(OBJECTS)
	@echo LD $@
	@$(CC) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
#------------------- Binary generator -----------------------------------------	
	@echo Create $(notdir $@)
	@$(OBJCOPY) -O binary $(BUILD)/$(PROJECT_NAME).out $(BIN)/$(PROJECT_NAME).bin
	@echo Create $(PROJECT_NAME).packed.bin
	@-$(MY_PYTHON) utils/fw-pack.py $(BIN)/$(PROJECT_NAME).bin $(AUTHOR_STRING) $(VERSION_STRING) $(BIN)/$(PROJECT_NAME).packed.bin


prog: all
	$(K5PROG) $(BIN)/$(PROJECT_NAME).bin

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

# Clean build artifacts
clean:
	@if exist $(BUILD) $(RM) $(BUILD)
	@if exist $(BIN) $(RM) $(BIN)

# Print help information
help:
	@echo Makefile targets:
	@echo   all     - Build all
	@echo   prog    - Flash firmware
	@echo   clean   - Remove all build artifacts