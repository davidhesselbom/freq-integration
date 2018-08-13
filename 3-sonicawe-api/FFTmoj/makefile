SHELL = "C:\Program Files\Git\bin\bash"

#DATA=data
DATA=G:\\data

containing = $(foreach word,$2,$(if $(findstring $1,$(word)),$(word)))
not-containing = $(foreach word,$2,$(if $(findstring $1,$(word)),,$(word)))
interp = $(foreach word,$1,$(addprefix $(word),$2))
interp3=$(call interp,$1,$(call interp,$2,$3))
interp4=$(call interp,$1,$(call interp3,$2,$3,$4))
interp5=$(call interp,$1,$(call interp4,$2,$3,$4,$5))
interp6=$(call interp,$1,$(call interp5,$2,$3,$4,$5,$6))
interp7=$(call interp,$1,$(call interp6,$2,$3,$4,$5,$6,$7))

SEPARATOR=/
SETS=1 2 3 4 5
RUNS=1 2 3
BENCHMARKS=Wall
LIBRARIES=Ooura ClFft ClAmdFft
PLATFORMS=$(MACHINE)
FIRSTPARTOFTARGETS=$(call interp3,data$(SEPARATOR),$(PLATFORMS),$(SEPARATOR))
LASTPARTOFTARGETS=$(call interp7,$(SEPARATOR)set,$(SETS),$(SEPARATOR)run,$(RUNS),$(SEPARATOR),$(BENCHMARKS),Times1024.dat)

all:
	@echo "Pick a library: Ooura, ClFft, ClAmdFft, or Batches (for ClAmdFft batches)."

.PHONY: $(LIBRARIES) clean $(interp,clean,$(LIBRARIES)) $(interp,$(LIBRARIES),Binary)

release/cpu/FFTmoj.exe: tst_FFTmojtest.cpp
	$(error Ooura binary needs to be rebuilt!)

release/opencl/amdapple/FFTmoj.exe: tst_FFTmojtest.cpp
	$(error ClFft binary needs to be rebuilt!)

release/opencl/amdamd/FFTmoj.exe: tst_FFTmojtest.cpp
	$(error ClAmdFft binary needs to be rebuilt!)

Ooura: release/cpu/FFTmoj.exe $(call interp3,$(FIRSTPARTOFTARGETS),Ooura,$(LASTPARTOFTARGETS))
ClFft: release/opencl/amdapple/FFTmoj.exe $(call interp3,$(FIRSTPARTOFTARGETS),ClFft,$(LASTPARTOFTARGETS))
ClAmdFft: release/opencl/amdamd/FFTmoj.exe $(call interp3,$(FIRSTPARTOFTARGETS),ClAmdFft,$(LASTPARTOFTARGETS))
Batches: release/opencl/amdamd/FFTmoj.exe $(call interp3,$(FIRSTPARTOFTARGETS),ClAmdFft,$(LASTPARTOFTARGETS))

$(call interp3,$(FIRSTPARTOFTARGETS),Ooura,$(LASTPARTOFTARGETS)): release/cpu/FFTmoj.exe
	@echo "Making $@"
	@mkdir -p $(dir $@) && echo $(word 2,$(subst $(SEPARATOR), ,$@)) $(subst set,,$(word 4,$(subst $(SEPARATOR), ,$@))) $(subst run,,$(word 5,$(subst $(SEPARATOR), ,$@))) bench > argv.txt && ./runfft.bat

$(call interp3,$(FIRSTPARTOFTARGETS),ClFft,$(LASTPARTOFTARGETS)): release/opencl/amdapple/FFTmoj.exe
	@echo "Making $@"
	@mkdir -p $(dir $@) && echo $(word 2,$(subst $(SEPARATOR), ,$@)) $(subst set,,$(word 4,$(subst $(SEPARATOR), ,$@))) $(subst run,,$(word 5,$(subst $(SEPARATOR), ,$@))) bench > argv.txt && ./runclfft.bat

$(call interp3,$(FIRSTPARTOFTARGETS),ClAmdFft,$(LASTPARTOFTARGETS)): release/opencl/amdamd/FFTmoj.exe
	@echo "Making $@"
	@mkdir -p $(dir $@) && echo $(word 2,$(subst $(SEPARATOR), ,$@)) $(subst set,,$(word 4,$(subst $(SEPARATOR), ,$@))) $(subst run,,$(word 5,$(subst $(SEPARATOR), ,$@))) bench > argv.txt && ./runclamdfft.bat

$(call interp3,$(FIRSTPARTOFTARGETS),ClAmdFft,$(LASTPARTOFTARGETS)): release/opencl/amdamd/FFTmoj.exe
	@echo "Making $@"
	@mkdir -p $(dir $@) && echo $(word 2,$(subst $(SEPARATOR), ,$@)) $(subst set,,$(word 4,$(subst $(SEPARATOR), ,$@))) $(subst run,,$(word 5,$(subst $(SEPARATOR), ,$@))) batch > argv.txt && ./runclamdfft.bat

clean: $(call interp,clean,$(LIBRARIES))
	

cleanall:
	rm -rf $(call interp,data$(SEPARATOR),$(PLATFORMS))
cleanOoura:
	@rm -f $(call interp3,$(FIRSTPARTOFTARGETS),Ooura,$(LASTPARTOFTARGETS))
cleanClFft:
	@rm -f $(call interp3,$(FIRSTPARTOFTARGETS),ClFft,$(LASTPARTOFTARGETS))
cleanClAmdFft:
	@rm -f $(call interp3,$(FIRSTPARTOFTARGETS),ClAmdFft,$(LASTPARTOFTARGETS))

sanityCheck:
	octave-cli --eval "sanity(\"$(DATA)\")"
