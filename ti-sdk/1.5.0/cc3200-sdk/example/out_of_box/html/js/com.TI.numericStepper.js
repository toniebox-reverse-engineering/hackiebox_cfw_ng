/**
*  TI Numeric Stepper conrol
*  It's just an input box with left and right arrows on it to increment/decrement
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
(function( $ ){
	var _NAMESPACE_ = "TINumericStepper";
	
	// define some defaults at this level so that we can overwrite after it's been extended
	var defaults = {
			"defaultValue":		50,
			"max":				150,
			"min":	 			0,
			"refreshRate": 		50, 
			"autoIncAfter": 	3,	// will start auto increment after autoIncAfter * refreshRate
			"onUpdate": 		function(val){}
		};
		
	
	var methods = {
		/**
		 * Initialization method for mega flyouts
		 * @param {Object} options	Object containing all the options for configuring the Selection Tool
		 */
		init : function(options) {
			// extend the default fields
			var options = $.extend(defaults, options),
				touchEnabled = false;
			// after extending options remember the options as the new defaults
			defaults = options;

			// check for touch events
			try {
				touchEnabled = 'ontouchstart' in document;
			} catch (e) {
				// keep going
			}

			// build out each component (everything in here actually acts like our objects)
			return this.each(function(){
				
				// realized that we need to keep this out here
				var _target = $(this),
					_data = _target.data(_NAMESPACE_),
					_inputType = (touchEnabled) ? "number" : "text";
					_container = $('<form onSubmit="return false;"></form>'),
					_textInput = $('<input type="' + _inputType + '" value="'+options.defaultValue+'" />'),
					_leftBtn = $('<a class="TINSLeftBtn">&laquo;</a>'),
					_rightBtn = $('<a class="TINSRightBtn">&raquo;</a>'),
					_currentValue = options.defaultValue,
					_stepDirction = 1,	// 1 for up, -1 for down
					_beforeIncHolder = 0,
					_runInterval = false;
                
				// there's data, then the component has probably been initialized already
				// we probably don't need to continue with the rest of initialization
				if (_data) return;
				
				/* component initialization */
				// initialize the component
				_target.data(_NAMESPACE_, defaults);

				var INSTANCE_INTERVAL_ID = setInterval(function(){
						if (_runInterval) {
							_beforeIncHolder++;
							if (_beforeIncHolder > options.autoIncAfter) {
								_currentValue += _stepDirction;
								if (_currentValue >= options.min && _currentValue <= options.max) {
									_textInput.val(_currentValue);	
									options.onUpdate(_currentValue);
								}
							}
						}
					}, options.refreshRate);
				
				function build() {
					_container.append(_leftBtn)
							.append(_textInput)
							.append(_rightBtn);
					_target.append(_container);
				}

				function bindControls() {
					_textInput.focus(function(){
						// var val = _textInput.val();
					});
					
					_textInput.blur(function(){
						var val = _textInput.val();
						// make sure the data is whole number, if not, reset
						if (!val.match(/[0-9]+/)) {
							_textInput.val(_currentValue);	
						} else {
							options.onUpdate(val);
						}
					});
					
					_textInput.bind('keyup input', function(e){
						// TODO make sure keycode was numerid
						options.onUpdate(_textInput.val());
					});

					if (touchEnabled) {
						_leftBtn.on('touchstart', function(e){
							e.preventDefault();

							_stepDirction = -1;
							_beforeIncHolder = 0;
							_runInterval = true;

							var val = _textInput.val();
							val--;
							if (val >= defaults.min) {
								_currentValue = val;
								_textInput.val(_currentValue);
								options.onUpdate(_currentValue);
							}
						}).on('touchend', function(e){
							e.preventDefault();
							_runInterval = false;
						}).on('touchcancel', function(e){
							e.preventDefault();
							_runInterval = false;
						});
						_rightBtn.on('touchstart', function(e){
							e.preventDefault();

							_stepDirction = 1;
							_beforeIncHolder = 0;
							_runInterval = true;

							var val = _textInput.val();
							val++;
							if (val <= defaults.max) {
								_currentValue = val;
								_textInput.val(_currentValue);
								options.onUpdate(_currentValue);
							}
						}).on('touchend', function(e){
							e.preventDefault();
							_runInterval = false;
						}).on('touchcancel', function(e){
							e.preventDefault();
							_runInterval = false;
						});

						// skip the mousedown/mouseup stuff
						return;
					}

					_leftBtn.on('mousedown', function(e){
						_stepDirction = -1;
						_beforeIncHolder = 0;
						_runInterval = true;
					}).on('mouseup', function(e){
						_runInterval = false;
						var val = _textInput.val();
						val--;
						if (val >= defaults.min) {
							_currentValue = val;
							_textInput.val(_currentValue);
							options.onUpdate(_currentValue);
						}
					});
					_rightBtn.on('mousedown', function(e){
						_stepDirction = 1;
						_beforeIncHolder = 0;
						_runInterval = true;
					}).on('mouseup', function(e){
						_runInterval = false;
						var val = _textInput.val();
						val++;
						if (val <= defaults.max) {
							_currentValue = val;
							_textInput.val(_currentValue);
							options.onUpdate(_currentValue);
						}
					});
				}
				
				build();
				bindControls();
			});
		}, // end init
		destroy : function() {
			return this.each(function(){
				var $this = $(this),
					data = $this.data(_NAMESPACE_);
				data[_NAMESPACE_].remove();
				$this.removeData(_NAMESPACE_);
			});
		} // end destroy (not fully implemented yet)
	};

	$.fn.TINumericStepper = function(method) {
		if (methods[method]) {
			return methods[method].apply(this, Array.prototype.slice.call( arguments, 1 ));
		} else if ( typeof method === 'object' || ! method ) {
			return methods.init.apply(this, arguments);
		} else {
			$.error( 'Method ' +  method + ' does not exist on TINumericStepper' );
		}
	};
})( jQuery );