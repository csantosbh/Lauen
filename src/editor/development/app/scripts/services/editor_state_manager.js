'use strict';

/**
 * @ngdoc service
 * @name lauEditor.editorStateManager
 * @description
 * # editorStateManager
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('editorStateManager', function () {
  // Loading screens, toggle edit mode, etc
  var editMode_ = true;
  function isEditMode() {
    return editMode_;
  }
  function disableEditMode() {
    editMode_ = false;
  }
  function enableEditMode() {
    editMode_ = true;
  }

  return {
    isEditMode: isEditMode,
    disableEditMode: disableEditMode,
    enableEditMode: enableEditMode,
  };
});
