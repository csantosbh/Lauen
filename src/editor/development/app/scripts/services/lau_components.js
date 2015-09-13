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
  function Component() { }
  Component.prototype = {
    checkPrefabSynchronization: function() {
      if(this.parent.isPrefab)
        return;

      // Check which fields are up to date with my prefab
      for(let field in this.fields) {
        if(this.fields.hasOwnProperty(field)) {
          this.checkPrefabFieldSynchronization(field);
        }
      }
    },
    checkPrefabFieldSynchronization: function(field) {
      if(this.parent.isPrefab)
        return;

      // Check which elements of this field are up to date with my prefab
      if(this.parent.parentPrefabId) {
        let myPrefab = $gom.prefabManager.getPrefab(this.parent.parentPrefabId);
        // TODO create a game-object specific id for components to tell duplicate components from eachother
        let prefabComponent;
        if(this.type == 'transform')
          prefabComponent = myPrefab.gameObject.transform;
        else
          prefabComponent = myPrefab.gameObject.getComponentsById(this.flyweight.id)[0];

        function recurseCmp(prefabSyncObj, levelFields, prefabCorrespondents) {
          for(let i in levelFields) {
            if(levelFields.hasOwnProperty(i)) {
              if(typeof(levelFields[i]) == 'object')
                recurseCmp(prefabSyncObj[i], levelFields[i], prefabCorrespondents[i]);
              else {
                prefabSyncObj[i] = (levelFields[i] == prefabCorrespondents[i]);
              }
            }
          }
        }
        if(typeof(this.fields[field]) == 'object')
          recurseCmp(this.prefabSync[field], this.fields[field], prefabComponent.fields[field]);
        else
          this.prefabSync[field] = (this.fields[field] == prefabComponent.fields[field]);
      }
    },
    syncComponentToPrefab: function() {
      let prefabId = this.parent.parentPrefabId;
      if(prefabId == null) { // No prefab
        this.resetPrefabSync();
      } else {
        let myPrefab = $gom.prefabManager.getPrefab(prefabId);
        // TODO create a game-object specific id for components to tell duplicate components from eachother
        let prefabComponent;
        if(this.type == 'transform')
          prefabComponent = myPrefab.gameObject.transform;
        else
          prefabComponent = myPrefab.gameObject.getComponentsById(this.flyweight.id)[0];

        function recurseUpdate(levelFields, prefabSync, prefabCorrespondents) {
          for(let i in levelFields) {
            if(levelFields.hasOwnProperty(i)) {
              if(typeof(levelFields[i]) == 'object')
                recurseUpdate(levelFields[i], prefabSync[i], prefabCorrespondents[i]);
              else if(prefabSync[i]) {
                levelFields[i] = prefabCorrespondents[i];
              }
            }
          }
        }
        recurseUpdate(this.fields, this.prefabSync, prefabComponent.fields);
      }
    },
  };

  // Camera component
  function CameraComponent(gameObject, componentFlyWeight, instanceId) {
    this.type = 'camera';

    this.fields = {
      near: componentFlyWeight.fields.near,
      far: componentFlyWeight.fields.far,
      width: componentFlyWeight.fields.width,
      priority: componentFlyWeight.fields.priority,
    };
    this.resetPrefabSync = function() {
      this.prefabSync = {
        near: true,
        far: true,
        width: true,
        priority: true,
        fov: true,
      };
    }
    this.resetPrefabSync();

    this.instanceId = _allocComponentId(instanceId);

    Object.defineProperty(this.fields, 'fov', {
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
              let comp = gameObject.getComponentByInstanceId(this._component);
              comp.fields[field] = this._before;
              $this.checkPrefabFieldSynchronization(field);
            },
            redo: function() {
              var gameObject = $gom.getGameObject(this._gameObj);
              let comp = gameObject.getComponentByInstanceId(this._component);
              comp.fields[field] = this._after;
              $this.checkPrefabFieldSynchronization(field);
            }
          });

          $this.checkPrefabFieldSynchronization(field);
        };
      }
    }

  }
  CameraComponent.prototype = Object.create(Component.prototype);
  LAU.Utils.deepCopy({
    export: function() {
      return {
        type: this.flyweight.type,
        id: this.flyweight.id,
        instanceId: this.instanceId,
        fields: {
          near: this.fields.near,
          far: this.fields.far,
          width: this.fields.width,
          priority: this.fields.priority,
        }
      };
    },
    setValues: function(flyweight) {
      this.fields.near     = flyweight.fields.near;
      this.fields.far      = flyweight.fields.far;
      this.fields.width    = flyweight.fields.width;
      this.fields.priority = flyweight.fields.priority;
    },
    destroy: function() {
      _freeComponentId(this.instanceId);
    }
  }, CameraComponent.prototype);

  // Mesh component
  function MeshComponent(gameObject, componentFlyWeight, instanceId) {
    this.type = 'mesh';
    this.fields = {
      mesh: componentFlyWeight.fields.mesh,
    };
    this.resetPrefabSync = function() {
      this.prefabSync = {
        mesh: true,
      };
    }
    this.resetPrefabSync();
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
      Object.observe(this.fields, function(changes) {
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
  MeshComponent.prototype = Object.create(Component.prototype);
  LAU.Utils.deepCopy({
    export: function() {
      return {
        type: this.flyweight.type,
        id: this.flyweight.id,
        instanceId: this.instanceId,
        fields: {
          mesh: this.fields.mesh,
        }
      };
    },
    setValues: function(flyweight) {
      this.fields.mesh = flyweight.fields.mesh;
    },
    destroy: function() {
      if($editCanvas.isEditMode()) {
        // Remove this mesh from the hierarchy that groups everything from this game object
        this.parent.transform.hierarchyGroup.remove(this.meshGeometry);
      }
      _freeComponentId(this.instanceId);
    }
  }, MeshComponent.prototype);

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
  MeshRendererComponent.prototype = Object.create(Component.prototype);
  LAU.Utils.deepCopy({
    export: function() {
      return {
        type: this.flyweight.type,
        id: this.flyweight.id,
        fields: {},
        instanceId: this.instanceId,
      };
    },
    setValues: function(flyweight) { },
    destroy: function() {
      _freeComponentId(this.instanceId);
    },
    updateModels: function() {
      var transform = this.parent.transform;
      var meshComponents = this.parent.getComponentsByType('mesh');
      for(var i = 0; i < meshComponents.length; ++i) {
        transform.hierarchyGroup.add(meshComponents[i].meshGeometry);
      }
    }
  }, MeshRendererComponent.prototype);

  // Script Component
  function ScriptComponent(gameObject, componentFlyWeight, instanceId) {
    this.type = 'script';
    this.fields = {};
    this.resetPrefabSync = function() {
      this.prefabSync = {};
      function recurse_(fieldsObj, prefabSyncObj) {
        for(let i in fieldsObj) {
          if(fieldsObj.hasOwnProperty(i)) {
            if(typeof(fieldsObj[i]) == 'object') {
              prefabSyncObj[i] = {};
              recurse_(fieldsObj[i], prefabSyncObj[i]);
            }
            else
              prefabSyncObj[i] = true;
          }
        }
      }
      recurse_(this.fields, this.prefabSync);
    }
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
              let comp = gameObject.getComponentByInstanceId(this._component);
              LAU.Utils.deepCopy(this._before, comp.fields[field]);
              $this.checkPrefabFieldSynchronization(field);
            },
            redo: function() {
              var gameObject = $gom.getGameObject(this._gameObj);
              let comp = gameObject.getComponentByInstanceId(this._component);
              LAU.Utils.deepCopy(this._after, comp.fields[field]);
              $this.checkPrefabFieldSynchronization(field);
            }
          });

          $this.checkPrefabFieldSynchronization(field);
        };
      };

    }
  }
  ScriptComponent.prototype = Object.create(Component.prototype);
  LAU.Utils.deepCopy({
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
    refreshFlyweight: function(newFlyweight) {
      this.flyweight = newFlyweight;

      function processRemoved_(myFields, prefabSync, prefabFields) {
        // Remove fields that do not exist in the script flyweight -- these
        // fields may still be present due to trying to restore this component
        // from an outdated serialized reference
        for(let i in myFields) {
          if(myFields.hasOwnProperty(i)) {
            if(!prefabFields.hasOwnProperty(i)) {
              delete myFields[i];
              delete prefabSync[i];
            }
            else if(typeof(myFields[i]) == 'object') {
              processRemoved_(myFields[i], prefabSync[i], prefabFields[i]);
            }
          }
        }
      }
      function processAdded_(myFields, prefabSync, prefabFields) {
        // Add fields that may have been created in the script flyweight.
        for(let i in prefabFields) {
          if(prefabFields.hasOwnProperty(i)) {
            if(!myFields.hasOwnProperty(i)) {
              if(typeof(prefabFields[i]) == 'object') {
                if(Array.isArray(prefabFields[i])) {
                  myFields[i] = [];
                  prefabSync[i] = [];
                } else {
                  myFields[i] = {};
                  prefabSync[i] = {};
                }
                LAU.Utils.deepCopy(prefabFields[i], myFields[i]);
              } else {
                myFields[i] = prefabFields[i];
                prefabSync[i] = true;
              }
            }
            else if(typeof(prefabFields[i]) == 'object') {
              processAdded_(myFields[i], prefabFields[i]);
            }
          }
        }
      }
      processRemoved_(this.fields, this.prefabSync, this.flyweight.fields);
      processAdded_(this.fields, this.prefabSync, this.flyweight.fields);
    },
    setValues: function(flyweight) {
      // Initialize script fields
      var givenFields = flyweight.fields;
      LAU.Utils.deepCopy(flyweight.fields, this.fields);

      this.resetPrefabSync();
    },
    destroy: function() {
      _freeComponentId(this.instanceId);
    }
  }, ScriptComponent.prototype);

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
    Component: Component,
  };
}]);
