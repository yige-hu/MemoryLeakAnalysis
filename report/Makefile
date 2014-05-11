MAIN_LATEX=memLeak.tex
OUT_PDF=memLeak.pdf
OUT_PS=$(patsubst %.pdf,%.ps,$(OUT_PDF))
OUT_DVI=$(patsubst %.pdf,%.dvi,$(OUT_PDF))

DEPS=$(wildcard *.tex) $(wildcard figs/*) $(wildcard *.bib)

SHELL=/bin/bash

all: $(OUT_PDF)

.PHONY: all clean public

$(OUT_PDF): $(DEPS)
	rubber -vv -pd memLeak
	@grep --color undefined memLeak.log && notify-send --urgency=critical "WARNING: UNDEFINED REF" || true

clean:
	rubber -v -pd --clean memLeak
