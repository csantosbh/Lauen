'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:projectPanel
 * @description
 * # projectPanel
 */
angular.module('lauEditor').directive('projectPanel', ['$timeout', 'gameObjectManager', function ($timeout, $gom) {
  // TODO this was moved here because the RPCs are not guaranteeded to obey any
  // particular call order. Implement some order-filtering to RPC calls. Maybe a
  // 'requires' parameter that specifies its dependencies.
  function handleIOEvents(sceneData, $scope) {
    $timeout(function() {
      // Setup game objects
      for(var i = 0; i < sceneData.length; ++i) {
        var gameObjComponents = [];
        var comps = sceneData[i].components;
        for(var c = 0; c < comps.length; ++c) {
          var component = LAU.Components.createComponentFromId(comps[c].id, $scope);
          if(component == null) {
            console.log('[warning] could not create component from id ' + comps[c].id);
            continue;
          }
          component.setValues(comps[c]);
          gameObjComponents.push(component);
        }

        $gom.pushGameObject(new LAU.GameObject($scope, sceneData[i].name, gameObjComponents));
      }
    });
  }

  return {
    templateUrl: 'views/directives/project_panel.html',
    restrict: 'E',
    link: function postLink(scope, element, attrs) {
      scope.projectPanel = {
        projectFiles: [],
        onDrop: function() {
          scope.gameObjectEditor.addComponent(scope.projectPanel.dropBucket);
        }
      };

      $event.listen('AssetWatch', function(data) {
        $timeout(function() {
          var projFiles = scope.projectPanel.projectFiles;
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

      $rpc.call('getAssetList', null, function(fileList) {
        $timeout(function() {
          for(var i=0; i < fileList.length; ++i) {
            fileList[i].type = 'script';
            scope.projectPanel.projectFiles.push({
              menu_label: LAU.IO.getFileNameFromPath(fileList[i].path),
              flyweight: fileList[i]
            });
          }
          $event.broadcast('initialAssetList', scope.projectPanel.projectFiles);
          $rpc.call('loadCurrentScene', null, function(data) {
            handleIOEvents(data, scope);
          });
        });
      });
    }
  };
}]);
