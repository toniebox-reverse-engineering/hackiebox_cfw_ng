/**
* General page controls for demos and tabs...
*
* Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
* 
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*  
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*  
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
if(!window.com) com = new Object();
if(!com.TI) com.TI = new Object();
com.TI.toggleLED = function(whichOne) {
    var LEDnum = "1",
        params = "LED"+LEDnum;
    params += whichOne;
    $.post("No_content", {"__SL_P_ULD": params});
}

$(document).ready(function(){ 

	var filename = window.location.href.substr(window.location.href.lastIndexOf("/")+1);
	if (filename.indexOf('overview') > -1) {
		$("#overview").toggleClass("active");
	} else if (filename.indexOf('about') > -1) {
		$("#about").toggleClass("active");
	} else if (filename.indexOf('setup') > -1) {
		$("#setup").toggleClass("active");
	} else if (filename.indexOf('portal') > -1) {
		$("#portal").toggleClass("active");
	} else if (filename.indexOf('demos') > -1) {
		$("#demos").toggleClass("active");
	}


	// session storage for online status (keep from showing offline during page refreshes)
	var ONLINE_STATUS_KEY = 'ONLINE_STATUS_KEY';
	if ($.browser.msie) {
		// use cookies (IE still honours session cookise)
		com.TI.tempVarKeeper = {
			"get": function(key) {
				var name = key + "=";
				var ca = document.cookie.split(';');
				for(var i=0; i<ca.length; i++) {
					var c = $.trim(ca[i]);
					if (c.indexOf(name)==0) return c.substring(name.length,c.length);
				}
				return "";
			},
			"set": function(key, val) {
				document.cookie = key + "=" + val + ";";
			} 
		};
	} else if (typeof(Storage)!=="undefined") {
		// use session storage
		com.TI.tempVarKeeper = {
			"get": function(key) {
				if (sessionStorage.hasOwnProperty(key)) return sessionStorage[key];
				return '';
			},
			"set": function(key, val) {
				sessionStorage[key] = val;
			} 
		};
	} else {
		com.TI.tempVarKeeper = {
			"get": function(key) { return ''; },
			"set": function(key, val) {} 
		};
	}

	// initialize online status polling
	var statusBar = $('div.header div.status-bar'),
		onlineStatus = new com.TI.tokenPoller({
			"paramPage": "param_online.html", 
			"refreshRate": 5 * 1000,	// every 5 seconds
			"valueMap": [{
				"paramId": "online_status",
				"outputSuccess": function(output) {
					if (output == "1") {
						statusBar.html(" Current Status: <em>Online</em>");
						statusBar.addClass('online');
						com.TI.tempVarKeeper.set(ONLINE_STATUS_KEY,output);
					} else {
						statusBar.html(" Current Status: <em>Offline</em>");
						statusBar.removeClass('online');
						com.TI.tempVarKeeper.set(ONLINE_STATUS_KEY,output);
					}
				},
				"outputDefault": function() {
					statusBar.html(" Current Status: <em>Offline</em>");
				}
			}]
		});
	if (com.TI.tempVarKeeper.get(ONLINE_STATUS_KEY) == "1") {
		statusBar.html(" Current Status: <em>Online</em>");
		statusBar.addClass('online');
		com.TI.tempVarKeeper.set(ONLINE_STATUS_KEY,"1");
	} else {
		statusBar.html(" Current Status: <em>Offline</em>");
		statusBar.removeClass('online');
		com.TI.tempVarKeeper.set(ONLINE_STATUS_KEY,"");
	}

    // turn off the LED in case the user came from another demo
    com.TI.toggleLED('_OFF');
});