'use strict';

/**
 * @ngdoc service
 * @name lauEditor.lauGameObject
 * @description
 * # lauGameObject
 * Service in the lauEditor.
 */
angular.module('lauEditor')
.service('lauGameObject', ['editCanvasManager', function ($editCanvas) {
  // AngularJS will instantiate a singleton by calling "new" on this function
  function GameObject(name, instanceId) {
    this.name = name ? name : 'unnamed';
    this.components = [];

    if(instanceId != undefined)
      this.instanceId = instanceId;
  }

  GameObject.prototype = {
    getComponentsById: function(id) {
      var components = [];
      for(var i = 0; i < this.components.length; ++i) {
        var comp = this.components[i];
        if(comp.flyweight.id == id)
          components.push(comp);
      }

      return components;
    },
    getComponentByInstanceId: function getComponentByInstanceId(id) {
      for(var i = 0; i < this.components.length; ++i) {
        var comp = this.components[i];
        if(comp.instanceId == id)
          return comp;
      }

      console.error("No component with instance id "+id+" found");
      return null;
    },
    updateStates: function(currentStates) {
      for(var j = 0; j < currentStates.components.length; ++j) {
        var srcComponent = currentStates.components[j];
        var dstComponent = this.getComponentByInstanceId(srcComponent.instanceId);
        dstComponent.setValues(srcComponent);
      }
    },
    removeScriptsByPath: function(scriptPath) {
      for(var c = this.components.length-1; c >= 0; --c) {
        var comp = this.components[c];
        if(comp.type=='script' && comp.flyweight.path == scriptPath) {
          this.components.splice(c, 1);
        }
      }
    },
    destroy: function() {
      for(var i = 0; i < this.components.length; ++i) {
        this.components[i].destroy();
      }
    }
  };

  return {
    GameObject: GameObject
  };
}]);
