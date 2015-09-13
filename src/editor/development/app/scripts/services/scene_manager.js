'use strict';

/**
 * @ngdoc service
 * @name lauEditor.sceneManager
 * @description
 * # sceneManager
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('sceneManager', ['gameObjectManager', function ($gom) {
  function saveScene() {
    let sceneData = {
      gameObjects: $gom.serializeGameObjects(),
      prefabs: $gom.prefabManager.serializePrefabs()
    };
    $rpc.call('save', sceneData, function(status) {
      if(!status) {
        console.error('Could not save scene!');
      }
      else
        console.log('Scene saved.');
    });
  }

  return {
    saveScene: saveScene
  };
}]);
