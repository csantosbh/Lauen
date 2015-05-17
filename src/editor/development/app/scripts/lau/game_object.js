'use strict';

LAU.GameObject = (function() {
  function GameObject(name, components) {
    this.name = name ? name : 'unnamed';
    this.components = components ? components : [];
  }

  return GameObject;
})();
