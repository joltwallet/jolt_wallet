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
static lv_style_t def;

static lv_style_t scrollbar_style;

/*Static style definitions*/
static lv_style_t light_plain;
static lv_style_t dark_plain;
static lv_style_t light_frame;
static lv_style_t dark_frame;

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
    lv_style_copy(&def, &lv_style_plain);  /*Initialize the default style*/
    def.body.main_color = LV_COLOR_WHITE;
    def.body.grad_color = LV_COLOR_WHITE;
    def.body.radius = 0;
    def.body.opa = LV_OPA_COVER;
    def.body.padding.hor = 1;
    def.body.padding.ver = 1;
    def.body.padding.inner = 0;
    def.body.border.color = LV_COLOR_WHITE;
    def.body.border.width = 1;
    def.body.border.opa = LV_OPA_COVER;
    def.body.border.part = 0;

    def.text.font = _font;
    def.text.color = LV_COLOR_BLACK;
    def.text.letter_space = 1;
    def.text.line_space = 1;

    def.line.color = LV_COLOR_BLACK;
    def.line.opa = LV_OPA_COVER;
    def.line.width = 1;

    def.image.color = LV_COLOR_BLACK;
    def.image.intense = LV_OPA_TRANSP;
    def.image.opa = LV_OPA_COVER;

    lv_style_copy(&light_plain, &def); // White background, dark borders

    lv_style_copy(&light_frame, &light_plain);
    light_frame.body.border.color = LV_COLOR_BLACK;
    light_frame.body.border.part = LV_BORDER_FULL;
    light_frame.body.radius = 2;

    lv_style_copy(&dark_plain, &light_plain);
    dark_plain.body.main_color   = LV_COLOR_BLACK;
    dark_plain.body.grad_color   = LV_COLOR_BLACK;
    dark_plain.body.border.color = LV_COLOR_WHITE;
    dark_plain.text.color        = LV_COLOR_WHITE;
    dark_plain.line.color        = LV_COLOR_WHITE;
    dark_plain.image.color       = LV_COLOR_WHITE;

    lv_style_copy(&dark_frame, &dark_plain);
    dark_frame.body.border.part = LV_BORDER_FULL;
    dark_frame.body.radius = 2;

    lv_style_copy(&scrollbar_style, &dark_frame);
    scrollbar_style.body.padding.hor = -1; // Scrollbar distance from the Right 
    scrollbar_style.body.padding.inner = 3; // Scrollbar's Width 

    theme.bg = &def;
    theme.panel = &light_frame;
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
    theme.btn.rel = &light_frame;
    theme.btn.pr = &dark_frame;
    theme.btn.tgl_rel = &dark_frame;
    theme.btn.tgl_pr =  &light_frame;
    theme.btn.ina =  &light_frame;
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
    theme.img.dark = &def;
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
    static lv_style_t led;
    lv_style_copy(&led, &light_frame);
    led.body.radius = LV_RADIUS_CIRCLE;
    led.body.shadow.width = LV_DPI / 8;
    led.body.shadow.color = LV_COLOR_BLACK;
    led.body.shadow.type = LV_SHADOW_FULL;

    theme.led = &led;
#endif
}

static void bar_init(void)
{
#if USE_LV_BAR
    theme.bar.bg = &light_frame;
    theme.bar.indic = &dark_frame;
#endif
}

static void slider_init(void)
{
#if USE_LV_SLIDER != 0
    static lv_style_t slider_knob;
    lv_style_copy(&slider_knob, &light_frame);
    slider_knob.body.radius = LV_RADIUS_CIRCLE;
    slider_knob.body.padding.hor = LV_DPI / 30;
    slider_knob.body.padding.ver = LV_DPI / 30;

    theme.slider.bg = theme.bar.bg;
    theme.slider.indic = theme.bar.indic;
    theme.slider.knob = &slider_knob;
#endif
}

static void sw_init(void)
{
#if USE_LV_SW != 0
    static lv_style_t bg, indic;
    static lv_style_t knob_on, knob_off;

    lv_style_copy(&bg, theme.slider.bg);
    lv_style_copy(&indic, theme.slider.indic);
    lv_style_copy(&knob_on, theme.slider.knob);
    lv_style_copy(&knob_off, theme.slider.knob);

    bg.body.border.width = 1;
    bg.body.border.part = LV_BORDER_FULL;
    bg.body.padding.hor = 3;
    bg.body.padding.ver = 3;
    bg.body.radius = LV_RADIUS_CIRCLE;
    bg.body.main_color = LV_COLOR_WHITE;
    bg.body.grad_color = LV_COLOR_WHITE;

    indic.body.padding.hor = 0;  /* horizontal gap between border and the line indicator (bigger numbers make it shorter) */
    indic.body.padding.ver = 0;  /* vertical gap between border and the line indicator (bigger numbers make it skinnier) */
    indic.body.radius = LV_RADIUS_CIRCLE;
    indic.body.border.width = 1;
    indic.body.border.color = LV_COLOR_BLACK;
    indic.body.border.part = LV_BORDER_FULL;
    indic.body.border.opa = LV_OPA_COVER;

    knob_off.body.radius = LV_RADIUS_CIRCLE;
    knob_off.body.border.color = LV_COLOR_BLACK;
    knob_off.body.main_color = LV_COLOR_BLACK;
    knob_off.body.grad_color = LV_COLOR_BLACK;

    knob_on.body.radius = LV_RADIUS_CIRCLE;
    knob_on.body.border.color = LV_COLOR_BLACK;
    knob_on.body.main_color = LV_COLOR_BLACK;
    knob_on.body.grad_color = LV_COLOR_BLACK;

    theme.sw.bg = &bg;
    theme.sw.indic = &indic;
    theme.sw.knob_off = &knob_off;
    theme.sw.knob_on = &knob_on;
#endif
}


static void lmeter_init(void)
{
#if USE_LV_LMETER != 0
    static lv_style_t lmeter_bg;
    lv_style_copy(&lmeter_bg, &light_frame);
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
    theme.chart = &light_frame;
#endif
}

static void calendar_init(void)
{
#if USE_LV_CALENDAR
    static lv_style_t box;
    lv_style_copy(&box, &light_plain);
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
    theme.cb.box.rel = &light_frame;
    theme.cb.box.pr = &dark_frame;
    theme.cb.box.tgl_rel = &dark_frame;
    theme.cb.box.tgl_pr = &light_frame;
    theme.cb.box.ina = &light_frame;
#endif
}


static void btnm_init(void)
{
#if USE_LV_BTNM
    theme.btnm.bg = &light_frame;
    theme.btnm.btn.rel = &light_frame;
    theme.btnm.btn.pr = &dark_frame;
    theme.btnm.btn.tgl_rel = &dark_frame;
    theme.btnm.btn.tgl_pr = &light_frame;
    theme.btnm.btn.ina = &light_frame;
#endif
}

static void kb_init(void)
{
#if USE_LV_KB
    theme.kb.bg = &lv_style_transp_fit;
    theme.kb.btn.rel = &light_frame;
    theme.kb.btn.pr = &light_frame;
    theme.kb.btn.tgl_rel = &dark_frame;
    theme.kb.btn.tgl_pr = &dark_frame;
    theme.kb.btn.ina = &light_frame;
#endif
}

static void mbox_init(void)
{
#if USE_LV_MBOX
    theme.mbox.bg = &dark_frame;
    theme.mbox.btn.bg = &lv_style_transp_fit;
    theme.mbox.btn.rel = &light_frame;
    theme.mbox.btn.pr = &dark_frame;
#endif
}

static void page_init(void)
{
#if USE_LV_PAGE
    theme.page.bg = &light_plain;
    theme.page.scrl = &light_plain;
    theme.page.sb = &scrollbar_style;
#endif
}

static void ta_init(void)
{
#if USE_LV_TA
    theme.ta.area = &light_frame;
    theme.ta.oneline = &light_frame;
    theme.ta.cursor = NULL;     /*Let library to calculate the cursor's style*/
    theme.ta.sb = &dark_frame;
#endif
}

static void list_init(void)
{
#if USE_LV_LIST != 0
    static lv_style_t scrl;
    lv_style_copy(&scrl, &light_plain);

    scrl.body.padding.ver = 1; // V padding whole list
    scrl.body.padding.hor = 1; // H padding whole list
    scrl.body.padding.inner = 1; // Space between list elements

    theme.list.sb = &scrollbar_style;
    theme.list.bg = &light_plain;
    theme.list.scrl = &scrl;
    theme.list.btn.rel = &light_plain;
    theme.list.btn.pr = &dark_plain;
    theme.list.btn.tgl_rel = &dark_plain;
    theme.list.btn.tgl_pr = &light_plain;
    theme.list.btn.ina = &light_plain;
#endif
}

static void ddlist_init(void)
{
#if USE_LV_DDLIST != 0
    static lv_style_t bg;
    lv_style_copy(&bg, &light_frame);

    theme.ddlist.bg = &bg;
    theme.ddlist.sel = &dark_plain;
    theme.ddlist.sb = &dark_frame;
#endif
}

static void roller_init(void)
{
#if USE_LV_ROLLER != 0
    static lv_style_t bg;
    lv_style_copy(&bg, &light_frame);
    bg.body.padding.hor = 3; // Width Padding of the entire roller
    bg.body.padding.inner = 0;
    bg.text.font = &lv_font_crox3hb_numeric;
    bg.text.line_space = 6; // Distance between options

    theme.roller.bg = &bg;
    theme.roller.sel = &dark_frame;
#endif
}

static void tabview_init(void)
{
#if USE_LV_TABVIEW != 0
    theme.tabview.bg = &light_frame;
    theme.tabview.indic = &light_plain;
    theme.tabview.btn.bg = &lv_style_transp_fit;
    theme.tabview.btn.rel = &light_frame;
    theme.tabview.btn.pr = &dark_frame;
    theme.tabview.btn.tgl_rel = &dark_frame;
    theme.tabview.btn.tgl_pr = &light_frame;
#endif
}


static void win_init(void)
{
#if USE_LV_WIN != 0
    static lv_style_t win_header;
    lv_style_copy(&win_header, &dark_plain);
    win_header.body.padding.hor = LV_DPI / 30;
    win_header.body.padding.ver = LV_DPI / 30;

    theme.win.bg = &light_frame;
    theme.win.sb = &dark_frame;
    theme.win.header = &win_header;
    theme.win.content.bg = &lv_style_transp;
    theme.win.content.scrl = &lv_style_transp;
    theme.win.btn.rel = &light_frame;
    theme.win.btn.pr = &dark_frame;
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
