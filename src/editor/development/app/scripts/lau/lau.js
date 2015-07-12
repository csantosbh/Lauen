'use strict';
var LAU = {};

LAU.Utils = {
  clone: function(variable) {
    return JSON.parse(JSON.stringify(variable));
  }
}
