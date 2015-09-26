'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:projectPanel
 * @description
 * # projectPanel
 */
angular.module('lauEditor').directive('projectPanel', ['$timeout', 'gameObjectManager', 'componentManager', 'lauGameObject', 'dragdropManager', 'sceneManager', function ($timeout, $gom, $cm, $lgo, $dm, $sm) {
  // TODO this was moved here because the RPCs are not guaranteeded to obey any
  // particular call order. Implement some order-filtering to RPC calls. Maybe a
  // 'requires' parameter that specifies its dependencies.
  function handleIOEvents(sceneData, $scope) {
    $timeout(function() {
      // Setup prefabs
      let prefabs = $cm.getComponents().prefab;
      for(let i = 0; i < prefabs.length; ++i) {
        let pf = prefabs[i].flyweight.content;
        $lgo.createPrefabFromFlyweight(pf);
      }
      // Setup game objects
      for(let i = 0; i < sceneData.length; ++i) {
        $gom.pushGameObject(new $lgo.GameObject(sceneData[i]));
      }
    });
  }

  function getProjectFiles() {
    let components = $cm.getComponents();
    var assets = []
      .concat(components.script)
      .concat(components.prefab);
    return assets;
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

      scope.onAssetClick = function(event, file) {
        switch(file.flyweight.type) {
          case 'prefab':
            let prefab = $gom.prefabManager.getPrefab(file.flyweight.content.instanceId);
            $gom.selectPrefab(prefab);
            break;
          default:
            return;
        }
      }

      $dm.registerAction('dragid_game_obj_hierarchy', 'dropid_project_panel', function(draggedScope, dropScope) {
        // Create prefab requested!
        let go = $gom.getGameObject(draggedScope.gameObject.instanceId);
        let newPrefab = $lgo.createPrefabFromGameObject(go);
        go.setPrefabParent(newPrefab.instanceId);
        $rpc.call('savePrefab', newPrefab.export(), function(status) {
          if(status) {
            $sm.saveScene();
          } else {
            console.error('Could not save prefab '+newPrefab.name);
          }
        });
      });

      $rpc.call('getAssetList', null, function(fileList) {
        $timeout(function() {
          for(var i=0; i < fileList.length; ++i) {
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
