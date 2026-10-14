const CLAYDICT_KEY = "CLAYDICT";

var messageKeys = require('message_keys');

var Clay = require('@rebble/clay');
var clayConfig = require('./config.json');
var clay = new Clay(clayConfig, null, {autoHandleEvents: false});

var clayDict = null; // reference to saved settings
var selectedLocation = 0;

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
    function(e) {
        console.log('PebbleKit JS ready!');

        // Update s_js_ready on watch
        Pebble.sendAppMessage({'JSReady': 1});
    }
);

// Listen for appmessage
Pebble.addEventListener('appmessage', function(e) {
    var dict = e.payload;
    console.log("Got message: " + JSON.stringify(dict));

    if (clayDict == null)
    {
        clayDict = JSON.parse(localStorage.getItem(CLAYDICT_KEY));
    }

    // every time the watch wants fresh heading info it needs to re-send the selectedlocation //TODO implement on watch side
    if ('SelectedLocation' in dict)
    {
        selectedLocation = dict.SelectedLocation;
        switch (selectedLocation) 
        {
            case 0:
                console.log("Selected North.");
                break;
            case 1: 
                console.log("Selected " + clayDict[messageKeys.Location1Name] + " Lat: " + clayDict[messageKeys.Location1Lat] + " Long: " + clayDict[messageKeys.Location1Long]);
                GetAndSendHeadingInfo(clayDict[messageKeys.Location1Lat], clayDict[messageKeys.Location1Long]);
                break;
            case 2: 
                console.log("Selected " + clayDict[messageKeys.Location2Name] + " Lat: " + clayDict[messageKeys.Location2Lat] + " Long: " + clayDict[messageKeys.Location2Long]);
                GetAndSendHeadingInfo(clayDict[messageKeys.Location2Lat], clayDict[messageKeys.Location2Long]);
                break;
            case 3: 
                console.log("Selected " + clayDict[messageKeys.Location3Name] + " Lat: " + clayDict[messageKeys.Location3Lat] + " Long: " + clayDict[messageKeys.Location3Long]);
                GetAndSendHeadingInfo(clayDict[messageKeys.Location3Lat], clayDict[messageKeys.Location3Long]);
                break;
            case 4: 
                console.log("Selected " + clayDict[messageKeys.Location4Name] + " Lat: " + clayDict[messageKeys.Location4Lat] + " Long: " + clayDict[messageKeys.Location4Long]);
                GetAndSendHeadingInfo(clayDict[messageKeys.Location4Lat], clayDict[messageKeys.Location4Long]);
                break;
            case 5: 
                console.log("Selected " + clayDict[messageKeys.Location5Name] + " Lat: " + clayDict[messageKeys.Location5Lat] + " Long: " + clayDict[messageKeys.Location5Long]);
                GetAndSendHeadingInfo(clayDict[messageKeys.Location5Lat], clayDict[messageKeys.Location5Long]);
                break;
            default:
                console.log("Hit default");
                break;
        }
    }
});

// We need to implement this since we are overriding events in webviewclosed
Pebble.addEventListener('showConfiguration',
    function(e) {
        clay.config = clayConfig;
        clay.setSettings("PebbleID", Pebble.getWatchToken());
        Pebble.openURL(clay.generateUrl());
    }
);

// Listen for when web view is closed
Pebble.addEventListener('webviewclosed',
    function(e) {
        if (e && !e.response) { return; }

        clayDict = clay.getSettings(e.response);
        localStorage.setItem(CLAYDICT_KEY, JSON.stringify(clayDict));

        // only send what's necessary to watch
        Pebble.sendAppMessage(
            {
                'UseMetricSystem' : clayDict[messageKeys.UseMetricSystem],
                'Location1Name' : clayDict[messageKeys.Location1Name],
                'Location2Name' : clayDict[messageKeys.Location2Name],
                'Location3Name' : clayDict[messageKeys.Location3Name],
                'Location4Name' : clayDict[messageKeys.Location4Name],
                'Location5Name' : clayDict[messageKeys.Location5Name]
            }
        );
    }
);

function SendDictRetrying(dict)
{
    Pebble.sendAppMessage(dict,
    () => { console.log("Success"),
    () => { // on fail
        console.log("Retrying...");
        setTimeout(() => {
            SendDictRetrying(dict);
        }, 100);
    }});
}

// Calculate heading relative to true north and send it to watch
function GetAndSendHeadingInfo(lat, long) // TODO this must be requested by the watch by sending the selectedLocationindex every x time
{
    //TODO
    //1. get current position or use cached
    //2. get heading from current position to lat, long (= degrees relative to true north)
    //3. get distance in metric or imperial
    //4. send this angle and distance to watch using SendDictRetrying
}

// Get difference from magnetic to true north and send it to watch
function GetAndSendMagneticDeclination() //TODO this must be requested by the watch, perhaps once on app start (if it fails, just ignore it or fall back to persistant value on watch)
{
    //TODO https://randomapi.dev/api/magnetic-declination?lat=50.85&lng=4.35
}