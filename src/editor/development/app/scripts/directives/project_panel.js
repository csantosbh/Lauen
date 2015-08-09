'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:projectPanel
 * @description
 * # projectPanel
 */
angular.module('lauEditor').directive('projectPanel', ['$timeout', 'gameObjectManager', 'componentManager', 'lauGameObject', function ($timeout, $gom, $cm, $lgo) {
  // TODO this was moved here because the RPCs are not guaranteeded to obey any
  // particular call order. Implement some order-filtering to RPC calls. Maybe a
  // 'requires' parameter that specifies its dependencies.
  function handleIOEvents(sceneData, $scope) {
    $timeout(function() {
      // Setup game objects
      for(var i = 0; i < sceneData.length; ++i) {
        var gameObj = new $lgo.GameObject(sceneData[i].name);
        var comps = sceneData[i].components;
        for(var c = 0; c < comps.length; ++c) {
          var component = $cm.createComponentFromId(gameObj, comps[c].id);
          if(component == null) {
            console.log('[warning] could not create component from id ' + comps[c].id);
            continue;
          }
          component.setValues(comps[c]);
          gameObj.components.push(component);
        }

        $gom.pushGameObject(gameObj);
      }
    });
  }

  function getProjectFiles() {
    var components = $cm.getComponents();
    return components.script;
  }

  return {
    templateUrl: 'views/directives/project_panel.html',
    restrict: 'E',
    link: function postLink(scope, element, attrs) {
      scope.projectPanel = {
        onDrop:function() {
          scope.gameObjectEditor.addComponent(scope.dropBucket);
        },
        projectFiles: getProjectFiles,
      };

      $rpc.call('updateAllAssets', null, function() {
        $rpc.call('getAssetList', null, function(fileList) {
          $timeout(function() {
            for(var i=0; i < fileList.length; ++i) {
              fileList[i].type = 'script';
              $cm.pushComponent({
                menu_label: LAU.IO.getFileNameFromPath(fileList[i].path),
                flyweight: fileList[i]
              });
            }
            $rpc.call('loadCurrentScene', null, function(data) {
              handleIOEvents(data, scope);
            });
          });
        });
      });
    }
  };
}]);
