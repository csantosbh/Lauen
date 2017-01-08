'use strict';
var LAU = {};

LAU.Utils = {
  clone: function(variable) {
    return JSON.parse(JSON.stringify(variable));
  },
  deepCopy: function(src, dst) {
    function recurse_(src_, dst_) {
      for(let i in src_) {
        if(src_.hasOwnProperty(i)) {
          if(typeof(src_[i])=='object') {
            if(!dst_.hasOwnProperty(i))
              dst_[i] = Array.isArray(src_[i]) ? [] : {};
            recurse_(src_[i], dst_[i]);
          }
          else
            dst_[i] = src_[i];
        }
      }
    }
    recurse_(src,dst);
  },
  notificationSystem: function() {
    let _changeSubscribers = {};

    function watchChanges(instanceId, watcherFunction) {
      if(!_changeSubscribers.hasOwnProperty(instanceId)) {
        _changeSubscribers[instanceId] = watcherFunction;
      } else {
        console.error('Object trying to watch changes multiple times');
      }
    }

    function notifySubscribers() {
      for(let i in _changeSubscribers) {
        _changeSubscribers[i]();
      }
    }

    return {
      watchChanges: watchChanges,
      notifySubscribers: notifySubscribers
    };
  }
}
