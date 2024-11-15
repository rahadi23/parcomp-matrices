UTILS := $(wildcard src/utils/*.c)
OUTPUTS := out/cg.o

all: out logs $(OUTPUTS)

out logs:
	mkdir -p $@

clean:
	rm -f out/*

$(OUTPUTS): $(UTILS) src/main.c
	mpicc $^ -lm -o $@

