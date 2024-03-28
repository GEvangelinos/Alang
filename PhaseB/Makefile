.PHONY: makeGeneratedDirectories bisonModule

compile: makeGeneratedDirectories bisonModule flexModule
	

bisonModule:
	$(MAKE) -C BisonParser

flexModule:
	$(MAKE) -C FlexScanner

clean: