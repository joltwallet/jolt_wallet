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
 * @brief CLI registerable function for an app to have a contact book.
 *
 * Notes:
 *     * The contact book is stored in the app's json file under the key `contacts`
 *     * All indexs are 0-indexed.
 *     * Only the `read` command returns data.
 *     * Consumer of the contact book must validate addresses.
 *
 * Has the following commands:
 *
 * add - Add a name/address pair to the contact book. No error-checking is
 *       performed on the address.
 *     Synopsis:
 *         contact add [name] [address]
 *     Example:
 *         jolt> APP_NAME contact add "Bob Smith" "some address"
 *
 * read - Print the contact book
 *     Synopsis:
 *         contact
 *     Example:
 *         jolt> APP_NAME contact read
 *         {"contacts":[{"name":"Bob Smith","address":"some address"}]}
 *
 * update - Update an existing contact by index.
 *     Synopsis:
 *         contact udpate [index] [new_name] [new_address]
 *     Example:
 *         jolt> APP_NAME contact update 0 "Bob Smith" "some address"
 *
 * delete - Delete a contact by index.
 *     Synopsis:
 *         contact delete [index]
 *     Example:
 *         jolt> APP_NAME contact delete 5
 *
 */
int jolt_app_cmd_contact( int argc, char **argv );

#endif
