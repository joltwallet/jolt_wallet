#ifndef JOLT_APP_CMD_CONTACTS_H__
#define JOLT_APP_CMD_CONTACTS_H__

enum {
    JOLT_APP_CMD_CONTACT_ERROR = -1,
    JOLT_APP_CMD_CONTACT_OK    = 0,
    JOLT_APP_CMD_CONTACT_NOT_IMPLEMENTED,
    JOLT_APP_CMD_CONTACT_INVALID_ARGS,
    JOLT_APP_CMD_CONTACT_OOM,
    JOLT_APP_CMD_CONTACT_DECLINED,
};

/**
 *
 */
int jolt_app_cmd_contact( int argc, const char **argv );

#endif
