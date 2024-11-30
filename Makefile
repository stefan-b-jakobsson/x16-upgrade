BUILD_DIR=build
SRC_FILES=$(wildcard *.asm *.inc)

$(BUILD_DIR)/$(PRGFILE): $(SRC_FILES)
	@mkdir -p $(BUILD_DIR)
	cl65 -o $(BUILD_DIR)/X16UPGRADE.PRG -u __EXEHDR__ -t cx16 -C cx16-asm.cfg main.asm
	rm -f main.o

# Clean
clean:
	rm -f -r $(BUILD_DIR)/*
