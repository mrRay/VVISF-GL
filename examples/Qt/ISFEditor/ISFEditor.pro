TEMPLATE = subdirs

CONFIG = ordered

SUBDIRS += \
	fftreal \
    ISFEditor_app

ISFEditor_app.depends += fftreal

