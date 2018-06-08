SHELL := "C:\Program Files\Git\git-bash"

containing = $(foreach word,$2,$(if $(findstring $1,$(word)),$(word)))
not-containing = $(foreach word,$2,$(if $(findstring $1,$(word)),,$(word)))
interp = $(foreach word,$1,$(addprefix $(word),$2))
interp3=$(call interp,$1,$(call interp,$2,$3))
interp4=$(call interp,$1,$(call interp3,$2,$3,$4))
interp5=$(call interp,$1,$(call interp4,$2,$3,$4,$5))
interp6=$(call interp,$1,$(call interp5,$2,$3,$4,$5,$6))

SEPARATOR=/
SETS=1 2 3
RUNS=1 2 3 4 5
BENCHMARKS=Wall
LIBRARIES=Ooura ClFft ClAmdFft
PLATFORMS=$(MACHINE)
FIRSTPARTOFTARGETS=$(call interp5,data$(SEPARATOR),$(PLATFORMS),$(SEPARATOR)set,$(SETS),$(SEPARATOR))
LASTPARTOFTARGETS=$(call interp5,$(SEPARATOR)run,$(RUNS),$(SEPARATOR),$(BENCHMARKS),Times.dat)

all:
	@echo "Pick a library: Ooura, ClFft, or ClAmdFft."

.PHONY: $(LIBRARIES) clean $(interp,clean,$(LIBRARIES))

Ooura: $(call interp3,$(FIRSTPARTOFTARGETS),Ooura,$(LASTPARTOFTARGETS))
ClFft: $(call interp3,$(FIRSTPARTOFTARGETS),ClFft,$(LASTPARTOFTARGETS))
ClAmdFft: $(call interp3,$(FIRSTPARTOFTARGETS),ClAmdFft,$(LASTPARTOFTARGETS))

# TODO: FFTmoj should refuse to run if source is newer than binary.
$(call interp3,$(FIRSTPARTOFTARGETS),Ooura,$(LASTPARTOFTARGETS)):
	@echo "Making $@"
	@mkdir -p $(dir $@) && echo $(word 2,$(subst $(SEPARATOR), ,$@)) $(subst set,,$(word 3,$(subst $(SEPARATOR), ,$@))) $(subst run,,$(word 5,$(subst $(SEPARATOR), ,$@))) bench > argv.txt && ./runfft.bat

$(call interp3,$(FIRSTPARTOFTARGETS),ClFft,$(LASTPARTOFTARGETS)):
	@echo "Making $@"
	@mkdir -p $(dir $@) && echo $(word 2,$(subst $(SEPARATOR), ,$@)) $(subst set,,$(word 3,$(subst $(SEPARATOR), ,$@))) $(subst run,,$(word 5,$(subst $(SEPARATOR), ,$@))) bench > argv.txt && ./runclfft.bat

$(call interp3,$(FIRSTPARTOFTARGETS),ClAmdFft,$(LASTPARTOFTARGETS)):
	@echo "Making $@"
	@mkdir -p $(dir $@) && echo $(word 2,$(subst $(SEPARATOR), ,$@)) $(subst set,,$(word 3,$(subst $(SEPARATOR), ,$@))) $(subst run,,$(word 5,$(subst $(SEPARATOR), ,$@))) bench > argv.txt && ./runclamdfft.bat

clean: $(interp,clean,$(LIBRARIES))

# TODO: This only removes WallTimes, should remove all of e.g. Ooura, including folder
cleanOoura:
	@rm -f $(call interp3,$(FIRSTPARTOFTARGETS),Ooura,$(LASTPARTOFTARGETS))
cleanClFft:
	@rm -f $(call interp3,$(FIRSTPARTOFTARGETS),ClFft,$(LASTPARTOFTARGETS))
cleanClAmdFft:
	@rm -f $(call interp3,$(FIRSTPARTOFTARGETS),ClAmdFft,$(LASTPARTOFTARGETS))