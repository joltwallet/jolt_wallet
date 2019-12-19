#include "jolt_gui.h"
#include "jolt_gui/menus/settings/bluetooth_pair.h"

#if CONFIG_BT_ENABLED

    #include "services/gap/ble_svc_gap.h"
int jolt_gui_gap_cb( struct ble_gap_event *event, void *arg ) { return jolt_gui_bluetooth_pair_gap_cb( event, arg ); }

#endif
