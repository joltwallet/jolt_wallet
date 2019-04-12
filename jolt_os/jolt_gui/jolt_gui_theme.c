/**
 * @file lv_theme_templ.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_theme_t theme;

/*Static style definitions*/
static lv_style_t def;
static lv_style_t light_frame;
static lv_style_t light_frame_round;
static lv_style_t dark_plain;
static lv_style_t dark_plain_round;
static lv_style_t scrollbar_style;

/*Saved input parameters*/
static const lv_font_t * _font;

/**********************
 *      MACROS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void basic_init(void)
{
    /* Notes: 
     *    * On the 1bpp OLED screen, white is off, black is on.
     */

    /* Default configuration:
     *     * white body
     *     * no frame
     */

    lv_style_copy(&def, &lv_style_plain);  /*Initialize the default style*/

    /* Body Configuration */
    def.body.main_color = LV_COLOR_WHITE;
    def.body.grad_color = LV_COLOR_WHITE;
    def.body.radius = 0;
    def.body.opa = LV_OPA_COVER;

    def.body.padding.left = 1;
    def.body.padding.right = 1;
    def.body.padding.top = 1;
    def.body.padding.bottom = 1;
    def.body.padding.inner = 1;

    def.body.border.color = LV_COLOR_WHITE;
    def.body.border.width = 1;
    def.body.border.opa = LV_OPA_COVER;
    def.body.border.part = LV_BORDER_NONE;

    /* Text Configuration */
    def.text.font         = _font; // de
    def.text.color        = LV_COLOR_BLACK;
    def.text.letter_space = 1;
    def.text.line_space   = 1;

    /* Line Configuration */
    def.line.color = LV_COLOR_BLACK;
    def.line.opa = LV_OPA_COVER;
    def.line.width = 1;

    /* Image Configuration */
    def.image.color = LV_COLOR_BLACK;
    def.image.intense = LV_OPA_TRANSP;
    def.image.opa = LV_OPA_COVER;

    /******************************************
     * light_frame: white body, black borders *
     ******************************************/
    lv_style_copy(&light_frame, &def);
    light_frame.body.border.color = LV_COLOR_BLACK;
    light_frame.body.border.width = 1;
    light_frame.body.border.opa = LV_OPA_COVER;
    light_frame.body.border.part = LV_BORDER_FULL;

    /***************************************************
     * light_frame_round: light_frame with round radius *
     ***************************************************/
    lv_style_copy(&light_frame_round, &light_frame);
    light_frame_round.body.radius = LV_RADIUS_CIRCLE;

    /*****************************************************
     * dark_plain: black body, white borders, white text *
     *****************************************************/
    lv_style_copy(&dark_plain, &def);
    dark_plain.body.main_color   = LV_COLOR_BLACK;
    dark_plain.body.grad_color   = LV_COLOR_BLACK;
    dark_plain.body.border.color = LV_COLOR_WHITE;
    dark_plain.text.color        = LV_COLOR_WHITE;
    dark_plain.line.color        = LV_COLOR_WHITE;
    dark_plain.image.color       = LV_COLOR_WHITE;

    /**************************************************
     * dark_plain_round: dark_plain with round radius *
     **************************************************/
    lv_style_copy(&dark_plain_round, &dark_plain);
    dark_plain_round.body.radius = LV_RADIUS_CIRCLE;

    /*******************
     * Scrollbar style *
     *******************/
    lv_style_copy(&scrollbar_style, &dark_plain);
    scrollbar_style.body.border.width = 1;
    scrollbar_style.body.border.opa = LV_OPA_COVER;
    scrollbar_style.body.border.part = LV_BORDER_FULL;
    scrollbar_style.body.padding.left = -2; // Scrollbar distance from the Right 
    scrollbar_style.body.padding.right = -2; // Scrollbar distance from the Right 
    scrollbar_style.body.padding.inner = 3; // Scrollbar's Width 

    theme.style.bg = &def;
    theme.style.panel = &def;
}

#if LV_USE_GROUP
static void style_mod(lv_group_t * group, lv_style_t * style) {
    /* Do Nothing */
    (void) group;
    (void) style;
}

static void style_mod_edit(lv_group_t * group, lv_style_t * style) {
    /* Do Nothing */
    (void) group;
    (void) style;
}
#endif

static void cont_init(void)
{
#if LV_USE_CONT != 0
    theme.style.cont = &def;
#endif
}

static void btn_init(void)
{
#if LV_USE_BTN != 0
    theme.style.btn.rel = &def;
    theme.style.btn.pr = &dark_plain;
    theme.style.btn.tgl_rel = &dark_plain;
    theme.style.btn.tgl_pr =  &def;
    theme.style.btn.ina =  &def;
#endif
}


static void label_init(void)
{
#if LV_USE_LABEL != 0
    theme.style.label.prim = NULL;
    theme.style.label.sec = NULL;
    theme.style.label.hint = NULL;
#endif
}

static void img_init(void)
{
#if LV_USE_IMG != 0
    theme.style.img.light = &def;
    theme.style.img.dark = &dark_plain;
#endif
}

static void line_init(void)
{
#if LV_USE_LINE != 0
    theme.style.line.decor = NULL;
#endif
}

static void led_init(void)
{
#if LV_USE_LED != 0
    theme.style.led = &dark_plain_round;
#endif
}

static void bar_init(void)
{
#if LV_USE_BAR
    static lv_style_t bg, indic;

    lv_style_copy(&bg, &light_frame_round);
    bg.body.padding.left = 3;
    bg.body.padding.right = 3;
    bg.body.padding.top = 3;
    bg.body.padding.bottom = 3;

    lv_style_copy(&indic, &dark_plain);
    indic.body.radius       = bg.body.radius;
    indic.body.padding.left  = 2;
    indic.body.padding.right  = 2;
    indic.body.padding.top  = 2;
    indic.body.padding.bottom  = 2;
    indic.body.border.width = 0;

    theme.style.bar.bg = &bg;
    theme.style.bar.indic = &indic;
#endif
}

static void slider_init(void)
{
#if LV_USE_SLIDER != 0
    theme.style.slider.bg    = theme.style.bar.bg;
    theme.style.slider.indic = theme.style.bar.indic;
    theme.style.slider.knob  = &light_frame_round;
#endif
}

static void sw_init(void)
{
#if LV_USE_SW != 0
    lv_style_t indic;
    lv_style_copy(&indic, theme.style.slider.indic);
    indic.body.padding.left = 0;
    indic.body.padding.right = 0;
    indic.body.padding.top = 0;
    indic.body.padding.bottom = 0;

    theme.style.sw.bg = theme.style.slider.bg;
    theme.style.sw.indic = &indic;
    theme.style.sw.knob_off = &light_frame_round;
    theme.style.sw.knob_on = &dark_plain_round;
#endif
}


static void lmeter_init(void)
{
#if LV_USE_LMETER != 0
    static lv_style_t lmeter_bg;
    lv_style_copy(&lmeter_bg, &def);
    lmeter_bg.body.empty = 1;
    lmeter_bg.body.main_color = LV_COLOR_BLACK;
    lmeter_bg.body.grad_color = LV_COLOR_BLACK;
    lmeter_bg.body.padding.left = LV_DPI / 20;
    lmeter_bg.body.padding.right = LV_DPI / 20;
    lmeter_bg.body.padding.inner = LV_DPI / 8;
    lmeter_bg.line.color = LV_COLOR_WHITE;
    lmeter_bg.line.width = 1;

    theme.style.lmeter = &lmeter_bg;
#endif
}

static void gauge_init(void)
{
#if LV_USE_GAUGE != 0
    static lv_style_t gauge_bg;
    lv_style_copy(&gauge_bg, theme.style.lmeter);
    gauge_bg.line.color = LV_COLOR_BLACK;
    gauge_bg.line.width = 1;

    theme.style.gauge = &gauge_bg;
#endif
}

static void chart_init(void)
{
#if LV_USE_CHART
    theme.style.chart = &def;
#endif
}

static void calendar_init(void)
{
#if LV_USE_CALENDAR
    static lv_style_t box;
    lv_style_copy(&box, &def);
    box.body.padding.ver = LV_DPI / 20;

    /*Can't handle highlighted dates in this theme*/
    theme.style.calendar.week_box = &box;
    theme.style.calendar.today_box = &box;
#endif
}

static void cb_init(void)
{
#if LV_USE_CB != 0
    theme.style.cb.bg = &lv_style_transp;
    theme.style.cb.box.rel = &def;
    theme.style.cb.box.pr = &dark_plain;
    theme.style.cb.box.tgl_rel = &dark_plain;
    theme.style.cb.box.tgl_pr = &def;
    theme.style.cb.box.ina = &def;
#endif
}

static void btnm_init(void)
{
#if LV_USE_BTNM
    theme.style.btnm.bg = &def;
    theme.style.btnm.btn.rel = &def;
    theme.style.btnm.btn.pr = &dark_plain;
    theme.style.btnm.btn.tgl_rel = &dark_plain;
    theme.style.btnm.btn.tgl_pr = &def;
    theme.style.btnm.btn.ina = &def;
#endif
}

static void kb_init(void)
{
#if LV_USE_KB
    theme.style.kb.bg = &lv_style_transp_fit;
    theme.style.kb.btn.rel = &def;
    theme.style.kb.btn.pr = &def;
    theme.style.kb.btn.tgl_rel = &dark_plain;
    theme.style.kb.btn.tgl_pr = &dark_plain;
    theme.style.kb.btn.ina = &def;
#endif
}

static void mbox_init(void)
{
#if LV_USE_MBOX
    theme.style.mbox.bg = &dark_plain;
    theme.style.mbox.btn.bg = &lv_style_transp_fit;
    theme.style.mbox.btn.rel = &def;
    theme.style.mbox.btn.pr = &dark_plain;
#endif
}

static void page_init(void)
{
#if LV_USE_PAGE
    theme.style.page.bg = &def;
    theme.style.page.scrl = &def;
    theme.style.page.sb = &scrollbar_style;
#endif
}

static void preload_init(void)
{
#if LV_USE_PRELOAD
    theme.style.preload = &def;
#endif
}

static void ta_init(void)
{
#if LV_USE_TA
    theme.style.ta.area = &def;
    theme.style.ta.oneline = &def;
    theme.style.ta.cursor = NULL;     /*Let library to calculate the cursor's style*/
    theme.style.ta.sb = &dark_plain;
#endif
}

static void list_init(void)
{
#if LV_USE_LIST != 0
    theme.style.list.sb = &scrollbar_style;
    theme.style.list.bg = &def;
    theme.style.list.scrl = &def;
    theme.style.list.btn.rel     = &def;
    theme.style.list.btn.pr      = &dark_plain;
    theme.style.list.btn.tgl_rel = &dark_plain;
    theme.style.list.btn.tgl_pr  = &def;
    theme.style.list.btn.ina     = &def;
#endif
}

static void ddlist_init(void)
{
#if LV_USE_DDLIST != 0
    theme.style.ddlist.bg = &def;
    theme.style.ddlist.sel = &dark_plain;
    theme.style.ddlist.sb = &dark_plain;
#endif
}

static void roller_init(void)
{
#if LV_USE_ROLLER != 0
    static lv_style_t bg;
    lv_style_copy(&bg, &def);
    bg.body.padding.left = 3; // Width Padding of the entire roller
    bg.body.padding.right = 3; // Width Padding of the entire roller
    bg.body.padding.inner = 0;
    bg.text.font = &lv_font_crox3hb_numeric;
    bg.text.line_space = 6; // Distance between options

    theme.style.roller.bg = &bg;
    theme.style.roller.sel = &dark_plain;
#endif
}

static void tabview_init(void)
{
#if LV_USE_TABVIEW != 0
    theme.style.tabview.bg = &def;
    theme.style.tabview.indic = &def;
    theme.style.tabview.btn.bg = &lv_style_transp_fit;
    theme.style.tabview.btn.rel = &def;
    theme.style.tabview.btn.pr = &dark_plain;
    theme.style.tabview.btn.tgl_rel = &dark_plain;
    theme.style.tabview.btn.tgl_pr = &def;
#endif
}


static void win_init(void)
{
#if LV_USE_WIN != 0
    static lv_style_t win_header;
    lv_style_copy(&win_header, &dark_plain);
    win_header.body.padding.left = LV_DPI / 30;
    win_header.body.padding.right = LV_DPI / 30;
    win_header.body.padding.top = LV_DPI / 30;
    win_header.body.padding.bottom = LV_DPI / 30;

    theme.style.win.bg = &def;
    theme.style.win.sb = &dark_plain;
    theme.style.win.header = &win_header;
    theme.style.win.content.bg = &lv_style_transp;
    theme.style.win.content.scrl = &lv_style_transp;
    theme.style.win.btn.rel = &def;
    theme.style.win.btn.pr = &dark_plain;
#endif
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/



/**
 * Initialize the jolt theme
 * @param hue [0..360] hue value from HSV color space to define the theme's base color
 * @param font pointer to a font (NULL to use the default)
 * @return pointer to the initialized theme.style
 */
lv_theme_t * jolt_gui_theme_init(uint16_t hue, const lv_font_t * font)
{
    (void) hue;
    if(font == NULL) font = LV_FONT_DEFAULT;

    _font = font;

#if 1
    /*For backward compatibility initialize all theme elements with a default style */
    {
        lv_style_t ** style_p = (lv_style_t **) &theme.style;
        for(uint16_t i = 0; i < LV_THEME_STYLE_COUNT; i++) {
            *style_p = &def;
            style_p++;
        }
    }
#endif

    basic_init();

    bar_init();
    btn_init();
    btnm_init();
    calendar_init();
    cb_init();
    chart_init();
    cont_init();
    ddlist_init();
    gauge_init();
    img_init();
    kb_init();
    label_init();
    led_init();
    line_init();
    list_init();
    lmeter_init();
    mbox_init();
    page_init();
    preload_init();
    roller_init();
    slider_init();
    sw_init();
    ta_init();
    tabview_init();
    win_init();

#if LV_USE_GROUP
    theme.group.style_mod = style_mod;
    theme.group.style_mod_edit = style_mod_edit;
#endif

    return &theme;
}

/**
 * Get a pointer to the theme
 * @return pointer to the theme
 */
lv_theme_t * jolt_gui_theme_get(void)
{
    return &theme;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
