export CC=$(CROSS_COMPILE)gcc
SRC_DIR=src
EXEC=$(SRC_DIR)/atqt1_sc_demo $(SRC_DIR)/atqt1_mc_demo $(SRC_DIR)/atqt2_demo $(SRC_DIR)/atqt6_demo

all: $(EXEC)

$(EXEC):
	@(cd $(SRC_DIR) && $(MAKE))

.PHONY: clean mrpoper $(EXEC)
clean:
	@(cd $(SRC_DIR) && $(MAKE) $@)

mrproper: clean
	@(cd $(SRC_DIR) && $(MAKE) $@)
