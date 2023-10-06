TOOLS=../tools

INPUT_DIR=.
INPUT_SUFFIXES=fnt tga

# The names of the input files (without suffixes)
NAMES=$(basename $(wildcard $(addprefix $(INPUT_DIR)/*.,$(INPUT_SUFFIXES))))

# The names of the output files
OUTPUT_FILES_CPP=$(addsuffix .cpp,$(NAMES))
OUTPUT_FILES_H=$(addsuffix .h,$(NAMES))

all: $(OUTPUT_FILES_CPP) $(OUTPUT_FILES_H) $(TOOLS)/bin2c.py

# Produce .h and .cpp files from the inputs
%.cpp %.h: %.fnt $(TOOLS)/bin2c.py
	python3 $(TOOLS)/bin2c.py $< $(basename $<)

%.cpp %.h: %.tga $(TOOLS)/bin2c.py
	python3 $(TOOLS)/bin2c.py $< $(basename $<)

clean:
	rm -f $(OUTPUT_FILES_CPP) $(OUTPUT_FILES_H)
