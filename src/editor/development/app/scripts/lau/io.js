'use strict';

LAU.IO = (function() {
  function getFileNameFromPath(name) {
    return name.substring(name.lastIndexOf('/')+1);
  }

  return {
    getFileNameFromPath: getFileNameFromPath,
  };
})();
