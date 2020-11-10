var wage = document.getElementById("SearchBox");
wage.addEventListener("keydown", function (e) {
if (e.keyCode === 13) {  //checks whether the pressed key is "Enter"
validate(e);
}
});

function validate(e) 
{
//validation of the input...
    window.location.href = "search="+document.getElementById("SearchBox").value.replace(" ","+");
}