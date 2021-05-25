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
  var SQLRowsLength = SQLRows.length;
  //vi uppdtaerar tablen
  let ErrorMessage = document.getElementById("DBStatus");
  if(Data.MBDBAPI_Status != "ok")
  {
    ErrorMessage.style.color = "red";
    ErrorMessage.innerHTML = "Error in database: "+Data.MBDBAPI_Status;
    return;
  }
  else
  {
    ErrorMessage.innerHTML = "";
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
      if(ObjectKeys[j] == "ColumnCount")
      {
        continue;
      }
      let ColumnValue = SQLRows[i][ObjectKeys[j]];
      if(ColumnValue != null)
      {
        let ValueToAdd = ColumnValue;
        if(StringIsUrl(String(ColumnValue)))
        {
          //ValueToAdd = "<a href=\""+String(ValueToAdd+"\">"+String(ValueToAdd)+"</a>");
          console.log(String(ColumnValue),"Is a URL");
          let URLType = GetURLResourceType(String(ColumnValue));
          if(URLType == "Picture")
          {
            ValueToAdd = GetEmbeddedPictureString(String(ColumnValue));
          }
          else if(URLType == "Video")
          {
            ValueToAdd = GetEmbeddedVideoString(String(ColumnValue));
          }
          else if(URLType == "Audio")
          {
            ValueToAdd = GetEmbeddedAudioString(String(ColumnValue));
          }
          else
          {
            ValueToAdd = "<a href=\""+String(ValueToAdd+"\">"+String(ValueToAdd)+"</a>");
          }
        }
        else if(StringIsPath(String(ColumnValue)))
        {
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