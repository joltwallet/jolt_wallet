#ifndef JOLT_CMDS_H__
#define JOLT_CMDS_H__

int jolt_cmd_app_key(int argc, char** argv);
int jolt_cmd_upload_firmware(int argc, char** argv);
int jolt_cmd_free(int argc, char** argv);
int jolt_cmd_task_status(int argc, char** argv);
int jolt_cmd_top(int argc, char** argv);
int jolt_cmd_reboot(int argc, char** argv);
int jolt_cmd_wifi_update(int argc, char** argv);
int jolt_cmd_mnemonic_restore(int argc, char** argv);
int jolt_cmd_jolt_cast_update(int argc, char** argv);
int jolt_cmd_pmtop(int argc, char** argv);

int jolt_cmd_bt_whitelist(int argc, char **argv);

int jolt_cmd_run(int argc, char** argv);

int jolt_cmd_upload(int argc, char** argv);
int jolt_cmd_download(int argc, char** argv);
int jolt_cmd_ls(int argc, char** argv);
int jolt_cmd_mv(int argc, char** argv);
int jolt_cmd_rm(int argc, char** argv);

#endif
