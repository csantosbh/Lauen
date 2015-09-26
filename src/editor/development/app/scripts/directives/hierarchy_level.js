'use strict';

angular.module('lauEditor').directive('hierarchyLevel', ['gameObjectManager', '$compile', 'historyManager', 'lauGameObject', 'dragdropManager', function ($gom, $compile, $hm, $lgo, $dm) {
  return {
    restrict: "E",
    replace: true,
    scope: {
      gameObject: '=',
      level: '=',
      historyHandlerCallback: '='
    },
    templateUrl: 'views/directives/hierarchy_level.html',
    link: function (scope, element, attrs) {
      scope.selectGameObject = function(event, gameObj) {
        $gom.selectGameObject(gameObj);

        event.preventDefault();
        event.stopPropagation();
      };
      scope.removeGameObject = function(event, gameObj) {
        // Handle undo/redo
        $hm.pushCommand({
          _serializedGameObj: gameObj.export(),
          _instanceId: gameObj.instanceId,
          _parentId: gameObj.parent==null?null:gameObj.parent.instanceId,
          _hierarchyId: gameObj.hierarchyId,
          _position: $gom.getMenuPosition(gameObj.instanceId),
          undo: function() {
            var newGameObj = new $lgo.GameObject(this._serializedGameObj, this._instanceId);
            $gom.pushGameObject(newGameObj);
            if(this._parentId != null) {
              $gom.moveGameObjectTo(newGameObj, $gom.getGameObject(this._parentId), this._hierarchyId);
            }
            $gom.setMenuPosition(newGameObj, this._position);
          },
          redo: function() {
            $gom.removeGameObject(this._instanceId);
          }
        });

        $gom.removeGameObject(gameObj.instanceId);

        event.preventDefault();
        event.stopPropagation();
      };
      scope.onDrop = function(event, draggedElement) {
        $dm.dispatchAction(draggedElement, scope, 'dropid_game_obj_hierarchy');
      }
      scope.dragid = 'dragid_game_obj_hierarchy';

      $dm.registerAction('dragid_project_panel', 'dropid_game_obj_hierarchy', function(draggedScope, dropScope) {
        let flyweight = draggedScope.file.flyweight;
        if(flyweight.type == 'prefab') {
          let prefab = $gom.prefabManager.getPrefab(flyweight.content.instanceId);
          let gameObj = prefab.instantiate();

          // Handle undo/redo
          $hm.pushCommand({
            _instanceId: gameObj.instanceId,
            _prefabId: flyweight.content.instanceId,
            undo: function() {
              if(this._instanceId)
                $gom.removeGameObject(this._instanceId);
            },
            redo: function() {
              if($gom.prefabManager.prefabExists(this._prefabId)) {
                let prefab = $gom.prefabManager.getPrefab(this._prefabId);
                prefab.instantiate(this._instanceId);
              } else this._instanceId = null;
            }
          });

        }
      });

      if (angular.isArray(scope.gameObject.children)) {
        var nestedElement = angular.element('<hierarchy-level level="level+1" ng-repeat="obj in gameObject.children" history-handler-callback="historyHandlerCallback" game-object="obj"></hierarchy-level>');
        $compile(nestedElement)(scope)
        element.append(nestedElement);
      }
    }
  }
}]);

