'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:editCanvas
 * @description
 * # editCanvas
 */
angular.module('lauEditor')
.directive('editCanvas', function ($http, $q) {
  var scene, camera, renderer;
  var geometry, material;
  var planeGeometry = new THREE.PlaneBufferGeometry(2.0, 2.0);;
  console.log(planeGeometry);

  var gameObjects = [];

  function animate() {
    renderer.render(scene, camera);
    requestAnimationFrame(animate);
  }

  function addGameObject($scope, gameObjectId) {
    var components = $scope.gameObjects[gameObjectId].components;
    for(var i = 0; i < components.length; ++i) {
      if(components[i].type == 'transform') {
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

      // Horizontal plane shader
      var vsPromise = $http.get('shaders/horizontal_plane.vs');
      var fsPromise = $http.get('shaders/horizontal_plane.fs');
      $q.all([vsPromise, fsPromise]).then(function(shaders) {
        var vs = shaders[0].data, fs = shaders[1].data;

        var mat = new THREE.ShaderMaterial({
          uniforms: {
            color: {type: 'f', value: 1.0}
          },
          transparent: true,
          vertexShader: vs,
          fragmentShader: fs
        });

        var obj = new THREE.Mesh(planeGeometry, mat);
        scene.add( obj );
      });
  }

  return {
    restrict: 'E',
    link: function postLink(scope, element, attrs) {
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
