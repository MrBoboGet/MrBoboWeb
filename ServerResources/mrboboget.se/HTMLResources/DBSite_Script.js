var wage = document.getElementById("SearchBox");
wage.addEventListener("keydown", function (e) {
if (e.keyCode === 13) 
{  //checks whether the pressed key is "Enter"
validate(e);
}
});

function CreateTableRow(RowValues)
{
  var ReturnValue = document.createElement("tr");
  var RowSize = RowValues.length;
  for(var i = 0; i < RowSize;i++)
  {
    var NewRowValue = document.createElement("td");
    NewRowValue.innerHTML = RowValues[i];
    ReturnValue.appendChild(NewRowValue); 
  }
  return(ReturnValue);
}
  function validate(e) 
{
//validation of the input...
     fetch("/DBSite", 
     {
      method: "POST", 
      body: document.getElementById("SearchBox").value,
      headers: {
    'Content-Type': 'text/plain'
    // 'Content-Type': 'application/x-www-form-urlencoded',
  },
}).then(res => res.json()).then(Data =>{
  console.log("Response is:",Data);  
  var SQLRows = Data.Rows;
  console.log("Request complete! response:", SQLRows);
  //vi uppdtaerar tablen
  var SQLRowsLength = SQLRows.length;
  if(SQLRowsLength == 0)
  {
    return;
  }
  console.log("SQLRowslength", SQLRowsLength  );
  var TableToModify = document.getElementById("SQLResultTable");
  TableToModify.innerHTML = "";
  for(var i = 0; i < SQLRowsLength;i++)
  {
    var ObjectKeys = Object.keys(SQLRows[i]);
    var TableValues = []
    for(j = 0;j < ObjectKeys.length;j++)
    {
      TableValues.push(SQLRows[i][ObjectKeys[j]]);
    }
    TableToModify.appendChild(CreateTableRow(TableValues));
    console.log("Appended row: ",TableValues,i,SQLRowsLength);
  }
});
}