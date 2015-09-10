'use strict';

/**
 * @ngdoc service
 * @name lauEditor.lauPrefab
 * @description
 * # lauPrefab
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('lauPrefab', ['lauGameObject', function ($lgo) {
  function Prefab(serializedData) {
    let $this = this;
    this.gameObject = new $lgo.GameObject(serializedData);

    function makeObserver(i) {
      return function(changes) {
        console.log('change id: '+$this.gameObject.components[i].flyweight.id);
      }
    }

    let observers = {};
    function setupWatchCallback(watchFunc, gameObj) {
      for(let i = 0; i < gameObj.components.length; ++i) {
        if(!observers.hasOwnProperty(i))
          observers[i] = makeObserver(i);
        watchFunc(gameObj.components[i], observers[i]);
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
    };

    setupWatchCallback(Object.observe, this.gameObject);
  }
  Prefab.prototype = {
  };

  return {
    Prefab: Prefab
  };
}]);
