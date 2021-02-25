async function GetTableInfo(TableName)
{
    ReturnValue = [];
    Result =  fetch("/DBGeneralAPI/", 
    {
     method: "POST", 
     body: "GetTableInfo "+TableName,
     headers: {
   'Content-Type': 'text/plain'
   // 'Content-Type': 'application/x-www-form-urlencoded',
 },
}).then(res => res.json()).then(Data =>{
    //console.log(Data);
    //console.log(Data.TableNames);
    ReturnValue = Data.TableInfo;
    console.log(ReturnValue);
    //console.log(ReturnValue);   
});
    FinishedResult = await Result;
    //console.log("InnanReturn");
    //console.log(ReturnValue);
    console.log(ReturnValue);
    return(ReturnValue);
}
async function GetAvailableTables()
{
    ReturnValue = [];
    Result =  fetch("/DBGeneralAPI/", 
    {
     method: "POST", 
     body: "GetTableNames",
     headers: {
   'Content-Type': 'text/plain'
   // 'Content-Type': 'application/x-www-form-urlencoded',
 },
}).then(res => res.json()).then(Data =>{
    //console.log(Data);
    //console.log(Data.TableNames);
    ReturnValue = Data.TableNames;
    //console.log(ReturnValue);   
});
    FinishedResult = await Result;
    //console.log("InnanReturn");
    //console.log(ReturnValue);
    return(ReturnValue);
}
function GetTableMetadata()
{
    return("Fel");
}

async function DBAddMain()
{
    var MainDiv = document.getElementById("DBAddMainDiv");
    //var TableMetadata = GetTableMetadata();
    var NewTable = document.createElement("table");
    NewTable.classList.add("SQLTable")
    var AvailableTables = await GetAvailableTables();
    var URLTableName = window.location.pathname.split("/")[2];
    
    var NameInTable = false;
    var TableNamesLength = AvailableTables.length;
    for(var i = 0;i<TableNamesLength;i++)
    {
        if(URLTableName == AvailableTables[i])
        {
            NameInTable = true;
            break;
        }
    }

    //console.log(TableNames);
    if(NameInTable)
    {
        var TableInfo = await GetTableInfo(URLTableName);
        var ColumnNames = [];
        var ColumnTypes = [];
        var InputFields = [];
        var InputTemplate = "<textarea style=\"width: 100%;height:100%;resize:vertical\"></textarea>"
        for(let i = 0;i<TableInfo.length;i++)
        {
            ColumnNames.push(TableInfo[i].ColumnName);
            ColumnTypes.push(TableInfo[i].ColumnType);
            InputFields.push(InputTemplate);
        }
        console.log(TableInfo);
        NewTable.appendChild(CreateTableRow(ColumnNames));
        NewTable.appendChild(CreateTableRow(ColumnTypes));
        NewTable.appendChild(CreateTableRow(InputFields));
        MainDiv.appendChild(NewTable);
        console.log("Funktionen Klar");
    }
    else
    {

    }
}
DBAddMain();