//var wage = document.getElementById("SearchBox");
//wage.addEventListener("keydown", function (e) {
//if (e.keyCode === 13) 
//{  //checks whether the pressed key is "Enter"
//validate(e);
//}
//});

let G_ResultMediaElements = [];
let G_CurrentMediaElementIndex = 0;
let G_ShuffledMediaElementsIndex = [];
let G_ShouldShuffle = false;
let G_CurrentPlayingElement = null;

function DBSite_ContinueMediaQueue(CurrentPlayingElement)
{
  let ElementToPlay = null;
  if(G_ShouldShuffle == false)
  {
    let ElementIndex = 0;
    if(CurrentPlayingElement != null)
    {
      for(let i = 0;i < G_ResultMediaElements.length;i++)
      {
        if(G_ResultMediaElements[i] == CurrentPlayingElement)
        {
          ElementIndex = i+1;
          break;
        }
      }
    }
    if(ElementIndex < G_ResultMediaElements.length)
    {
      ElementToPlay = G_ResultMediaElements[ElementIndex];
    }
  }
  else
  {
    if(G_ShuffledMediaElementsIndex.length > 0)
    {
      let ElementToPlayIndex = G_ShuffledMediaElementsIndex.pop();
      ElementToPlay = G_ResultMediaElements[ElementToPlayIndex];
    }
  }
  G_CurrentPlayingElement = ElementToPlay;
  if(ElementToPlay != null)
  {
    ElementToPlay.currentTime = 0;
    ElementToPlay.play();
  }
}

function DBSite_MediaFinishedCallback(e)
{
  DBSite_ContinueMediaQueue(e.target);
}
function DBSite_PlaybuttonCallback(e)
{
  if(G_CurrentPlayingElement != null)
  {
    if(G_CurrentPlayingElement.paused)
    {
      G_CurrentPlayingElement.play();
    }
    else
    {
      G_CurrentPlayingElement.pause();
    }
  }
}
function DBSite_SkipbuttonCallback(e)
{
  if(G_CurrentPlayingElement != null)
  {
    G_CurrentPlayingElement.pause();
  }
  DBSite_ContinueMediaQueue(G_CurrentPlayingElement);
}
function DBSite_ShuffleCheckboxCallback(e)
{
  console.log("Kommer hit");
  if(e.target.checked)
  {
    G_ShouldShuffle = true;
  }
  else
  {
    G_ShouldShuffle = false;
  }
}
function DBSite_CreateMediaElementsControl()
{
  let ElementsControl = document.createElement("div");
  let ElementControlsWidth = 200;
  let ElementControlsHeight = 50;
  let ElementControlStyleString = "position: fixed; border: 3px solid #00FF00; width: " +ElementControlsWidth.toString()+"px; height: " +ElementControlsHeight.toString()+"px; bottom: 0; right: 50%; background: #000000;";
  ElementsControl.style = ElementControlStyleString;
  console.log(ElementControlStyleString);
  let PlayButton = document.createElement("button");
  PlayButton.classList = "MBButton";
  PlayButton.style.position = "relative";
  PlayButton.style.height = "50%";
  PlayButton.style.marginLeft = "20px";
  PlayButton.style.width = (ElementControlsHeight/2).toString()+"px";
  //PlayButton.style.marginLeft = "-"+(ElementControlsHeight/4).toString()+"px";
  PlayButton.style.top = "50%";
  PlayButton.style.marginTop = "-7.5%";
  PlayButton.innerHTML = "&#x23F8";
  PlayButton.addEventListener("click",DBSite_PlaybuttonCallback);

  let SkipButton = document.createElement("button");
  SkipButton.classList = "MBButton";
  SkipButton.style.position = "relative";
  SkipButton.style.height = "50%";
  SkipButton.style.marginLeft = "20px";
  SkipButton.style.width = (ElementControlsHeight/2).toString()+"px";
  SkipButton.style.top = "50%";
  SkipButton.style.marginTop = "-7.5%";
  SkipButton.innerHTML = "&#x23ED";
  SkipButton.addEventListener("click",DBSite_SkipbuttonCallback);

  let ShuffleDiv = document.createElement("div");
  ShuffleDiv.style.marginLeft = (ElementControlsHeight/2*2+20*3).toString()+"px";
  let ShuffleCheckbox = document.createElement("input");
  ShuffleCheckbox.type = "checkbox";
  ShuffleCheckbox.id = "Shuffle";
  ShuffleCheckbox.addEventListener("change",DBSite_ShuffleCheckboxCallback);
  let ShuffleCheckboxLabel = document.createElement("label");
  ShuffleCheckboxLabel.for = "Shuffle";
  ShuffleCheckboxLabel.innerHTML = "Shuffle";

  ShuffleDiv.appendChild(ShuffleCheckbox);
  ShuffleDiv.appendChild(ShuffleCheckboxLabel);

  ElementsControl.appendChild(PlayButton);
  ElementsControl.appendChild(SkipButton);
  ElementsControl.appendChild(ShuffleDiv);
  //ElementsControl.appendChild(ShuffleCheckbox);
  //ElementsControl.appendChild(ShuffleCheckboxLabel);
  document.body.appendChild(ElementsControl);
}
function DBSite_CreateMediaElementPlaylist()
{
  G_ResultMediaElements = [];
  G_CurrentMediaElementIndex = 0;
  G_ShuffledMediaElementsIndex = [];
  let AllNodes = document.body.getElementsByTagName("*");
  for(let i = 0; i < AllNodes.length; i++)
  {
    if(AllNodes[i] instanceof HTMLMediaElement)
    {
      AllNodes[i].addEventListener('ended',DBSite_MediaFinishedCallback);
      G_ShuffledMediaElementsIndex.push(G_ResultMediaElements.length);
      G_ResultMediaElements.push(AllNodes[i]);
    }
  }
  if(G_ResultMediaElements.length != 0)
  {
    ShuffleArray(G_ShuffledMediaElementsIndex);
    DBSite_CreateMediaElementsControl();
  }
};

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
  DBSite_CreateMediaElementPlaylist();
});
}

function DBSite_UpdateSearchBar()
{
  let SearchBar = document.getElementById("SearchBox");
  if(SearchBar != null)
  {
    let SearchParams = new URLSearchParams(window.location.search);
    if(SearchParams.has("SQLQuerry"))
    {
      SearchBar.value = SearchParams.get("SQLQuerry");
    }
  }
}
DBSite_CreateMediaElementPlaylist();
DBSite_UpdateSearchBar();