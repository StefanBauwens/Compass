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

var prevDistance = 0;
var prevBearing = 0;

var xhrRequest = function (url, type, data, callback, errorCallback, timeout = 10000) {
  var xhr = new XMLHttpRequest();

  xhr.timeout = timeout; // in milliseconds (10s default)

  xhr.onload = function () {
    if (xhr.status >= 200 && xhr.status < 300) {
      callback(xhr.responseText);
    } else {
      if (errorCallback) {
        errorCallback(xhr.status, xhr.responseText);
      }
    }
  };

  xhr.onerror = function () {
    if (errorCallback) {
      errorCallback('network_error', null);
    }
  };

  xhr.ontimeout = function () {
    if (errorCallback) {
      errorCallback('timeout', null);
    }
  };

  xhr.open(type, url);

  // Set JSON header if sending data
  if (data) {
    xhr.setRequestHeader('Content-Type', 'application/json;charset=UTF-8');
    xhr.send(JSON.stringify(data));
  } else {
    xhr.send();
  }
};


function GetClayDict()
{
    if (clayDict == null)
    {
        clayDict = JSON.parse(localStorage.getItem(CLAYDICT_KEY));
    }

    return clayDict;
}

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

    GetClayDict(); // makes sure clayDict is populated

    if ('SelectedLocation' in dict)
    {
        selectedLocation = dict.SelectedLocation;

        // Clear the location watch and stop receiving updates
        navigator.geolocation.clearWatch(watchID); 

        switch (selectedLocation) 
        {
            case 0:
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
            // Subscribe to get location updates
            watchId = navigator.geolocation.watchPosition(
                (position) => { // on succesful callback (will be called periodically when phone position changes)
                    if (selectedLocation == 0)
                    {
                        navigator.geolocation.clearWatch(watchID); 
                    }
                    else
                    {
                        GetAndSendHeadingInfo(position.coords.latitude, position.coords.longitude, selectedLocationLatitude, selectedLocationLongitude);
                    }
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

    if ('Declination' in dict) // watch is requesting declination
    {
        GetAndSendMagneticDeclination();
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

function GetDistanceAndBearing(lat1, lon1, lat2, lon2) // Based on: https://www.movable-type.co.uk/scripts/latlong.html
{
    // Haversine formula
    const R = 6371e3; // earth radius in metres
    const lat1Radians = lat1 * Math.PI / 180;
    const lat2Radians = lat2 * Math.PI / 180;
    const deltaLatRadians = (lat2-lat1) * Math.PI / 180;
    const deltaLonRadians = (lon2-lon1) * Math.PI / 180;

    const a = Math.sin(deltaLatRadians/2) * Math.sin(deltaLatRadians/2) + Math.cos(lat1Radians) * Math.cos(lat2Radians) * Math.sin(deltaLonRadians/2) * Math.sin(deltaLonRadians/2);
    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));

    const distance = R * c; // distance in meters

    // bearing
    const y = Math.sin(deltaLonRadians) * Math.cos(lat2Radians);
    const x = Math.cos(lat1Radians)*Math.sin(lat2Radians) -
          Math.sin(lat1Radians)*Math.cos(lat2Radians)*Math.cos(deltaLonRadians);
    const bearingRadians = Math.atan2(y, x);
    const bearingDegrees = (bearingRadians * 180 / Math.PI + 360) % 360;

    return { distance: distance, bearing: bearingDegrees};
}

// Calculate heading relative to true north and send it to watch
function GetAndSendHeadingInfo(originLat, originLong, targetLat, targetLong)
{
    const distanceAndBearing = GetDistanceAndBearing(originLat, originLong, targetLat, targetLong);

    if (Math.abs(distanceAndBearing.distance - prevDistance) >=1 || Math.abs(distanceAndBearing.bearing - prevBearing) > 1)
    {
        prevDistance = distanceAndBearing.distance;
        prevBearing = distanceAndBearing.bearing;

        console.log("Distance And Bearing: " + JSON.stringify(distanceAndBearing));

        let dictToSend = {
            'DistanceToLocation': distanceAndBearing.distance,
            'BearingToLocation': distanceAndBearing.bearing
        }

        SendDictRetrying(dictToSend);
    }
}

// Get difference from magnetic to true north and send it to watch
function GetAndSendMagneticDeclination() 
{
    console.log("Watch requesting declination...");
    // get current position
    navigator.geolocation.getCurrentPosition((position) => 
        { // on succesful callback 
            const url = 'https://randomapi.dev/api/magnetic-declination?lat=' + position.coords.latitude + '&lng=' + position.coords.longitude;
            
            // try to get declination
            xhrRequest(url, 'GET', null, 
                (responseText) => { // callback on success
                    const responseObj = JSON.parse(responseText);
                    console.log("Declination deg: "  + responseObj.data.declinationDegrees);

                    // send to watch
                    let dictToSend = {
                        'Declination' : responseObj.data.declinationDegrees
                    }
                    SendDictRetrying(dictToSend);
                }, 
                (error, responseText) => { // callback on error
                    console.log("Error trying to fetch magnetic declination: " + error + " : " + responseText);
                }
            );
        },
        (error) => { // on error
            console.log('Location error when trying to get magnetic declination (' + err.code + '): ' + err.message);
        },
        { // options
            enableHighAccuracy: true,
            maximumAge: 10000,
            timeout: 10000
        }
    );
}