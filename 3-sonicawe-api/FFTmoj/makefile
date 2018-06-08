containing = $(foreach word,$2,$(if $(findstring $1,$(word)),$(word)))
not-containing = $(foreach word,$2,$(if $(findstring $1,$(word)),,$(word)))
interp = $(foreach word,$1,$(addprefix $(word),$2))
interp3=$(call interp,$1,$(call interp,$2,$3))
interp4=$(call interp,$1,$(call interp3,$2,$3,$4))
interp5=$(call interp,$1,$(call interp4,$2,$3,$4,$5))
interp6=$(call interp,$1,$(call interp5,$2,$3,$4,$5,$6))

SETS=1 2 3
RUNS=1 2 3 4 5
BENCHMARKS=Wall
LIBRARIES=Ooura
PLATFORMS=$(MACHINE)
LIBRARYTARGETS=$(call interp6,data/,$(PLATFORMS),/set,$(SETS),/,$(LIBRARIES))
TARGETS=$(call interp6,$(LIBRARYTARGETS),/run,$(RUNS),/,$(BENCHMARKS),Times.dat)

all: $(TARGETS)
	
$(TARGETS):
	@echo "Making $@"
	@mkdir -p $(dir $@)
	@echo $(word 2,$(subst /, ,$@)) > argv.txt
	@echo $(subst set,,$(word 3,$(subst /, ,$@))) >> argv.txt
	@echo $(subst run,,$(word 5,$(subst /, ,$@))) >> argv.txt
	@echo "bench" >> argv.txt