function EmbedResourceViewer()
{
    DBResource =  window.location.pathname;
    DBResource = DBResource.substring(DBResource.indexOf("DBView/")+7);
    var ElementToModify = document.getElementById("ViewDiv");
    fetch("/DBViewEmbedd"+"/"+DBResource, {method: "GET"}).then(res => res.text()).then(Data =>
    {
        ElementToModify.innerHTML = Data;
    });
    NewElements = ElementToModify.children;
    for(i = 0; i < NewElements.length;i++)
    {
        if (NewElements[i] instanceof HTMLScriptElement) 
        {
            eval(NewElement.body);
        }
    }
    console.log(NewElements.length);
}


document.addEventListener('DOMContentLoaded', EmbedResourceViewer, false);