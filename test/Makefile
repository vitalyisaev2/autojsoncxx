INCLUDE ?= -I../include -I../rapidjson/include -I../catch/single_include
TARGET ?= autojsoncxx_catch

PYTHON ?= python
GENERATOR_SCRIPT = ../autojsoncxx.py
DEFINITION_FILE ?= userdef.hpp
GENERATED_HPP ?= userdef_helper.hpp
CODE_TEMPLATE = ../code_template
CLANG_PATH ?= 

all: $(TARGET)

$(TARGET): main.o test.o $(GENERATED_HPP)
	$(CXX) $(CXXFLAGS) main.o test.o -o $(TARGET)
	
$(GENERATED_HPP): $(DEFINITION_FILE) $(GENERATOR_SCRIPT) $(CODE_TEMPLATE)
	$(PYTHON) $(GENERATOR_SCRIPT) --args=-DCODEGEN --args=-std=c++0x  --output=$(GENERATED_HPP) --input=$(DEFINITION_FILE) --clang=$(CLANG_PATH)

main.o: main.cpp 
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c main.cpp -o main.o

test.o: test.cpp $(GENERATED_HPP)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c test.cpp -o test.o
	
.PHONY: clean test

test: $(TARGET)
	./$(TARGET) --success

clean:
	rm *.o $(TARGET) $(GENERATED_HPP)
