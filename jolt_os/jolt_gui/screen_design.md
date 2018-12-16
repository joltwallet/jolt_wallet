
# Introduction

Every screen has the typical layout:

```
 * SCREEN_PARENT
 *   +--CONT_TITLE
 *   |   +--LABEL_0 (title)
 *   +--CONT_BODY
 *   |   +--LABEL_0
 *   +--VIRTUAL_BACK_BUTTON
 *   +--VIRTUAL_ENTER_BUTTON
```

The Title Container is a solid box above the StatusBar divider. It typically only 
contains a single `lv_label` object with `free_num` parameter `JOLT_GUI_OBJ_ID_LABEL_TITLE`.

The Body Container is a solid box occupying the entire drawable space below the
StatusBar divider.

# Template

```
lv_obj_t *jolt_gui_scr_NAME_create(const char *title, const char *body) {
    JOLT_GUI_SCR_PREAMBLE( title );

    jolt_gui_scr_set_back_action(parent, NULL);
    jolt_gui_scr_set_enter_action(parent, NULL);

exit:
    return parent;
}
```

All screen creation functions must have an `exit` label for error handling.

## Preamble

The `JOLT_GUI_SCR_PREAMBLE( title_text )` macro is defined in `jolt_gui.h`.

The creates the following object:

* `parent` - The dummy parent object that all elements in a screen are children of.

* `label_title` - Title label object with the specified title string.

* `cont_body` - Body Container which will be parent of all other screen objects.

## JOLT_GUI_OBJ_CHECK

After ever `lv_obj` creation, `JOLT_GUI_OBJ_CHECK` must be called.

This macro checks if the provided value is `NULL`. If it is `NULL`, delete the 
`parent` object, and `goto exit`.

# Screen Extra Data

Sometimes its necessary for a screen to have additional data. Use the `lv_obj` 
attribute `free_ptr` to whatever structure is required for the screen. If the 
pointer is not null, it will be deleted when `lv_scr_del()` is called.

# Deleting a screen

Only delete a screen via `lv_scr_del()`. This will delete the currently focused 
screen and release resources.

