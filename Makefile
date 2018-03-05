export CC=$(CROSS_COMPILE)gcc
SRC_DIR=src

all:
	@(cd $(SRC_DIR) && $(MAKE) $@)

.PHONY: clean mrpoper $(EXEC)
clean:
	@(cd $(SRC_DIR) && $(MAKE) $@)

mrproper: clean
	@(cd $(SRC_DIR) && $(MAKE) $@)
