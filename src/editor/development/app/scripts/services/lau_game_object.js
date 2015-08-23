'use strict';

/**
 * @ngdoc service
 * @name lauEditor.lauGameObject
 * @description
 * # lauGameObject
 * Service in the lauEditor.
 */
angular.module('lauEditor')
.service('lauGameObject', ['editCanvasManager', 'componentManager', function ($editCanvas, $cm) {
  var _gameObjIds = new Set();
  function _allocGameObjectId() {
    function genId() {
      return Math.pow(2,32)*Math.random();
    }

    let id = genId();
    while(_gameObjIds.has(id))
      id = genId();
    _gameObjIds.add(id);

    return id;
  }
  function _freeGameObjectId(id) {
    _gameObjIds.delete(id);
  }

  // AngularJS will instantiate a singleton by calling "new" on this function
  function GameObject(fields, uniqueNumericId) {
    this.name = fields.name;
    this.components = [];
    this.children = [];

    if(fields.components != undefined) {
      // Initialize components
      var comps = fields.components;
      for(var c = 0; c < comps.length; ++c) {
        var component = $cm.createComponentFromId(this, comps[c].id);
        if(component == null) {
          console.log('[warning] could not create component from id ' + comps[c].id);
          continue;
        }
        component.setValues(comps[c]);
        this.components.push(component);
      }
      // Initialize children game objects
      var child = fields.children;
      for(var g = 0; g < child.length; ++g) {
        this.children.push(new GameObject(child[g]));
      }
    }

    if(uniqueNumericId != undefined) {
      this.instanceId = uniqueNumericId;
      this._managedId = false;
    } else {
      this.instanceId = _allocGameObjectId();
      this._managedId = true;
    }
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
    getComponentsByType: function(type) {
      var components = [];
      for(var i = 0; i < this.components.length; ++i) {
        var comp = this.components[i];
        if(comp.flyweight.type == type)
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
    addChild: function(childGameObj) {
      this.children.push(childGameObj);
    },
    isParentOf: function(gameObj) {
      for(var g = 0; g < this.children.length; ++g) {
        if(this.children[g].instanceId == gameObj.instanceId) {
          return true;
        } else if(this.children[g].isParentOf(gameObj)) {
          return true;
        }
      }
      return false;
    },
    export: function() {
      // Export components
      var exportedComps = [];
      for(var c = 0; c < this.components.length; ++c) {
        exportedComps.push(this.components[c].export());
      }

      // Export children
      var children = [];
      for(var g = 0; g < this.children.length; ++g) {
        children.push(this.children[g].export());
      }

      return {
        name: this.name,
        components: exportedComps,
        children: children
      };
    },
    destroy: function() {
      for(var i = 0; i < this.components.length; ++i) {
        this.components[i].destroy();
      }
      for(var i = 0; i < this.children.length; ++i) {
        this.children[i].destroy();
      }

      if(this._managedId)
        _freeGameObjectId(this.instanceId);
    }
  };

  return {
    GameObject: GameObject
  };
}]);
