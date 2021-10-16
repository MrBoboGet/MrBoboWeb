async function GetTableInfo(TableName)
{
    ReturnValue = [];
    Result =  fetch("/DBGeneralAPI/", 
    {
     method: "POST", 
     body: "GetTableInfo "+TableName.length.toString()+" "+TableName,
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
    ReturnValue = Data;
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

//Resize kod
function ResizeRow(e)
{
    var RowElement = e.target.parentElement.parentElement;
    var RowCells = RowElement.childNodes;
    for(var i = 0; i < RowCells.length;i++)
    {
        RowCells[i].childNodes[0].style.height = e.target.style.height;
    }
}

let MouseHold = false;
var CurrentMouseElement = 0;
function MouseUpCheck()
{
    console.log("MouseUp");
    MouseHold = false;
}
function MouseDownCheck(e)
{
    console.log("MouseDown");
    MouseHold = true;
    CurrentMouseElement = e;
    ResizeCallback();
}
function ResizeCallback()
{
    if(MouseHold)
    {
        ResizeRow(CurrentMouseElement);
        setTimeout(ResizeCallback,1);
    }
}
//End resize kod

//Submit kod
var G_CurrentTableColumnNames = [];
async function SubmitRow()
{
    var MainDiv = document.getElementById("DBAddMainDiv");
    //

    let SubmissionFields = document.querySelectorAll("[data-InputField]");
    var URLPaths = window.location.pathname.split("/");
    var TableName = URLPaths[URLPaths.length-1];
    let SubmissionValues = [TableName];
    //console.log(SubmissionFields);
    for(let i = 0;i < SubmissionFields.length;i++)
    {
        //console.log(SubmissionFields[i].innerHTML);
        let ColumnIndex = SubmissionFields[i].getAttribute("data-inputcolumnindex");
        console.log(ColumnIndex);
        let NewSubbmissionValue = G_CurrentTableColumnNames[ColumnIndex]+":"+ ColumnIndex.toString()+":"+SubmissionFields[i].value;
        //let NewSubbmissionValue = SubmissionFields[i].value;
        SubmissionValues.push(NewSubbmissionValue);
    }
    console.log(SubmissionValues);
    var DirectiveString ="AddEntryToTable ";
    DirectiveString += MBDBAPI_EncodeArguments(SubmissionValues);
    console.log(DirectiveString);
    let ResultElement = document.getElementById("DBADD_Result");
    ResultElement.innerHTML = "sending data..."
    ResultElement.style.color = "orange";
    let APIResult = await MBDBAPI_SendDirective(DirectiveString);

    if(APIResult.MBDBAPI_Status == "ok")
    {
        ResultElement.innerHTML = "Successfully added entry!"
        ResultElement.style.color = "green";
        for(let i = 0;i < SubmissionFields.length;i++)
        {
            SubmissionFields[i].innerHTML = "";
        }
    }
    else
    {
        ResultElement.innerHTML = "Error adding element: "+APIResult.MBDBAPI_Status;
        ResultElement.style.color = "red";
    }
}
//
async function DBAddMain()
{
    var MainDiv = document.getElementById("DBAddMainDiv");
    //var TableMetadata = GetTableMetadata();
    var NewTable = document.createElement("table");
    NewTable.classList.add("SQLTable");
    var AvailableTablesResponse = await GetAvailableTables();
    if(AvailableTablesResponse.MBDBAPI_Status != "ok")
    {
        let ResultElement = document.getElementById("DBADD_Result");
        ResultElement.innerHTML = "Error:"+AvailableTablesResponse.MBDBAPI_Status;
        ResultElement.style.color = "red";
        return;
    }
    var AvailableTables = AvailableTablesResponse.TableNames;
    var URLTableName = window.location.pathname.split("/").pop();
    
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

    console.log(AvailableTables);
    console.log(NameInTable);
    if(NameInTable)
    {
        var TableInfo = await GetTableInfo(URLTableName);
        var ColumnNames = [];
        var ColumnTypes = [];
        var InputFields = [];
        var InputTemplate = "<textarea style=\"width: 100%;height:100%;resize:vertical\" data-InputField=\"True\"></textarea>"
        for(let i = 0;i<TableInfo.length;i++)
        {
            ColumnNames.push(TableInfo[i].ColumnName);
            ColumnTypes.push(TableInfo[i].ColumnType);
            if(TableInfo[i].ColumnType == "int" && TableInfo[i].PrimaryKeyIndex == 1)
            {
                InputFields.push("<div>AutoIncrement</div>");
            }
            else
            {
                InputFields.push(InputTemplate);
            }
        }
        //SÄTTER EN GLOBAL VARIABEL
        G_CurrentTableColumnNames = ColumnNames;
        //
        console.log(TableInfo);
        NewTable.appendChild(CreateTableRow(ColumnNames));
        NewTable.appendChild(CreateTableRow(ColumnTypes));
        var InputRow = CreateTableRow(InputFields);
        //console.log(InputRow);
        for(var i = 0; i < InputRow.childNodes.length; i++)
        {
            //console.log(InputRow.childNodes[i]);
            InputRow.childNodes[i].childNodes[0].addEventListener("mousedown",MouseDownCheck);
            InputRow.childNodes[i].childNodes[0].addEventListener("mouseup",MouseUpCheck);
            InputRow.childNodes[i].childNodes[0].setAttribute("data-inputcolumnindex",i);
        }
        NewTable.appendChild(InputRow);
        MainDiv.appendChild(NewTable);

        let SubmitButton = document.createElement("button");
        SubmitButton.type = "button";
        SubmitButton.innerHTML = "Add Entry";
        SubmitButton.classList.add("center");
        SubmitButton.addEventListener("click",SubmitRow);
        MainDiv.appendChild(SubmitButton);
        
        console.log("Funktionen Klar");
    }
    else
    {
        if(URLTableName == "" || URLTableName == "DBAdd")
        {
            for(var i = 0;i<AvailableTables.length;i++)
            {
                NewTable.appendChild(CreateTableRow(["<a href=\"/DBAdd/"+AvailableTables[i]+"\">"+AvailableTables[i]+"</a>"]));
            }
            MainDiv.appendChild(NewTable);
        }
        else
        {
            let ResultElement = document.getElementById("DBADD_Result");
            ResultElement.innerHTML = "Error: no table named "+URLTableName;
            ResultElement.style.color = "red";
        }
    }
}

DBAddMain();