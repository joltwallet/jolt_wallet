# This is an include file for Makefiles. It provides rules for building
# .pb.c and .pb.h files out of .proto, as well the path to nanopb core.
#
PB_SRCS += $(shell find $(JOLT_WALLET_PATH)/jolt_os/ -type f -name '*.proto')
PB_GENS := $(patsubst %.proto, %.pb.c, $(PB_SRCS)) $(patsubst %.proto, %.pb.h, $(PB_SRCS))

# Path to the nanopb root directory
NANOPB_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))../components/nanopb/nanopb)

# Files for the nanopb core
NANOPB_CORE = $(NANOPB_DIR)/pb_encode.c $(NANOPB_DIR)/pb_decode.c $(NANOPB_DIR)/pb_common.c

# Check if we are running on Windows
ifdef windir
WINDOWS = 1
endif
ifdef WINDIR
WINDOWS = 1
endif

# Check whether to use binary version of nanopb_generator or the
# system-supplied python interpreter.
ifneq "$(wildcard $(NANOPB_DIR)/generator-bin)" ""
	# Binary package
	PROTOC = $(NANOPB_DIR)/generator-bin/protoc
	PROTOC_OPTS = 
else
	# Source only or git checkout
	ifdef WINDOWS
	    PROTOC = "python $(NANOPB_DIR)/generator/protoc"
	    PROTOC_OPTS = --plugin=protoc-gen-nanopb=$(NANOPB_DIR)/generator/protoc-gen-nanopb.bat
	else
	    PROTOC = $(NANOPB_DIR)/generator/protoc
		PROTOC_OPTS = --plugin=protoc-gen-nanopb=$(NANOPB_DIR)/generator/protoc-gen-nanopb
	endif
endif

# Rule for building .pb.c and .pb.h
%.pb.c %.pb.h: %.proto $(wildcard %.options)
	$(PROTOC) $(PROTOC_OPTS) \
		--nanopb_out=$(JOLT_WALLET_PATH)/jolt_os/ \
		--proto_path=$(JOLT_WALLET_PATH)/jolt_os/ \
		$<

