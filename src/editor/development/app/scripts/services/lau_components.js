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
        var transform = this.parent.getComponentsByType('transform');
        if(transform.length != 0) {
          transform = transform[0];
          transform.hierarchyGroup.remove(this.meshGeometry);
        }
      }
    }
  };

  // Mesh Renderer component
  function MeshRendererComponent(gameObject, componentFlyWeight) {
    this.type = 'mesh_renderer';
    this.flyweight = componentFlyWeight;
    this.parent = gameObject;

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
    },
    updateModels: function() {
      var transformComponent = this.parent.getComponentsByType('transform');
      if(transformComponent.length > 0) {
        transformComponent = transformComponent[0];
        var meshComponents = this.parent.getComponentsByType('mesh');
        for(var i = 0; i < meshComponents.length; ++i) {
          transformComponent.hierarchyGroup.add(meshComponents[i].meshGeometry);
        }
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
      case 'mesh':
        result = new MeshComponent(gameObject, componentFlyWeight);
      break;
      case 'mesh_renderer':
        result = new MeshRendererComponent(gameObject, componentFlyWeight);
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
    createComponentFromFlyWeight: createComponentFromFlyWeight,
  };
}]);
