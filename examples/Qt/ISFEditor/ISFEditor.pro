TEMPLATE = subdirs

CONFIG = ordered

SUBDIRS += \
	fftreal \
    ISFEditor_app

app.depends += fftreal

