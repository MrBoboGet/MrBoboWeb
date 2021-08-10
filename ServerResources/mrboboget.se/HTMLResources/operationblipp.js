
async function operationblipp_submit(event)
{
    event.preventDefault();
}


function operationblipp_main()
{
    let FormElement = document.getElementById("BlippUpdate");
    FormElement.addEventListener("submit",operationblipp_submit);
}