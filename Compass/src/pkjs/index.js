const CLAYDICT_KEY = "CLAYDICT";

var messageKeys = require('message_keys');

var Clay = require('@rebble/clay');
var clayConfig = require('./config.json');
var clay = new Clay(clayConfig, null, {autoHandleEvents: false});

var clayDict = null; // reference to saved settings
var selectedLocation = 0;
var selectedLocationName = "North";
var selectedLocationLatitude = 0;
var selectedLocationLongitude = 0;
var watchID;

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
                // Clear the location watch and stop receiving updates
                navigator.geolocation.clearWatch(watchID); 
                selectedLocationName = "North";
                selectedLocationLatitude = 0;
                selectedLocationLongitude = 0;
                break;
            case 1: 
                selectedLocationName = clayDict[messageKeys.Location1Name];
                selectedLocationLatitude = clayDict[messageKeys.Location1Lat];
                selectedLocationLongitude = clayDict[messageKeys.Location1Long];
                break;
            case 2: 
                selectedLocationName = clayDict[messageKeys.Location2Name];
                selectedLocationLatitude = clayDict[messageKeys.Location2Lat];
                selectedLocationLongitude = clayDict[messageKeys.Location2Long];
                break;
            case 3: 
                selectedLocationName = clayDict[messageKeys.Location3Name];
                selectedLocationLatitude = clayDict[messageKeys.Location3Lat];
                selectedLocationLongitude = clayDict[messageKeys.Location3Long];
                break;
            case 4: 
                selectedLocationName = clayDict[messageKeys.Location4Name];
                selectedLocationLatitude = clayDict[messageKeys.Location4Lat];
                selectedLocationLongitude = clayDict[messageKeys.Location4Long];
                break;
            case 5: 
                selectedLocationName = clayDict[messageKeys.Location5Name];
                selectedLocationLatitude = clayDict[messageKeys.Location5Lat];
                selectedLocationLongitude = clayDict[messageKeys.Location5Long];
                break;
            default:
                selectedLocationLatitude = 0;
                selectedLocationLongitude = 0;
                console.log("[ALERT] Hit default. Should not happen. SelectedLocation: " + selectedLocation);
                break;
        }

        if (selectedLocation != 0) // if not north
        {
            console.log("Selected " + selectedLocationName + " Lat: " + selectedLocationLatitude + " Long: " + selectedLocationLongitude);
            // Get location updates
            watchId = navigator.geolocation.watchPosition(
                (position) => { // on succesful callback (will be called periodically when phone position changes)
                    GetAndSendHeadingInfo(position.coords.latitude, position.coords.longitude, selectedLocationLatitude, selectedLocationLongitude);
                },
                (error) => { // on error
                    console.log('location error (' + err.code + '): ' + err.message);
                },
                { // options
                    enableHighAccuracy: true,
                    maximumAge: 0,
                    timeout: 5000
                }
            );     
        }
        else // if north
        {
            console.log("Selected North.");
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

// Callback when position changed
function success(pos) {
  console.log('Location changed!');
  console.log('lat= ' + pos.coords.latitude + ' lon= ' + pos.coords.longitude);
}

function error(err) {
  console.log('location error (' + err.code + '): ' + err.message);
}

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
function GetAndSendHeadingInfo(originLat, originLong, targetLat, targetLong) // TODO this must be requested by the watch by sending the selectedLocationindex every x time OR use geolocation watchposition?
{
    //TODO
    //1. get current position or use cached
    //2. calculate bearing from current position to lat, long (= degrees relative to true north)
    //3. get distance in metric or imperial
    //4. send this angle and distance to watch using SendDictRetrying
}

// Get difference from magnetic to true north and send it to watch
function GetAndSendMagneticDeclination() //TODO this must be requested by the watch, perhaps once on app start (if it fails, just ignore it or fall back to persistant value on watch)
{
    //TODO https://randomapi.dev/api/magnetic-declination?lat=50.85&lng=4.35
}