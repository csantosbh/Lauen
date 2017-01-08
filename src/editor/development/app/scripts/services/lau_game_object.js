'use strict';

/**
 * @ngdoc service
 * @name lauEditor.lauGameObject
 * @description
 * # lauGameObject
 * Service in the lauEditor.
 */
angular.module('lauEditor')
.service('lauGameObject', ['$timeout', 'editCanvasManager', 'componentManager', 'historyManager', 'gameObjectManager', 'lauComponents', 'editorStateManager', function ($timeout, $editCanvas, $cm, $hm, $gom, $lc, $esm) {
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

    if(fields.hasOwnProperty('hierarchyId'))
      this.hierarchyId = fields.hierarchyId;
    else
      this.hierarchyId = 0;

    if(fields.parentPrefabId && $gom.prefabManager.prefabExists(fields.parentPrefabId)) {
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
        var newChild = new GameObject(child[g], child[g].instanceId, isPrefab);
        newChild.setParent(this, child[g].hierarchyId);
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
        myPrefab.componentWasAdded(this.hierarchyId, component);
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
          myPrefab.componentBeingRemoved(this.hierarchyId, this.components[compIdx]);
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
    setParent: function(parentGameObj, hierarchyId) {
      if(parentGameObj != null) {
        if(hierarchyId == undefined)
          this.hierarchyId = parentGameObj._generateHierarchyId();
        else
          this.hierarchyId = hierarchyId;

        parentGameObj._addChild(this);
      } else {
        this.hierarchyId = 0;
      }

      if($esm.isEditMode()) {
        this.transform.setHierarchyParent(parentGameObj);
      }

      this.parent = parentGameObj;
    },
    _generateHierarchyId: function() {
      if(this.parent == null) {
        let allIds = new Set([this.hierarchyId]);

        function _getAllIdsRec(children) {
          for(let i = 0; i < children.length; ++i) {
            allIds.add(children[i].hierarchyId);
            _getAllIdsRec(children[i].children);
          }
        }
        _getAllIdsRec(this.children);

        let newId = randomInteger();
        while(allIds.has(newId)) {
          newId = randomInteger();
        }

        return newId;
      } else {
        return this.parent._generateHierarchyId();
      }
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
        hierarchyId: this.hierarchyId,
      };
    },
    setPrefabParent: function(prefabId) {
      function setParentRecurse_(children) {
        for(let i = 0; i < children.length; ++i) {
          children[i].parentPrefabId = prefabId;
          setParentRecurse_(children[i].children);
        }
      }

      function syncComponentsRecurse_(children) {
        for(let i = 0; i < children.length; ++i) {
          children[i].syncComponentsToPrefab();
          setParentRecurse_(children[i].children);
        }
      }

      let rootParent = this;
      while(rootParent.parent != null)
        rootParent = rootParent.parent;

      rootParent.parentPrefabId = prefabId;
      setParentRecurse_(rootParent.children);

      if(!rootParent.isPrefab)
        syncComponentsRecurse_(rootParent.children);
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
    },
    _nameCommitCallback: function() {
      var $this = this;
      return function(oldValue, newValue) {
        $hm.pushCommand({
          _before: oldValue,
          _after: newValue,
          _gameObj: $this.instanceId,
          undo: function() {
            var gameObj = $gom.getGameObject(this._gameObj);
            gameObj.name = this._before;
          },
          redo: function() {
            var gameObj = $gom.getGameObject(this._gameObj);
            gameObj.name = this._after;
          }
        });
      };
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

    function makeObserver(hierarchyId, componentId) {
      return function(changes) {
        let changedComponentId;
        if(componentId != 'transform') {
          let gameObj = $this.getObjectInHierarchy(hierarchyId);
          changedComponentId = gameObj.getComponentByInstanceId(componentId).flyweight.id;
        }
        else {
          changedComponentId = 'transform';
        }

        let gameObjs = $gom.getInstancesOfPrefab($this.instanceId, hierarchyId);

        for(let i = 0; i < gameObjs.length; ++i) {
          gameObjs[i].syncComponentToPrefab(changedComponentId);
        }
      }
    }

    let observers_ = {};
    function initializeWatchCallback(gameObj) {
      // TODO watch for newly added components
      observers_[gameObj.hierarchyId] = {
        transform: makeObserver(gameObj.hierarchyId, 'transform')
      };

      gameObj.transform.watchChanges($this.instanceId, observers_[gameObj.hierarchyId]['transform']);

      for(let i = 0; i < gameObj.components.length; ++i) {
        if(gameObj.components[i].hasOwnProperty('fields')) {
          let cId = gameObj.components[i].instanceId;
          if(!observers_[gameObj.hierarchyId].hasOwnProperty(cId))
            observers_[gameObj.hierarchyId][cId] = makeObserver(gameObj.hierarchyId, cId);

          let fields = gameObj.components[i].fields;
          // We have to observe the whole fields object to account for changes
          // in primitive types
          gameObj.components[i].watchChanges($this.instanceId, observers_[gameObj.hierarchyId][cId]);
        }
      }
    }

    function destroyWatchCallback(gameObj) {
      for(let i in gameObj.transform.fields)
        Object.unobserve(gameObj.transform.fields[i], observers_[gameObj.hierarchyId]['transform']);

      for(let i = 0; i < gameObj.components.length; ++i) {
        let cId = gameObj.components[i].instanceId;
        if(gameObj.components[i].hasOwnProperty('fields') && observers_[gameObj.hierarchyId].hasOwnProperty(cId)) {
          let fields = gameObj.components[i].fields;
          Object.unobserve(fields, observers_[gameObj.hierarchyId][cId]);
          for(let f in fields) {
            if(fields.hasOwnProperty(f) && typeof(fields[f]) == 'object') {
              Object.unobserve(fields[f], observers_[gameObj.hierarchyId][cId]);
            }
          }
        }
      }
    }

    this.destroy = function() {
      function recurse_(gameObj) {
        for(let i = 0; i < gameObj.children.length; ++i)
          recurse_(gameObj.children[i]);

        destroyWatchCallback(gameObj);
        gameObj.destroy();
      }
      recurse_(this.gameObject);

      $gom.prefabManager.destroyPrefab(this.instanceId);
    };

    this.componentWasAdded = function(hierarchyId, component) {
      let cId = component.instanceId;
      let gameObjs = $gom.getInstancesOfPrefab(this.instanceId, hierarchyId);

      for(let i = 0; i < gameObjs.length; ++i) {
        gameObjs[i].addComponent($lc.createComponentFromFlyWeight(gameObjs[i], component.flyweight));
      }

      // Update watchers
      observers_[hierarchyId][cId] = makeObserver(cId);

      let fields = component.fields;
      // We have to observe the whole fields object to account for changes
      // in primitive types
      //Object.observe(fields, observers_[hierarchyId][cId]);
      for(let f in fields) {
        if(fields.hasOwnProperty(f) && typeof(fields[f]) == 'object') {
          // And we have to deep watch objects/arrays
          //Object.observe(fields[f], observers_[hierarchyId][cId]);
        }
      }
    };

    this.componentBeingRemoved = function(hierarchyId, component) {
      let componentId = component.flyweight.id;
      let cId = component.instanceId;
      let gameObjs = $gom.getInstancesOfPrefab(this.instanceId, hierarchyId);

      for(let i = 0; i < gameObjs.length; ++i) {
        let comp = gameObjs[i].getComponentsById(componentId)[0];
        gameObjs[i].removeComponent(comp.instanceId);
      }

      let fields = component.fields;
      Object.unobserve(fields, observers_[hierarchyId][cId]);
      for(let f in fields) {
        if(fields.hasOwnProperty(f) && typeof(fields[f]) == 'object') {
          Object.unobserve(fields[f], observers_[hierarchyId][cId]);
        }
      }
      delete observers_[hierarchyId][cId];
    };

    (function recurse_(gameObj){
      initializeWatchCallback(gameObj);
      for(let i = 0; i < gameObj.children.length; ++i) {
        recurse_(gameObj.children[i]);
      }
    })(this.gameObject);
  }
  Prefab.prototype = {
    export: function() {
      return {
        gameObject: this.gameObject.export(),
        instanceId: this.instanceId,
        name: this.name,
      };
    },
    instantiate: function(uniqueId) {
      let gameObj = new GameObject(this.gameObject.export(), uniqueId);
      $gom.pushGameObject(gameObj);
      gameObj.setPrefabParent(this.instanceId);
      return gameObj;
    },
    getObjectInHierarchy: function(hierarchyId) {
      function recurse_(obj) {
        if(obj.hierarchyId == hierarchyId)
          return obj;
        for(let i = 0; i < obj.children.length; ++i) {
          let recRes = recurse_(obj.children[i]);
          if(recRes != null)
            return recRes;
        }
        return null;
      }
      return recurse_(this.gameObject);
    }
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
  function randomInteger() {
    return Math.pow(2,32)*Math.random();
  }

  function _allocGameObjectId() {
    let id = randomInteger();
    while(_gameObjIds.has(id))
      id = randomInteger();
    _gameObjIds.add(id);

    return id;
  }
  function _freeGameObjectId(id) {
    _gameObjIds.delete(id);
  }

  // Transform logic
  function Transform(gameObject) {
    var $this = this;

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

    let notificationSystem = LAU.Utils.notificationSystem();
    this.watchChanges = notificationSystem.watchChanges;

    // TODO send to deepcopy down below...
    this.propagatePositionChange = function() {
      if($this.hierarchyGroup !== undefined) {
        $this.hierarchyGroup.position.fromArray($this.fields.position);
      }

      notificationSystem.notifySubscribers();
    }

    this.propagateRotationChange = function() {
      if($this.hierarchyGroup !== undefined) {
        $this.hierarchyGroup.rotation.fromArray($this.fields.rotation);
      }

      notificationSystem.notifySubscribers();
    }

    this.propagateScaleChange = function() {
      if($this.hierarchyGroup !== undefined) {
        $this.hierarchyGroup.scale.fromArray($this.fields.scale);
      }

      notificationSystem.notifySubscribers();
    }

    this.propagateChange = function(field) {
      switch(field) {
        case 'position':
          return $this.propagatePositionChange();
        case 'rotation':
          return $this.propagateRotationChange();
        case 'scale':
          return $this.propagateScaleChange();
      }
    }
    this.propagateGenericChange = function() {
      $this.propagatePositionChange();
      $this.propagateRotationChange();
      $this.propagateScaleChange();
    }

    this.type = "transform";
    this.parent = gameObject;

    if($esm.isEditMode()) {
      if(!this.parent.isPrefab) {
        ////
        // Bind transform to edit canvas
        this.hierarchyGroup = $editCanvas.createGroup();
        this.canvasHandle = $editCanvas.createAxesHandle();
        this.canvasHandle.__lauGameObject = this.parent;
        this.hierarchyGroup.add(this.canvasHandle);
        this._parentGroup = $editCanvas.scene;

        $editCanvas.scene.add(this.hierarchyGroup);
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
              $this.propagateChange(field);
              $this.checkPrefabFieldSynchronization(field);
            },
            redo: function() {
              var gameObj = $gom.getGameObject(this._gameObj);
              LAU.Utils.deepCopy(this._after, gameObj.transform.fields[field]);
              $this.propagateChange(field);
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
      this.propagateGenericChange();
    },
    destroy: function() {
      if($esm.isEditMode() && !this.parent.isPrefab) {
        this._parentGroup.remove(this.hierarchyGroup);
      }
    },
    setHierarchyParent: function(newParent) {
      if(!this.parent.isPrefab) {
        var parentGroup = $editCanvas.scene;
        if(newParent != null) {
          parentGroup = newParent.transform.hierarchyGroup;
        }
        this._parentGroup.remove(this.hierarchyGroup);
        parentGroup.add(this.hierarchyGroup);
        this._parentGroup = parentGroup;
      }
    }
  }, Transform.prototype);

  return {
    GameObject: GameObject,
    createPrefabFromFlyweight: createPrefabFromFlyweight,
    createPrefabFromGameObject: createPrefabFromGameObject,
  };
}]);
