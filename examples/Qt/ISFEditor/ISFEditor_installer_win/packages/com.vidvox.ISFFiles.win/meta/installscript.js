
function Component()
{
}

Component.prototype.createOperationsForArchive = function(archive)
{
    //	if the /ProgramData/ISF folder doesn't exist yet, create it
    var         rootDirPath = installer.value("RootDir");
    //console.log("root dir is " + rootDirPath);
    var         tmpPath = rootDirPath+"ProgramData/ISF"
    if (!installer.fileExists(tmpPath)) {
        component.addOperation("Mkdir", tmpPath);
    }
    component.addOperation("Extract", archive, tmpPath);

    if (systemInfo.productType === "windows")    {
        component.addOperation("CreateShortcut",
                               tmpPath,
                               "@StartMenuDir@/ISF Files.lnk",
                               "description=Global ISF files directory");
    }
}
