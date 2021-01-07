/**
*  TI CC31xx Token Poller
*  Allows us to poll an HTML page and get tokens to refresh other values on page.
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
com.TI.tokenPoller = function(options) {
	var WIDGET_NAMESPACE = 'TITokenPoller',
		INSTANCE_INTERVAL_ID;
				
	// define some defaults at this level so that we can overwrite after it's been extended
	var defaults = {
			"paramPage": "somepage.html", 
			"refreshRate": 10 * 1000,
			"valueMap": [{
				"paramId": "",
				"outputSuccess": function(output) {},
				"outputDefault": function() {}
			}]
		},
		options = $.extend(defaults, options),
		invervalRunning = false;

	function intervalFunc() {
		$.ajax({
			"type": "GET",
			"url": options.paramPage,
			"cache": false,
			"dataType": "html"
		}).done(function(data, status, xhr) {
			var i = 0,
				len = options.valueMap.length;
			if (status != 'success') {
				for (i=0;i<len;i++) {
					options.valueMap[i].outputDefault();
				}
			} else {
				var dataPage = $(data);
				for (i=0;i<len;i++) {
					options.valueMap[i].outputSuccess(dataPage.filter('#'+options.valueMap[i].paramId).text());
				}	
			}
			invervalRunning = false;
		});
	}

	INSTANCE_INTERVAL_ID = setInterval(function(){
		if (!invervalRunning) {
			invervalRunning = true;
			intervalFunc();
		}
	}, options.refreshRate);

	this.destroy = function() {
		invervalRunning = false;
		clearInterval(INSTANCE_INTERVAL_ID);
	};
};