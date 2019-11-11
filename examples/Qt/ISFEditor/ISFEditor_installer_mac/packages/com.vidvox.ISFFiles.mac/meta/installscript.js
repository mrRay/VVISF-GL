
function Component()
{
}

Component.prototype.createOperationsForArchive = function(archive)
{
    //	if the /Library/Graphics/ISF folder doesn't exist yet, create it
    if (!installer.fileExists("/Library/Graphics/ISF"))	{
		component.addElevatedOperation("Mkdir", "/Library/Graphics/ISF");
    }
    //	extract this component (the files, and only the files) to the folder
    component.addElevatedOperation("Extract", archive, "/Library/Graphics/ISF");
    //  make the files in the directory writable
    component.addElevatedOperation("Execute", "chmod", "-R", "u+w", "/Library/Graphics/ISF");
    component.addElevatedOperation("Execute", "chmod", "-R", "g+w", "/Library/Graphics/ISF");
    component.addElevatedOperation("Execute", "chmod", "-R", "o+w", "/Library/Graphics/ISF");
}
