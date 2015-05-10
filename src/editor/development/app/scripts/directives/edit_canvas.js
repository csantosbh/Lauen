'use strict';
var $canvas;

/**
 * @ngdoc directive
 * @name lauEditor.directive:editCanvas
 * @description
 * # editCanvas
 */
angular.module('lauEditor')
.directive('editCanvas', function () {
  var scene, camera, renderer, planeMesh = null;
  var geometry, material;

  var gameObjects = [];

  function animate() {
    renderer.render(scene, camera);
    requestAnimationFrame(animate);
  }

  function addGameObject($scope, gameObjectId) {
    var components = $scope.gameObjects[gameObjectId].components;
    for(var i = 0; i < components.length; ++i) {
      if(components[i].type === 'transform') {
        // Only visualize the current game object IF it has a transform component
        var gameObjectData = {
          mesh: new THREE.Mesh( geometry, material ),
        };

        // Bind transform to UI
        components[i].rotation = gameObjectData.mesh.rotation;
        components[i].position = gameObjectData.mesh.position;
        components[i].scale = gameObjectData.mesh.scale;

        scene.add( gameObjectData.mesh );
        gameObjects.push(gameObjectData);
        break;
      }
    }
  }

  function initCanvas(containerElement) {
      scene = new THREE.Scene();

      var width = containerElement.width(), height = containerElement.height();
      camera = new THREE.PerspectiveCamera( 75, width / height, 1, 10000 );
      camera.position.z = 1000;

      renderer = new THREE.WebGLRenderer();
      renderer.setSize( width, height );
      containerElement.append(renderer.domElement);
      planeMesh = new THREE.GridHelper(1000.0, 100);
      scene.add( planeMesh );
      $canvas = renderer.domElement;
  }

  return {
    restrict: 'E',
    link: function postLink(scope, element) {
      // TODO: Auto resize when window resizes
      initCanvas(element);

      geometry = new THREE.BoxGeometry( 200, 200, 200 );
      material = new THREE.MeshBasicMaterial( { color: 0xff0000, wireframe: true } );

      $event.listen('gameObjectCreated', function(gameObjectId) {
        addGameObject(scope, gameObjectId);
      });

      animate();
    }
  };
});
