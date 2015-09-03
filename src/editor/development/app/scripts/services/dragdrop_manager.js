'use strict';

/**
 * @ngdoc service
 * @name lauEditor.dragdropManager
 * @description
 * # dragdropManager
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('dragdropManager', function () {
  var registeredActions = {};

  function registerAction(dragid, dropid, action) {
    if(!registeredActions.hasOwnProperty(dragid))
      registeredActions[dragid] = {};

    registeredActions[dragid][dropid] = action;
  }

  function dispatchAction(draggedElement, dropScope, dropid) {
    let dragScope = draggedElement.draggable.scope();
    let dragid = dragScope.dragid;
    if(registeredActions.hasOwnProperty(dragid) && registeredActions[dragid].hasOwnProperty(dropid)) {
      registeredActions[dragid][dropid](dragScope, dropScope);
      return;
    }

    console.error('Called for unregistered action: [drag='+dragid+'][drop='+dropid+']');
  }

  return {
    dispatchAction: dispatchAction,
    registerAction: registerAction,
  };
});
