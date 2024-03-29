
G_DBUpdate_TableInfo = {};
async function DBUpdate_Main()
{
    let MainDiv = document.getElementById("DBUpdateMainDiv");
    var URLPaths = window.location.pathname.split("/");
    var TableName = URLPaths[URLPaths.length-1];
    let NewTable = document.createElement("table");
    NewTable.style.width = "100%";
    let DBDirective = {Directive: "GetTableNames",DirectiveArguments: {}};
    let TableNamesResponse = await MBDBAPI_SendDirective(DBDirective);
    let ErrorElement = document.getElementById("DBStatus");
    console.log(TableNamesResponse);
    if(TableNamesResponse.MBDBAPI_Status != "ok")
    {
        ErrorElement.style.color = "red";
        ErrorElement.innerHTML = "Error in database: "+TableNamesResponse.MBDBAPI_Status;
        return;
    }
    if(TableName != "DBUpdate" && TableName != "")
    {
        //kollar om tablen existerar
        TableExists = false;
        for(let i = 0; i<TableNamesResponse.DirectiveResponse.TableNames.length;i++)
        {
            if(TableNamesResponse.DirectiveResponse.TableNames[i] == TableName)
            {
                TableExists = true;
                break;
            }
        }
        if(!TableExists)
        {
            ErrorElement.style.color = "red";
            ErrorElement.innerHTML = "Error: no table named \""+TableName+"\" exists";
            return;
        }
        //tablet existerar
        //NewTable.appendChild(CreateTableRow(TableNamesResponse.TableNames));
        let TableInfoResponse = await MBDBAPI_SendDirective({Directive: "GetTableInfo",DirectiveArguments: {TableName: TableName}});  
        G_DBUpdate_TableInfo = {TableInfo: TableInfoResponse.DirectiveResponse.ColumnInfo};
        console.log(TableInfoResponse);
        let ColumnNames = [];
        let ColumnTypes = [];
        for(let i = 0; i<TableInfoResponse.DirectiveResponse.ColumnInfo.length;i++)
        {
            ColumnNames.push(TableInfoResponse.DirectiveResponse.ColumnInfo[i].ColumnName);
            ColumnTypes.push(TableInfoResponse.DirectiveResponse.ColumnInfo[i].ColumnType);
        }
        ColumnNames.push("");
        ColumnTypes.push("");

        NewTable.appendChild(CreateTableRow(ColumnNames));
        NewTable.appendChild(CreateTableRow(ColumnTypes));
        //if(TableInfo[i].ColumnType == "int" && TableInfo[i].PrimaryKeyIndex == 1)
        //{
        //    InputFields.push("<div>AutoIncrement</div>");
        //}
    }
    else
    {
        for(let i = 0; i < TableNamesResponse.DirectiveResponse.TableNames.length;i++)
        {
            let NewTableName = TableNamesResponse.DirectiveResponse.TableNames[i];
            NewTable.appendChild(CreateTableRow([`<a href=/DBUpdate/${NewTableName}>${NewTableName}</a>`]));
        }
    }
    MainDiv.appendChild(NewTable);
    let ClearDiv = document.createElement("div");
    ClearDiv.style = "clear:both;display: block;";
    //let LineBreak = document.createElement("br");
    MainDiv.appendChild(ClearDiv);
    document.getElementById("SearchBox").addEventListener("keydown",DBUpdate_SendQuerry);
    console.log("Funktione slutar normalt");
}
async function DBUpdate_SendQuerry(e)
{
    var URLPaths = window.location.pathname.split("/");
    var TableName = URLPaths[URLPaths.length-1];
    if(e.key == "Enter")
    {
        let DBDirective = {Directive: "SearchTableWithWhere",DirectiveArguments: {TableName: TableName,Query: document.getElementById("SearchBox").value}};
        let DBResponse = await MBDBAPI_SendDirective(DBDirective);
        if(DBResponse.MBDBAPI_Status == "ok")
        {
            //tar bort förra tablen
            let OldDiv = document.getElementById("WhereSearchResult");
            if(OldDiv != null)
            {
                OldDiv.remove();
            }
            let NewTable = document.createElement("table");
            NewTable.style.width = "100%";
            NewTable.id = "UpdateTable";
            let NewDiv = document.createElement("div");
            NewDiv.id = "WhereSearchResult";
            for(let i = 0; i < DBResponse.DirectiveResponse.Rows.length;i++)
            {
                let NewRow = [];
                for(let j = 0; j<DBResponse.DirectiveResponse.Rows[i].length;j++)
                {
                    if(DBResponse.DirectiveResponse.Rows[i][j] != null)
                    {
                        NewRow.push(DBResponse.DirectiveResponse.Rows[i][j])
                    }
                    else
                    {
                        NewRow.push("null");
                    }
                    
                }
                let NewRowElement = CreateTableRow(NewRow);
                let UpdateButton = document.createElement("button");
                UpdateButton.innerHTML = "Update";
                UpdateButton.addEventListener("click",DBUpdate_UpdateRow);
                let NewCell = document.createElement("td");
                NewCell.appendChild(UpdateButton);
                NewRowElement.appendChild(NewCell);
                NewTable.appendChild(NewRowElement);
            }
            //let MainDiv = document.getElementById("DBUpdateMainDiv");
            NewDiv.appendChild(NewTable);
            document.getElementsByTagName("BODY")[0].appendChild(NewDiv);
        }
        else
        {
            let ErrorElement = document.getElementById("DBStatus");
            ErrorElement.style.color = "red";
            ErrorElement.innerHTML = "Error in database: "+DBResponse.MBDBAPI_Status;
        }
    }
}
DBUpdate_Main();

//Resize kod
function ResizeRow(e)
{
    var RowElement = e.target.parentElement.parentElement;
    var RowCells = RowElement.childNodes;
    for(var i = 0; i < RowCells.length;i++)
    {
        if(RowCells[i].childNodes[0].hasOwnProperty("style"))
        {
            RowCells[i].childNodes[0].style.height = e.target.style.height;
		}
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

async function DBUpdate_UpdateRow(e)
{
    console.log("Hej");
    let RowIndex = e.target.parentNode.parentNode.rowIndex;
    let MainTable = document.getElementById("UpdateTable");
    if(e.target.innerHTML == "Update")
    {
        e.target.innerHTML = "Hide";
        let InputTableRow = MainTable.insertRow(RowIndex+1);
        for(let i = 0; i < G_DBUpdate_TableInfo.TableInfo.length; i++)
        {
            let NewTableCell = document.createElement("td");
            if(G_DBUpdate_TableInfo.TableInfo[i].ColumnType == "int" && G_DBUpdate_TableInfo.TableInfo[i].PrimaryKeyIndex == 1)
            {
                NewTableCell.innerHTML = "Auto increment";
            }
            else
            {
                //"<textarea style=\"width: 100%;height:100%;resize:vertical\" data-InputField=\"True\"></textarea>"
                let NewInputElement = document.createElement("textarea");
                NewInputElement.style = "width: 100%;height:100%;resize:vertical"
                NewInputElement.setAttribute("data-inputfield",true);
                NewInputElement.addEventListener("mousedown",MouseDownCheck);
                NewInputElement.addEventListener("mouseup",MouseUpCheck);
                NewInputElement.setAttribute("data-inputcolumnindex",i);
                NewTableCell.appendChild(NewInputElement);
            }
            InputTableRow.appendChild(NewTableCell);
        }
        let NewTableCell = document.createElement("td");
        let SendButton = document.createElement("button");
        SendButton.addEventListener("click",DBUpdate_SendUpdate)
        SendButton.innerHTML = "Send Update";
        NewTableCell.appendChild(SendButton);
        InputTableRow.appendChild(NewTableCell);
    }
    else
    {
        e.target.innerHTML = "Update";
        MainTable.deleteRow(RowIndex+1);
        console.log("kommer hit");
    }
    //den som faktiskt insertar
}
async function DBUpdate_SendUpdate(e)
{
    let MainTable = document.getElementById("UpdateTable");
    let InputRow = e.target.parentNode.parentNode;
    let OldValuesRow = MainTable.rows[InputRow.rowIndex-1];

    let SubmissionFields = InputRow.querySelectorAll("[data-inputfield]");
    var URLPaths = window.location.pathname.split("/");
    var TableName = URLPaths[URLPaths.length-1];
    //console.log(SubmissionFields);
    let CurrentSubmissionfieldIndex = 0;
   
    let ColumnNames = [];
    let OldColumnValues = [];
    let NewColumnValues = []; 


    for(let i = 0;i < G_DBUpdate_TableInfo.TableInfo.length;i++)
    {
        let NewColumnName = G_DBUpdate_TableInfo.TableInfo[i].ColumnName;
        let OldValue = htmlDecode(OldValuesRow.cells[i].innerHTML);
        let NewValue = OldValue;
        if(SubmissionFields[CurrentSubmissionfieldIndex].getAttribute("data-inputcolumnindex") == i)
        {
            if(SubmissionFields[CurrentSubmissionfieldIndex].value != "")
            {
                NewValue = htmlDecode(SubmissionFields[CurrentSubmissionfieldIndex].value);
            }
            CurrentSubmissionfieldIndex+=1;
        }
        ColumnNames.push(NewColumnName);
        OldColumnValues.push(OldValue);
        NewColumnValues.push(NewValue);
    }
    let ResultElement = document.getElementById("DBStatus");
    ResultElement.innerHTML = "sending data..."
    ResultElement.style.color = "orange";
    let APIResult = await MBDBAPI_SendDirective({Directive: "UpdateTableRow",
                                                 DirectiveArguments: 
                                                 {TableName: TableName,ColumnNames: ColumnNames,OldColumnValues: OldColumnValues,NewColumnValues: NewColumnValues}});

    if(APIResult.MBDBAPI_Status == "ok")
    {
        ResultElement.innerHTML = "Successfully updated row!"
        ResultElement.style.color = "green";
        //uppdatera raderna som det är i databasen
        for(let i = 0; i < OldValuesRow.cells.length-1; i++)
        {
            OldValuesRow.cells[i].innerHTML = NewColumnValues[i];
        }
    }
    else
    {
        ResultElement.innerHTML = "Error updating row: "+APIResult.MBDBAPI_Status;
        ResultElement.style.color = "red";
    }
}