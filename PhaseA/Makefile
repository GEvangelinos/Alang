CXX =  g++
CXX_FLAGS = -std=c++20 # UNUSED #
EXECUTABLE_NAME = al
FLEX_SOURCE_GENERATED_FILE = alphaFlexScanner.cpp
FLEX_HEADER_GENERATED_FILE = alphaFlexScanner.hpp

$(EXECUTABLE_NAME): alphaDriver.cpp alphaLang.cpp alphaFlexScanner.cpp
	$(CXX) -o $@ $^

alphaFlexScanner.cpp: alphaFlexScanner.l
	flex $^


clean:
	-rm $(EXECUTABLE_NAME)
	-rm $(FLEX_HEADER_GENERATED_FILE)
	-rm $(FLEX_SOURCE_GENERATED_FILE)