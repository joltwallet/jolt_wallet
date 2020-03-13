COMPONENT_SRCDIRS := \
	secp256k1/src \

COMPONENT_ADD_INCLUDEDIRS := \
	secp256k1/include/ \

COMPONENT_PRIV_INCLUDEDIRS := .\
	secp256k1/

COMPONENT_OBJEXCLUDE := \
	secp256k1/src/valgrind_ctime_test.o \
	secp256k1/src/bench_ecdh.o \
	secp256k1/src/bench_ecmult.o \
	secp256k1/src/bench_internal.o \
	secp256k1/src/bench_recover.o \
	secp256k1/src/bench_sign.o \
	secp256k1/src/bench_verify.o

# SECP256k1 stuff
CFLAGS += \
	-DHAVE_CONFIG_H \
	-Wno-nonnull-compare \
	-Wno-nonnull

