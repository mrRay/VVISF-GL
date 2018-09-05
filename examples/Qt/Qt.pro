TEMPLATE = subdirs

CONFIG = ordered

SUBDIRS += \
    VVGL \
    VVGLTestApp \
	VVISF \
	VVISFTestApp \
	TexUploadBenchmark \
	TexDownloadBenchmark \

TexUploadBenchmark.depends += VVGL
TexDownloadBenchmark.depends += VVISF
VVGLTestApp.depends += VVGL
VVISF.depends += VVGL
VVISFTestApp.depends += VVISF

