'use strict';

var $event = (function() {
  function arrayHasElement(myArray, searchTerm) {
    for(var i = 0, len = myArray.length; i < len; i++) {
      if (myArray[i] === searchTerm) return i;
    }
    return -1;
  }

  var events = {};
  function EventSystem() { }
  EventSystem.prototype = {
    listen: function(eventName, callback) {
      if(!events.hasOwnProperty(eventName))
        events[eventName] = [];

      events[eventName].push(callback);
    },
    broadcast: function(eventName, eventData) {
      if(events.hasOwnProperty(eventName)) {
        for(var i=0; i < events[eventName].length; ++i) {
          events[eventName][i](eventData);
        }
      } else {
        console.log('warning: broadcast event ' + eventName + ' reached no listeners');
      }
    }
  };
  return new EventSystem();
})();
