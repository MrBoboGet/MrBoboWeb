function DBLogin_Main()
{
    let PasswordField = document.getElementById("Password");
    let UsernameField = document.getElementById("Username");
    UsernameField.addEventListener("keydown",DBLogin_SendLoginInfo);
    PasswordField.addEventListener("keydown",DBLogin_SendLoginInfo);
}
async function DBLogin_SendLoginInfo(e)
{
    if(e.key == "Enter")
    {
        let PasswordField = document.getElementById("Password");
        let UsernameField = document.getElementById("Username");
        const encoder = new TextEncoder();
        const Decoder = new TextDecoder();
        const data = encoder.encode(PasswordField.value);
        const hash = await crypto.subtle.digest('SHA-256', data);
        console.log(hash);
        let PasswordHash = toHexString(new Uint8Array(hash));
        console.log(PasswordHash);
        //let DBDirective = "Login "+MBDBAPI_EncodeArguments([UsernameField.value,PasswordHash]);
        let DBDirective = {};
        DBDirective.Directive = "Login";
        DBDirective.DirectiveArguments = {Username: UsernameField.value,Password: PasswordHash};
        let DBResponse = await MBDBAPI_SendDirective(DBDirective);
        if(DBResponse.MBDBAPI_Status == "ok")
        {
            location.reload();
        }
        else
        {
            let ErrorParagraph = document.getElementById("DBStatus");
            ErrorParagraph.style.color = "red";
            ErrorParagraph.innerHTML = "Error: "+DBResponse.MBDBAPI_Status;
        }
    }
}
DBLogin_Main();
