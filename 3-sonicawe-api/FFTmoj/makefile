SHELL := "C:\Program Files\Git\git-bash"

containing = $(foreach word,$2,$(if $(findstring $1,$(word)),$(word)))
not-containing = $(foreach word,$2,$(if $(findstring $1,$(word)),,$(word)))
interp = $(foreach word,$1,$(addprefix $(word),$2))
interp3=$(call interp,$1,$(call interp,$2,$3))
interp4=$(call interp,$1,$(call interp3,$2,$3,$4))
interp5=$(call interp,$1,$(call interp4,$2,$3,$4,$5))
interp6=$(call interp,$1,$(call interp5,$2,$3,$4,$5,$6))

SEPARATOR=/
SETS=1
RUNS=1
BENCHMARKS=Wall
LIBRARIES=Ooura
PLATFORMS=$(MACHINE)
LIBRARYTARGETS=$(call interp6,data$(SEPARATOR),$(PLATFORMS),$(SEPARATOR)set,$(SETS),$(SEPARATOR),$(LIBRARIES))
TARGETS=$(call interp6,$(LIBRARYTARGETS),$(SEPARATOR)run,$(RUNS),$(SEPARATOR),$(BENCHMARKS),Times.dat)

all: $(TARGETS)
	
$(TARGETS):
	@mkdir -p $(dir $@)
	@echo $(word 2,$(subst $(SEPARATOR), ,$@)) $(subst set,,$(word 3,$(subst $(SEPARATOR), ,$@))) $(subst run,,$(word 5,$(subst $(SEPARATOR), ,$@))) bench > argv.txt
	./runfft.bat