DESCRIPTION = "Kernel Module Development Test"
HOMEPAGE = "http://www.example.com"
SECTION = "kernel/modules"
PRIORITY = "optional"
LICENSE = "none"

RDEPENDS = "kernel (2.6.32)"
DEPENDS = "virtual/kernel"
PR = "r1"

SRC_URI = " \
	file://kmodule_test.c \
	file://kmodule_test.h \
	file://Makefile \
"

S = "${WORKDIR}"

inherit module

do_compile () {
	unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS CC LD CPP
	oe_runmake 'MODPATH="${D}${base_libdir}/modules/2.6.32/kernel/drivers/ecu"' \
		'KERNEL_SOURCE="${STAGING_KERNEL_DIR}"' \
		'KDIR="${STAGING_KERNEL_DIR}"' \
		'KERNEL_VERSION="2.6.32"' \
		'CC="${KERNEL_CC}"' \
		'LD="${KERNEL_LD}"'

}

do_install () {
	install -d ${D}${base_libdir}/modules/2.6.32/kernel/drivers/test
	install -m 0644 ${S}/kmodule_test*${KERNEL_OBJECT_SUFFIX} ${D}${base_libdir}/modules/2.6.32/kernel/drivers/test
}

