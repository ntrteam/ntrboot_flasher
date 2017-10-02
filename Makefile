#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
# SPECS is the directory containing the important build and link files
#---------------------------------------------------------------------------------
export TARGET		:=	$(shell basename $(CURDIR))
BUILD		:=	build
SOURCES		:=	source source/common source/gamecart flashcart_core flashcart_core/devices
DATA		:=	data
INCLUDES	:=	source source/common source/gamecart flashcart_core  flashcart_core/devices

#---------------------------------------------------------------------------------
# Setup some defines
#---------------------------------------------------------------------------------
LIBELM := $(CURDIR)/libelm3ds

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-marm

CFLAGS	:=	-g -O2 -Wall -Wextra -Wpedantic -Wno-unused-variable -Wno-unused-parameter\
			-march=armv5te -mtune=arm946e-s -fomit-frame-pointer\
			-ffunction-sections -fdata-sections\
			-ffast-math -fdiagnostics-color=always\
			$(ARCH)

CFLAGS	+=	$(INCLUDE) -DARM9 -D_GNU_SOURCE\
			-DNTRBOOT_FLASHER_VERSION=\"$(NTRBOOT_FLASHER_VERSION)\"\
			-DFLASHCART_CORE_VERSION=\"$(FLASHCART_CORE_VERSION)\"

ifdef DEBUG
CFLAGS += -D LOG_LEVEL=LOG_DEBUG
endif

CXXFLAGS	:= $(CFLAGS) -std=c++14 -fno-rtti -fno-exceptions -fno-use-cxa-atexit

CFLAGS	+=	-std=c11

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-g --specs=../stub.specs $(ARCH) -Wl,-Map,$(TARGET).map,--gc-sections

LIBS	:= -lelm3ds

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(LIBELM) $(PORTLIBS)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

export NTRBOOT_FLASHER_VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)
export FLASHCART_CORE_VERSION := $(shell git -C flashcart_core describe --abbrev=4 --dirty --always --tags)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES_BIN	:= $(addsuffix .o,$(BINFILES))
export OFILES_SRC	:= $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES := $(OFILES_BIN) $(OFILES_SRC)
export HFILES	:= $(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD) firm

$(LIBELM)/lib/libelm3ds.a: $(LIBELM)
	@$(MAKE) -C $<

$(BUILD): $(LIBELM)/lib/libelm3ds.a
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

firm: $(BUILD)
	@firmtool build $(OUTPUT).firm -n 0x20600000 -e 0 -D $(OUTPUT).bin -A 0x20600000 -C NDMA -i

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).bin $(TARGET).elf $(TARGET).firm
	@$(MAKE) -C $(LIBELM) clean


#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).bin	:	$(OUTPUT).elf
$(OUTPUT).elf	:	$(OFILES)

$(OFILES_SRC)	: $(HFILES)


#---------------------------------------------------------------------------------
%.bin: %.elf
	@$(OBJCOPY) -O binary $< $@
	@echo built ... $(notdir $@)

#---------------------------------------------------------------------------------
%_bin.h %.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)


-include $(DEPENDS)


#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
