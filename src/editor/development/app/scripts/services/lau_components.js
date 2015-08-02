'use strict';

/**
 * @ngdoc service
 * @name lauEditor.lauComponents
 * @description
 * # lauComponents
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('lauComponents', ['editCanvasManager', function ($editCanvas) {
  // Return default initial value for each field type

  ///
  // Component types
  ///

  // Transform component
  function TransformComponent(gameObject, componentFlyWeight) {
    this.type = 'transform';
    this.position = this.rotation = this.scale = null;
    this.flyweight = componentFlyWeight;
    this.parent = gameObject;

    if($editCanvas.isEditMode()) {
      ////
      // Bind to edit canvas
      var $this = this;
      this._boundingBox = $editCanvas.getBoundingBox();
      $editCanvas.scene.add(this._boundingBox);

      function updatePosition(newValue) {
        if(newValue != null)
          $this._boundingBox.position.fromArray(newValue);
      }
      function positionObserver(changes) {
        var newValue = changes[changes.length-1].object;
        updatePosition(newValue);
      }
      function updateRotation(newValue) {
        if(newValue != null)
          $this._boundingBox.rotation.fromArray(newValue);
      }
      function rotationObserver(changes) {
        var newValue = changes[changes.length-1].object;
        updateRotation(newValue);
      }
      function updateScale(newValue) {
        if(newValue != null)
          $this._boundingBox.scale.fromArray(newValue);
      }
      function scaleObserver(changes) {
        var newValue = changes[changes.length-1].object;
        updateScale(newValue);
      }
      Object.observe(this, function(changes) {
        // TODO investigate if this will leak memory (Im not-explicitly ceasing to observe the older position)
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
    }
  }
  TransformComponent.prototype = {
    export: function() {
      return {
        type: this.flyweight.type,
        id: this.flyweight.id,
        fields: {
          position: this.position,
          rotation: this.rotation,
          scale: this.scale
        }
      };
    },
    setValues: function(flyweight) {
      this.position = LAU.Utils.clone(flyweight.fields.position);
      this.rotation = LAU.Utils.clone(flyweight.fields.rotation);
      this.scale = LAU.Utils.clone(flyweight.fields.scale);
    },
    destroy: function() {
      if($editCanvas.isEditMode()) {
        $editCanvas.scene.remove(this._boundingBox);
      }
    }
  };

  // Mesh component
  function MeshComponent(gameObject, componentFlyWeight) {
    this.type = 'mesh';
    this.mesh = componentFlyWeight.fields.mesh;
    this.flyweight = componentFlyWeight;
    this.parent = gameObject;

    if($editCanvas.isEditMode()) {
      ////
      // Bind to edit canvas
      var $this = this;
      // TODO make sure that this game object has a Transform
      //this._mesh = $editCanvas.getMesh();
      //$editCanvas.scene.add(this._mesh);

      function updateMesh(newValue) {
        if(newValue != null) {
          // TODO update THREE.js mesh
          // $this._mesh = ...;
        }
      }
      function meshObserver(changes) {
        var newValue = changes[changes.length-1].object;
        updateMesh(newValue);
      }
      Object.observe(this, function(changes) {
        // TODO investigate if this will leak memory (Im not-explicitly ceasing to observe the older position)
        for(var i = 0; i < changes.length; ++i) {
          var cng = changes[i];
          if(cng.name == "mesh" && cng.type=="update") {
            updateMesh($this.mesh);
            Object.observe($this.mesh, meshObserver);
          }
        }
      });
    }
  }
  MeshComponent.prototype = {
    export: function() {
      return {
        type: this.flyweight.type,
        id: this.flyweight.id,
        fields: {
          mesh: this.mesh,
        }
      };
    },
    setValues: function(flyweight) {
      this.scale = LAU.Utils.clone(flyweight.fields.mesh);
    },
    destroy: function() {
      if($editCanvas.isEditMode()) {
        //$editCanvas.scene.remove(this._mesh);
      }
    }
  };

  // Script Component
  function ScriptComponent(gameObject, componentFlyWeight) {
    this.type = 'script';
    this.fields = {};
    this.flyweight = componentFlyWeight;
    this.parent = gameObject;
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
    }
  };

  // Instantiate new components (component factory)
  function createComponentFromFlyWeight(gameObject, componentFlyWeight, instanceId) {
    // The switch rules match the component menu label
    var result;
    switch(componentFlyWeight.type) {
      case 'transform':
        result = new TransformComponent(gameObject, componentFlyWeight);
      break;
      case 'mesh':
        result = new MeshComponent(gameObject, componentFlyWeight);
      break;
      case 'script':
        result = new ScriptComponent(gameObject, componentFlyWeight);
      break;
    }

    // Set initial values from flyweight defaults
    result.setValues(componentFlyWeight);
    if(typeof instanceId !== 'undefined')
      result.instanceId = instanceId;

    return result;
  }

  return {
    TransformComponent: TransformComponent,
    createComponentFromFlyWeight: createComponentFromFlyWeight,
  };
}]);
