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
static uint16_t _hue;
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
    def.body.padding.hor = 1;
    def.body.padding.ver = 1;
    def.body.padding.inner = 1;
    def.body.border.width = 0;

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

    /*****************************************
     * dark_plain: black body, white borders *
     *****************************************/
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
    scrollbar_style.body.padding.hor = -2; // Scrollbar distance from the Right 
    scrollbar_style.body.padding.inner = 3; // Scrollbar's Width 

    theme.bg = &def;
    theme.panel = &def;
}

static void cont_init(void)
{
#if USE_LV_CONT != 0
    theme.cont = &def;
#endif
}

static void btn_init(void)
{
#if USE_LV_BTN != 0
    theme.btn.rel = &def;
    theme.btn.pr = &dark_plain;
    theme.btn.tgl_rel = &dark_plain;
    theme.btn.tgl_pr =  &def;
    theme.btn.ina =  &def;
#endif
}


static void label_init(void)
{
#if USE_LV_LABEL != 0
    theme.label.prim = NULL;
    theme.label.sec = NULL;
    theme.label.hint = NULL;
#endif
}

static void img_init(void)
{
#if USE_LV_IMG != 0
    theme.img.light = &def;
    theme.img.dark = &dark_plain;
#endif
}

static void line_init(void)
{
#if USE_LV_LINE != 0
    theme.line.decor = NULL;
#endif
}

static void led_init(void)
{
#if USE_LV_LED != 0
    theme.led = &dark_plain_round;
#endif
}

static void bar_init(void)
{
#if USE_LV_BAR
    static lv_style_t bg, indic;

    lv_style_copy(&bg, &light_frame_round);
    bg.body.padding.hor = 3;
    bg.body.padding.ver = 3;

    lv_style_copy(&indic, &dark_plain);
    indic.body.radius       = bg.body.radius;
    indic.body.padding.hor  = 2;
    indic.body.padding.ver  = 2;
    indic.body.border.width = 0;

    theme.bar.bg = &bg;
    theme.bar.indic = &indic;
#endif
}

static void slider_init(void)
{
#if USE_LV_SLIDER != 0
    theme.slider.bg    = theme.bar.bg;
    theme.slider.indic = theme.bar.indic;
    theme.slider.knob  = &light_frame_round;
#endif
}

static void sw_init(void)
{
#if USE_LV_SW != 0
    lv_style_t indic;
    lv_style_copy(&indic, theme.slider.indic);
    indic.body.padding.hor = 0;
    indic.body.padding.ver = 0;

    theme.sw.bg = theme.slider.bg;
    theme.sw.indic = &indic;
    theme.sw.knob_off = &light_frame_round;
    theme.sw.knob_on = &dark_plain_round;
#endif
}


static void lmeter_init(void)
{
#if USE_LV_LMETER != 0
    static lv_style_t lmeter_bg;
    lv_style_copy(&lmeter_bg, &def);
    lmeter_bg.body.empty = 1;
    lmeter_bg.body.main_color = LV_COLOR_BLACK;
    lmeter_bg.body.grad_color = LV_COLOR_BLACK;
    lmeter_bg.body.padding.hor = LV_DPI / 20;
    lmeter_bg.body.padding.inner = LV_DPI / 8;
    lmeter_bg.line.color = LV_COLOR_WHITE;
    lmeter_bg.line.width = 1;

    theme.lmeter = &lmeter_bg;
#endif
}

static void gauge_init(void)
{
#if USE_LV_GAUGE != 0
    static lv_style_t gauge_bg;
    lv_style_copy(&gauge_bg, theme.lmeter);
    gauge_bg.line.color = LV_COLOR_BLACK;
    gauge_bg.line.width = 1;

    theme.gauge = &gauge_bg;
#endif
}

static void chart_init(void)
{
#if USE_LV_CHART
    theme.chart = &def;
#endif
}

static void calendar_init(void)
{
#if USE_LV_CALENDAR
    static lv_style_t box;
    lv_style_copy(&box, &def);
    box.body.padding.ver = LV_DPI / 20;

    /*Can't handle highlighted dates in this theme*/
    theme.calendar.week_box = &box;
    theme.calendar.today_box = &box;
#endif
}

static void cb_init(void)
{
#if USE_LV_CB != 0
    theme.cb.bg = &lv_style_transp;
    theme.cb.box.rel = &def;
    theme.cb.box.pr = &dark_plain;
    theme.cb.box.tgl_rel = &dark_plain;
    theme.cb.box.tgl_pr = &def;
    theme.cb.box.ina = &def;
#endif
}

static void btnm_init(void)
{
#if USE_LV_BTNM
    theme.btnm.bg = &def;
    theme.btnm.btn.rel = &def;
    theme.btnm.btn.pr = &dark_plain;
    theme.btnm.btn.tgl_rel = &dark_plain;
    theme.btnm.btn.tgl_pr = &def;
    theme.btnm.btn.ina = &def;
#endif
}

static void kb_init(void)
{
#if USE_LV_KB
    theme.kb.bg = &lv_style_transp_fit;
    theme.kb.btn.rel = &def;
    theme.kb.btn.pr = &def;
    theme.kb.btn.tgl_rel = &dark_plain;
    theme.kb.btn.tgl_pr = &dark_plain;
    theme.kb.btn.ina = &def;
#endif
}

static void mbox_init(void)
{
#if USE_LV_MBOX
    theme.mbox.bg = &dark_plain;
    theme.mbox.btn.bg = &lv_style_transp_fit;
    theme.mbox.btn.rel = &def;
    theme.mbox.btn.pr = &dark_plain;
#endif
}

static void page_init(void)
{
#if USE_LV_PAGE
    theme.page.bg = &def;
    theme.page.scrl = &def;
    theme.page.sb = &scrollbar_style;
#endif
}

static void ta_init(void)
{
#if USE_LV_TA
    theme.ta.area = &def;
    theme.ta.oneline = &def;
    theme.ta.cursor = NULL;     /*Let library to calculate the cursor's style*/
    theme.ta.sb = &dark_plain;
#endif
}

static void list_init(void)
{
#if USE_LV_LIST != 0
    theme.list.sb = &scrollbar_style;
    theme.list.bg = &def;
    theme.list.scrl = &def;
    theme.list.btn.rel = &def;
    theme.list.btn.pr = &dark_plain;
    theme.list.btn.tgl_rel = &dark_plain;
    theme.list.btn.tgl_pr = &def;
    theme.list.btn.ina = &def;
#endif
}

static void ddlist_init(void)
{
#if USE_LV_DDLIST != 0
    theme.ddlist.bg = &def;
    theme.ddlist.sel = &dark_plain;
    theme.ddlist.sb = &dark_plain;
#endif
}

static void roller_init(void)
{
#if USE_LV_ROLLER != 0
    static lv_style_t bg;
    lv_style_copy(&bg, &def);
    bg.body.padding.hor = 3; // Width Padding of the entire roller
    bg.body.padding.inner = 0;
    bg.text.font = &lv_font_crox3hb_numeric;
    bg.text.line_space = 6; // Distance between options

    theme.roller.bg = &bg;
    theme.roller.sel = &dark_plain;
#endif
}

static void tabview_init(void)
{
#if USE_LV_TABVIEW != 0
    theme.tabview.bg = &def;
    theme.tabview.indic = &def;
    theme.tabview.btn.bg = &lv_style_transp_fit;
    theme.tabview.btn.rel = &def;
    theme.tabview.btn.pr = &dark_plain;
    theme.tabview.btn.tgl_rel = &dark_plain;
    theme.tabview.btn.tgl_pr = &def;
#endif
}


static void win_init(void)
{
#if USE_LV_WIN != 0
    static lv_style_t win_header;
    lv_style_copy(&win_header, &dark_plain);
    win_header.body.padding.hor = LV_DPI / 30;
    win_header.body.padding.ver = LV_DPI / 30;

    theme.win.bg = &def;
    theme.win.sb = &dark_plain;
    theme.win.header = &win_header;
    theme.win.content.bg = &lv_style_transp;
    theme.win.content.scrl = &lv_style_transp;
    theme.win.btn.rel = &def;
    theme.win.btn.pr = &dark_plain;
#endif
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/



/**
 * Initialize the jolt theme
 * @param hue [0..360] hue value from HSV color space to define the theme's base color
 * @param font pointer to a font (NULL to use the default)
 * @return pointer to the initialized theme
 */
lv_theme_t * jolt_gui_theme_init(uint16_t hue, const lv_font_t * font)
{
    if(font == NULL) font = LV_FONT_DEFAULT;

    _hue = hue;
    _font = font;

    /*For backward compatibility initialize all theme elements with a default style */
    uint16_t i;
    lv_style_t ** style_p = (lv_style_t **) &theme;
    for(i = 0; i < sizeof(lv_theme_t) / sizeof(lv_style_t *); i++) {
        *style_p = &def;
        style_p++;
    }

    basic_init();
    cont_init();
    btn_init();
    label_init();
    img_init();
    line_init();
    led_init();
    bar_init();
    slider_init();
    sw_init();
    lmeter_init();
    gauge_init();
    chart_init();
    calendar_init();
    cb_init();
    btnm_init();
    kb_init();
    mbox_init();
    page_init();
    ta_init();
    list_init();
    ddlist_init();
    roller_init();
    tabview_init();
    win_init();

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
