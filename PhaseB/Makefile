.PHONY: bisonModule flexModule

compile: bisonModule flexModule
	

bisonModule:
	$(MAKE) -C BisonParser alphaBisonParser

flexModule:
	$(MAKE) -C FlexScanner alphaFlexScanner

clean:
	$(MAKE) -C BisonParser clean
	$(MAKE) -C FlexScanner clean
	clear