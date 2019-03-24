#include "jolt_lib.h"
extern const jolt_version_t JOLT_OS_VERSION;   /**< JoltOS version */
extern const jolt_version_t JOLT_JELF_VERSION; /**< Used to determine app compatibility */
extern const jolt_version_t JOLT_HW_VERSION;   /**< To check hardware compatability */


int jolt_cmd_about(int argc, char** argv) {
    printf("Hardware: %d.%d.%d\n", JOLT_HW_VERSION.major, JOLT_HW_VERSION.minor, JOLT_HW_VERSION.patch);
    printf("JoltOS: %d.%d.%d %s\n", JOLT_OS_VERSION.major, JOLT_OS_VERSION.minor, JOLT_OS_VERSION.patch, JOLT_OS_COMMIT);
    printf("JELF Loader: %d.%d.%d\n", JOLT_JELF_VERSION.major, JOLT_JELF_VERSION.minor, JOLT_JELF_VERSION.patch);

    return 0;
}
