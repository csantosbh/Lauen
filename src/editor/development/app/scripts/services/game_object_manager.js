'use strict';

/**
 * @ngdoc service
 * @name lauEditor.gameObjectManager
 * @description
 * # gameObjectManager
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('gameObjectManager', ['historyManager', 'editCanvasManager', function ($hm, $editCanvas) {
  // AngularJS will instantiate a singleton by calling "new" on this function

  var currentlySelectedGameObj = null;
  var gameObjects = [];

  var editorGameObjects_; // Backup for the real gameobjects from the edit mode

  function getGameObjects() {
    return gameObjects;
  }

  function selectGameObject(gameObj) {
    if(gameObj != null) {
      let gameObjBefore = selectedGameObject();
      if($editCanvas.isEditMode()) {
        $hm.pushCommand({
          _selectedGameObjBefore: gameObjBefore==null?null:gameObjBefore.instanceId,
          _selectedGameObjAfter: gameObj==null?null:gameObj.instanceId,
          redo: function() {
            if(this._selectedGameObjAfter != null)
              currentlySelectedGameObj = getGameObject(this._selectedGameObjAfter);
          },
          undo: function() {
            if(this._selectedGameObjBefore != null)
              currentlySelectedGameObj = getGameObject(this._selectedGameObjBefore);
          },
          settings: {
            passthrough: true
          }
        });
      }
    }
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
  function popGameObject_(instanceId) {
    function recurse_(objs) {
      // Look for game object
      for(var i = 0; i < objs.length; ++i) {
        if(objs[i].instanceId == instanceId) {
          var obj = objs[i];
          objs.splice(i, 1);
          return obj;
        } else {
          var obj = recurse_(objs[i].children);
          if(obj != null)
            return obj;
        }
      }
      return null;
    }

    return recurse_(gameObjects);
  }

  function removeGameObject(instanceId) {
    var obj = popGameObject_(instanceId);

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
    if(popGameObject_(gameObj.instanceId)) {
      gameObj.setParent(destination);
      if(destination == null) {
        // Move gameObj to root node
        gameObjects.push(gameObj);
      }
    } else {
      console.error('Could not find game object ['+gameObj.name+'] parent.');
    }
  }

  function getGameObject(instanceId) {
    function recurse_(objs) {
      // Look for game object
      for(var i = 0; i < objs.length; ++i) {
        if(objs[i].instanceId == instanceId) {
          return objs[i];
        } else if(objs[i].children.length > 0) {
          let go = recurse_(objs[i].children);
          if(go != null)
            return go;
        }
      }
      return null;
    }
    function prefabRecurse_(prefs) {
      for(var i in prefs) {
        if(!prefs.hasOwnProperty(i))
          continue;

        if(prefs[i].gameObject.instanceId == instanceId) {
          return prefs[i].gameObject;
        } else if(prefs[i].gameObject.children.length > 0) {
          let go = recurse_(prefs[i].gameObject.children);
          if(go != null)
            return go;
        }
      }
      return null;
    }

    let go = recurse_(gameObjects);
    if(go == null) {
      go = prefabRecurse_(prefabManager.getPrefabs());
      if(go == null)
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
    function recurse_(objs) {
      // Look for game object
      for(var i = 0; i < objs.length; ++i) {
        if(objs[i].instanceId == instanceId) {
          return i;
        } else if(objs[i].children.length > 0) {
          let idx = recurse_(objs[i].children);
          if(idx >= 0)
            return idx;
        }
      }
      return -1;
    }
    return recurse_(gameObjects);
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

  function getInstancesOfPrefab(prefabId) {
    function recurse_(gameObjs) {
      let results = [];
      for(let i = 0; i < gameObjs.length; ++i) {
        if(gameObjs[i].parentPrefabId == prefabId) {
          results.push(gameObjs[i]);
        }
        results = results.concat(recurse_(gameObjs[i].children));
      }
      return results;
    }
    return recurse_(gameObjects);
  }

  /// Prefab Manager
  var prefabManager = (function (){
    // TODO remove prefab when it is deleted from filesystem
    var allPrefabs_ = {};
    function addPrefab(prefab) {
      allPrefabs_[prefab.instanceId] = prefab;
    }
    function destroyPrefab(prefabId) {
      // TODO beware of other scenes with instances of this prefab
      let instances = getInstancesOfPrefab(prefabId);

      for(let i = 0; i < instances.length; ++i) {
        instances[i].setPrefabParent(null);
      }

      freePrefabId(prefabId);
      delete allPrefabs_[prefabId];
    }
    var prefabIds_ = new Set();
    function allocPrefabId(requestedId) {
      function genId() {
        return Math.pow(2,32)*Math.random();
      }

      if(requestedId != undefined) {
        prefabIds_.add(requestedId);
        return requestedId;
      } else {
        let id = genId();
        while(prefabIds_.has(id))
          id = genId();
        prefabIds_.add(id);

        return id;
      }
    }
    function freePrefabId(id) {
      prefabIds_.delete(id);
    }

    function getPrefab(prefabId) {
      if(allPrefabs_.hasOwnProperty(prefabId))
        return allPrefabs_[prefabId];
      else {
        console.error('Could not find prefab of id ['+prefabId+']');
        return null;
      }
    }

    function prefabExists(prefabId) {
      return allPrefabs_.hasOwnProperty(prefabId);
    }

    function getPrefabs() {
      return allPrefabs_;
    }

    function serializePrefabs() {
      let serializedPrefabs = [];
      for(var i in allPrefabs_) {
        if(allPrefabs_.hasOwnProperty(i)) {
          serializedPrefabs.push(allPrefabs_[i].export());
        }
      }
      return serializedPrefabs;
    }

    return {
      addPrefab: addPrefab,
      destroyPrefab: destroyPrefab,
      allocPrefabId: allocPrefabId,
      prefabExists: prefabExists,
      getPrefab: getPrefab,
      getPrefabs: getPrefabs,
      serializePrefabs: serializePrefabs,
    };
  })();

  ////
  // Internal functions
  $event.listen('togglePreviewMode', function(isPreviewing) {
    currentlySelectedGameObj = null;
    if(isPreviewing) {
      editorGameObjects_ = gameObjects;
      gameObjects = [];
    } else {
      for(var i = 0; i < gameObjects.length; ++i) {
        gameObjects[i].destroy();
      }

      gameObjects = editorGameObjects_;
    }
  });

  return {
    getGameObjects: getGameObjects,
    selectGameObject: selectGameObject,
    selectedGameObject: selectedGameObject,
    pushGameObject: pushGameObject,
    moveGameObjectTo: moveGameObjectTo,
    removeGameObject: removeGameObject,
    getGameObject: getGameObject,
    serializeGameObjects: serializeGameObjects,
    removeScriptFromGameObjects: removeScriptFromGameObjects,
    getMenuPosition: getMenuPosition,
    setMenuPosition: setMenuPosition,
    getInstancesOfPrefab: getInstancesOfPrefab,
    prefabManager: prefabManager,
  };
}]);
