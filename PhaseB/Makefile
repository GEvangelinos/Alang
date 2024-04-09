EXECUTABLE_NAME = alphaSyntaxAnalyzer.out
CXX = g++ -std=c++17


.PHONY: createGenerateCodeDirectory bisonModule flexModule



$(EXECUTABLE_NAME): generateBinaryDependecies
	$(CXX) -o $(EXECUTABLE_NAME)  alphaParserDriver.cpp ./BinaryFiles/*

generateBinaryDependecies: generateDependencies
	$(CXX) -c ./GeneratedFiles/alphaBisonParser.cpp  -o ./BinaryFiles/alphaBisonParser.o
	$(CXX) -c ./GeneratedFiles/alphaFlexScanner.cpp -o ./BinaryFiles/alphaFlexScanner.o
	$(CXX) -c ./FlexScanner/alphaLang.cpp -o ./BinaryFiles/alphaLang.o
	$(CXX) -c ./symbolTable.cpp -o ./BinaryFiles/symbolTable.o

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
