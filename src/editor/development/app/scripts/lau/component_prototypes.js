'use strict';

LAU.Components = (function() {
  // Return default initial value for each field type

  ///
  // Component types
  ///

  // Transform component
  function TransformComponent(componentFlyWeight) {
    this.type = 'transform';
    this.position = this.rotation = this.scale = null; // Will be set by the threejs canvas
    this.flyweight = componentFlyWeight;
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
      this.position = flyweight.fields.position;
      this.rotation = flyweight.fields.rotation;
      this.scale = flyweight.fields.scale;
    },
    destroy: function() {
    }
  };

  // Script Component
  function ScriptComponent(componentFlyWeight) {
    this.type = 'script';
    this.fields = {};
    this.flyweight = componentFlyWeight;
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
        if(givenFields.hasOwnProperty(f)) {
          this.fields[f] = givenFields[f];
        }
      }
    }
  };

  // Instantiate new components (component factory)
  function createComponentFromFlyWeight(componentFlyWeight, instanceId) {
    // The switch rules match the component menu label
    var result;
    switch(componentFlyWeight.type) {
      case 'transform':
        result = new TransformComponent(componentFlyWeight);
        break;
      case 'script':
        result = new ScriptComponent(componentFlyWeight);
        break;
    }

    // Set initial values from flyweight defaults
    result.setValues(componentFlyWeight);
    if(typeof instanceId !== 'undefined')
      result.instanceId = instanceId;

    return result;
  }

  function getFlyweightById(componentMenu, id) {
    for(var i = 0; i < componentMenu.length; ++i) {
      if(componentMenu[i].hasOwnProperty('children')) {
        var deepSearchResult = getFlyweightById(componentMenu[i].children, id);
        if(deepSearchResult != null)
          return deepSearchResult;
      } else if(componentMenu[i].flyweight.id == id) {
        return componentMenu[i].flyweight;
      }
    }
    return null;
  }

  // TODO maybe make this an injectable module so I can have the scope for free?
  function createComponentFromId(id, scope, instanceId) {
    // Search for component flyweight.
    var component = getFlyweightById(scope.gameObjectEditor.componentMenu, id);
    return createComponentFromFlyWeight(component, instanceId);
  }

  return {
    TransformComponent: TransformComponent,
    createComponentFromFlyWeight: createComponentFromFlyWeight,
    createComponentFromId: createComponentFromId
  };
})();
