TEMPLATE = subdirs

CONFIG = ordered

SUBDIRS += \
    VVGL \
    VVGLTestApp \
	VVISF \
	VVISFTestApp \
    TexUploadBenchmark

TexUploadBenchmark.depends += VVGL
VVGLTestApp.depends += VVGL
VVISF.depends += VVGL
VVISFTestApp.depends += VVISF

