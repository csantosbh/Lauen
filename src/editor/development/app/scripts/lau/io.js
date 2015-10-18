'use strict';

LAU.IO = (function() {
  function getFileNameFromPath(name) {
    return name.substring(name.lastIndexOf('/')+1);
  }
  function getFileExtensionFromPath(name) {
    return name.substring(name.lastIndexOf('.')+1);
  }
  function getFileIconFromPath(name) {
    var extensionMappings = {
      prefab: 'prefab',
      hpp: 'script',
      lmf: 'model',
    };
    return 'images/icons/file-'+extensionMappings[LAU.IO.getFileExtensionFromPath(name)]+'.svg';
  }

  return {
    getFileNameFromPath: getFileNameFromPath,
    getFileExtensionFromPath: getFileExtensionFromPath,
    getFileIconFromPath: getFileIconFromPath,
  };
})();
