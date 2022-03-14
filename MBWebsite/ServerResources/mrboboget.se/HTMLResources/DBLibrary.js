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

function MBDBAPI_EncodeArguments(ArgumentStringArray)
{
  let ReturnValue = "";
  const encoder = new TextEncoder();
  const Decoder = new TextDecoder();
  for(let i = 0; i < ArgumentStringArray.length;i++)
  {
    let ArgumentBytes = encoder.encode(ArgumentStringArray[i]);
    //console.log(ArgumentBytes);
    //console.log(Decoder.decode(ArgumentBytes));
    ReturnValue+=ArgumentBytes.length.toString()+" "+Decoder.decode(ArgumentBytes);
    if(i+1<ArgumentStringArray.length)
    {
      ReturnValue+=" ";
    }
  }
  //console.log(ReturnValue);
  return(ReturnValue);
}

async function MBDBAPI_SendDirective(DirectiveData)
{
  let ReturnValue = {};
  Result =  fetch("/DBGeneralAPI/", 
  {
   method: "POST", 
   body: JSON.stringify(DirectiveData),
   headers: {
 'Content-Type': 'application/octet-stream'
 // 'Content-Type': 'application/x-www-form-urlencoded',
},
}).then(res => res.json()).then(Data =>{
  //console.log(Data);
  //console.log(Data.TableNames);
  ReturnValue = Data;
  //console.log(ReturnValue);   
});
  await Result;
  return(ReturnValue)
}



function StringIsPath(StringToCheck)
{
  //console.log("Kommer till path grejen")
  if(typeof(StringToCheck) != typeof("Hej"))
  {
    return(false);
  }
  if(StringToCheck.length > 100)
  {
    return(false);
  }
  //console.log("ska kolla om inkluderar tecken")
  let ContainsSpace = (StringToCheck.indexOf(" ") != -1);
  let ContainsSlash = (StringToCheck.indexOf("/") != -1);
  let ContainsNewline = (StringToCheck.indexOf("\n") != -1);
  if(ContainsSpace || ContainsNewline || !ContainsSlash)
  {
    //console.log(ContainsSpace,ContainsSlash,ContainsNewline);
    return(false);
  }
  return(true);
}
function StringIsUrl(StringToCheck)
{
  if(!StringIsPath(StringToCheck))
  {
    return(false);
  }
  if(StringToCheck.substring(0,4) == "http")
  {
    return(true);
  }
  if(StringToCheck.substring(0,4) == "www.")
  {
    return(true);
  }
}

async function MBGetEmbeddedResource(ResourceToGet)
{
  let ReturnValue = {};
  Result =  fetch("/DBViewEmbedd/"+String(ResourceToGet), 
  {
   method: "GET", 
}).then(res => res.text()).then(Data =>{
  //console.log(Data);
  //console.log(Data.TableNames);
  ReturnValue = Data;
  //console.log(ReturnValue);   
});
  await Result;
  return(ReturnValue)
}

function GetPathStem(PathToProcess)
{
  let PathEntries = PathToProcess.split("/");
  for(let i = PathEntries.length-1; i >= 0;i--)
  {
    if(PathEntries[i] != "")
    {
      return(PathEntries[i]);
    }
  }
}

function toHexString(byteArray) {
  var s = '';
  byteArray.forEach(function(byte) {
    s += ('0' + (byte & 0xFF).toString(16)).slice(-2);
  });
  return s;
}

function htmlDecode(input) {
  var doc = new DOMParser().parseFromString(input, "text/html");
  //console.log("HTML Decode");
  return doc.documentElement.textContent;
}

var PictureExtension = ["png","jpg","webp"];
var VideoExtension = ["mp4","webm"]
var AudioExtension = ["mp3"]

function ValueInArray(ArrayToCheck,ValueToSearch)
{
  ReturnValue = false;
  for(let i = 0;i < ArrayToCheck.length; i++)
  {
    if(ArrayToCheck[i] == ValueToSearch)
    {
      ReturnValue = true;
      break;
    }
  }
  return(ReturnValue);
}
function GetURLResourceType(URLToCheck)
{
  let ReturnValue = "";
  let Extension = URLToCheck.lastIndexOf(".");
  if(Extension != -1)
  {
    Extension = URLToCheck.substr(Extension+1);
    //console.log("Extension is",Extension);  
    if(ValueInArray(PictureExtension,Extension))
    {
      ReturnValue = "Picture";
    }
    else if(ValueInArray(VideoExtension,Extension))
    {
      ReturnValue = "Video";
    }
    else if(ValueInArray(AudioExtension,Extension))
    {
      ReturnValue = "Audio"
    }
  }
  return(ReturnValue);
}
function GetEmbeddedVideoString(URLToEmbedd)
{
  let ReturnValue = "<video src=\""+URLToEmbedd+"\" style=\"width:auto;max-width:100%\"></img>"
  return(ReturnValue);
}
function GetEmbeddedPictureString(URLToEmbedd)
{
  let ReturnValue = "<img src=\""+URLToEmbedd+"\" style=\"width:auto;max-width:100%\"></img>"
  return(ReturnValue);
}
function GetEmbeddedAudioString(URLToEmbedd)
{
  let ReturnValue = "<audio src=\""+URLToEmbedd+"\" style=\"width:auto;max-width:100%\"></img>"
  return(ReturnValue);
}

function ShuffleArray(array) {
  var currentIndex = array.length,  randomIndex;

  // While there remain elements to shuffle...
  while (0 !== currentIndex) {

    // Pick a remaining element...
    randomIndex = Math.floor(Math.random() * currentIndex);
    currentIndex--;

    // And swap it with the current element.
    [array[currentIndex], array[randomIndex]] = [
      array[randomIndex], array[currentIndex]];
  }

  return array;
}