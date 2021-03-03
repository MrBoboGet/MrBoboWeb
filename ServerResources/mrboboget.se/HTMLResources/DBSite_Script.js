var wage = document.getElementById("SearchBox");
wage.addEventListener("keydown", function (e) {
if (e.keyCode === 13) 
{  //checks whether the pressed key is "Enter"
validate(e);
}
});

async function validate(e) 
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
}).then(res => res.json()).then(async Data =>{
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
      let ColumnValue = SQLRows[i][ObjectKeys[j]];
      if(ColumnValue != null)
      {
        let ValueToAdd = ColumnValue;
        if(StringIsUrl(String(ColumnValue)))
        {
          ValueToAdd = "<a href=\""+String(ValueToAdd+"\">"+String(ValueToAdd)+"</a>");
        }
        else if(StringIsPath(String(ColumnValue)))
        {
          console.log("Kommer hit");
          ValueToAdd = await MBGetEmbeddedResource(ValueToAdd);
        }
        TableValues.push(ValueToAdd);
      }
      else
      {
        TableValues.push("Null");
      }
    }
    TableToModify.appendChild(CreateTableRow(TableValues));
  }
});
}