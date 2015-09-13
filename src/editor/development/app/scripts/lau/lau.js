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
  }
}
