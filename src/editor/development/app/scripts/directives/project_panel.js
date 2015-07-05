'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:projectPanel
 * @description
 * # projectPanel
 */
angular.module('lauEditor').directive('projectPanel', ['$timeout', function ($timeout) {
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

        $scope.gameObjects.push(new LAU.GameObject($scope, sceneData[i].name, gameObjComponents));

        $event.broadcast('gameObjectCreated', i);
      }

      // TODO remove line below when the hierarchy panel is correctly created (with blur events to un-select game objects)
      $scope.currentGameObjectId = $scope.gameObjects.length-1;
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
