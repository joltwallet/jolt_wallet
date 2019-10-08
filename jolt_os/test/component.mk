COMPONENT_ADD_LDFLAGS = -Wl,--whole-archive -l$(COMPONENT_NAME) -Wl,--no-whole-archive

COMPONENT_SRCDIRS := .\
	crypto \
	crypto/algorithms \
	hal \
	hal/storage \
	hash/ \
	hash/algorithms \
	jolt_gui \
	syscore \
	syscore/encoding
