function DEBUG_MediaTracksMain()
{
    let MediaElement = document.getElementById("ViewDiv").childNodes[1];
    console.log(MediaElement);
    MediaElement.addEventListener("loadedmetadata",DEBUG_PrintMediaTracks)
}
function DEBUG_PrintMediaTracks(event)
{
    console.log("Metadata inladdat");
    let AudioTracks = event.target.audioTracks;
    console.log("Audio tracksen:");
    for(let i = 0; i < AudioTracks.length;i++)
    {
        console.log(AudioTracks[i]);
    }
    let TextTracks = event.target.textTracks;
    console.log(TextTracks);
    console.log("Subtitle Tracksen:");
    for(let i = 0;i < TextTracks.length;i++)
    {
        console.log(TextTracks[i]);
    }

}
DEBUG_MediaTracksMain();