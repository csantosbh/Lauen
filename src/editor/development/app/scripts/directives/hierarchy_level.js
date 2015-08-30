'use strict';

angular.module('lauEditor').directive('hierarchyLevel', ['gameObjectManager', '$compile', 'historyManager', 'lauGameObject', function ($gom, $compile, $hm, $lgo) {
  return {
    restrict: "E",
    replace: true,
    scope: {
      gameObject: '=',
      level: '='
    },
    templateUrl: 'views/directives/hierarchy_level.html',
    link: function (scope, element, attrs) {
      scope.selectGameObject = function(event, gameObj) {
        let gameObjBefore = $gom.selectedGameObject();
        $hm.pushCommand({
          _selectedGameObjBefore: gameObjBefore==null?null:gameObjBefore.instanceId,
          _selectedGameObjAfter: gameObj==null?null:gameObj.instanceId,
          redo: function() {
            if(this._selectedGameObjAfter != null)
              $gom.selectGameObject($gom.getGameObject(this._selectedGameObjAfter));
          },
          undo: function() {
            if(this._selectedGameObjBefore != null)
              $gom.selectGameObject($gom.getGameObject(this._selectedGameObjBefore));
          },
          settings: {
            passthrough: true
          }
        });
        $gom.selectGameObject(gameObj);

        event.preventDefault();
        event.stopPropagation();
      };
      scope.removeGameObject = function(event, gameObj) {
        console.log(gameObj);
        // Handle undo/redo
        $hm.pushCommand({
          _serializedGameObj: gameObj.export(),
          _instanceId: gameObj.instanceId,
          _parentId: gameObj.parent==null?null:gameObj.parent.instanceId,
          _position: $gom.getMenuPosition(gameObj.instanceId),
          undo: function() {
            var newGameObj = new $lgo.GameObject(this._serializedGameObj, this._instanceId);
            $gom.pushGameObject(newGameObj);
            if(this._parentId != null) {
              $gom.moveGameObjectTo(newGameObj, $gom.getGameObject(this._parentId));
            }
            $gom.setMenuPosition(newGameObj, this._position);
          },
          redo: function() {
            $gom.removeGameObject(this._instanceId);
          }
        });

        $gom.removeGameObject(gameObj.instanceId);

        event.stopPropagation();
      };
      scope.onDrop = function() {
        $gom.moveGameObjectTo(scope.dropBucket, scope.gameObject);
      }
      scope.optionsList = {
        accept: function(draggedElement) {
          var draggedGameObj = draggedElement.draggable().scope().gameObject;

          return draggedGameObj != undefined &&
            draggedGameObj.instanceId != scope.gameObject.instanceId &&
            !draggedGameObj.isParentOf(scope.gameObject);
        }
      };

      if (angular.isArray(scope.gameObject.children)) {
        var nestedElement = angular.element('<hierarchy-level level="level+1" ng-repeat="obj in gameObject.children" game-object="obj"></hierarchy-level>');
        $compile(nestedElement)(scope)
        element.append(nestedElement);
      }
    }
  }
}]);

