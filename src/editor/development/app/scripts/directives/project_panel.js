'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:projectPanel
 * @description
 * # projectPanel
 */
angular.module('lauEditor').directive('projectPanel', ['$timeout', 'gameObjectManager', 'componentManager', 'lauGameObject', 'dragdropManager', function ($timeout, $gom, $cm, $lgo, $dm) {
  // TODO this was moved here because the RPCs are not guaranteeded to obey any
  // particular call order. Implement some order-filtering to RPC calls. Maybe a
  // 'requires' parameter that specifies its dependencies.
  function handleIOEvents(sceneData, $scope) {
    $timeout(function() {
      // Setup game objects
      for(var i = 0; i < sceneData.length; ++i) {
        $gom.pushGameObject(new $lgo.GameObject(sceneData[i]));
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
    scope: true,
    link: function postLink(scope, element, attrs) {
      scope.projectPanel = {
        projectFiles: getProjectFiles,
      };
      scope.onDrop = function(event, draggedElement) {
        $dm.dispatchAction(draggedElement, scope, 'dropid_project_panel');
      }
      scope.dragid = 'dragid_project_panel';

      $dm.registerAction('dragid_game_obj_hierarchy', 'dropid_project_panel', function(draggedScope, dropScope) {
        // Create prefab requested!
        $rpc.call('createPrefab', draggedScope.gameObject.export(), function() {
        });
      });

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
    }
  };
}]);
