TEMPLATE = subdirs

SUBDIRS += \
    VVGL \
    VVGLTestApp \
    VVISF \
    VVISFTestApp

VVGLTestApp.depends += VVGL
VVISF.depends += VVGL
VVISFTestApp.depends += VVISF
