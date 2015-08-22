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

  var currentlySelectedGameObj = null;
  var gameObjects = [];
  var gameObjectManager = {
    getGameObjects: getGameObjects,
    selectGameObject: selectGameObject,
    selectedGameObject: selectedGameObject,
    pushGameObject: pushGameObject,
    removeGameObject: removeGameObject,
    addComponentToSelectedGameObject: addComponentToSelectedGameObject,
    getGameObject: getGameObject,
    serializeGameObjects: serializeGameObjects,
    removeScriptFromGameObjects: removeScriptFromGameObjects,
  };

  var _editorGameObjects; // Backup for the real gameobjects from the edit mode

  function getGameObjects() {
    return gameObjects;
  }

  function selectGameObject(gameObj) {
    currentlySelectedGameObj = gameObj;
  }
  function selectedGameObject() {
    return currentlySelectedGameObj;
  }
  function pushGameObject(go) {
    gameObjects.push(go);

    // TODO remove line below when the hierarchy panel is correctly created (with blur events to un-select game objects)
    currentlySelectedGameObj = gameObjects[gameObjects.length-1];
  }

  function removeGameObject(instanceId) {
    function _recurse(objs) {
      // Look for game object
      for(var i = 0; i < objs.length; ++i) {
        if(objs[i].instanceId == instanceId) {
          // Unselect game object, if it is selected
          if(currentlySelectedGameObj != null &&
             currentlySelectedGameObj.instanceId == instanceId) {
            currentlySelectedGameObj = null;
          }

          objs[i].destroy();
          objs.splice(i, 1);
          return true;
        } else if(_recurse(objs[i].children)) {
          return true;
        }
      }
      return false;
    }

    if(!_recurse(gameObjects)) {
      console.error("No game object of instance id ["+instanceId+"] to remove.");
    }
  }

  function addComponentToSelectedGameObject(component) {
    currentlySelectedGameObj.components.push(component);
  }

  function getGameObject(id) {
    function _recurse(objs) {
      // Look for game object
      for(var i = 0; i < objs.length; ++i) {
        if(objs[i].instanceId == instanceId) {
          return objs[i];
        } else if(objs[i].children.length > 0) {
          let go = _recurse(objs[i].children);
          if(go != null)
            return go;
        }
      }
      return null;
    }

    let go = _recurse(gameObjects);
    if(go == null) {
      console.error("Could not get game object of instance id ["+id+"].");
    }
    return go;
  }

  function serializeGameObjects() {
    var exportedObjs = [];
    for(var g = 0; g < gameObjects.length; ++g) {
      exportedObjs.push(gameObjects[g].export());
    }
    return exportedObjs;
  }

  function removeScriptFromGameObjects(scriptFlyweight) {
    for(var g = 0; g < gameObjects.length; ++g) {
      gameObjects[g].removeScriptsByPath(scriptFlyweight.path);
    }
  }

  ////
  // Internal functions
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
