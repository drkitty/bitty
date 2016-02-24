RE2C_DIR := lib/re2c/re2c/


$(RE2C_DIR)configure: $(RE2C_DIR)autogen.sh
	cd $(RE2C_DIR) && ./autogen.sh

$(RE2C_DIR)Makefile: $(RE2C_DIR)configure
	cd $(RE2C_DIR) && ./configure

$(RE2C_DIR)re2c: $(RE2C_DIR)Makefile
	make -C $(RE2C_DIR)
