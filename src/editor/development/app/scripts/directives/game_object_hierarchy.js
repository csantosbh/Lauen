'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:gameObjectHierarchy
 * @description
 * # gameObjectHierarchy
 */
angular.module('lauEditor')
  .directive('gameObjectHierarchy', ['gameObjectManager', 'lauGameObject', 'historyManager', 'dragdropManager', function ($gom, $lgo, $hm, $dm) {
    return {
      templateUrl: 'views/directives/game_object_hierarchy.html',
      restrict: 'E',
      scope: true,
      link: function postLink(scope, element, attrs) {
        $dm.registerAction('dragid_game_obj_hierarchy',
                           'dropid_game_obj_hierarchy', function(draggedScope, dropScope) {
          var draggedGameObj = draggedScope.gameObject;

          var parentObj = null;
          if(dropScope.gameObject != undefined)
            parentObj = dropScope.gameObject;

          // Handle undo/redo
          scope.gameObjectHierarchy.historyHandlerCallback(draggedGameObj, parentObj);

          $gom.moveGameObjectTo(draggedGameObj, parentObj);
        });

        scope.dragid = 'dragid_game_obj_hierarchy';
        scope.gameObjectHierarchy = {
          createGameObject: function() {
            var newGameObj = new $lgo.GameObject({name: 'Unnamed'});
            $gom.pushGameObject(newGameObj);

            // Handle undo/redo
            $hm.pushCommand({
              _instanceId: newGameObj.instanceId,
              undo: function() {
                $gom.removeGameObject(this._instanceId);
              },
              redo: function() {
                $gom.pushGameObject(new $lgo.GameObject({name: 'Unnamed'}, this._instanceId));
              }
            });
          },
          gameObjects: $gom.getGameObjects,
          selectedGameObject: $gom.selectedGameObject,
          droppableOptions: {
            accept: function(draggedElement) {
              var draggedGameObj = draggedElement.draggable().scope().gameObject;
              return draggedGameObj != undefined;
            }
          },
          historyHandlerCallback: function(draggedGameObj, newParentObj) {
            // Push command to history manager
            let oldParent = draggedGameObj.parent == null ? null :
              draggedGameObj.parent.instanceId;
            let newParent = newParentObj == null ? null :
              newParentObj.instanceId;
            $hm.pushCommand({
              _objId: draggedGameObj.instanceId,
              _oldParentId: oldParent,
              _oldPosition: $gom.getMenuPosition(draggedGameObj.instanceId),
              _newParentId: newParent,
              undo: function() {
                let dst = this._oldParentId == null ? null :
                  $gom.getGameObject(this._oldParentId);
                let gameObj = $gom.getGameObject(this._objId);
                $gom.moveGameObjectTo(gameObj, dst);
                $gom.setMenuPosition(gameObj, this._oldPosition);
              },
              redo: function() {
                var dst = this._newParentId == null ? null :
                  $gom.getGameObject(this._newParentId);
                $gom.moveGameObjectTo($gom.getGameObject(this._objId), dst);
              }
            });
          },
          onDrop: function(event, draggedElement) {
            $dm.dispatchAction(draggedElement, scope, 'dropid_game_obj_hierarchy');
          }
        };
      }
    };
  }]);
