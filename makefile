.SILENT:
.PHONY: all dir clean build

# compiler settings
CXX      = g++
CXXFLAGS = -Wall -std=c++20
INCFLAGS = -Isrc
# LIBFLAGS     = -lsomelib
# LIBPATHFLAGS = -L/path/to/some/lib

# directories
OUT_DIR = bin
SRC_DIR = src/examples
INC_DIR = src

# files
SRC = $(wildcard ${SRC_DIR}/*.cpp)
BIN = $(addprefix bin/, $(notdir $(patsubst %.cpp, %, $(SRC))))

# phony targets
all: clean dir build
	echo "examples built"

clean:
	rm -rf ${OUT_DIR}

dir:
	mkdir -p ${OUT_DIR}

build: dir ${BIN}

# rules
${BIN}: ${SRC}
	${CXX} ${CXXFLAGS} $< -o ${@}

