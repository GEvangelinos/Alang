EXECUTABLE_NAME = alphaSyntaxAnalyzer.out
CXX = g++ -std=c++20
CXXFLAGS = -g

.PHONY: createGenerateCodeDirectory bisonModule flexModule



$(EXECUTABLE_NAME): generateBinaryDependecies
	$(CXX) $(CXXFLAGS) -o $(EXECUTABLE_NAME)  alphaParserDriver.cpp ./BinaryFiles/*

generateBinaryDependecies: generateDependencies
	$(CXX) $(CXXFLAGS) -c ./GeneratedFiles/alphaBisonParser.cpp  -o ./BinaryFiles/alphaBisonParser.o
	$(CXX) $(CXXFLAGS) -c ./GeneratedFiles/alphaFlexScanner.cpp -o ./BinaryFiles/alphaFlexScanner.o
	$(CXX) $(CXXFLAGS) -c ./FlexScanner/alphaLang.cpp -o ./BinaryFiles/alphaLang.o
	$(CXX) $(CXXFLAGS) -c ./symbolTable.cpp -o ./BinaryFiles/symbolTable.o
	$(CXX) $(CXXFLAGS) -c ./errorTracker.cpp -o ./BinaryFiles/errorTracker.o


generateDependencies: generateDirectories generateAlphaLexer generateAlphaParser
	

generateDirectories:
	-mkdir GeneratedFiles
	-mkdir BinaryFiles

generateAlphaLexer:
	$(MAKE) -C FlexScanner alphaFlexScanner

generateAlphaParser:
	$(MAKE) -C BisonParser alphaBisonParser

clean:
	$(MAKE) -C BisonParser clean
	$(MAKE) -C FlexScanner clean
	-rm -r ./GeneratedFiles/
	-rm -r ./BinaryFiles
	-rm $(EXECUTABLE_NAME)
	clear
