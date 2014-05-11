MAIN_LATEX=memLeak.tex
OUT_PDF=memLeak.pdf
OUT_PS=$(patsubst %.pdf,%.ps,$(OUT_PDF))
OUT_DVI=$(patsubst %.pdf,%.dvi,$(OUT_PDF))

DEPS=$(wildcard *.tex) $(wildcard figs/*) $(wildcard *.bib)

SHELL=/bin/bash

all: $(OUT_PDF)

.PHONY: all clean public

$(OUT_PDF): $(DEPS)
	rubber -vv -pd osdi14gpunet
	@grep --color undefined osdi14gpunet.log && notify-send --urgency=critical "WARNING: UNDEFINED REF" || true

public: 
	sed -i 's/publicversion}{false/publicversion}{true/' osdi14gpunet.tex
	rubber -vv -pd osdi14gpunet
	cp $(OUT_PDF) osdi14gpunet_submitted.pdf
	sed -i 's/publicversion}{true/publicversion}{false/' osdi14gpunet.tex
	rubber -vv -pd osdi14gpunet
	sha1sum osdi14gpunet_submitted.pdf
	@notify-send --urgency=critical `sha1sum osdi14gpunet_submitted.pdf`

clean:
	rubber -v -pd --clean osdi14gpunet
