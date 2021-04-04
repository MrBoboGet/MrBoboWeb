
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
    console.log(FolderToGet);
    DirectiveString += MBDBAPI_EncodeArguments([FolderToGet]);
    let DirectoryEntries = await MBDBAPI_SendDirective(DirectiveString);
    console.log(DirectoryEntries);
    if(DirectoryEntries.MBDBAPI_Status == "ok")
    {
        for(let i = 0; i<DirectoryEntries.DirectoryEntries.length;i++)
        {
            let NewRowValue = "<a href=\""+"/DBView"+DirectoryEntries.DirectoryEntries[i].Path+"\">";
            NewRowValue+=GetPathStem(DirectoryEntries.DirectoryEntries[i].Path);
            if(DirectoryEntries.DirectoryEntries[i].Type == "Directory")
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