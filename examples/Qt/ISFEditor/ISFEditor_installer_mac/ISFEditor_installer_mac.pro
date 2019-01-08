TEMPLATE = aux

# this file is only included if it's a mac build, but just in case...
mac	{
	# the installer target's contents are only executed during release builds
	CONFIG(release, debug|release)	{

		BINARYCREATOR = $$dirname(QMAKE_QMAKE)/../../../Tools/QtInstallerFramework/3.0/bin/binarycreator
		REPOGEN = $$dirname(QMAKE_QMAKE)/../../../Tools/QtInstallerFramework/3.0/bin/repogen

		#INPUT = $$PWD/config/config.xml $$PWD/packages
		INPUT = $$PWD/config/config.xml
	
		offline_installer_creator.input = INPUT
		offline_installer_creator.output = ISFEditor_installer
		offline_installer_creator.clean_commands = rm -Rf "$$OUT_PWD/$$offline_installer_creator.output\.app";
		offline_installer_creator.commands += echo "building offline installer...";
		offline_installer_creator.commands += rm -Rf "$$OUT_PWD/$$offline_installer_creator.output\.app";
		offline_installer_creator.commands += $$BINARYCREATOR -c $$PWD/config/config.xml -p $$PWD/packages ${QMAKE_FILE_OUT};
		#offline_installer_creator.commands += $$BINARYCREATOR --offline-only -c $$PWD/config/config.xml -p $$PWD/packages ${QMAKE_FILE_OUT};
		offline_installer_creator.CONFIG += target_predeps no_link combine

		repo_creator.input = INPUT
		repo_creator.output = repo_creator_output
		repo_creator.clean_commands = rm -Rf "$$PWD/repository";
		repo_creator.commands += echo "building repository...";
		repo_creator.commands += rm -Rf "$$PWD/repository";
		repo_creator.commands += $$REPOGEN -p $$PWD/packages $$PWD/repository;
		repo_creator.CONFIG += target_predeps no_link combine

		#QMAKE_EXTRA_COMPILERS += installer_prebuild
		QMAKE_EXTRA_COMPILERS += offline_installer_creator
		QMAKE_EXTRA_COMPILERS += repo_creator

		OTHER_FILES = README
	}
}

