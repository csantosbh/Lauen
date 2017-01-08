'use strict';

/**
 * @ngdoc service
 * @name lauEditor.lauComponents
 * @description
 * # lauComponents
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('lauComponents', ['editCanvasManager', 'editorStateManager', 'historyManager', 'gameObjectManager', 'dragdropManager', function ($editCanvas, $esm, $hm, $gom, $dm) {
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
        let correspondingGameObj = myPrefab.getObjectInHierarchy(this.parent.hierarchyId);
        // TODO create a game-object specific id for components to tell duplicate components from eachother
        let prefabComponent;
        if(this.type == 'transform')
          prefabComponent = correspondingGameObj.transform;
        else
          prefabComponent = correspondingGameObj.getComponentsById(this.flyweight.id)[0];

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
      let $this = this;
      let prefabId = this.parent.parentPrefabId;

      if(prefabId == null) { // No prefab
        this.resetPrefabSync();
      } else {
        let myPrefab = $gom.prefabManager.getPrefab(prefabId);
        let correspondingGameObj = myPrefab.getObjectInHierarchy(this.parent.hierarchyId);
        // TODO create a game-object specific id for components to tell duplicate components from eachother
        let prefabComponent;
        if(this.type == 'transform')
          prefabComponent = correspondingGameObj.transform;
        else
          prefabComponent = correspondingGameObj.getComponentsById(this.flyweight.id)[0];

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

        recurseUpdate($this.fields, $this.prefabSync, prefabComponent.fields);
        // TODO standardize propagateGenericChange
        this.propagateGenericChange();
      }
    },
    watchChanges: function(instanceId, observer) {
      console.error('Unimplemented watchChanges called ['+instanceId+']');
    },
    propagateGenericChange: function() {
      console.error('Unimplemented propagateGenericChange called');
    }
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

    let notificationSystem = LAU.Utils.notificationSystem();

    this.watchChanges = notificationSystem.watchChanges;
    this.propagateChange = function(field) {
      // This function will be called whenever a field is updated
      notificationSystem.notifySubscribers();
    }
    this.propagateGenericChange = function() {
      notificationSystem.notifySubscribers();
    }

    // TODO add visual interpretation of camera
    if($esm.isEditMode()) {
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

      this.propagateGenericChange();
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

    if($esm.isEditMode()) {
      var $this = this;

      // Undo/redo support
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
              let $hm_this = this;

              comp.fields[field] = $hm_this._before;
              $this.checkPrefabFieldSynchronization(field);

              $this.propagateChange(field);
            },
            redo: function() {
              var gameObject = $gom.getGameObject(this._gameObj);
              let comp = gameObject.getComponentByInstanceId(this._component);
              let $hm_this = this;

              comp.fields[field] = $hm_this._after;
              $this.checkPrefabFieldSynchronization(field);

              $this.propagateChange(field);
            }
          });

          $this.checkPrefabFieldSynchronization(field);
        };
      }
      this._meshCommitCallback = this._editorCommitCallback('mesh');

      ////
      // Bind to game object editor
      this.getModelName_ = function() {
        return LAU.IO.getFileNameFromPath(this.fields.mesh);
      }
      this.onDrop_ = function(event, draggedElement) {
        $dm.dispatchAction(draggedElement, this, 'dropid_mesh_component_editor');
      }
      this.dragid = 'dropid_mesh_component_editor';

      $dm.registerAction('dragid_project_panel', 'dropid_mesh_component_editor', function(draggedScope, dropScope) {
        if(draggedScope.file.flyweight.type == 'model') {
          var previousMesh = dropScope.component.fields.mesh;
          dropScope.component.fields.mesh = draggedScope.file.flyweight.path;
          dropScope.component._meshCommitCallback(previousMesh, dropScope.component.fields.mesh);
          dropScope.component.propagateMeshChange();
        }
      });

      ////
      // Bind to edit canvas
      if(!this.parent.isPrefab) {
        $this.meshGeometry = null;
        this._reloadMesh = function() {
          $editCanvas.createMesh($this.fields.mesh, function(newMesh) {
            // Update mesh model
            $this.meshGeometry = newMesh;

            // Draw the mesh, if there's a mesh renderer
            var meshRenderer = $this.parent.getComponentsByType('mesh_renderer');
            if(meshRenderer.length == 0)
              meshRenderer = $this.parent.getComponentsByType('skinned_mesh_renderer');

            if(meshRenderer.length != 0) {
              meshRenderer = meshRenderer[0];
              meshRenderer.updateModels();
            }
          });
        }

        this._reloadMesh();

        // TODO send to deep copy down below...
        // TODO allow for listener subscription
        $this.propagateMeshChange = function() {
          if($this.fields.mesh != null) {
            // Remove old mesh
            $this.removeCurrentMesh();
            $this._reloadMesh();
          }
        }
        $this.propagateChange = function(type) {
          switch(type) {
            case 'mesh':
              $this.propagateMeshChange();
            break;
            default:
              console.error('ERROR propagating changer function of type: '+type);
          }
        }
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

      this.propagateGenericChange();
    },
    removeCurrentMesh: function() {
      if($esm.isEditMode()) {
        // Remove this mesh from the hierarchy that groups everything from this
        // game object
        if(!this.parent.isPrefab)
          this.parent.transform.hierarchyGroup.remove(this.meshGeometry);
      }
    },
    destroy: function() {
      this.removeCurrentMesh();
      _freeComponentId(this.instanceId);
    }
  }, MeshComponent.prototype);

  // Mesh Renderer component
  function MeshRendererComponent(gameObject, componentFlyWeight, instanceId) {
    this.type = 'mesh_renderer';
    this.flyweight = componentFlyWeight;
    this.parent = gameObject;

    this.resetPrefabSync = function() {}

    this.instanceId = _allocComponentId(instanceId);

    if($esm.isEditMode()) {
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
    setValues: function(flyweight) {
      this.propagateGenericChange();
    },
    destroy: function() {
      _freeComponentId(this.instanceId);
    },
    updateModels: function() {
      if(!this.parent.isPrefab) {
        var transform = this.parent.transform;
        var meshComponents = this.parent.getComponentsByType('mesh');
        for(var i = 0; i < meshComponents.length; ++i) {
          if(meshComponents[i].meshGeometry != null)
            transform.hierarchyGroup.add(meshComponents[i].meshGeometry);
        }
      }
    }
  }, MeshRendererComponent.prototype);

  // Light component
  function LightComponent(gameObject, componentFlyWeight, instanceId) {
    this.type = 'light';
    this.flyweight = componentFlyWeight;
    this.parent = gameObject;

    this.instanceId = _allocComponentId(instanceId);

    this.fields = {
      color: Number(componentFlyWeight.fields.color).toString(16)
    };

    this.resetPrefabSync = function() {
      this.prefabSync = {
        color: true,
      };
    }
    this.resetPrefabSync();
  }
  LightComponent.prototype = Object.create(Component.prototype);
  LAU.Utils.deepCopy({
    export: function() {
      return {
        type: this.flyweight.type,
        id: this.flyweight.id,
        fields: {
          color: Number.parseInt(this.fields.color, 16),
        },
        instanceId: this.instanceId,
      };
    },
    setValues: function(flyweight) {
      this.fields.color = Number(flyweight.fields.color).toString(16);

      this.propagateGenericChange();
    },
    destroy: function() {
      _freeComponentId(this.instanceId);
    },
    _editorCommitCallback: function(field) {
      if($esm.isEditMode()) {
        var $this = this;

        // TODO refatorar o metodo commitCallback, ficando apenas em Component.
        // Cuidado com o deepcopy, que soh funciona em arrays/objects.
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
    },
  }, LightComponent.prototype);

  // Skinned Mesh Renderer component
  function SkinnedMeshRendererComponent(gameObject, componentFlyWeight, instanceId) {
    this.type = 'skinned_mesh_renderer';
    this.flyweight = componentFlyWeight;
    this.parent = gameObject;
    this.fields = {
      animation: componentFlyWeight.fields.animation
    };

    this.resetPrefabSync = function() {
      this.prefabSync = {
        animation: true,
      };
    }
    this.resetPrefabSync();

    this.instanceId = _allocComponentId(instanceId);

    // Used by the component editor to list the current model animations
    this.getAvailableAnimations = function() {
      let mesh = this.parent.getComponentsByType('mesh');
      if(mesh.length > 0) {
        mesh = mesh[0];
        let animNames = [];
        if(mesh.meshGeometry.geometry.animations) {
          let animList = mesh.meshGeometry.geometry.animations;
          for(let i = 0; i < animList.length; ++i) {
            animNames.push(animList[i].name);
          }
        }
        else
          animNames.push(mesh.meshGeometry.geometry.animation.name);

        return animNames;
      }
      return [];
    }

    // TODO use $hm service to add undo/redo support to changes in animation
    if($esm.isEditMode() && !this.parent.isPrefab) {
      ////
      // Bind to edit canvas
      this.updateModels();
    }
  }
  SkinnedMeshRendererComponent.prototype = Object.create(Component.prototype);
  LAU.Utils.deepCopy({
    export: function() {
      return {
        type: this.flyweight.type,
        id: this.flyweight.id,
        fields: {
          animation: this.fields.animation
        },
        instanceId: this.instanceId,
      };
    },
    setValues: function(flyweight) {
      this.fields.animation = flyweight.fields.animation;

      this.propagateGenericChange();
    },
    destroy: function() {
      _freeComponentId(this.instanceId);
    },
    updateModels: function() {
      if(!this.parent.isPrefab) {
        var transform = this.parent.transform;
        var meshComponents = this.parent.getComponentsByType('mesh');
        for(var i = 0; i < meshComponents.length; ++i) {
          if(meshComponents[i].meshGeometry != null)
            transform.hierarchyGroup.add(meshComponents[i].meshGeometry);
        }
      }
    }
  }, SkinnedMeshRendererComponent.prototype);

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

    if($esm.isEditMode()) {
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

      this.propagateGenericChange();
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
      case 'light':
        result = new LightComponent(gameObject, componentFlyWeight, instanceId);
        break;
      case 'skinned_mesh_renderer':
        result = new SkinnedMeshRendererComponent(gameObject, componentFlyWeight, instanceId);
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
