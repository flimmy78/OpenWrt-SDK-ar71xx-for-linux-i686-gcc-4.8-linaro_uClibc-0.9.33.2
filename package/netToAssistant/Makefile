#OpenWrt Makefile for netToAssistant program
########################################

include $(TOPDIR)/rules.mk
PKG_NAME:=netToAssistant
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk
define Package/netToAssistant
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=netToAssistant -- prints a snarky message
	DEPENDS:=+libc +libpthread +librt
endef
define Package/netToAssistant/description
	If you can't figure out what this program does, you're probably
	brain-dead and need immediate medical attention.
endef
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef
define Package/netToAssistant/install
	mkdir -p $(1)/usr/bin/dataroute
	$(INSTALL_DIR) $(1)/usr/bin/dataroute
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/netToAssistant $(1)/usr/bin/dataroute
endef
$(eval $(call BuildPackage,netToAssistant))

