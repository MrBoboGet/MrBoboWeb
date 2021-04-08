function DBViewMediaMain()
{
    //let MediaElement = document.getElementById("ViewDiv").childNodes[1];
    console.log("Ska skapa en MediaElement")
    let MediaElementID = document.getElementById("ViewDiv").childNodes[1].id;
    var player = new MediaElement(MediaElementID, {
		success: function(mediaElement, originalNode) {
			// do things
            console.log("Lyckades Skapa MediaElement");
		}
	});
}
DBViewMediaMain();