alphaFlexScanner: alphaFlexScanner.l
	flex $^
	-mv alphaFlexScanner.cpp ../GeneratedFiles/
	-mv alphaFlexScanner.hpp ../GeneratedFiles/
	-cp *.hpp ../GeneratedFiles/

clean:
	-rm alphaFlexScanner.cpp
	-rm alphaFlexScanner.hpp
