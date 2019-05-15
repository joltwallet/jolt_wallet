# Introduction

Every screen in JoltOS has the typical layout:

 * Parent (dummy parent object)
     * `JOLT_GUI_OBJ_ID_LABEL_TITLE`; Statusbar title object.
     * `JOLT_GUI_OBJ_ID_BACK`; Invisible `lv_btn` thats mapped to the "left" input.
     * `JOLT_GUI_OBJ_ID_ENTER`; Invisible `lv_btn` thats mapped to the "right" input
     * `JOLT_GUI_OBJ_ID_CONT_BODY`; `lv_cont` representing the entire drawable space below the statusbar. All visible have this container as their parent.
         * CUSTOM_OBJECTS specific objects for this screen

Each lvgl object will have it's `user_data.id` field populated with a `jolt_gui_obj_id_t` for easier identification.
The dummy parent object must be at the top level (child of `lv_scr_act()`). The parent's `id` field will be populated with a `jolt_gui_scr_id_t`. 

## Notes on Meta-Programming

JoltOS uses `mp.h`, a header-only macro library by Simon Tatham to simplify meta-programming in C.
JoltOS creates context macros to perform actions on init, exit, and errors.
Inside any context, a `break` can be used to exit the context. Usually breaks are used on errors.
NEVER perform any of the following within a context:
    * `return`; this will cause the context exit code to not trigger 
    * `goto`; this will cause the context exit code to not trigger. `switch` statements are ok.

# Thread Safety

All direct lvgl functions must be called in a JOLT_GUI_CTX:

```
JOLT_GUI_CTX {
    // Call lv_* functions here
}
```

The `JOLT_GUI_CTX` handles the GUI mutex to provide a safe context to manipulate
graphical elements.

# Template

From the introduction, we can see that every screen needs to create a few standard objects with error checking.
Because of this, there is a special `JOLT_GUI_SCR_CTX`. This creates (with NULL-checking) the following objects:
    * `parent`
    * `label_title` populated with the provided title
    * `cont_body`

```
lv_obj_t *jolt_gui_scr_NAME_create(const char *title, const char *body) {
    JOLT_GUI_SCR_CTX(title) {
        jolt_gui_scr_set_back_action(parent, NULL);
        jolt_gui_scr_set_enter_action(parent, NULL);
    }
    return parent;
}
```
If you break from this context, the parent object will be deleted and the pointer will be set to NULL. It is often convenient to wrap the creation of lvgl objects with `BREAK_IF_NULL( lv_*() )`.

# Screen Extra Data

Sometimes its necessary for a screen to pass additional data to it's back/enter callbacks.
Use `jolt_gui_scr_set_back_param` and `jolt_gui_scr_set_enter_param` to pass pointers to data.

# Deleting a screen

A screen can be deleted via `jolt_gui_scr_del( NULL )` or by `jolt_gui_obj_del(parent)`. Do not delete screens via `lv_scr_del` as there is additional cleanup that needs to be performed on some screens.

