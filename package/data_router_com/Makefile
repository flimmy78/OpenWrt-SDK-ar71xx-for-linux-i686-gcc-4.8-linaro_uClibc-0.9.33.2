#OpenWrt Makefile for data_router_com program
########################################

include $(TOPDIR)/rules.mk
PKG_NAME:=data_router_com
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk
define Package/data_router_com
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=data_router_com -- prints a snarky message
	DEPENDS:=+libc +libpthread +librt 
endef
define Package/data_router_com/description
	If you can't figure out what this program does, you're probably
	brain-dead and need immediate medical attention.
endef
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef
define Package/data_router_com/install	
	mkdir -p $(1)/usr/bin/dataroute
	$(INSTALL_DIR) $(1)/usr/bin/dataroute
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/data_router_com $(1)/usr/bin/dataroute
endef
$(eval $(call BuildPackage,data_router_com))

