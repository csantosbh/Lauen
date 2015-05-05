var $event = (function() {
  var events = {};
  var pendingBroadcasts = {};
  function EventSystem() { }
  EventSystem.prototype = {
    listen: function(eventName, callback) {
      if(!events.hasOwnProperty(eventName))
        events[eventName] = [];

      events[eventName].push(callback);

      if(pendingBroadcasts.hasOwnProperty(eventName)) {
        for(var i = 0; i < pendingBroadcasts[eventName].length; ++i) {
          callback(pendingBroadcasts[eventName][i]);
        }
        delete pendingBroadcasts[eventName];
      }
    },
    broadcast: function(eventName, eventData) {
      if(!events.hasOwnProperty(eventName)) {
        if(!pendingBroadcasts.hasOwnProperty(eventName))
          pendingBroadcasts[eventName] = [];

        // TODO: Remove old pending broadcasts!
        pendingBroadcasts[eventName].push(eventData);
      } else {
        for(var i=0; i < events[eventName].length; ++i) {
          events[eventName][i](eventData);
        }
      }
    }
  };
  return new EventSystem();
})();
