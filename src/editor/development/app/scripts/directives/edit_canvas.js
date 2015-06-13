'use strict';
var $canvas;

/**
 * @ngdoc directive
 * @name lauEditor.directive:editCanvas
 * @description
 * # editCanvas
 */
angular.module('lauEditor')
.directive('editCanvas', function ($timeout) {
  var scene, camera, renderer, planeMesh = null;
  var geometry, material;

  var gameObjects = [];

  function animate() {
    renderer.render(scene, camera);
    requestAnimationFrame(animate);
  }

  function registerGameObject(transformComponent) {
    var gameObjectData = {
      mesh: new THREE.Mesh( geometry, material ),
    };
    // Bind transform to UI
    transformComponent.rotation = gameObjectData.mesh.rotation;
    transformComponent.position = gameObjectData.mesh.position;
    transformComponent.scale = gameObjectData.mesh.scale;

    scene.add( gameObjectData.mesh );
    gameObjects.push(gameObjectData);
  }

  function addGameObject($scope, gameObjectId) {
    var components = $scope.gameObjects[gameObjectId].components;
    for(var i = 0; i < components.length; ++i) {
      // Only visualize the current game object IF it has a transform component
      if(components[i].type === 'transform') {
        registerGameObject(components[i]);
        break;
      }
    }
  }

  function initHorizontalGrid() {
      planeMesh = new THREE.GridHelper(1000.0, 100);
      planeMesh.setColors(0x4a4a4a, 0x4a4a4a);
      scene.add( planeMesh );
  }
  function initCamera() {
    // TODO: Handle scroll (maybe change pivot and sensitivities)
    var pivot_distance = 1000.00;
    var translate_sensitivity = 3.0;

    function translateCamera(moveE) {
      var cameraQuat = camera.getWorldQuaternion();
      var X = new THREE.Vector3(1,0,0).applyQuaternion(cameraQuat);
      var Y = new THREE.Vector3(0,1,0).applyQuaternion(cameraQuat);

      X.multiplyScalar(-moveE.movementX * translate_sensitivity);
      Y.multiplyScalar(moveE.movementY * translate_sensitivity);

      camera.position.add(X.add(Y));
    }

    function rotateCamera(moveE) {
      var focalLength = 10.0;

      var pivot = new THREE.Vector3(0,0,-pivot_distance);
      var rotationAxis = new THREE.Vector3().crossVectors(
        new THREE.Vector3(0,0,-focalLength).normalize(),
        new THREE.Vector3(moveE.movementX, -moveE.movementY, -focalLength).normalize()
      ).normalize();
      var movement = Math.sqrt(moveE.movementY*moveE.movementY +
                               moveE.movementX*moveE.movementX);
      var rotationAngle = Math.atan2(movement, 100.0);
      var rotation = new THREE.Quaternion().setFromAxisAngle(
        rotationAxis, rotationAngle
      );

      // Rotate/translate camera
      var origPivot = pivot.clone().applyQuaternion(camera.quaternion);
      camera.quaternion.multiply(rotation);
      var rotatedPivot = pivot.clone().applyQuaternion(camera.quaternion);
      camera.position.add(origPivot).sub(rotatedPivot);
    }

    // Prevent context menu
    $(renderer.domElement).on('contextmenu', function(e){return false;});
    // Setup mousedown
    renderer.domElement.onmousedown = function(downE) {
      var eventHandler = null;
      switch(downE.button) {
        case 0: // Left
          return;
        case 1: // Middle
          eventHandler = translateCamera;
        break;
        case 2: // Right
          eventHandler = rotateCamera;
        break;
      }

      $canvas.requestPointerLock();
      renderer.domElement.addEventListener('mousemove', eventHandler);
      renderer.domElement.addEventListener('mouseup', function() {
        renderer.domElement.removeEventListener('mousemove', eventHandler);
        document.exitPointerLock();
      });
    };
  }

  function initCanvas(containerElement) {
      scene = new THREE.Scene();

      var width = containerElement.width(), height = containerElement.height();
      camera = new THREE.PerspectiveCamera( 75, width / height, 1, 10000 );
      camera.position.z = 1000;

      renderer = new THREE.WebGLRenderer();
      renderer.setSize( width, height );
      renderer.setClearColor(0x393939, 1.0);
      containerElement.append(renderer.domElement);
      $canvas = renderer.domElement;

      initHorizontalGrid();
      initCamera();
  }

  return {
    restrict: 'E',
    link: function postLink(scope, element) {
      scope.canvas = {
        editMode: true,
        toggleEditMode: function() {
          this.editMode = !this.editMode;
        }
      };

      // TODO: Auto resize when window resizes
      initCanvas(element);

      geometry = new THREE.BoxGeometry( 200, 200, 200 );
      material = new THREE.MeshBasicMaterial( { color: 0xff0000, wireframe: true } );

      $event.listen('transformComponentAdded', function(evData) {
        registerGameObject(evData);
      });

      animate();
    }
  };
});
