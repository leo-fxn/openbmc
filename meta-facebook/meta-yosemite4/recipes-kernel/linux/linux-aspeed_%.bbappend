FILESEXTRAPATHS:prepend := "${THISDIR}/linux-aspeed:"
SRC_URI += " \
    file://1000-arm-dts-aspeed-yosemite4-enable-Jtag-on-yosemite4.patch \
    file://1001-arm-dts-aspeed-yosemite4-add-mac-config-property.patch \
    file://1002-arm-dts-aspeed-yosemite4-Revise-i2c-duty-cycle.patch \
    file://1003-arm-dts-aspeed-yosemite4-add-power-and-adc-sensors-o.patch \
    file://1004-arm-dts-aspeed-yosemite4-add-multi-source-of-power-s.patch \
    file://1005-arm-dts-aspeed-yosemite4-set-TACH-config-for-MAX3179.patch \
    file://1006-arm-dts-aspeed-yosemite4-add-2nd-source-fan-IC-NCT73.patch \
    file://1007-arm-dts-aspeed-yosemite4-add-EMC1403-for-Terminus-NI.patch \
    file://1008-arm-dts-aspeed-yosemite4-Support-MP5990-for-all-Sent.patch \
    file://1009-ARM-dts-aspeed-yosemite4-add-I3C-config-in-DTS.patch \
    file://1010-ARM-dts-aspeed-yosemite4-add-fan-led-config.patch \
    file://1011-arm-dts-aspeed-yosemite4-add-gpio-related-settings.patch \
    file://1012-ARM-dts-aspeed-yosemite4-modify-I2C-mode-and-freq.patch \
    file://1013-yosemite4-support-CAT9532-led-controller.patch \
    file://1014-adc128-filter-out-0x1ff.patch \
    file://2000-ubifs-Fix-memory-leak-of-bud-log_hash.patch \
    file://2001-ubifs-ubifs_link-Fix-wrong-name-len-calculating-when.patch \
    file://2002-ubifs-Check-c-dirty_-n-p-n_cnt-and-c-nroot-state-und.patch \
    file://2003-ubifs-fix-sort-function-prototype.patch \
    file://2004-ubifs-Remove-unreachable-code-in-dbg_check_ltab_lnum.patch \
    file://2005-ubifs-dbg_check_idx_size-Fix-kmemleak-if-loading-zno.patch \
    file://2006-ubifs-Queue-up-space-reservation-tasks-if-retrying-m.patch \
    file://2007-ubifs-Fix-unattached-xattr-inode-if-powercut-happens.patch \
    file://2008-ubifs-Fix-adding-orphan-entry-twice-for-the-same-ino.patch \
    file://2009-ubifs-Fix-space-leak-when-powercut-happens-in-linkin.patch \
    file://2010-ubifs-Fix-unattached-inode-when-powercut-happens-in-.patch \
    file://2011-ubifs-dbg_orphan_check-Fix-missed-key-type-checking.patch \
    file://2012-ubifs-add-check-for-crypto_shash_tfm_digest.patch \
    file://2013-ubifs-Don-t-add-xattr-inode-into-orphan-area.patch \
    file://2014-ubifs-Remove-insert_dead_orphan-from-replaying-orpha.patch \
    file://yosemite4-local.cfg \
"
