CC ?= cc
CFLAGS ?= -Wall -Wextra -std=c17 -g
PROGRAMS = pipe-parent-to-child pipe-oneway pipe-string pipeline

.PHONY: all clean

all: $(PROGRAMS)

pipe-parent-to-child: pipe-parent-to-child.c
	$(CC) $(CFLAGS) $< -o $@
	
pipe-oneway: pipe-oneway.c
	$(CC) $(CFLAGS) $< -o $@
	
pipe-string: pipe-string.c
	$(CC) $(CFLAGS) $< -o $@
	
pipeline: pipeline.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(PROGRAMS)
