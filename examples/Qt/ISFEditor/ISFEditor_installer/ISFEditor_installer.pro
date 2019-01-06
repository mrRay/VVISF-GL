TEMPLATE = aux

# the installer target's contents are only executed during release builds
CONFIG(release, debug|release)	{

	BINARYCREATOR = $$dirname(QMAKE_QMAKE)/../../../Tools/QtInstallerFramework/3.0/bin/binarycreator
	REPOGEN = $$dirname(QMAKE_QMAKE)/../../../Tools/QtInstallerFramework/3.0/bin/repogen

	INPUT = $$PWD/config/config.xml $$PWD/packages

	offline_installer_creator.input = INPUT
	offline_installer_creator.output = my_offline_installer
	offline_installer_creator.clean_commands = rm -Rf "$$OUT_PWD/$$offline_installer_creator.output\.app";
	offline_installer_creator.commands += $$BINARYCREATOR -c $$PWD/config/config.xml -p $$PWD/packages ${QMAKE_FILE_OUT};
	offline_installer_creator.CONFIG += target_predeps no_link combine

	#online_installer.input = INPUT
	#online_installer.output = my_online_installer
	#online_installer.commands = $$BINARYCREATOR --online-only -c $$PWD/config/config.xml -p $$PWD/packages ${QMAKE_FILE_OUT};
	#online_installer.CONFIG += target_predeps no_link combine

	repo_creator.input = INPUT
	repo_creator.output = repo_creator_output
	repo_creator.clean_commands = rm -Rf "$$PWD/repository";
	repo_creator.commands += $$REPOGEN -p $$PWD/packages $$PWD/repository;
	repo_creator.CONFIG += target_predeps no_link combine

	QMAKE_EXTRA_COMPILERS += offline_installer_creator
	#QMAKE_EXTRA_COMPILERS += online_installer
	QMAKE_EXTRA_COMPILERS += repo_creator

	OTHER_FILES = README
}


