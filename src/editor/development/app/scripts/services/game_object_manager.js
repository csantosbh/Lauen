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
    moveGameObjectTo: moveGameObjectTo,
    removeGameObject: removeGameObject,
    addComponentToSelectedGameObject: addComponentToSelectedGameObject,
    getGameObject: getGameObject,
    serializeGameObjects: serializeGameObjects,
    removeScriptFromGameObjects: removeScriptFromGameObjects,
    getMenuPosition: getMenuPosition,
    setMenuPosition: setMenuPosition,
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

  // Removes `gameObj` from whatever container it is
  function _popGameObject(instanceId) {
    function _recurse(objs) {
      // Look for game object
      for(var i = 0; i < objs.length; ++i) {
        if(objs[i].instanceId == instanceId) {
          var obj = objs[i];
          objs.splice(i, 1);
          return obj;
        } else {
          var obj = _recurse(objs[i].children);
          if(obj != null)
            return obj;
        }
      }
      return null;
    }

    return _recurse(gameObjects);
  }

  function removeGameObject(instanceId) {
    var obj = _popGameObject(instanceId);

    if(obj == null) {
      console.error("No game object of instance id ["+instanceId+"] to remove.");
    } else {
      // Unselect game object, if it is selected
      if(currentlySelectedGameObj != null &&
         currentlySelectedGameObj.instanceId == instanceId) {
        currentlySelectedGameObj = null;
      }

      obj.destroy();
    }
  }

  // Makes `gameObj` child of `destination`
  function moveGameObjectTo(gameObj, destination) {
    // First, remove `gameObj` from whatever array it is
    if(_popGameObject(gameObj.instanceId)) {
      gameObj.setParent(destination);
      if(destination == null) {
        // Move gameObj to root node
        gameObjects.push(gameObj);
      }
    } else {
      console.error('Could not find game object ['+gameObj.name+'] parent.');
    }
  }

  function addComponentToSelectedGameObject(component) {
    currentlySelectedGameObj.components.push(component);
  }

  function getGameObject(instanceId) {
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
      console.error("Could not get game object of instance id ["+instanceId+"].");
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

  // Returns the position of the game object inside its menu
  function getMenuPosition(instanceId) {
    function _recurse(objs) {
      // Look for game object
      for(var i = 0; i < objs.length; ++i) {
        if(objs[i].instanceId == instanceId) {
          return i;
        } else if(objs[i].children.length > 0) {
          let idx = _recurse(objs[i].children);
          if(idx >= 0)
            return idx;
        }
      }
      return -1;
    }
    return _recurse(gameObjects);
  }

  function setMenuPosition(gameObject, newPos) {
    var parent = gameObject.parent;
    var parentChildren;
    if(parent == null)
      parentChildren = gameObjects;
    else
      parentChildren = parent.children;

    var oldPos = 0;
    while(oldPos < parentChildren.length &&
          parentChildren[oldPos].instanceId != gameObject.instanceId)
      oldPos++;

    parentChildren.splice(oldPos, 1);
    parentChildren.splice(newPos, 0, gameObject);
  }

  ////
  // Internal functions
  $event.listen('togglePreviewMode', function(isPreviewing) {
    currentlySelectedGameObj = null;
    if(isPreviewing) {
      _editorGameObjects = gameObjects;
      gameObjects = [];
    } else {
      for(var i = 0; i < gameObjects.length; ++i) {
        gameObjects[i].destroy();
      }

      gameObjects = _editorGameObjects;
    }
  });

  return gameObjectManager;
});
