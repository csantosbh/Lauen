'use strict';

angular.module('lauEditor').directive('hierarchyLevel', ['gameObjectManager', '$compile', function ($gom, $compile) {
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
        event.stopPropagation();
        $gom.selectGameObject(gameObj);
      };
      scope.removeGameObject = function(event, gameObj) {
        event.stopPropagation();
        $gom.removeGameObject(gameObj.instanceId);
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

