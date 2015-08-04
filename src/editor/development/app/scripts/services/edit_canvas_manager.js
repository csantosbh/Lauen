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

  function createGroup() {
    return new THREE.Group();
  }

  function createMesh(modelPath) {
    var modelGeometry = new THREE.BoxGeometry( 100, 100, 100 ); // TODO load the actual model
    // TODO after that, cache models
    return new THREE.Mesh(modelGeometry, boundingBoxMaterial);
  }

  function createBoundingBox() {
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
    createGroup: createGroup,
    createMesh: createMesh,
    createBoundingBox: createBoundingBox,
    disableEditMode: disableEditMode,
    enableEditMode: enableEditMode,
  };
});
