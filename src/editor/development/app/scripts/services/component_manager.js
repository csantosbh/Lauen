'use strict';

/**
 * @ngdoc service
 * @name lauEditor.componentManager
 * @description
 * # componentManager
 * Service in the lauEditor.
 */
angular.module('lauEditor')
  .service('componentManager', ['$timeout', 'gameObjectManager', 'lauComponents', function ($timeout, $gom, $lauComps) {
    ////
    // Internal fields
    var componentFlyweights = {
      mesh: {menu_label: 'Mesh', flyweight: null},
      mesh_renderer: {menu_label: 'Mesh Renderer', flyweight: null},
      script: [],
    };
    var componentMenu = [
      {menu_label: 'Meshes', children: [
        componentFlyweights.mesh,
        componentFlyweights.mesh_renderer,
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
      var gameObjects = $gom.getGameObjects();
      for(var g = 0; g < gameObjects.length; ++g) {
        var gameObj = gameObjects[g];
        for(var c = gameObj.components.length-1; c >= 0; --c) {
          var comp = gameObj.components[c];
          if(comp.type=='script' && comp.flyweight.path == scriptFlyweight.path) {
            // Backup original data
            var componentBackup = comp.export();
            // Update flyweight
            gameObj.components[c] = $lauComps.createComponentFromFlyWeight(gameObj, scriptFlyweight);
            // Restore data
            gameObj.components[c].setValues(componentBackup);
          }
        }
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
            $gom.removeScriptFromGameObjects(componentFlyweights[compInd.type][compInd.idx].flyweight);
            componentFlyweights[compInd.type].splice(compInd.idx, 1);
          } else {
            console.error('Could not delete asset of path "' + data.path+ '" because it doesn\'t exist in the flyweight list');
          }
        } else if(data.event == 'update') {
          // Find asset index
          var compInd = findComponentByPath(data.asset.path);

          // TODO data.asset.type must come from the server
          data.asset.type = 'script';
          var flyweightWrapper = {
            menu_label: LAU.IO.getFileNameFromPath(data.asset.path),
            flyweight: data.asset
          };

          if(compInd.idx == -1) {
            // New file created
            componentFlyweights[data.asset.type].push(flyweightWrapper);
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
