CXX = clang++
INCLUDES = -I ../cpp_fixed
# CXXFLAGS = -std=c++20 -O0 -Wall -fsanitize=address -fno-omit-frame-pointer -pedantic-errors -g ${INCLUDES}
# CXXFLAGS = -std=c++20 -Wall -pedantic-errors -g ${INCLUDES}
CXXFLAGS = -std=c++20 -O3 -Wall -pedantic-errors -g ${INCLUDES}
# CXXFLAGS = -std=c++20 -O3 -fprofile-generate -Wall -pedantic-errors -g ${INCLUDES}
# CXXFLAGS = -std=c++20 -O3 -fprofile-use=default.profdata -Wall -pedantic-errors -g ${INCLUDES}

TEST_SRCS = ${wildcard *_test.cpp}
TEST_OBJS = $(addprefix bin/, $(TEST_SRCS:.cpp=.o))
TEST_MAINS = $(addprefix bin/, $(TEST_SRCS:.cpp=))

HEADERS = ${wildcard *.h}

SRCS = fix_parser.cpp fieldmap.cpp fix_builder.cpp

OBJS = $(addprefix bin/, $(SRCS:.cpp=.o))

MAIN = bin/fix_parser_test
MAIN_OBJ = ${basename ${MAIN}}.o

LIB = bin/fix_codec.a

.PRECIOUS: bin/%.o

all: ${MAIN} $(TEST_MAINS) ${LIB}
	@echo compile finished

test: ${TEST_MAINS}

run_tests: ${TEST_MAINS}
	for main in $^ ; do \
		$$main; \
	done

${LIB}: ${OBJS}
	ar r ${LIB} ${OBJS}

${MAIN}: ${MAIN_OBJ} ${LIB}
	${CXX} ${CXXFLAGS} ${MAIN_OBJ} ${LIB} -o ${MAIN}

bin/%_test: bin/%_test.o ${LIB}
	${CXX} ${CXXFLAGS} $@.o ${LIB} -o $@ 

bin/%.o: %.cpp ${HEADERS}
	@ mkdir -p bin
	${CXX} ${CXXFLAGS} -c $(notdir $(basename $@).cpp) -o $@

clean:
	rm -rf bin *~.


