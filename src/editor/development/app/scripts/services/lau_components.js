'use strict';

/**
 * @ngdoc service
 * @name lauEditor.lauComponents
 * @description
 * # lauComponents
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('lauComponents', ['editCanvasManager', 'historyManager', 'gameObjectManager', function ($editCanvas, $hm, $gom) {
  // Return default initial value for each field type

  ///
  // Component types
  ///

  // Camera component
  function CameraComponent(gameObject, componentFlyWeight, instanceId) {
    this.type = 'camera';

    this.near = componentFlyWeight.fields.near;
    this.far = componentFlyWeight.fields.far;
    this.width = componentFlyWeight.fields.width;
    this.priority = componentFlyWeight.fields.priority;

    this.instanceId = _allocComponentId(instanceId);

    Object.defineProperty(this, 'fov', {
      set: function(fov) {
        let radFov = fov*Math.PI/180.0;
        this.width = 2.0*this.near*Math.tan(radFov/2.0);
      },
      get: function() {
        return Number((2.0*Math.atan2(this.width,2.0*this.near)*180.0/Math.PI).toFixed(2));
      }
    });

    this.flyweight = componentFlyWeight;
    this.parent = gameObject;

    // TODO add visual interpretation of camera
    if($editCanvas.isEditMode()) {
      var $this = this;

      this._editorCommitCallback = function(field) {
        return function(oldValue, newValue) {
          $hm.pushCommand({
            _before: oldValue,
            _after: newValue,
            _gameObj: $this.parent.instanceId,
            _component: $this.instanceId,
            undo: function() {
              var gameObject = $gom.getGameObject(this._gameObj);
              gameObject.getComponentByInstanceId(this._component)[field] = this._before;
            },
            redo: function() {
              var gameObject = $gom.getGameObject(this._gameObj);
              gameObject.getComponentByInstanceId(this._component)[field] = this._after;
            }
          });
        };
      }
    }

  }
  CameraComponent.prototype = {
    export: function() {
      return {
        type: this.flyweight.type,
        id: this.flyweight.id,
        instanceId: this.instanceId,
        fields: {
          near: this.near,
          far: this.far,
          width: this.width,
          priority: this.priority,
        }
      };
    },
    setValues: function(flyweight) {
      this.near = LAU.Utils.clone(flyweight.fields.near);
      this.far = LAU.Utils.clone(flyweight.fields.far);
      this.width = LAU.Utils.clone(flyweight.fields.width);
      this.priority = LAU.Utils.clone(flyweight.fields.priority);
    },
    destroy: function() {
      _freeComponentId(this.instanceId);
    }
  };

  // Mesh component
  function MeshComponent(gameObject, componentFlyWeight, instanceId) {
    this.type = 'mesh';
    this.mesh = componentFlyWeight.fields.mesh;
    this.flyweight = componentFlyWeight;
    this.parent = gameObject;

    this.instanceId = _allocComponentId(instanceId);

    if($editCanvas.isEditMode()) {
      ////
      // Bind to edit canvas
      var $this = this;
      this.meshGeometry = $editCanvas.createMesh(this.mesh);

      function updateMesh(newValue) {
        if(newValue != null) {
          // TODO update THREE.js mesh
          // remove old mesh...
          // $this.meshGeometry = ...;
          // transform.group.add($this.meshGeometry)... if theres a mesh renderer
        }
      }
      Object.observe(this, function(changes) {
        // TODO investigate if this will leak memory (Im not-explicitly ceasing to observe the older position)
        // Only the last change to this.mesh interests us. Break after it's found.
        for(var i = changes.length-1; i >= 0; --i) {
          var cng = changes[i];
          if(cng.name == "mesh" && cng.type=="update") {
            updateMesh($this.mesh);
            break;
          }
        }
      });

      // Draw the mesh, if there's a mesh renderer
      var meshRenderer = this.parent.getComponentsByType('mesh_renderer');
      if(meshRenderer.length != 0) {
        meshRenderer = meshRenderer[0];
        meshRenderer.updateModels();
      }
    }
  }
  MeshComponent.prototype = {
    export: function() {
      return {
        type: this.flyweight.type,
        id: this.flyweight.id,
        instanceId: this.instanceId,
        fields: {
          mesh: this.mesh,
        }
      };
    },
    setValues: function(flyweight) {
      this.mesh = LAU.Utils.clone(flyweight.fields.mesh);
    },
    destroy: function() {
      if($editCanvas.isEditMode()) {
        // Remove this mesh from the hierarchy that groups everything from this game object
        this.parent.transform.hierarchyGroup.remove(this.meshGeometry);
      }
      _freeComponentId(this.instanceId);
    }
  };

  // Mesh Renderer component
  function MeshRendererComponent(gameObject, componentFlyWeight, instanceId) {
    this.type = 'mesh_renderer';
    this.flyweight = componentFlyWeight;
    this.parent = gameObject;

    this.instanceId = _allocComponentId(instanceId);

    if($editCanvas.isEditMode()) {
      ////
      // Bind to edit canvas
      this.updateModels();
    }
  }
  MeshRendererComponent.prototype = {
    export: function() {
      return {
        type: this.flyweight.type,
        id: this.flyweight.id,
        instanceId: this.instanceId,
        fields: {
          mesh: this.mesh,
        }
      };
    },
    setValues: function(flyweight) { },
    destroy: function() {
      if($editCanvas.isEditMode()) {
        //$editCanvas.scene.remove(this.meshGeometry);
      }
      _freeComponentId(this.instanceId);
    },
    updateModels: function() {
      var transform = this.parent.transform;
      var meshComponents = this.parent.getComponentsByType('mesh');
      for(var i = 0; i < meshComponents.length; ++i) {
        transform.hierarchyGroup.add(meshComponents[i].meshGeometry);
      }
    }
  };

  // Script Component
  function ScriptComponent(gameObject, componentFlyWeight, instanceId) {
    this.type = 'script';
    this.fields = {};
    this.flyweight = componentFlyWeight;
    this.parent = gameObject;

    this.instanceId = _allocComponentId(instanceId);

    if($editCanvas.isEditMode()) {
      var $this = this;

      this._editorCommitCallback = function(field) {
        return function(oldValue, newValue) {
          $hm.pushCommand({
            _before: oldValue,
            _after: newValue,
            _gameObj: $this.parent.instanceId,
            _component: $this.instanceId,
            undo: function() {
              var gameObject = $gom.getGameObject(this._gameObj);
              gameObject.getComponentByInstanceId(this._component).fields[field] = this._before;
            },
            redo: function() {
              var gameObject = $gom.getGameObject(this._gameObj);
              gameObject.getComponentByInstanceId(this._component).fields[field] = this._after;
            }
          });
        };
      };

    }
  }
  ScriptComponent.prototype = {
    export: function() {
      var exported_fields = {};
      for(var f in this.fields) {
        if(this.fields.hasOwnProperty(f)) {
          exported_fields[f] = this.fields[f];
        }
      }

      return {
        type: this.type,
        path: this.flyweight.path,
        namespace: this.flyweight.namespace,
        id: this.flyweight.id,
        instanceId: this.instanceId,
        fields: exported_fields
      };
    },
    setValues: function(flyweight) {
      // Initialize script fields
      var givenFields = flyweight.fields;
      for(var f in givenFields) {
        if(givenFields.hasOwnProperty(f) && this.flyweight.fields.hasOwnProperty(f)) {
          this.fields[f] = LAU.Utils.clone(givenFields[f]);
        }
      }
    },
    destroy: function() {
      _freeComponentId(this.instanceId);
    }
  };

  // Instantiate new components (component factory)
  function createComponentFromFlyWeight(gameObject, componentFlyWeight, instanceId) {
    // The switch rules match the component menu label
    var result;
    switch(componentFlyWeight.type) {
      case 'camera':
        result = new CameraComponent(gameObject, componentFlyWeight, instanceId);
      break;
      case 'mesh':
        result = new MeshComponent(gameObject, componentFlyWeight, instanceId);
      break;
      case 'mesh_renderer':
        result = new MeshRendererComponent(gameObject, componentFlyWeight, instanceId);
      break;
      case 'script':
        result = new ScriptComponent(gameObject, componentFlyWeight, instanceId);
      break;
    }

    // Set initial values from flyweight defaults
    result.setValues(componentFlyWeight);

    return result;
  }

  ///
  // Internal logic
  ///
  var _componentIds = new Set();
  function _allocComponentId(id) {
    function genId() {
      return Math.pow(2,32)*Math.random();
    }

    if(id == undefined) {
      id = genId();
      while(_componentIds.has(id))
        id = genId();
    }
    _componentIds.add(id);

    return id;
  }
  function _freeComponentId(id) {
    _componentIds.delete(id);
  }

  return {
    createComponentFromFlyWeight: createComponentFromFlyWeight,
  };
}]);
