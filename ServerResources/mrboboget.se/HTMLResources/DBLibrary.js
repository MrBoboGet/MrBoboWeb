function CreateTableRow(RowValues)
{
  var ReturnValue = document.createElement("tr");
  var RowSize = RowValues.length;
  for(var i = 0; i < RowSize;i++)
  {
    var NewRowValue = document.createElement("td");
    NewRowValue.style = "height:100%"
    NewRowValue.innerHTML = RowValues[i];
    ReturnValue.appendChild(NewRowValue); 
  }
  return(ReturnValue);
}
function CreateResizeableTableRow(RowValues)
{
  var ReturnValue = document.createElement("tr");
  var RowSize = RowValues.length;
  for(var i = 0; i < RowSize;i++)
  {
    var NewRowValue = document.createElement("td");
    var ResizeableDiv = document.createElement("div");
    ResizeableDiv.classList.add("ResizeableTableCell");
    ResizeableDiv.innerHTML = RowValues[i];
    NewRowValue.appendChild(ResizeableDiv);

    ReturnValue.appendChild(NewRowValue); 
  }
  return(ReturnValue);
}