CXX =  g++
EXECUTABLE_NAME = al
FLEX_GENERATED_FILE = alphaFLexer.cpp

$(EXECUTABLE_NAME): alphaDriver.cpp alphaLang.cpp alphaFLexer.cpp
	$(CXX) -o $@ $^

alphaFLexer.cpp: alphaFLexer.l
	flex --outfile $(FLEX_GENERATED_FILE) $^


clean:
	-rm $(EXECUTABLE_NAME)
	-rm $(FLEX_GENERATED_FILE)