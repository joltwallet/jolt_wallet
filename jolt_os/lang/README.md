Jolt supports multiple languages that can be dynamically changed at runtime.
Languages can be enabled/disabled at compile time to make the resulting 
binary larger/smaller.

At this time, only left to right languages are supported.

Only GUI elements are translated at this time; logging strings are not. This is
to save on storage as well as translation efforts.

# Adding a language

todo: Make this entire process more automatic.

When adding a language, always add it to the bottom of the current list. This 
is to make adding new languages as backwards compatible as possible. The language
name should be the *full name in english*. In this document, substitute your language's
name for all instances of `<name>`.

Examples in this page will be for adding spanish.

## Add option to Kconfig

To make your language toggable in `make menuconfig`, Kconfig must be updated.

Under the `Language` menu, make the following additions:

1. A choice for you language as the default language in the `Select Default Language` choice:
```
            config JOLT_LANG_DEFAULT_<name>
                depends on JOLT_LANG_<name>_EN
                bool "<name>"
```
for example:
```
            config JOLT_LANG_DEFAULT_SPANISH
                depends on JOLT_LANG_SPANISH_EN
                bool "Spanish"
```
the suffix `_EN` stands for "enable", not "english".

2. Underneith the above `choice` section, update `JOLT_LANG_DEFAULT`. This convert 
the above boolean choice into an `enum int`. This must match the `enum` value in 
`lang/lang.h` (see next section).
```
        config JOLT_LANG_DEFAULT
            int
            # Make sure these values match jolt_language_t in lang/lang.h 
            default 0
            default 0 if JOLT_LANG_DEFAULT_ENGLISH
            default <next_integer> if JOLT_LANG_DEFAULT_<name>
```
for example:
```
        config JOLT_LANG_DEFAULT
            int
            # Make sure these values match jolt_language_t in lang/lang.h 
            default 0
            default 0 if JOLT_LANG_DEFAULT_ENGLISH
            default 1 if JOLT_LANG_DEFAULT_SPANISH
```


3. Finally, add a new configuration option to enable/disable compiling your 
language into the firmware
```
        config JOLT_LANG_<name>_EN
            bool
            prompt "Enable <name> Support"
            default y
```
for example:
```
        config JOLT_LANG_SPANISH_EN
            bool
            prompt "Enable Spanish Support"
            default y
```

## Update enumerated types

In `lang.h`, update the `jolt_lang_t` to match the updated `Kconfig` file.
```
typedef enum jolt_lang_t {
    JOLT_LANG_ENGLISH = 0,
    JOLT_LANG_<name> = <next_int>,
} jolt_lang_t;
```
for example:
```
typedef enum jolt_lang_t {
    JOLT_LANG_ENGLISH = 0,
    JOLT_LANG_SPANISH = 1,
} jolt_lang_t;
```

## Make language available via `lang.c`

In the function `jolt_lang_set`, append code to make your language settable:
```
#if CONFIG_JOLT_LANG_<name>_EN
    if( JOLT_LANG_<name> == lang ){
        lang_pack = jolt_lang_<name>;
        font = jolt_lang_<name>_font;
    }
#endif
```
for example:
```
#if CONFIG_JOLT_LANG_SPANISH_EN
    if( JOLT_LANG_SPANISH == lang ){
        lang_pack = jolt_lang_spanish;
        font = jolt_lang_spanish_font;
    }
#endif
```

We must tell JoltOS that the language is now available, so in the function
`jolt_lang_available` append the code to the `available_lang` variable initialization. 
```
#if CONFIG_JOLT_LANG_<name>_EN
        SHIFT( JOLT_LANG_<name> )
#endif
```
for example:
```
#if CONFIG_JOLT_LANG_SPANISH_EN
        SHIFT( JOLT_LANG_SPANISH )
#endif
```

## Create string list c file

Your language should be place in this (`lang`) folder. The name of the file 
should be of form `<name>.c`. 
For example, `spanish.c`.

The `c` file has the following form:

```
#include "jolt_gui/jolt_gui.h"

#if CONFIG_JOLT_LANG_<name>_EN

const lv_font_t *jolt_lang_<name>_font = &lv_font_pixelmix_7;

const char *jolt_lang_<name>[JOLT_TEXT_LAST_STR] = {
   /* Your language's strings go here. */
};

#endif
```
It's probably best to copy `english.c` and change the strings in place. If you language 
needs a larger character set, the selected font must support it.

# Font Support
```
todo: macros in lv_conf to enable/disable fonts based on these configurations.
```
