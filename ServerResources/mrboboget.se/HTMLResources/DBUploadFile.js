async function DBUploadFileSubmit(event)
{
    //event.target.files[0].name filename
    //console.log("Skickar Formulär");
    //console.log(event);
    //let FilenameToUse = document.getElementById("FileToUploadName").value;
    //console.log(event.target.files[0].name);
    //fd.append('file[]', event.target.files[0], newName);
    let ResultParagraph = document.getElementById("DBUploadFile_Result");
    ResultParagraph.style.color = "orange";
    ResultParagraph.innerHTML = "Uploading file...";
    event.preventDefault();
    let FileToUpload = document.getElementById("FileToUpload").files[0];  // file from input
    let formData = new FormData();
    formData.append("File", FileToUpload,document.getElementById("FileToUploadName").value);
    let Result = await fetch('/UploadFile', {method: "POST", body: formData});
    Result = await Result.json();
    console.log(Result);
    if(Result.MBDBAPI_Status == "ok")
    {
        ResultParagraph.style.color = "green";
        ResultParagraph.innerHTML = "File successfully uploaded!";
    }
    else
    {
        ResultParagraph.style.color = "red";
        ResultParagraph.innerHTML = "Error uploading file: "+Result.MBDBAPI_Status;
    }
    return(Result);
}

document.getElementById("DBUploadFileForm").addEventListener("submit",DBUploadFileSubmit)
