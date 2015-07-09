'use strict';

/**
 * @ngdoc service
 * @name lauEditor.gameObjectManager
 * @description
 * # gameObjectManager
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('gameObjectManager', function () {
  // AngularJS will instantiate a singleton by calling "new" on this function

  var currentGameObjectId = -1;
  var gameObjectManager = {
    gameObjects: [],
    selectGameObject: selectGameObject,
    selectedGameObject: selectedGameObject,
    pushGameObject: pushGameObject,
    removeGameObjectByInstanceId: removeGameObjectByInstanceId,
    addComponentToSelectedGameObject: addComponentToSelectedGameObject,
    getGameObjectByInstanceId: getGameObjectByInstanceId,
    serializeGameObjects: serializeGameObjects
  };

  var _editorGameObjects; // Backup for the real gameobjects from the edit mode

  function selectGameObject(i) {
    currentGameObjectId = i;
  }
  function selectedGameObject() {
    return currentGameObjectId;
  }
  function pushGameObject(go) {
    gameObjectManager.gameObjects.push(go);

    // TODO remove line below when the hierarchy panel is correctly created (with blur events to un-select game objects)
    currentGameObjectId = gameObjectManager.gameObjects.length-1;
  }

  function removeGameObjectByInstanceId(instanceId) {
    // Look for deleted game object
    var gameObjs = gameObjectManager.gameObjects;
    for(var j = 0; j < gameObjs.length; ++j) {
      if(gameObjs[j].instanceId == instanceId) {
        gameObjs.splice(j, 1);
        return;
      }
    }
  }

  function addComponentToSelectedGameObject(component) {
    gameObjectManager.gameObjects[currentGameObjectId].components.push(component);
  }

  function getGameObjectByInstanceId(id) {
    for(var i = 0; i < gameObjectManager.gameObjects.length; ++i) {
      if(gameObjectManager.gameObjects[i].instanceId == id)
        return gameObjectManager.gameObjects[i];
    }
    // TODO assert that this line will never be achieved
    return null;
  }

  function serializeGameObjects() {
    var exportedObjs = [];
    for(var g = 0; g < gameObjectManager.gameObjects.length; ++g) {
      var gameObjComps = gameObjectManager.gameObjects[g].components;
      var exportedComps = [];
      for(var c = 0; c < gameObjComps.length; ++c) {
        exportedComps.push(gameObjComps[c].export());
      }
      exportedObjs.push({
        name: gameObjectManager.gameObjects[g].name,
        components: exportedComps
      });
    }
    return exportedObjs;
  }

  $event.listen('togglePreviewMode', function(isPreviewing) {
    if(isPreviewing) {
      _editorGameObjects = gameObjectManager.gameObjects;
      gameObjectManager.gameObjects = [];
    } else {
      gameObjectManager.gameObjects = _editorGameObjects;
    }
  });

  return gameObjectManager;
});
