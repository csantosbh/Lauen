'use strict';

/**
 * @ngdoc service
 * @name lauEditor.editCanvasManager
 * @description
 * # editCanvasManager
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('editCanvasManager', function ($rootScope) {
  ////
  // Public fields
  var scene = new THREE.Scene();

  ////
  // Public functions
  function isEditMode() {
    return editMode;
  }

  function getBoundingBox() {
    return new THREE.Mesh(boundingBoxGeometry, boundingBoxMaterial);
  }

  function disableEditMode() {
    editMode = false;
  }

  function enableEditMode() {
    editMode = true;
  }

  ////
  // Internal fields
  var editMode = true;

  ////
  // Internal functions
  var boundingBoxGeometry = new THREE.BoxGeometry( 200, 200, 200 );
  var boundingBoxMaterial = new THREE.MeshBasicMaterial( { color: 0xff0000, wireframe: true } );

  return {
    isEditMode: isEditMode,
    scene: scene,
    getBoundingBox: getBoundingBox,
    disableEditMode: disableEditMode,
    enableEditMode: enableEditMode,
  };
});
