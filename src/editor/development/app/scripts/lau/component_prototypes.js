'use strict';

LAU.Components = (function() {
  ///
  // Auxiliary functions
  ///

  // Return default initial value for each field type
  function getDefaultScriptFieldValue(type) {
    // TODO: Document initialization rules for ALL types of controllers
    switch(type) {
      case 'int': case 'float':
        return 0;
      break;
    }
  }

  ///
  // Component types
  ///

  // Transform component
  function TransformComponent(obj) {
    this.type = 'transform';
    this.position = this.rotation = this.scale = null; // Will be set by the threejs canvas

    // This will make THREE.js bind this object to a visual entity
    $event.broadcast('transformComponentAdded', this);

    if(obj != null) {
      this.id = obj.id;
      if(obj.hasOwnProperty('x')) {
        this.position.x = obj.x, this.position.y = obj.y, this.position.z = obj.z;
        this.rotation.x = obj.rx, this.rotation.y = obj.ry, this.rotation.z = obj.rz;
        this.scale.x = obj.sx, this.scale.y = obj.sy, this.scale.z = obj.sz;
      }
    }
  }
  TransformComponent.prototype = {
    export: function() {
      return {
        type: this.type,
        id: this.id,
        x: this.position.x, y: this.position.y, z: this.position.z,
        rx: this.rotation.x, ry: this.rotation.y, rz: this.rotation.z,
        sx: this.scale.x, sy: this.scale.y, sz: this.scale.z
      };
    },
    setField: function(name, value) {
      // TODO this is redundant with the initialization seen previously. Refactor.
      switch(name) {
        case 'rx':
          this.rotation.x = value;
          break;
        case 'ry':
          this.rotation.y = value;
          break;
        case 'rz':
          this.rotation.z = value;
          break;
      }
    }
  };

  // Script Component
  function ScriptComponent(componentFlyWeight) {
    this.type = 'script';
    this.fields = {};

    if(componentFlyWeight == null) return;

    this.id = componentFlyWeight.id;
    this.class = componentFlyWeight.class;
    this.path = componentFlyWeight.path;
    this.namespace = componentFlyWeight.namespace;

    // Initialize script fields
    var givenFields = componentFlyWeight.fields;
    for(var f in givenFields) {
      if(givenFields.hasOwnProperty(f)) {
        // TODO: Check for visibility constraints
        this.fields[f] = {
          value: givenFields[f].hasOwnProperty('value') ?
            givenFields[f].value : getDefaultScriptFieldValue(givenFields[f].type),
          type: givenFields[f].type
        };
      }
    }
  }
  ScriptComponent.prototype = {
    export: function() {
      var exported_fields = {};
      for(var f in this.fields) {
        if(this.fields.hasOwnProperty(f)) {
          exported_fields[this.fields[f].name] = {
            value: this.fields[f].value,
            type: this.fields[f].type
          };
        }
      }

      return {
        type: this.type,
        path: this.path,
        namespace: this.namespace,
        id: this.id,
        fields: exported_fields
      };
    },
    setField: function(name, value) {
      //console.log('fields:')
      //console.log(this.fields)
      this.fields[name].value = value;
    },
  };

  // Instantiate new components (component factory)
  // TODO rename this to createComponentFromFlyWeight and only pass the flyweight (the componenttype lies within it -- maybe we can even get rid of the componentType as a string and only use a numeric id)
  function componentFactory(componentType, componentFlyWeight) {
    // The switch rules match the component menu label
    switch(componentType) {
      case 'transform':
        return new TransformComponent(componentFlyWeight);
      case 'script':
        return new ScriptComponent(componentFlyWeight);
    }
  }

  function getFlyweightById(componentMenu, id) {
    for(var i = 0; i < componentMenu.length; ++i) {
      if(componentMenu[i].hasOwnProperty('children')) {
        var deepSearchResult = getFlyweightById(componentMenu[i].children, id);
        if(deepSearchResult != null)
          return deepSearchResult;
      } else if(componentMenu[i].flyweight.id == id) {
        return componentMenu[i];
      }
    }
    return null;
  }

  function createComponentFromId(id, scope) {
    // Search for component flyweight.
    var component = getFlyweightById(scope.gameObjectEditor.componentMenu, id);
    return componentFactory(component.type, component.flyweight);
    return res;
  }

  return {
    TransformComponent: TransformComponent,
    componentFactory: componentFactory,
    createComponentFromId: createComponentFromId
  };
})();
