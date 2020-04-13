# change sdkconfig defaults based on env var TARGET_BOARD

TARGET_BOARD ?= joltv1

export SDKCONFIG_DEFAULTS := \
	$(JOLT_WALLET_PATH)/sdkconfigs/sdkconfig.defaults \
	$(JOLT_WALLET_PATH)/sdkconfigs/sdkconfig.defaults.$(TARGET_BOARD) \
	$(JOLT_WALLET_PATH)/sdkconfigs/sdkconfig.personal \
	sdkconfig.japp \
	sdkconfig.personal
# sdkconfig.japp and sdkconfig.personal are intended for jolt apps
