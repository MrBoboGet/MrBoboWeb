
async function UpdateFolderTable()
{
    let MainDiv = document.getElementById("ViewDiv");
    let NewTable = document.createElement("table");
    let DirectiveString = "GetFolderContents ";
    let URLPaths = window.location.pathname.split("/");
    
    let FolderToGet = "/";
    let InRelativePath = false;
    for(let i = 0; i < URLPaths.length; i++)
    {
        if(InRelativePath)
        {
            FolderToGet +=URLPaths[i]+"/";
        }
        if(URLPaths[i] == "DBView")
        {
            InRelativePath = true;
        }
    }
    FolderToGet = decodeURIComponent(FolderToGet);
    let DirectiveToSend = {Directive: "GetFolderContents",DirectiveArguments:{DirectoryName: FolderToGet}};
    let DirectoryEntries = await MBDBAPI_SendDirective(DirectiveToSend);
    if(DirectoryEntries.MBDBAPI_Status == "ok")
    {
        console.log(DirectoryEntries);
        for(let i = 0; i<DirectoryEntries.DirectiveResponse.DirectoryEntries.length;i++)
        {
            let CurrentEntry = DirectoryEntries.DirectiveResponse.DirectoryEntries[i];
            let NewRowValue = "<a href=\""+"/DBView/"+FolderToGet+"/"+CurrentEntry.Name+"\">";
            NewRowValue+=CurrentEntry.Name;
            if(CurrentEntry.Type == "Directory")
            {
                NewRowValue+=" (Directory)";
            }
            NewRowValue+="</a>";

            NewTable.appendChild(CreateTableRow([NewRowValue]));
        }
        MainDiv.appendChild(NewTable);
    }
    else
    {
        MainDiv.innerHTML = DirectoryEntries.MBDBAPI_Status;
    }
}
UpdateFolderTable();