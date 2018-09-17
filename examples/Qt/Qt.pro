TEMPLATE = subdirs

CONFIG = ordered

SUBDIRS += \
    VVGL \
	VVGLTestApp \
	VVISF \
	VVISFTestApp \
	TexUploadBenchmark \
	TexDownloadBenchmark \
    ISFEditor

TexUploadBenchmark.depends += VVGL
TexDownloadBenchmark.depends += VVISF
VVGLTestApp.depends += VVGL
VVISF.depends += VVGL
VVISFTestApp.depends += VVISF
ISFEditor.depends += VVISF

