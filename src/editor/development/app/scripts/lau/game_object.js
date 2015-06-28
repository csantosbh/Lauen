'use strict';

LAU.GameObject = (function() {
  function GameObject(name, components, instanceId) {
    this.name = name ? name : 'unnamed';
    this.components = components ? components : [];

    if(instanceId != undefined)
      this.instanceId = instanceId;
  }

  return GameObject;
})();
