'use strict';

/**
 * @ngdoc directive
 * @name lauEditor.directive:editCanvas
 * @description
 * # editCanvas
 */
angular.module('lauEditor')
.directive('editCanvas', function () {
  var scene, camera, renderer;
  var geometry, material, mesh;

  function animate() {
    renderer.render(scene, camera);
    requestAnimationFrame(animate);
  }

  return {
    restrict: 'E',
    link: function postLink(scope, element, attrs) {
      // TODO: Auto resize when window resizes
      scene = new THREE.Scene();

      var width = element.width(), height = element.height();
      camera = new THREE.PerspectiveCamera( 75, width / height, 1, 10000 );
      camera.position.z = 1000;

      geometry = new THREE.BoxGeometry( 200, 200, 200 );
      material = new THREE.MeshBasicMaterial( { color: 0xff0000, wireframe: true } );

      mesh = new THREE.Mesh( geometry, material );
      scene.add( mesh );

      renderer = new THREE.WebGLRenderer();
      renderer.setSize( width, height );
      element.append(renderer.domElement);
      animate();
    }
  };
});
