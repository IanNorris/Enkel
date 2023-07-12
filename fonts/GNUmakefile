TOOLS=../tools

INPUT_DIR=.
INPUT_SUFFIXES=fnt tga

# The names of the input files (without suffixes)
NAMES=$(basename $(wildcard $(addprefix $(INPUT_DIR)/*.,$(INPUT_SUFFIXES))))

# The names of the output files
OUTPUT_FILES_C=$(addsuffix .c,$(NAMES))
OUTPUT_FILES_H=$(addsuffix .h,$(NAMES))

all: $(OUTPUT_FILES_C) $(OUTPUT_FILES_H)

# Produce .h and .c files from the inputs
%.c %.h: %.fnt
	python3 $(TOOLS)/bin2c.py $< $(basename $<)

%.c %.h: %.tga
	python3 $(TOOLS)/bin2c.py $< $(basename $<)

clean:
	rm -f $(OUTPUT_FILES_C) $(OUTPUT_FILES_H)
