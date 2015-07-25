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
  var gameObjects = [];
  var gameObjectManager = {
    getGameObjects: getGameObjects,
    selectGameObject: selectGameObject,
    selectedGameObject: selectedGameObject,
    pushGameObject: pushGameObject,
    removeGameObjectByIndex: removeGameObjectByIndex,
    removeGameObjectByInstanceId: removeGameObjectByInstanceId,
    addComponentToSelectedGameObject: addComponentToSelectedGameObject,
    getGameObjectByInstanceId: getGameObjectByInstanceId,
    serializeGameObjects: serializeGameObjects,
    removeScriptFromGameObjects: removeScriptFromGameObjects,
    updateScriptsFromFlyweight: updateScriptsFromFlyweight,
  };

  var _editorGameObjects; // Backup for the real gameobjects from the edit mode

  function getGameObjects() {
    return gameObjects;
  }

  function selectGameObject(i) {
    currentGameObjectId = i;
  }
  function selectedGameObject() {
    return currentGameObjectId;
  }
  function pushGameObject(go) {
    gameObjects.push(go);

    // TODO remove line below when the hierarchy panel is correctly created (with blur events to un-select game objects)
    currentGameObjectId = gameObjects.length-1;
  }

  function removeGameObjectByIndex(scope, idx) {
    // Look for deleted game object
    gameObjects[idx].destroy(scope);
    gameObjects.splice(idx, 1);
  }

  function removeGameObjectByInstanceId(scope, instanceId) {
    // Look for deleted game object
    var gameObjs = gameObjects;
    for(var j = 0; j < gameObjs.length; ++j) {
      if(gameObjs[j].instanceId == instanceId) {
        gameObjs[j].destroy(scope);
        gameObjs.splice(j, 1);
        return;
      }
    }
  }

  function addComponentToSelectedGameObject(component) {
    gameObjects[currentGameObjectId].components.push(component);
  }

  function getGameObjectByInstanceId(id) {
    for(var i = 0; i < gameObjects.length; ++i) {
      if(gameObjects[i].instanceId == id)
        return gameObjects[i];
    }
    // TODO assert that this line will never be achieved
    return null;
  }

  function serializeGameObjects() {
    var exportedObjs = [];
    for(var g = 0; g < gameObjects.length; ++g) {
      var gameObjComps = gameObjects[g].components;
      var exportedComps = [];
      for(var c = 0; c < gameObjComps.length; ++c) {
        exportedComps.push(gameObjComps[c].export());
      }
      exportedObjs.push({
        name: gameObjects[g].name,
        components: exportedComps
      });
    }
    return exportedObjs;
  }

  function removeScriptFromGameObjects(scriptFlyweight) {
    for(var g = 0; g < gameObjects.length; ++g) {
      var gameObj = gameObjects[g];
      // TODO implement component removal inside the GameObject class
      for(var c = gameObj.components.length-1; c >= 0; --c) {
        var comp = gameObj.components[c];
        if(comp.type=='script' && comp.flyweight.path == scriptFlyweight.path) {
          gameObj.components.splice(c, 1);
        }
      }
    }
  }

  function updateScriptsFromFlyweight(scriptFlyweight) {
    for(var g = 0; g < gameObjects.length; ++g) {
      var gameObj = gameObjects[g];
      for(var c = gameObj.components.length-1; c >= 0; --c) {
        var comp = gameObj.components[c];
        if(comp.type=='script' && comp.flyweight.path == scriptFlyweight.path) {
          // Backup original data
          var componentBackup = comp.export();
          // Update flyweight
          gameObj.components[c] = LAU.Components.createComponentFromFlyWeight(scriptFlyweight);
          // Restore data
          gameObj.components[c].setValues(componentBackup);
        }
      }
    }
  }

  $event.listen('togglePreviewMode', function(isPreviewing) {
    if(isPreviewing) {
      _editorGameObjects = gameObjects;
      gameObjects = [];
    } else {
      gameObjects = _editorGameObjects;
    }
  });

  return gameObjectManager;
});
