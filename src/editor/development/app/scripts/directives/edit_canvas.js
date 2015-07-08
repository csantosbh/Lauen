'use strict';
var $canvas;

/**
 * @ngdoc directive
 * @name lauEditor.directive:editCanvas
 * @description
 * # editCanvas
 */
angular.module('lauEditor').directive('editCanvas', ['$timeout', function ($timeout) {
  var scene, camera, renderer, planeMesh = null;
  var geometry, material;

  function animate() {
    renderer.render(scene, camera);
    requestAnimationFrame(animate);
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

  $event.listen('paneResized', function(resizeEvent) {
    if(resizeEvent.pane == 'canvas-container') {
      renderer.setSize( resizeEvent.size.width, resizeEvent.size.height );
      camera.aspect = resizeEvent.size.width/resizeEvent.size.height;
      camera.updateProjectionMatrix();
    }
  });

  function initCanvas(containerElement) {
      scene = new THREE.Scene();

      var width = containerElement.width(), height = containerElement.height();
      camera = new THREE.PerspectiveCamera( 75, width / height, 1, 10000 );
      camera.position.z = 1000;

      renderer = new THREE.WebGLRenderer();
      renderer.setSize( width, height );
      renderer.setClearColor(0x393939, 1.0);
      renderer.domElement.setAttribute('class', 'inner-canvas');

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
      };

      // TODO: Auto resize when window resizes
      initCanvas(element);

      geometry = new THREE.BoxGeometry( 200, 200, 200 );
      material = new THREE.MeshBasicMaterial( { color: 0xff0000, wireframe: true } );

      var trackedGameObjects = [];

      function trackPositionalComponent(gameObjectData) {
        var gameObj = gameObjectData.obj;

        // TODO maybe have a namespace that converts from a friendly string name to component id?
        var transformComponent = gameObj.getComponentById(0);
        if(transformComponent != null) {
          gameObjectData.mesh = new THREE.Mesh( geometry, material );

          // Initialize positional component
          gameObjectData.mesh.position.fromArray(transformComponent.position);
          gameObjectData.mesh.rotation.fromArray(transformComponent.rotation);
          gameObjectData.mesh.scale.fromArray(transformComponent.scale);

          scene.add( gameObjectData.mesh );

          // Watch for transform updates
          // TODO use Object.observe instead of scope.$watch! VERY IMPORTANT!
          scope.$watch(function() {
            return transformComponent;
          }, function(after, before) {
            gameObjectData.mesh.position.fromArray(transformComponent.position);
            gameObjectData.mesh.rotation.fromArray(transformComponent.rotation);
            gameObjectData.mesh.scale.fromArray(transformComponent.scale);
          }, true);
        }
      }

      function trackGameObject(gameObj) {
        if(!scope.canvas.editMode) // Ignore gameobjects created
          return;                  // during preview mode

        var gameObjectData = {
          mesh: null,
          obj: gameObj
        };

        trackPositionalComponent(gameObjectData);
        // TODO track mesh/color/material/etc components

        // @@ Watch for changes in the component list
        scope.$watchCollection(function() {
          return gameObj.components;
        }, function() {
          // TODO handle case when the positional component was removed
          if(gameObjectData.mesh == null) {
            // Positional component may have been added
            trackPositionalComponent(gameObjectData);
          }
          // TODO watch for mesh/color/material/etc updates
        });

        trackedGameObjects.push(gameObjectData);
      }

      function forgetGameObject(gameObj) {
        if(!scope.canvas.editMode) // Ignore gameobjects created
          return;                  // during preview mode

        // Find removed game object
        for(var i = 0; i < trackedGameObjects.length; ++i) {
          if(trackedGameObjects[i].obj == gameObj) {
            scene.remove(trackedGameObjects[i].mesh);
            trackedGameObjects.splice(i, 1);
            return;
          }
        }
      }

      scope.EditCanvas = {
        trackGameObject: trackGameObject,
        forgetGameObject: forgetGameObject
      };

      animate();
    }
  };
}]);
