TOOLS=../tools

INPUT_DIR=.
INPUT_SUFFIXES=fnt tga

# The names of the input files (without suffixes)
NAMES=$(basename $(wildcard $(addprefix $(INPUT_DIR)/*.,$(INPUT_SUFFIXES))))

# The names of the output files
OUTPUT_FILES_CPP=$(addsuffix .cpp,$(NAMES))
OUTPUT_FILES_H=$(addsuffix .h,$(NAMES))

all: $(OUTPUT_FILES_CPP) $(OUTPUT_FILES_H)

# Produce .h and .cpp files from the inputs
%.cpp %.h: %.fnt
	python3 $(TOOLS)/bin2c.py $< $(basename $<)

%.cpp %.h: %.tga
	python3 $(TOOLS)/bin2c.py $< $(basename $<)

clean:
	rm -f $(OUTPUT_FILES_CPP) $(OUTPUT_FILES_H)
