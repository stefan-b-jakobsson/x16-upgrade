BUILD_DIR=build
SRC_FILES=$(wildcard *.asm *.inc)

# Build program
$(BUILD_DIR)/X16UPDATE.PRG: $(SRC_FILES)
	@mkdir -p $(BUILD_DIR)
	cl65 -o $(BUILD_DIR)/X16UPGRADE.PRG -u __EXEHDR__ -t cx16 -C cx16-asm.cfg main.asm
	rm -f main.o

# Make custom package with GUI interface
package:
	python script/gui_pkg.py

# Download latest releases from Github and create package
latest:
	python script/latest.py

# Clean
clean:
	rm -f -r $(BUILD_DIR)/*
