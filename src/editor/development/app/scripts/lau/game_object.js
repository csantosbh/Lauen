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
    // TODO I might have many equal components. Return them all. This is URGENT.
    getComponentById: function(id) {
      for(var i = 0; i < this.components.length; ++i) {
        var comp = this.components[i];
        if(comp.flyweight.id == id)
          return comp;
      }

      return null;
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
    destroy: function(scope) {
      scope.EditCanvas.forgetGameObject(this);
    }
  };

  return GameObject;
})();
