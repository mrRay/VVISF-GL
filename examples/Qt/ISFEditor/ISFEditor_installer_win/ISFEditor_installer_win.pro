TEMPLATE = aux

# this file is only included if it's a windows build, but just in case...
win32	{
	# the installer target's contents are only executed during release builds
	CONFIG(release, debug|release)	{

		BINARYCREATOR = $$dirname(QMAKE_QMAKE)/../../../Tools/QtInstallerFramework/3.0/bin/binarycreator
		REPOGEN = $$dirname(QMAKE_QMAKE)/../../../Tools/QtInstallerFramework/3.0/bin/repogen

		#INPUT = $$PWD/config/config.xml $$PWD/packages
		INPUT = $$PWD/config/config.xml
	
		offline_installer_creator.input = INPUT
		offline_installer_creator.output = ISFEditor_installer
		offline_installer_creator.clean_commands = rm "$$OUT_PWD/$$offline_installer_creator.output" $$escape_expand(\n)
		offline_installer_creator.commands += rm $$shell_quote($$shell_path("$$OUT_PWD/$$offline_installer_creator.output")) $$escape_expand(\n)
		offline_installer_creator.commands += $$BINARYCREATOR -c $$shell_quote($$shell_path($$PWD/config/config.xml)) -p $$shell_quote($$shell_path($$PWD/packages)) ${QMAKE_FILE_OUT} $$escape_expand(\n)
		#offline_installer_creator.commands += $$BINARYCREATOR --offline-only -c $$shell_quote($$shell_path($$PWD/config/config.xml)) -p $$shell_quote($$shell_path($$PWD/packages)) ${QMAKE_FILE_OUT} $$escape_expand(\n)
		offline_installer_creator.CONFIG += target_predeps no_link combine

		repo_creator.input = INPUT
		repo_creator.output = repo_creator_output
		repo_creator.clean_commands = rm $$shell_quote($$shell_path("$$PWD/repository")) $$escape_expand(\n)
		repo_creator.commands += rmdir $$shell_quote($$shell_path("$$PWD/repository")) $$escape_expand(\n)
		repo_creator.commands += $$REPOGEN -p $$shell_quote($$shell_path($$PWD/packages $$PWD/repository)) $$escape_expand(\n)
		repo_creator.CONFIG += target_predeps no_link combine

		#QMAKE_EXTRA_COMPILERS += installer_prebuild
		QMAKE_EXTRA_COMPILERS += offline_installer_creator
		QMAKE_EXTRA_COMPILERS += repo_creator

		OTHER_FILES = README
	}
}

