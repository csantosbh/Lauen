'use strict';

/**
 * @ngdoc service
 * @name lauEditor.lauPrefab
 * @description
 * # lauPrefab
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('lauPrefab', ['lauGameObject', 'gameObjectManager', function ($lgo, $gom) {
  function Prefab(serializedData) {
    let $this = this;
    this.gameObject = new $lgo.GameObject(serializedData.gameObject);
    this.instanceId = _allocPrefabId(serializedData.instanceId);
    this.name = serializedData.name;

    function makeObserver(i) {
      return function(changes) {
        let changedComponentId = $this.gameObject.components[i].flyweight.id;
        let gameObjs = $gom.getInstancesOfPrefab($this.instanceId);

        for(let i = 0; i < gameObjs.length; ++i) {
          gameObjs[i].syncComponentToPrefab(changedComponentId, changes);
        }
      }
    }

    let observers = {};
    function setupWatchCallback(watchFunc, gameObj) {
      for(let i = 0; i < gameObj.components.length; ++i) {
        if(!observers.hasOwnProperty(i))
          observers[i] = makeObserver(i);
        watchFunc(gameObj.components[i], observers[i], ['update']);
        if(gameObj.components[i].hasOwnProperty('fields')) {
          let fields = gameObj.components[i].fields;
          watchFunc(fields, observers[i]);
          for(let f in fields) {
            if(fields.hasOwnProperty(f) && (Array.isArray(fields[f]) || typeof(fields[f]) == 'object')) {
              watchFunc(fields[f], observers[i]);
            }
          }
        }
      }
    }

    this.destroy = function() {
      setupWatchCallback(Object.unobserve, this.gameObject);
      this.gameObject.destroy();
      delete _allPrefabs[this.instanceId];
    };

    setupWatchCallback(Object.observe, this.gameObject);
  }
  Prefab.prototype = {
    export: function() {
      return {
        gameObject: this.gameObject.export(),
        instanceId: this.instanceId,
        name: this.name,
      };
    }
  };

  function createPrefabFromGameObject(gameObjectId) {
    let go = $gom.getGameObject(gameObjectId);
    return createPrefabFromFlyweight({
      gameObject: go.export(),
      name: go.name,
    });
  }
  function createPrefabFromFlyweight(flyweight) {
    let newPrefab = new Prefab(flyweight);
    _allPrefabs[newPrefab.instanceId] = newPrefab;
    return newPrefab;
  }
  function getPrefab(prefabId) {
    if(_allPrefabs.hasOwnProperty(prefabId))
      return _allPrefabs[prefabId];
    else return null;
  }

  ///
  // Internal logic
  ///
  var _allPrefabs = {}; // TODO remove prefab from here when it is deleted from filesystem
  var _prefabIds = new Set();
  function _allocPrefabId(requestedId) {
    function genId() {
      return Math.pow(2,32)*Math.random();
    }

    if(requestedId != undefined) {
      _prefabIds.add(requestedId);
      return requestedId;
    } else {
      let id = genId();
      while(_prefabIds.has(id))
        id = genId();
      _prefabIds.add(id);

      return id;
    }
  }
  function _freePrefabId(id) {
    _prefabIds.delete(id);
  }

  //////
 
  return {
    createPrefabFromGameObject: createPrefabFromGameObject,
    createPrefabFromFlyweight: createPrefabFromFlyweight,
    getPrefab: getPrefab,
  };
}]);
