CXX =  g++
EXECUTABLE_NAME = al

$(EXECUTABLE_NAME): alphaDriver.cpp alphaLang.cpp alphaFLexer.cpp
	$(CXX) -o $@ $^

alphaFLexer.cpp: alphaFLexer.l
	flex $^


clean: