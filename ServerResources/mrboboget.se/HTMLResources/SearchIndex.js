async function SearchIndex_Main()
{
    let IndexResult = await MBDBAPI_SendDirective("GetAvailableIndexes");
    if(IndexResult.MBDBAPI_Status != "ok")
    {
        let ErrorElement = document.getElementById("ErrorStatus");
        ErrorElement.style.color = "Red";
        ErrorElement.innerHTML = "Error: "+IndexResult.MBDBAPI_Status;
        return;
    }
    let SelectElement = document.getElementById("IndexToSearch");
    for(let i = 0; i < IndexResult.AvailableIndexes.length;i++)
    {
        let NewOption = document.createElement("option");
        NewOption.text = IndexResult.AvailableIndexes[i];
        SelectElement.add(NewOption);
    }
    document.getElementById("SearchButton").addEventListener("click",SearchIndex_GetSearchResult);
}
async function SearchIndex_GetSearchResult()
{
    console.log("Hej");
    let ErrorElement = document.getElementById("ErrorStatus");

    let IndexToSearchElement = document.getElementById("IndexToSearch");
    let IndexToSearch = IndexToSearchElement.value;
    let SearchTypeElement = document.getElementById("SearchTypeElement");
    let SearchType = SearchTypeElement.value;
    let SearchString = document.getElementById("SearchBox").value;
    let DirectiveArguments = [IndexToSearch,SearchType,SearchString];
    let SearchResult = await MBDBAPI_SendDirective("GetIndexSearchResult "+MBDBAPI_EncodeArguments(DirectiveArguments));
    if(SearchResult.MBDBAPI_Status != "ok")
    {
        ErrorElement.style.color = "Red";
        ErrorElement.innerHTML = "Error: "+SearchResult.MBDBAPI_Status;
    }
    else
    {
        console.log("börjar");
        ErrorElement.innerHTML = "";
        if(document.getElementById("SearchResult") != null)
        {
            document.getElementById("SearchResult").remove();
        }
        let ResultTable = document.createElement("table");
        ResultTable.id = "SearchResult";
        for(let i = 0; i < SearchResult.IndexSearchResult.length;i++)
        {
            let CurrentResult = SearchResult.IndexSearchResult[i];
            let StringToAdd = "";
            if(StringIsPath(CurrentResult))
            {
                StringToAdd = "<a href=\""+CurrentResult+"\">"+CurrentResult+"</a>"
            }
            else
            {
                StringToAdd = CurrentResult;
            }
            ResultTable.appendChild(CreateTableRow([StringToAdd]));
        }
        let ResultDiv = document.getElementById("ResultDiv");
        ResultDiv.appendChild(ResultTable);
        console.log("Klar");
    }
}
SearchIndex_Main();