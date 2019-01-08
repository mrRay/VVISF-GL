TEMPLATE = aux

# the installer target's contents are only executed during release builds
CONFIG(release, debug|release)	{

	BINARYCREATOR = $$dirname(QMAKE_QMAKE)/../../../Tools/QtInstallerFramework/3.0/bin/binarycreator
	REPOGEN = $$dirname(QMAKE_QMAKE)/../../../Tools/QtInstallerFramework/3.0/bin/repogen

	#INPUT = $$PWD/config/config.xml $$PWD/packages
	INPUT = $$PWD/config/config.xml
	
	#	this bit copies the compiled app and sample ISF files into the 'data' directory of their packages
	#	it's commented out because on os x control returns before the copy is complete, and the resulting installer is incomplete.  very disconcerting.
	#installer_prebuild.input = INPUT
	#installer_prebuild.output = my_installer_prebuild
	# clear the 'data' directory in the app package, copy the latest compiled application to it
	#PRO_DATA_PATH = "$$_PRO_FILE_PWD_/packages/com.vidvox.ISFEditor.mac/data"
	#installer_prebuild.commands += echo "prebuild, assembling mac app";
	#installer_prebuild.commands += rm -Rf $${PRO_DATA_PATH};
	#installer_prebuild.commands += mkdir -v $${PRO_DATA_PATH};
	#installer_prebuild.commands += cp -vaRf "$$OUT_PWD/../ISFEditor_app/ISFEditor.app" "$${PRO_DATA_PATH}/ISFEditor.app";
	# clear the 'data' directory in the isf files package, copy the isf files from the repos to it
	#PRO_DATA_PATH = "$$_PRO_FILE_PWD_/packages/com.vidvox.ISFFiles.mac/data"
	#installer_prebuild.commands += echo "prebuild, assembling isf files";
	#installer_prebuild.commands += rm -Rf $${PRO_DATA_PATH};
	#installer_prebuild.commands += mkdir -v $${PRO_DATA_PATH};
	#installer_prebuild.commands += cp -vaRf "$$_PRO_FILE_PWD_/../../../ISF-files/ISF/*" $${PRO_DATA_PATH};
	#installer_prebuild.CONFIG += target_predeps no_link combine
	
	offline_installer_creator.input = INPUT
	offline_installer_creator.output = my_offline_installer
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


