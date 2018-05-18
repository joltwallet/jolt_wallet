# How to add a coin
To begin adding a new coin, copy the template folder and name it your coin's 3/4 letter acronym `${coin}`. In this documentation, `${coin}` indicates lowercase while `${COIN}` indicates uppercase.

## ESP-IDF Build System
In order to properly link files, all subfolders must contain a `component.mk`. In the simplest case, this is just an empty file. The coin's folder and subfolder must be added to the `COMPONENT_SRCDIRS` path located in `main/component.mk`. This is typically just adding `coins/${coin}` and `coins/${coin}/menus`, but your project may have more subdirectories.
## Vault RPC Calls

Any sensitive information (such as signing information) must occur within the vault task. Actions that require sensitive information are obtained by creating and calling RPC commands to the vault task. The vault task automatically removes any secure information from RAM after a timeout period. The vault also organizes all sensitive code to a single area to make secure-programming easier. Within the vault task is a `struct vault_t` which contains:

1) Plain-text space separated 24-word mnemonic.
2) The 512-bit derived master seed (assumed empty-string passphrase). This is cached on correct pin-entry for user performance.

An rpc call  `struct vault_rpc_t` has three fields:

1) `enum vault_rpc_type_t type` indicates the action to be performed
2) `QueueHandle_t response_queue` a queue for the vault task to respond on RPC completion. This is taken care of for you.
3) Finally there's the union that holds any data (e.g. message to sign) that the RPC command will use as input.


### Vault RPC Registry
In `main/vault.h`, all RPC commands are registered in `typedef enum vault_rpc_type_t`. All command names start with `${COIN}_` and must be enumerated between `${COIN}_START` and `${COIN}_END`. These two dummy commands are only used for organizational purposes.

In `main/vault.c`, add
```
#include "coins/${coin}/rpc.h"
```

Next, add the following case statement to the switch statement marked `/* MASTER RPC SWITCH STATEMENT */`.

```
case ${COIN}_START ... ${COIN}_END:
    response = rpc_${coin}(vault, cmd);
    break;
```
This will forward the command and the vault to your coin's RPC definitions (next section).

### Vault RPC Definition

In `main/coins/${coin}/rpc.h`, change the include guard to
```
#ifndef __NANORAY_${COIN}_RPC_H__
```

Similarly, change the coin-specific rpc function definition to
```
vault_rpc_response_t rpc_${coin}(vault_t *vault, vault_rpc_t *cmd);

```

Finally, jumping into the heart of a coin's operations, the actual RPC commands for signing and other secret-related actions. All subsequent code in this subsection will be in `main/coins/${coin}/rpc.c`.

First, for logging purposes, change the `TAG` to `vault_${coin}`

Individual RPC commands should have a function declaration like:
```
static vault_rpc_response_t rpc_${coin}_commandtodo(vault_t *vault, vault_rpc_t *cmd)
```
Each RPC command function must take in only these two parameters.

All information to complete the task should be either available in the passed in cmd's payload or via calls to non-volatile storage. No calls to wifi or bluetooth should be done within the RPC task.

Finally, register your command in `/* COIN RPC SWITCH */`. For example,
```
case(${COIN}_MSG_SIGN):
    ESP_LOGI(TAG, "Executing ${COIN}_MSG_SIGN RPC.");
    response = rpc_${coin}_msg_sign(vault, cmd);
    break;
```

### RPC Command Payload
Since the RPC task can only respond with the status outcome of the task, additional information must be returned via the payload union in `struct vault_rpc_t` defined in `main/vault.h`. If your RPC command requires the passing of data, create an additional struct to the union with name a lowercase version of the RPC command name defined in `enum vault_rpc_type_t`. The RPC Command only uses pointers to this data, so outcomes like signatures can be written to the payload created by the RPC caller.

### Calling RPC commands
All RPC commands are called by `vault_rpc_response_t vault_rpc(vault_rpc_t *rpc)`. This command takes in the crafter rpc command, sends it to the rpc task, and blocks until the rpc task responds.

## Menuing
Jolt uses the `menu8g2` component to easily create and use menus. A menu8g2 struct contains:
1) Display object u8g2,
2) Mutex locks for the display buffer,
3) Current menu index
4) Buffered user input (pushbuttons).

Typically at the top of every menu function, the menu8g2 struct from the parenting menu is copied:

```
menu8g2_t menu;
menu8g2_copy(&menu, prev);

```

### Main Menu Registry
The first step is to add our coin to Jolt's main menu. In `main/gui.c`, first include our coin's menu header:
```
#include "coins/${coin}/menu.h"
```

To add a menu element option, add a menu element in the order you want it to appear near the area marked `/* MAIN MENU CONSTRUCTION */`.
```
menu8g2_set_element(&elements, "Coin Name To Display", &menu_${coin});
```
Because we added an element, we must increment the constant indicating the number of elements in the `menu8g2_elements_init` call.

### Coin-specific sub-menuing
In `main/coins/${coin}/menu.h`, change the include guard to
```
#ifndef __NANORAY_${COIN}_MENU_H__
```

Similarly, change the coin-specific menu function definition to
```
void menu_${coin}(menu8g2_t *prev);

```

In `main/coins/${coin}/menu.c` we define all of our coin menu options.

In `main/coins/${coin}/submenus/submenus.h` we include all the coin's submenu function prototypes.

In `main/coins/${coin}/submenus/menu_${choice_action}.c` we define the functionality of each menu option. The `balance.c` menu action demonstrates the general structure.

## Non-Volatile storage
If your coin utilizes some form of nonvolatile storage, use the all capital `${COIN}` NVS namespace. Open and close this NVS namespace on demand from within menu options or RPC commands. Do not store critical secret information (like private keys) that could result in the loss of funds in these namespaces.
