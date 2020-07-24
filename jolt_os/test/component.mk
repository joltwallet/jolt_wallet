COMPONENT_ADD_LDFLAGS = -Wl,--whole-archive -l$(COMPONENT_NAME) -Wl,--no-whole-archive

COMPONENT_ADD_INCLUDEDIRS := .

COMPONENT_SRCDIRS := .\
	app_cmd \
	crypto \
	crypto/algorithms \
	hal \
	hal/storage \
	hash/ \
	hash/algorithms \
	jolt_gui \
	nanopb \
	syscore \
	syscore/encoding
