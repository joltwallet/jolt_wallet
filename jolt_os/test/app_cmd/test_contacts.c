#include "jolt_lib.h"
#include "syscore/launcher.h"
#include "unity.h"

static const char MODULE_NAME[] = "[test_contacts]";

TEST_CASE( "add", MODULE_NAME )
{
    launch_set_name( "test" );
    jolt_json_del_app();

    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {"contact", "add", "Bob Jones", "crypto_address"};
        TEST_ASSERT_EQUAL_INT( JOLT_CLI_NON_BLOCKING, jolt_app_cmd_contact( argcount( argv ), argv ) );
        TEST_ASSERT_EQUAL_INT( 0, jolt_cli_get_return() );
        TEST_ASSERT_EQUAL_STRING( "", buf );
    }
    {
        /* Read back to make sure it's correct */
        char *buf;
        cJSON *json, *contacts, *contact;
        json = jolt_json_read_app();
        TEST_ASSERT_NOT_NULL( json );
        TEST_ASSERT_NOT_NULL( contacts = cJSON_Get( json, "contacts" ) );
        TEST_ASSERT_EQUAL_INT( 1, cJSON_GetArraySize( contacts ) );
        TEST_ASSERT_NOT_NULL( contact = cJSON_GetArrayItem( contacts, 0 ) );
        TEST_ASSERT_NOT_NULL( buf = cJSON_PrintUnformatted( contact ) );
        // TODO: Check string
        cJSON_Delete( json );
        free( buf );
    }

    // TODO
    jolt_json_del_app();
}
