#include "hal/display.h"
#include "jolt_gui/jolt_gui.h"
#include "unity.h"

static const char MODULE_NAME[] = "[jolt_gui_qr]";

TEST_CASE( "jolt_gui_qr", MODULE_NAME )
{
    jolt_gui_obj_t *scr;
    jolt_display_t actual;
    const char test_data_1[] = "test data";
    jolt_display_t expected;

    /* */
    scr = jolt_gui_scr_qr_create( "QR Test", test_data_1, strlen( test_data_1 ) );
    TEST_ASSERT_TRUE( jolt_display_copy( &actual ) );
    // TEST_ASSERT_DISPLAY(expected, actual);
    TEST_ASSERT_DISPLAY( NULL, &actual );
    jolt_display_free( &actual );
    jolt_gui_scr_del( scr );
}
