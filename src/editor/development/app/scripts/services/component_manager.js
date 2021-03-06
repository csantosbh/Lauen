'use strict';

/**
 * @ngdoc service
 * @name lauEditor.componentManager
 * @description
 * # componentManager
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('componentManager', ['$timeout', 'gameObjectManager', 'lauComponents', 'sceneManager', function ($timeout, $gom, $lauComps, $sm) {
  // TODO rename this to componentMenuManager
  ////
  // Internal fields
  var componentFlyweights = {
    camera: {menu_label: 'Camera', flyweight: null},
    mesh: {menu_label: 'Mesh', flyweight: null},
    mesh_renderer: {menu_label: 'Mesh Renderer', flyweight: null},
    script: [],
    prefab: [],
    light: {menu_label: 'Light', flyweight: null},
    skinned_mesh_renderer: {menu_label: 'Skinned Mesh Renderer', flyweight: null},
    model: [],
  };
  var componentMenu = [
    componentFlyweights.camera,
    componentFlyweights.light,
    {menu_label: 'Meshes', children: [
      componentFlyweights.mesh,
      componentFlyweights.mesh_renderer,
      componentFlyweights.skinned_mesh_renderer,
    ]},
    {menu_label: 'Scripts', children: componentFlyweights.script}
  ];

  ////
  // Public functions
  function getComponentMenu() {
    return componentMenu;
  }
  function getComponents() {
    return componentFlyweights;
  }
  function pushComponent(componentWrapper) {
    if(!componentFlyweights.hasOwnProperty(componentWrapper.flyweight.type)) {
      console.error('Component type not implemented: '+componentWrapper.flyweight.type);
      return;
    }

    if(Array.isArray(componentFlyweights[componentWrapper.flyweight.type]))
      componentFlyweights[componentWrapper.flyweight.type].push(componentWrapper);
    else
      componentFlyweights[componentWrapper.flyweight.type].flyweight = componentWrapper.flyweight;
  }
  function getMeshComponents() {
    return [];
  }
  function getFlyweightById(id, componentSubMenu) {
    var isRootCall = (componentSubMenu==undefined);
    if(isRootCall) componentSubMenu = componentMenu;

    for(var i = 0; i < componentSubMenu.length; ++i) {
      if(componentSubMenu[i].hasOwnProperty('children')) {
        var deepSearchResult = getFlyweightById(id, componentSubMenu[i].children);
        if(deepSearchResult != null)
          return deepSearchResult;
      } else if(componentSubMenu[i].flyweight != null && componentSubMenu[i].flyweight.id == id) {
        return componentSubMenu[i].flyweight;
      }
    }

    if(isRootCall)
      console.error('Cannot find component of id [' + id + ']');
    return null;
  }

  function createComponentFromId(gameObject, id, instanceId) {
    // Search for component flyweight.
    var flyweight = getFlyweightById(id);
    return $lauComps.createComponentFromFlyWeight(gameObject, flyweight, instanceId);
  }

  function updateGameObjectsAfterUpdatedFlyweight(scriptFlyweight) {
    function recurse_(gameObj){
      for(var c = gameObj.components.length-1; c >= 0; --c) {
        var comp = gameObj.components[c];
        if(comp.type=='script' && comp.flyweight.path == scriptFlyweight.path) {
          gameObj.components[c].refreshFlyweight(scriptFlyweight);
        }
      }
      for(let c = 0; c < gameObj.children.length; ++c) {
        recurse_(gameObj.children[c]);
      }
    }
    // Update prefabs
    let prefabs = $gom.prefabManager.getPrefabs();
    for(let p in prefabs)
      if(prefabs.hasOwnProperty(p))
        recurse_(prefabs[p].gameObject);

    // Update concrete game objects
    var gameObjects = $gom.getGameObjects();
    for(var g = 0; g < gameObjects.length; ++g) {
      recurse_(gameObjects[g]);
    }
  }

  ////
  // Event management
  function findComponentByPath(path) {
    // Find asset index
    for(var type in componentFlyweights) {
      if(componentFlyweights.hasOwnProperty(type)) {
        var compType = componentFlyweights[type];
        for(var i = 0; i < compType.length; ++i) {
          if(compType[i].flyweight.path == path) {
            return {type: type, idx: i};
          }
        }
      }
    }
    return {type: null, idx: -1};
  }

  $event.listen('AssetWatch', function(data) {
    $timeout(function() {
      if(data.event == 'delete') {
        var compInd = findComponentByPath(data.path);
        if(compInd.idx != -1) {
          switch(compInd.type) {
            case 'script':
              $gom.removeScriptFromGameObjects(componentFlyweights.script[compInd.idx].flyweight);
              break;
            case 'prefab':
              let prefabFlyweight = componentFlyweights.prefab[compInd.idx].flyweight.content;
              $gom.prefabManager.getPrefab(prefabFlyweight.instanceId).destroy();
              $sm.saveScene();
              break;
            case 'model':
              // TODO: Remove model from all assets
              let modelFlyweight = componentFlyweights.model[compInd.idx].flyweight;
              break;
          }
          componentFlyweights[compInd.type].splice(compInd.idx, 1);
        } else {
          console.error('Could not delete asset of path "' + data.path+ '" because it doesn\'t exist in the flyweight list');
        }
      } else if(data.event == 'update') {
        // Find asset index
        var compInd = findComponentByPath(data.asset.path);

        var flyweightWrapper = {
          menu_label: LAU.IO.getFileNameFromPath(data.asset.path),
          flyweight: data.asset
        };

        if(compInd.idx == -1) {
          if(componentFlyweights.hasOwnProperty(data.asset.type)) {
            // New file created
            componentFlyweights[data.asset.type].push(flyweightWrapper);
          } else {
            console.warn('Component of type [' + data.asset.type + '] ignored.');
          }
        } else {
          // File updated
          componentFlyweights[compInd.type][compInd.idx] = flyweightWrapper;
          updateGameObjectsAfterUpdatedFlyweight(flyweightWrapper.flyweight);
        }
      }
    });
  });

  $rpc.call('getDefaultComponents', null, function(dcs) {
    for(var i in dcs) {
      if(dcs.hasOwnProperty(i)) {
        var flyweightWrapper = {
          menu_label: LAU.IO.getFileNameFromPath(dcs[i].path),
          flyweight: dcs[i]
        };
        pushComponent(flyweightWrapper);
      }
    }
  });

  return {
    pushComponent: pushComponent,
    getComponents: getComponents,
    getComponentMenu: getComponentMenu,
    getFlyweightById: getFlyweightById,
    createComponentFromId: createComponentFromId,
  };
}]);
