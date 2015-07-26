'use strict';

/**
 * @ngdoc service
 * @name lauEditor.componentManager
 * @description
 * # componentManager
 * Service in the lauEditor.
 */
angular.module('lauEditor')
  .service('componentManager', ['$timeout', function ($timeout) {
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

    ////
    // Event management
    $event.listen('AssetWatch', function(data) {
      $timeout(function() {
        var projFiles = projectFiles;
        var currentInstanceIdx = -1;
        if(data.event == 'delete') {
          // Find asset index
          for(var i = 0; i < projFiles.length; ++i) {
            if(projFiles[i].flyweight.path == data.path) {
              currentInstanceIdx = i;
              break;
            }
          }

          if(currentInstanceIdx != -1) {
            $gom.removeScriptFromGameObjects(projFiles[currentInstanceIdx].flyweight);
            projFiles.splice(currentInstanceIdx, 1);
          } else {
            // TODO assert that this will never happen!
            console.error('Invalid deleted asset!');
          }
        } else if(data.event == 'update') {
          // Find asset index
          for(var i = 0; i < projFiles.length; ++i) {
            if(projFiles[i].flyweight.path == data.asset.path) {
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
            projFiles.push(assetFlyweight);
          } else {
            // File updated
            projFiles[currentInstanceIdx] = assetFlyweight;
            $gom.updateScriptsFromFlyweight(assetFlyweight.flyweight);
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
    };
  }]);
