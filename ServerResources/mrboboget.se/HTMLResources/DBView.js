function EmbedResourceViewer()
{
    DBResource =  window.location.pathname;
    DBResource = DBResource.substring(DBResource.indexOf("DBView/")+7);
    var ElementToModify = document.getElementById("ViewDiv");
    fetch("/DBViewEmbedd"+"/"+DBResource, {method: "GET"}).then(res => res.text()).then(Data =>
    {
        console.log("Response is: ",Data);
        ElementToModify.innerHTML = Data;
    });
    console.log("Funktione kommer hit");
    NewElements = ElementToModify.children;
    for(i = 0; i < NewElements.length;i++)
    {
        console.log("NewElement Has children",i);
        if (NewElements[i] instanceof HTMLScriptElement) 
        {
            eval(NewElement.body);
        }
    }
    console.log(NewElements.length);
}


document.addEventListener('DOMContentLoaded', EmbedResourceViewer, false);