.SILENT:
.PHONY: all dir clean build

# compiler settings
CXX      = g++
CXXFLAGS = -Wall -std=c++20 -Isrc

# directories
OUT_DIR = bin
OBJ_DIR = obj
SRC_DIR = src
EXMPL_DIR = src/examples
INC_DIR = src


# files
HDRS = $(wildcard ${SRC_DIR}/*.hpp)
SRC = $(wildcard ${SRC_DIR}/*.cpp)
OBJ = $(addprefix obj/, $(notdir $(patsubst %.cpp, %.o, $(SRC))))
EXMPL = $(wildcard ${EXMPL_DIR}/*.cpp)
EXOBJ = $(addprefix obj/, $(notdir $(patsubst %.cpp, %.o, $(EXMPL_DIR))))
BIN = $(addprefix bin/, $(notdir $(patsubst %.cpp, %, $(EXMPL))))

# phony targets
all: clean dir build

clean:
	rm -rf ${OBJ_DIR}
	rm -rf ${OUT_DIR}

dir:
	mkdir -p ${OBJ_DIR}
	mkdir -p ${OUT_DIR}

build: dir ${OBJ} ${BIN}

# rules

${OBJ}: ${SRC} 
	${CXX} ${CXXFLAGS} -c $< -o $@
${BIN}: ${EXMPL} ${OBJ}
	${CXX} ${CXXFLAGS} $< ${OBJ} -o $@

