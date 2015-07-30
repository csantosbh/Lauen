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
    var components = [];
    var standardComponents = {
      'transform': {menu_label:'Transform', flyweight: null },
    };
    var componentMenu = [
      standardComponents.transform,
      // TODO when we start having more components, this will no longer work.
      // I will need a getScriptComponents, and children will have to be
      // dynamic (maybe a function).
      {menu_label: 'Scripts', children: getComponents()}
    ];

    ////
    // Public functions
    function pushComponent(componentWrapper) {
      components.push(componentWrapper);
    }
    function getComponents() {
      return components;
    }
    function getComponentMenu() {
      return componentMenu;
    }
    function getFlyweightById(id, componentSubMenu) {
      if(!componentSubMenu) componentSubMenu = componentMenu;

      for(var i = 0; i < componentSubMenu.length; ++i) {
        if(componentSubMenu[i].hasOwnProperty('children')) {
          var deepSearchResult = getFlyweightById(id, componentSubMenu[i].children);
          if(deepSearchResult != null)
            return deepSearchResult;
        } else if(componentSubMenu[i].flyweight.id == id) {
          return componentSubMenu[i].flyweight;
        }
      }
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
    $event.listen('AssetWatch', function(data) {
      $timeout(function() {
        var currentInstanceIdx = -1;
        if(data.event == 'delete') {
          // Find asset index
          for(var i = 0; i < components.length; ++i) {
            if(components[i].flyweight.path == data.path) {
              currentInstanceIdx = i;
              break;
            }
          }

          if(currentInstanceIdx != -1) {
            $gom.removeScriptFromGameObjects(components[currentInstanceIdx].flyweight);
            components.splice(currentInstanceIdx, 1);
          } else {
            console.error('Could not delete asset of path "' + data.path+ '" because it doesn\'t exist in the flyweight list');
          }
        } else if(data.event == 'update') {
          // Find asset index
          for(var i = 0; i < components.length; ++i) {
            if(components[i].flyweight.path == data.asset.path) {
              currentInstanceIdx = i;
              break;
            }
          }

          data.asset.type = 'script';
          var assetFlyweight = {
            menu_label: LAU.IO.getFileNameFromPath(data.asset.path),
            flyweight: data.asset
          };
          if(currentInstanceIdx == -1) {
            // New file created
            components.push(assetFlyweight);
          } else {
            // File updated
            components[currentInstanceIdx] = assetFlyweight;
            updateGameObjectsAfterUpdatedFlyweight(assetFlyweight.flyweight);
          }
        }
      });
    });

    $rpc.call('getDefaultComponents', null, function(dcs) {
      for(var i in dcs) {
        if(dcs.hasOwnProperty(i)) {
          standardComponents[i].flyweight = dcs[i];
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
