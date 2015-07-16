'use strict';

LAU.GameObject = (function() {
  function GameObject(scope, name, components, instanceId) {
    this.name = name ? name : 'unnamed';
    this.components = components ? components : [];

    if(instanceId != undefined)
      this.instanceId = instanceId;

    scope.EditCanvas.trackGameObject(this);
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

      // TODO assert that this line will never be achieved
      return null;
    },
    updateStates: function(currentStates) {
      for(var j = 0; j < currentStates.components.length; ++j) {
        var srcComponent = currentStates.components[j];
        var dstComponent = this.getComponentByInstanceId(srcComponent.instanceId);
        dstComponent.setValues(srcComponent);
      }
    },
    // TODO having to pass the scope around sucks. Maybe this can be solved by
    // making everything as services.
    destroy: function(scope) {
      scope.EditCanvas.forgetGameObject(this);
    }
  };

  return GameObject;
})();
