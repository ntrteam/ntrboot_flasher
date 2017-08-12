rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/base_tools

name := ntrboot_flasher

dir_libelm := libelm
dir_ntrboot_flasher := ntrboot_flasher

.PHONY: $(dir_libelm)
.PHONY: $(dir_ntrboot_flasher)

.PHONY: all
.PHONY: firm

all: ntrboot_flasher.firm
ntrboot_flasher.firm: $(dir_ntrboot_flasher)/ntrboot_flasher.firm
	@cp $< $(@D)

$(dir_libelm)/libelm.a: $(dir_libelm)
	@$(MAKE) -C $<

$(dir_ntrboot_flasher)/ntrboot_flasher.firm: $(dir_ntrboot_flasher) $(dir_libelm)/libelm.a
	@$(MAKE) -C $<

.PHONY: clean
clean:
	@$(MAKE) -C $(dir_libelm) clean
	@$(MAKE) -C $(dir_ntrboot_flasher) clean
	rm -rf ntrboot_flasher.firm
