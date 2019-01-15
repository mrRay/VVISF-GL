
function Component()
{
}

Component.prototype.createOperations = function()
{
    //  call default implementation to actually install the files
    component.createOperations();

    if (systemInfo.productType === "windows")    {
        component.addElevatedOperation("CreateShortcut",
                               "@TargetDir@/ISFEditor.exe",
                               "@StartMenuDir@/ISF Editor.lnk",
                               "description=The ISF Editor");
    }
}
