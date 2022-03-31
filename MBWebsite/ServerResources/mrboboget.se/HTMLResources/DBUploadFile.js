var G_Uploading = false;
var G_CurrentRequest = 0;
function AddProgressElements()
{
    var ProgressDiv = document.getElementById("ProgressElementsDiv");
    var ProgressElement = document.createElement("progress");
    ProgressElement.id = "ProgressBar";
    ProgressElement.value = 0;
    ProgressElement.style = "width: 300px;";
    ProgressElement.max = 100;    
    ProgressDiv.appendChild(ProgressElement);

    var ByteCounterElement = document.createElement("p");
    ByteCounterElement.id = "ByteCounter";
    ProgressDiv.appendChild(ByteCounterElement);
}
function RemoveProgressElements()
{
    var ProgressDiv = document.getElementById("ProgressElementsDiv");
    ProgressDiv.innerHTML = "";
}
async function DBUploadFileSubmit(event)
{
    //event.target.files[0].name filename
    //console.log("Skickar Formulär");
    //console.log(event);
    //let FilenameToUse = document.getElementById("FileToUploadName").value;
    //console.log(event.target.files[0].name);
    //fd.append('file[]', event.target.files[0], newName);
    event.preventDefault();
    if(G_Uploading)
    {
        return;
    }
    var ServerFilename = document.getElementById("FileToUploadName").value;
    let ResultParagraph = document.getElementById("DBUploadFile_Result");
    
    if(ServerFilename == "")
    {
        ResultParagraph.innerHTML = "Invalid filename";
        ResultParagraph.style.color = "red";
        return;
    }
    let DirectiveToSend = {Directive: "FileExists",DirectiveArguments:{FilePath: ServerFilename}};
    let FileExistsResponse = await MBDBAPI_SendDirective(DirectiveToSend);
    if(FileExistsResponse.MBDBAPI_Status != "ok")
    {
        ResultParagraph.innerHTML = "Error: "+FileExistsResponse.MBDBAPI_Status;
        ResultParagraph.style.color = "red";
    }
    if(FileExistsResponse.DirectiveResponse.FileExists)
    {
        ResultParagraph.innerHTML = "File already exists";
        ResultParagraph.style.color = "red";
        return;
    }
    console.log(FileExistsResponse);
    if(!FileExistsResponse.DirectiveResponse.DirectoriesExists)
    {
        ResultParagraph.innerHTML = "No matching directories exists";
        ResultParagraph.style.color = "red";
        return;
    }
    G_Uploading = true;
    ResultParagraph.style.color = "orange";
    ResultParagraph.innerHTML = "Uploading file...";
    let FileToUpload = document.getElementById("FileToUpload").files[0];  // file from input
    let formData = new FormData();
    formData.append("File", FileToUpload,ServerFilename);

    var G_CurrentRequest = new XMLHttpRequest();
    G_CurrentRequest.open("POST", "/UploadFile");
    G_CurrentRequest.responseType = "json";
    G_CurrentRequest.upload.addEventListener("progress", progressHandler, false);
    G_CurrentRequest.addEventListener("load", completeHandler, false);
    G_CurrentRequest.addEventListener("error", errorHandler, false);
    G_CurrentRequest.addEventListener("abort", abortHandler, false);
    //G_CurrentRequest.onreadystatechange = UploadreadyStateChange;
    AddProgressElements();
    G_CurrentRequest.send(formData);


    //let Result = await fetch('/UploadFile', {method: "POST", body: formData});
    //Result = await Result.json();
}
//<progress id="ProgressBar" value="0" style="width: 300px;" max="100"></progress>
document.getElementById("DBUploadFileForm").addEventListener("submit",DBUploadFileSubmit)

var G_StatusElement = document.getElementById("DBUploadFile_Result");
function UploadreadyStateChange(event)
{
    console.log(event.target.readyState);
    if(event.target.readyState == 4)
    {
        if(!(event.target.status >=200 && event.target.status < 400))
        {
            //event.target.abort();
        }
    }
}
function progressHandler(event) 
{
    var ProgressBar = document.getElementById("ProgressBar");
    var ByteCounter = document.getElementById("ByteCounter");
    var percent = (event.loaded / event.total) * 100;
    ByteCounter.innerHTML = "Uploaded " + event.loaded + " bytes of " + event.total+" ("+Math.round(percent).toString()+"%)";
    ProgressBar.value = Math.round(percent);   
}
function completeHandler(event) 
{
    //G_StatusElement.innerHTML = event.target.responseText;
    G_Uploading = false;
    console.log(event.target);
    var Result = event.target.response;
    let ResultParagraph = document.getElementById("DBUploadFile_Result");
    if(Result == "ok")
    {
        ResultParagraph.style.color = "green";
        ResultParagraph.innerHTML = "File successfully uploaded!";
    }
    else
    {
        ResultParagraph.style.color = "red";
        ResultParagraph.innerHTML = "Error uploading file: "+Result.MBDBAPI_Status;
    }
    RemoveProgressElements();
}
  
function errorHandler(event) 
{
    G_Uploading = false;
    G_StatusElement.innerHTML = "Upload Failed unexpectedly";
    G_StatusElement.style.color = "red";
    RemoveProgressElements();
}
  
function abortHandler(event) 
{
    console.log("varför kommer vi hit?");
    G_Uploading = false;
    RemoveProgressElements();
    G_StatusElement.style.color = "red";
    G_StatusElement.innerHTML = "Upload Aborted";
}
