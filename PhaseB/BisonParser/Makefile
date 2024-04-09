alphaBisonParser: alphaBisonParser.y
	bison $^
	-mv alphaBisonParser.cpp ../GeneratedFiles/
	-mv alphaBisonParser.hpp ../GeneratedFiles/
	-cp *.hpp ../GeneratedFiles/

clean:
	-rm alphaBisonParser.cpp
	-rm alphaBisonParser.hpp