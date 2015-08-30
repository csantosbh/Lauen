'use strict';

/**
 * @ngdoc service
 * @name lauEditor.lauGameObject
 * @description
 * # lauGameObject
 * Service in the lauEditor.
 */
angular.module('lauEditor')
.service('lauGameObject', ['editCanvasManager', 'componentManager', 'historyManager', 'gameObjectManager', function ($editCanvas, $cm, $hm, $gom) {
  ///
  // Public GameObject API
  ///
  function GameObject(fields, uniqueNumericId) {
    this.name = fields.name;
    this.components = [];
    this.children = [];
    this.transform = new Transform(this);
    this.parent = null;

    if(fields.transform != undefined) {
      this.transform.setValues(fields.transform);
    }

    if(fields.components != undefined) {
      // Initialize components
      var comps = fields.components;
      for(var c = 0; c < comps.length; ++c) {
        var component = $cm.createComponentFromId(this, comps[c].id, comps[c].instanceId);
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
        var newChild = new GameObject(child[g], child[g].instanceId);
        newChild.setParent(this);
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
      this.transform.setValues(currentStates.transform);
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
    _addChild: function(childGameObj) {
      this.children.push(childGameObj);
    },
    setParent: function(parentGameObj) {
      if(parentGameObj != null)
        parentGameObj._addChild(this);

      if($editCanvas.isEditMode()) {
        this.transform.setHierarchyParent(parentGameObj);
      }

      this.parent = parentGameObj;
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
        instanceId: this.instanceId,
        transform: this.transform.export(),
        components: exportedComps,
        children: children
      };
    },
    destroy: function() {
      this.transform.destroy();

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

  ///
  // Internal logic
  ///
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

  // Transform logic
  function Transform(gameObject) {
    this.position = [0,0,0];
    this.rotation = [0,0,0];
    this.scale = [1,1,1];
    this.type = "transform";
    this.ownerGameObject = gameObject;

    if($editCanvas.isEditMode()) {
      ////
      // Bind transform to edit canvas
      this.hierarchyGroup = $editCanvas.createGroup();
      this.hierarchyGroup.add($editCanvas.createBoundingBox());
      this._parentGroup = $editCanvas.scene;

      $editCanvas.scene.add(this.hierarchyGroup);

      var $this = this;
      function updatePosition(newValue) {
        if(newValue != null)
          $this.hierarchyGroup.position.fromArray(newValue);
      }
      function positionObserver(changes) {
        var newValue = changes[changes.length-1].object;
        updatePosition(newValue);
      }
      function updateRotation(newValue) {
        if(newValue != null)
          $this.hierarchyGroup.rotation.fromArray(newValue);
      }
      function rotationObserver(changes) {
        var newValue = changes[changes.length-1].object;
        updateRotation(newValue);
      }
      function updateScale(newValue) {
        if(newValue != null)
          $this.hierarchyGroup.scale.fromArray(newValue);
      }
      function scaleObserver(changes) {
        var newValue = changes[changes.length-1].object;
        updateScale(newValue);
      }
      Object.observe(this, function(changes) {
        for(var i = 0; i < changes.length; ++i) {
          var cng = changes[i];
          if(cng.name == "position" && cng.type=="update") {
            updatePosition($this.position);
            Object.observe($this.position, positionObserver);
          }
          else if(cng.name == "rotation" && cng.type=="update") {
            updateRotation($this.rotation);
            Object.observe($this.rotation, rotationObserver);
          }
          else if(cng.name == "scale" && cng.type=="update") {
            updateScale($this.scale);
            Object.observe($this.scale, scaleObserver);
          }
        }
      });

      this._editorCommitCallback = function(field) {
        return function(oldValue, newValue) {
          $hm.pushCommand({
            _before: oldValue,
            _after: newValue,
            _gameObj: $this.ownerGameObject.instanceId,
            undo: function() {
              var gameObj = $gom.getGameObject(this._gameObj);
              gameObj.transform[field] = this._before;
            },
            redo: function() {
              var gameObj = $gom.getGameObject(this._gameObj);
              gameObj.transform[field] = this._after;
            }
          });
        };
      };
    }
  }
  Transform.prototype = {
    export: function() {
      return {
        position: this.position,
        rotation: this.rotation,
        scale: this.scale
      };
    },
    setValues: function(flyweight) {
      this.position = LAU.Utils.clone(flyweight.position);
      this.rotation = LAU.Utils.clone(flyweight.rotation);
      this.scale = LAU.Utils.clone(flyweight.scale);
    },
    destroy: function() {
      if($editCanvas.isEditMode()) {
        this._parentGroup.remove(this.hierarchyGroup);
      }
    },
    setHierarchyParent: function(newParent) {
      var parentGroup = $editCanvas.scene;
      if(newParent != null) {
        parentGroup = newParent.transform.hierarchyGroup;
      }
      this._parentGroup.remove(this.hierarchyGroup);
      parentGroup.add(this.hierarchyGroup);
      this._parentGroup = parentGroup;
    }
  };

  return {
    GameObject: GameObject
  };
}]);
