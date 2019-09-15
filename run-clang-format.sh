find jolt_os/ -iname *.h \
    -o -iname *.c \
    -o -iname *.cpp \
    -o -iname *.hpp \
    -o -path jolt_os/hal/lv_drivers -prune \
    | xargs clang-format -style=file -i -fallback-style=google
