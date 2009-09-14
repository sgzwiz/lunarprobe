String.prototype.trim = function() { return this.replace(/^\s+|\s+$/, ''); };
var EmailFilter = /^([a-zA-Z0-9_\.\-])+\@(([a-zA-Z0-9\-])+\.)+([a-zA-Z0-9]{2,4})+$/;

// 
// Make an ajax request to a server and send the response to a call back
//
function MakeAjaxRequest(method, uri, callback, data, multipart, headers)
{
    var httpRequest = GetHttpRequest(method, uri, multipart, headers)
    if (httpRequest == null)
    {
        alert("You are not ajax enabled!  This is being fixed!")
        return false;
    }

    var handler = function(evt)
    {
        callback(evt.currentTarget);
    }

    // 
    // Called as we get a list of chunks to fill up on
    //
    if (multipart == true)
    {
        httpRequest.onload = handler;
    }
    else
    {
        httpRequest.onreadystatechange = handler;
    }

    SendHttpRequest(httpRequest, data);

    return true;
}

function SendHttpRequest(httpRequest, data)
{
    if (data != null)
    {
        httpRequest.setRequestHeader("Content-Length", data.len);
    }
    else
    {
        data = null;
    }

    httpRequest.send(data);
}

// 
// This gets the XMLHttpRequest object for us 
// in a platform independant way
//
function GetHttpRequest(method, uri, multipart, headers)
{
    var xmlHttp;
    try
    {
        // Firefox, Opera 8.0+, Safari
        xmlHttp=new XMLHttpRequest();
    }
    catch (e)
    {
        // Internet Explorer
        try
        {
            xmlHttp=new ActiveXObject("Msxml2.XMLHTTP");
        }
        catch (e)
        {
            try
            {
                xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
            }
            catch (e)
            {
                alert("Your browser does not support AJAX!");
                return null;
            }
        }
    }

    if (xmlHttp != null)
    {
        if (multipart == true)
            xmlHttp.multipart = true;
        xmlHttp.open(method, uri, true)
        if (headers != null)
        {
            for (var hdr in headers)
            {
                xmlHttp.setRequestHeader(hdr, headers[hdr]);
            }
        }
    }

    return xmlHttp;
}

// 
// Note: This is not browser independent
//
function ElementById(name)
{
    return document.getElementById(name);
}
