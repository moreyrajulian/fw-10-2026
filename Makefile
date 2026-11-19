TOP = testbench
FILELIST = filelist.f
OBJ_DIR = obj_dir
WAVE = wave.vcd

VERILATOR = verilator
# VER_FLAGS = --binary --trace -Wall -f $(FILELIST)
VER_FLAGS = --binary --trace -Wno-fatal -f $(FILELIST)
# VER_FLAGS = --binary -DEBUG_PRINTS --trace -Wno-fatal -f $(FILELIST) 
# VER_FLAGS = --binary --trace --x-assign unique --x-initial unique -Wno-fatal -f $(FILELIST)

FW_DIR = fw
FW_HEX = $(FW_DIR)/fw.hex

.PHONY: all fw sim wave clean clean_sim build_project_config

all: build_project_config fw clean_sim sim wave

build_project_config:
	python3 build_project_config.py

fw:
	$(MAKE) -C $(FW_DIR)

sim: $(FW_HEX)
	$(VERILATOR) $(VER_FLAGS)
	./$(OBJ_DIR)/V$(TOP)

wave:
	gtkwave $(WAVE)

clean:
	rm -rf $(OBJ_DIR) *.vcd *.fst
	$(MAKE) -C $(FW_DIR) clean

clean_sim:
	rm -rf $(OBJ_DIR) *.vcd *.fst

.PHONY: test_uart_serial

test_uart_serial:
	$(MAKE) -C $(FW_DIR) TARGET=test_uart_serial
	$(VERILATOR) $(VER_FLAGS)
	./$(OBJ_DIR)/V$(TOP) \
		+UART1_SCRIPT=$(FW_DIR)/tests/test_uart_serial.txt \
		+MAX_CYCLES=10000000 \
		$(if $(filter 1,$(VERBOSE)),+VERBOSE) \
		> $(FW_DIR)/build/test_uart_serial.log 2>&1
	cat $(FW_DIR)/build/test_uart_serial.log
	grep -q '^RESULT: PASS$$' $(FW_DIR)/build/test_uart_serial.log