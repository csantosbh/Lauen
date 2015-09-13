'use strict';

/**
 * @ngdoc service
 * @name lauEditor.lauGameObject
 * @description
 * # lauGameObject
 * Service in the lauEditor.
 */
angular.module('lauEditor')
.service('lauGameObject', ['editCanvasManager', 'componentManager', 'historyManager', 'gameObjectManager', 'lauComponents', function ($editCanvas, $cm, $hm, $gom, $lc) {
  ///
  // Public GameObject API
  ///
  function GameObject(fields, uniqueNumericId, isPrefab) {
    this.name = fields.name;
    this.components = [];
    this.children = [];
    this.parent = null;

    if(isPrefab == undefined)
      this.isPrefab = false;
    else
      this.isPrefab = isPrefab;

    if(uniqueNumericId != undefined) {
      this.instanceId = uniqueNumericId;
      this._managedId = false;
    } else {
      this.instanceId = _allocGameObjectId();
      this._managedId = true;
    }

    if(fields.parentPrefabId && !isPrefab) {
      this.parentPrefabId = fields.parentPrefabId;
    }
    else
      this.parentPrefabId = null;

    ////
    // Set component values
    this.transform = new Transform(this);
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

    ////
    // Synchronize components with prefab
    if(fields.parentPrefabId && !isPrefab) {
      this.checkPrefabSynchronization();
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
    addComponent: function(component) {
      this.components.push(component);

      if(this.isPrefab) {
        let myPrefab = $gom.prefabManager.getPrefab(this.parentPrefabId);
        myPrefab.componentWasAdded(component);
      }
    },
    removeComponent: function(componentId) {
      let compIdx = -1;
      for(let i = 0; i < this.components.length; ++i) {
        let comp = this.components[i];
        if(comp.instanceId == componentId) {
          compIdx = i;
          break;
        }
      }

      if(compIdx >= 0) {
        if(this.isPrefab) {
          let myPrefab = $gom.prefabManager.getPrefab(this.parentPrefabId);
          myPrefab.componentBeingRemoved(this.components[compIdx]);
        }

        this.components.splice(compIdx, 1);
      } else {
        console.error('Could remove component of id '+componentId);
      }
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
        children: children,
        parentPrefabId: this.parentPrefabId,
      };
    },
    setPrefabParent: function(prefabId) {
      this.parentPrefabId = prefabId;
      if(!this.isPrefab)
        this.syncComponentsToPrefab();
    },
    checkPrefabSynchronization: function() {
      this.transform.checkPrefabSynchronization();
      for(let i = 0; i < this.components.length; ++i) {
        this.components[i].checkPrefabSynchronization();
      }
    },
    syncComponentsToPrefab: function() {
      this.transform.syncComponentToPrefab();
      for(let i = 0; i < this.components.length; ++i) {
        this.components[i].syncComponentToPrefab();
      }
    },
    syncComponentToPrefab: function(componentId) {
      if(componentId == 'transform') {
        this.transform.syncComponentToPrefab();
      } else {
        // TODO create a game-object specific id for components to tell duplicate components from eachother
        let comp = this.getComponentsById(componentId)[0];
        comp.syncComponentToPrefab();
      }
    },
    destroy: function() {
      this.transform.destroy();

      for(var i = 0; i < this.components.length; ++i) {
        this.components[i].destroy();
      }
      for(var i = 0; i < this.children.length; ++i) {
        this.children[i].destroy();
      }

      if($gom.selectedGameObject() != null &&
         $gom.selectedGameObject().instanceId == this.instanceId) {
        $gom.selectGameObject(null);
      }

      if(this._managedId)
        _freeGameObjectId(this.instanceId);
    }
  };

  ///
  // Public Game Object Prefab API
  ///
  function Prefab(serializedData) {
    let $this = this;
    this.gameObject = new GameObject(serializedData.gameObject, undefined, true);
    this.instanceId = $gom.prefabManager.allocPrefabId(serializedData.instanceId);
    this.gameObject.setPrefabParent(this.instanceId);
    this.name = serializedData.name;

    function makeObserver(id) {
      return function(changes) {
        let changedComponentId;
        if(id != 'transform') {
          changedComponentId = $this.gameObject.getComponentByInstanceId(id).flyweight.id;
        }
        else {
          changedComponentId = id;
        }

        let gameObjs = $gom.getInstancesOfPrefab($this.instanceId);

        for(let i = 0; i < gameObjs.length; ++i) {
          gameObjs[i].syncComponentToPrefab(changedComponentId);
        }
      }
    }

    let observers_ = {};
    function initializeWatchCallback(gameObj) {
      // TODO watch for newly added components
      observers_['transform'] = makeObserver('transform');
      for(let i in gameObj.transform.fields)
        Object.observe(gameObj.transform.fields[i], observers_['transform']);

      for(let i = 0; i < gameObj.components.length; ++i) {
        if(gameObj.components[i].hasOwnProperty('fields')) {
          let cId = gameObj.components[i].instanceId;
          if(!observers_.hasOwnProperty(cId))
            observers_[cId] = makeObserver(cId);

          let fields = gameObj.components[i].fields;
          // We have to observe the whole fields object to account for changes
          // in primitive types
          Object.observe(fields, observers_[cId]);
          for(let f in fields) {
            if(fields.hasOwnProperty(f) && typeof(fields[f]) == 'object') {
              // And we have to deep watch objects/arrays
              Object.observe(fields[f], observers_[cId]);
            }
          }
        }
      }
    }

    function destroyWatchCallback(gameObj) {
      for(let i in gameObj.transform.fields)
        Object.unobserve(gameObj.transform.fields[i], observers_['transform']);

      for(let i = 0; i < gameObj.components.length; ++i) {
        let cId = gameObj.components[i].instanceId;
        if(gameObj.components[i].hasOwnProperty('fields') && observers_.hasOwnProperty(cId)) {
          let fields = gameObj.components[i].fields;
          Object.unobserve(fields, observers_[cId]);
          for(let f in fields) {
            if(fields.hasOwnProperty(f) && typeof(fields[f]) == 'object') {
              Object.unobserve(fields[f], observers_[cId]);
            }
          }
        }
      }
    }

    this.destroy = function() {
      destroyWatchCallback(this.gameObject);
      this.gameObject.destroy();
      $gom.prefabManager.destroyPrefab(this.instanceId);
    };

    this.componentWasAdded = function(component) {
      let cId = component.instanceId;
      let gameObjs = $gom.getInstancesOfPrefab(this.instanceId);

      for(let i = 0; i < gameObjs.length; ++i) {
        gameObjs[i].addComponent($lc.createComponentFromFlyWeight(gameObjs[i], component.flyweight));
      }

      // Update watchers
      observers_[cId] = makeObserver(cId);

      let fields = component.fields;
      // We have to observe the whole fields object to account for changes
      // in primitive types
      Object.observe(fields, observers_[cId]);
      for(let f in fields) {
        if(fields.hasOwnProperty(f) && typeof(fields[f]) == 'object') {
          // And we have to deep watch objects/arrays
          Object.observe(fields[f], observers_[cId]);
        }
      }
    };

    this.componentBeingRemoved = function(component) {
      let componentId = component.flyweight.id;
      let cId = component.instanceId;
      let gameObjs = $gom.getInstancesOfPrefab(this.instanceId);

      for(let i = 0; i < gameObjs.length; ++i) {
        let comp = gameObjs[i].getComponentsById(componentId)[0];
        gameObjs[i].removeComponent(comp.instanceId);
      }

      let fields = component.fields;
      Object.unobserve(fields, observers_[cId]);
      for(let f in fields) {
        if(fields.hasOwnProperty(f) && typeof(fields[f]) == 'object') {
          Object.unobserve(fields[f], observers_[cId]);
        }
      }
      delete observers_[cId];
    };

    initializeWatchCallback(this.gameObject);
  }
  Prefab.prototype = {
    export: function() {
      return {
        gameObject: this.gameObject.export(),
        instanceId: this.instanceId,
        name: this.name,
      };
    },
  };

  function createPrefabFromGameObject(gameObj) {
    return createPrefabFromFlyweight({
      gameObject: gameObj.export(),
      name: gameObj.name
    });
  }
  function createPrefabFromFlyweight(flyweight) {
    let newPrefab = new Prefab(flyweight);
    $gom.prefabManager.addPrefab(newPrefab);
    return newPrefab;
  }

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
    this.fields = {
      position: [0,0,0],
      rotation: [0,0,0],
      scale: [1,1,1],
    };

    this.resetPrefabSync = function() {
      this.prefabSync = {
        position: [true,true,true],
        rotation: [true,true,true],
        scale: [true,true,true],
      };
    }
    this.resetPrefabSync();

    this.type = "transform";
    this.parent = gameObject;

    if($editCanvas.isEditMode()) {
      var $this = this;

      if(!this.parent.isPrefab) {
        ////
        // Bind transform to edit canvas
        this.hierarchyGroup = $editCanvas.createGroup();
        this.hierarchyGroup.add($editCanvas.createBoundingBox());
        this._parentGroup = $editCanvas.scene;

        $editCanvas.scene.add(this.hierarchyGroup);

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
        updatePosition($this.fields.position);
        Object.observe($this.fields.position, positionObserver);
        updateRotation($this.fields.rotation);
        Object.observe($this.fields.rotation, rotationObserver);
        updateScale($this.fields.scale);
        Object.observe($this.fields.scale, scaleObserver);
      }

      this._editorCommitCallback = function(field) {
        return function(oldValue, newValue) {
          $hm.pushCommand({
            _before: oldValue,
            _after: newValue,
            _gameObj: $this.parent.instanceId,
            undo: function() {
              var gameObj = $gom.getGameObject(this._gameObj);
              LAU.Utils.deepCopy(this._before, gameObj.transform.fields[field]);
              $this.checkPrefabFieldSynchronization(field);
            },
            redo: function() {
              var gameObj = $gom.getGameObject(this._gameObj);
              LAU.Utils.deepCopy(this._after, gameObj.transform.fields[field]);
              $this.checkPrefabFieldSynchronization(field);
            }
          });

          $this.checkPrefabFieldSynchronization(field);
        };
      };
    }
  }
  Transform.prototype = Object.create($lc.Component.prototype);
  LAU.Utils.deepCopy({
    export: function() {
      return {
        fields: {
          position: this.fields.position,
          rotation: this.fields.rotation,
          scale: this.fields.scale,
        }
      };
    },
    setValues: function(flyweight) {
      LAU.Utils.deepCopy(flyweight.fields, this.fields);
    },
    destroy: function() {
      if($editCanvas.isEditMode() && !this.parent.isPrefab) {
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
  }, Transform.prototype);

  return {
    GameObject: GameObject,
    createPrefabFromFlyweight: createPrefabFromFlyweight,
    createPrefabFromGameObject: createPrefabFromGameObject,
  };
}]);
