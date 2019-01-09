TEMPLATE = subdirs

CONFIG = ordered

SUBDIRS += \
	fftreal \
    ISFEditor_app \
	ISFEditor_installer_mac \
	ISFEditor_installer_win


ISFEditor_app.depends += fftreal
ISFEditor_installer_mac.depends += ISFEditor_app
ISFEditor_installer_win.depends += ISFEditor_app

