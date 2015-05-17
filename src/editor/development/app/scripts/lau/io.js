LAU.IO = (function() {
  // TODO receive this from the server
  var projectFolder = '/home/csantos/workspace/LauEngine/project_test';

  function getFileNameFromPath(name) {
    return name.substring(name.lastIndexOf('/')+1);
  }

  return {
    getFileNameFromPath: getFileNameFromPath,
  };
})();
