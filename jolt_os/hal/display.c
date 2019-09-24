#include "hal/display.h"
#include "hal/storage/storage.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"

static const char TAG[]                             = "display.c";
static ssd1306_t disp_hal                           = {0};
static DRAM_ATTR uint8_t vdb[JOLT_DISPLAY_BUF_SIZE] = {0};

void jolt_display_init()
{
    static lv_disp_drv_t lv_disp_drv;
    static lv_disp_buf_t disp_buf;

    /* Set reset pin as output */
    gpio_config_t io_config;
    io_config.pin_bit_mask = ( 1 << CONFIG_JOLT_DISPLAY_PIN_RST );
    io_config.mode         = GPIO_MODE_OUTPUT;
    io_config.pull_up_en   = GPIO_PULLUP_DISABLE;
    io_config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_config.intr_type    = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK( gpio_config( &io_config ) );

    disp_hal.protocol = SSD1306_PROTO_I2C;
    disp_hal.screen   = SSD1306_SCREEN;
    disp_hal.i2c_dev  = CONFIG_JOLT_DISPLAY_ADDRESS;
    disp_hal.rst_pin  = CONFIG_JOLT_DISPLAY_PIN_RST;
    disp_hal.width    = LV_HOR_RES_MAX;
    disp_hal.height   = LV_VER_RES_MAX;
    ESP_ERROR_CHECK( ssd1306_init( &disp_hal ) );

    /*inverse screen (180°) */
#if CONFIG_JOLT_DISPLAY_FLIP
    ESP_LOGI( TAG, "Flipping Display" );
    ssd1306_set_scan_direction_fwd( &disp_hal, true );
    ssd1306_set_segment_remapping_enabled( &disp_hal, false );
#else
    ssd1306_set_scan_direction_fwd( &disp_hal, false );
    ssd1306_set_segment_remapping_enabled( &disp_hal, true );
#endif

    lv_disp_drv_init( &lv_disp_drv );
    lv_disp_buf_init( &disp_buf, vdb, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX ); /*Initialize the display buffer*/
    lv_disp_drv.buffer     = &disp_buf;
    lv_disp_drv.flush_cb   = ssd1306_flush;
    lv_disp_drv.set_px_cb  = ssd1306_vdb_wr;
    lv_disp_drv.rounder_cb = ssd1306_rounder;
    lv_disp_drv_register( &lv_disp_drv );

    ssd1306_set_whole_display_lighting( &disp_hal, false );
    jolt_display_set_brightness( jolt_display_get_brightness() );
}

void jolt_display_print( const jolt_display_t *disp )
{
    uint8_t buf_copy[sizeof( vdb )];

    assert( LV_VER_RES == 64 );
    assert( LV_HOR_RES_MAX == 128 );
    printf( "\n" );
    char print_buf[LV_HOR_RES_MAX * 4 + 2] = {0};  // newline + null-terminator

    /* Fill buf_copy with non-encoded data */
    if( NULL == disp ) {
        JOLT_GUI_CTX
        {
            lv_obj_invalidate( lv_scr_act() );
            lv_refr_now( NULL );
            memcpy( buf_copy, vdb, sizeof( buf_copy ) );
        }
    }
    else {
        int len;
        jolt_encoding_t decoder;
        assert( disp->type == JOLT_DISPLAY_TYPE_SSD1306 );
        decoder = jolt_encoding_get_decoder( disp->encoding );
        len     = decoder( buf_copy, sizeof( buf_copy ), disp->data, disp->len );
        if( len <= 0 || len > sizeof( buf_copy ) ) return;
    }

    /* Print the resulting buf_copy */
    jolt_suspend_logging();
    for( uint8_t y1 = 0; y1 < LV_VER_RES_MAX - 1; y1 += 2 ) {
        memzero( print_buf, sizeof( print_buf ) );
        uint8_t y2 = y1 + 1;
        for( uint8_t x = 0; x < LV_HOR_RES_MAX; x++ ) {
            bool val1, val2;
            uint8_t *disp_buf_ptr;
            disp_buf_ptr = (uint8_t *)buf_copy + LV_HOR_RES_MAX * ( y1 >> 3 ) + x;
            val1         = (bool)( *disp_buf_ptr & ( 1 << ( y1 % 8 ) ) );
            disp_buf_ptr = (uint8_t *)buf_copy + LV_HOR_RES_MAX * ( y2 >> 3 ) + x;
            val2         = (bool)( *disp_buf_ptr & ( 1 << ( y2 % 8 ) ) );
            if( val1 && val2 )
                strlcat( print_buf, "█", sizeof( print_buf ) );
            else if( val1 )
                strlcat( print_buf, "▀", sizeof( print_buf ) );
            else if( val2 )
                strlcat( print_buf, "▄", sizeof( print_buf ) );
            else
                strlcat( print_buf, " ", sizeof( print_buf ) );
        }
        strlcat( print_buf, "\n", sizeof( print_buf ) );
        printf( print_buf );
    }
    jolt_resume_logging();
}

bool jolt_display_copy( jolt_display_t *copy )
{
    int encoded_len;
    jolt_encoding_t encoder;
    uint8_t buf_copy[sizeof( vdb )];

    if( NULL == copy ) return false;

    /* Copy over the current vdb */
    JOLT_GUI_CTX
    {
        lv_obj_invalidate( lv_scr_act() );
        lv_refr_now( NULL );
        memcpy( buf_copy, vdb, sizeof( vdb ) );
    }

    encoder = jolt_encoding_get_encoder( copy->encoding );

    /* Allocate output data and buffer */
    encoded_len = encoder( NULL, 0, buf_copy, sizeof( buf_copy ) );
    if( encoded_len <= 0 ) return false;

    if( NULL == ( copy->data = malloc( encoded_len ) ) ) return false;
    copy->len = encoder( copy->data, encoded_len, buf_copy, sizeof( buf_copy ) );
    if( copy->len < encoded_len ) {
        /* Shrink the allocated area */
        void *tmp;
        tmp = realloc( copy->data, copy->len );
        if( tmp ) copy->data = tmp;
    }
    copy->type = JOLT_DISPLAY_TYPE_SSD1306;

    return true;
}

void jolt_display_dump( const jolt_display_t *disp )
{
    uint8_t buf_copy[sizeof( vdb )];

    jolt_display_t default_disp = {0};

    if( NULL == disp ) {
        /* No encoding */
        default_disp.type     = JOLT_DISPLAY_TYPE_SSD1306;
        default_disp.encoding = JOLT_ENCODING_NONE;
        default_disp.data     = buf_copy;
        default_disp.len      = sizeof( buf_copy );

        JOLT_GUI_CTX
        {
            lv_obj_invalidate( lv_scr_act() );
            lv_refr_now( NULL );
            jolt_encoding_none_encode( default_disp.data, default_disp.len, vdb, sizeof( vdb ) );
        }
        disp = &default_disp;
    }

    assert( disp->type == JOLT_DISPLAY_TYPE_SSD1306 );

    /* Dump to stdout */
    printf( "\nDisplayDump (%s encoding, %d long):\n{\n", jolt_encoding_get_str( disp->encoding ), disp->len );
    for( int i = 0; i < disp->len; i++ ) {
        printf( "0x%02X", disp->data[i] );
        if( i == disp->len - 1 )
            printf( "\n" );
        else if( 0 == ( i + 1 ) % 16 )
            printf( ",\n" );
        else
            printf( ", " );
    }
    printf( "\n}\n" );
}

void jolt_display_free( jolt_display_t *disp )
{
    if( NULL == disp ) return;
    SAFE_FREE( disp->data );
    disp->len = 0;
}

static const uint8_t brightness_levels[] = {0, 1, 2, 50, 120, 255};
static const uint8_t precharge_levels[]  = {10, 10, 10, 90, 130, 255};

uint8_t jolt_display_get_brightness()
{
    /* Returns saved brightness level or default */
    uint8_t brightness;
    storage_get_u8( &brightness, "user", "disp_bright", CONFIG_JOLT_DISPLAY_BRIGHTNESS );
    ESP_LOGI( TAG, "brightness %d", brightness );
    return brightness;
}

void jolt_display_save_brightness( uint8_t level ) { storage_set_u8( level, "user", "disp_bright" ); }

void jolt_display_set_brightness( uint8_t level )
{
    ssd1306_set_contrast( &disp_hal, brightness_levels[level] );
    ssd1306_set_precharge_period( &disp_hal, precharge_levels[level] );
}
