'use strict';

/**
 * @ngdoc service
 * @name lauEditor.editCanvasManager
 * @description
 * # editCanvasManager
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('editCanvasManager', ['gameObjectManager', '$timeout', 'editorStateManager', function ($gom, $timeout, $esm) {
  ////
  // Public fields
  var scene = new THREE.Scene();

  ////
  // Public functions
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

  function getObjectsUnderCursor() {
    var vector = new THREE.Vector3( mouse.x, mouse.y, 0.5 ).unproject( camera );
    var raycaster = new THREE.Raycaster( camera.position, vector.sub( camera.position ).normalize() );

    function recurse_(objs) {
      let objsUnderCursor = [];
      for(let i in objs) {
        if(objs.hasOwnProperty(i)) {
          objs[i].transform.translateHandle.nome = objs[i].name;
          let handle = objs[i].transform.translateHandle;
          var intersect = raycaster.intersectObject(handle);

          objsUnderCursor = objsUnderCursor.concat(intersect.concat(recurse_(objs[i].children)));
        }
      }
      return objsUnderCursor;
    }

    let gameObjs = $gom.getGameObjects();
    let intersections = recurse_(gameObjs);

    intersections.sort(function(a,b) {
      return a.distance>b.distance;
    });

    return intersections;
  }

  ////
  // Internal fields
  var mouse = new THREE.Vector2();

  ////
  // Internal functions
  var boundingBoxGeometry = new THREE.BoxGeometry( 200, 200, 200 );
  var boundingBoxMaterial = new THREE.MeshBasicMaterial( { color: 0xff0000, wireframe: true } );

  var camera, renderer, planeMesh = null;

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

    ////
    // Object handles
    //
    // Translate
    function translateObject(threeObject) {
      let object = threeObject.object.__lauGameObject;
      return {
        begin: function() {
          let position = object.transform.fields.position;
          this._initialPosition = LAU.Utils.clone(position);
        },
        move: function(moveE) {
          var cameraQuat = camera.getWorldQuaternion();
          var X = new THREE.Vector3(1,0,0).applyQuaternion(cameraQuat);
          var Y = new THREE.Vector3(0,1,0).applyQuaternion(cameraQuat);

          X.multiplyScalar(moveE.movementX * translate_sensitivity);
          Y.multiplyScalar(-moveE.movementY * translate_sensitivity);
          let displacement = X.add(Y);

          $timeout(function() {
            let position = object.transform.fields.position;
            position[0] += displacement.x;
            position[1] += displacement.y;
            position[2] += displacement.z;
          });
        },
        end: function() {
          let position = LAU.Utils.clone(object.transform.fields.position);
          object.transform._editorCommitCallback('position')(this._initialPosition, position);
        }
      }
    }
    //
    // Rotate
    function rotateObject(threeObject) {
      let object = threeObject.object.__lauGameObject;
      let rotation_sensitivity = 0.01;
      let radius = (new THREE.Vector3()).subVectors(threeObject.point, threeObject.object.parent.position);
      return {
        begin: function() {
          let rotation = object.transform.fields.rotation;
          this._initialRotation = LAU.Utils.clone(rotation);
        },
        move: function(moveE) {
          var cameraQuat = camera.getWorldQuaternion();
          var X = new THREE.Vector3(1,0,0).applyQuaternion(cameraQuat);
          var Y = new THREE.Vector3(0,1,0).applyQuaternion(cameraQuat);

          let objRot = new THREE.Quaternion().setFromEuler(threeObject.object.parent.rotation);

          X.multiplyScalar(moveE.movementX);
          Y.multiplyScalar(-moveE.movementY);
          let displacement = X.add(Y);
          let normRadius = (new THREE.Vector3(radius.x,radius.y,radius.z)).normalize();
          normRadius.multiplyScalar(-1);
          let axis = (new THREE.Vector3()).crossVectors(displacement, normRadius);
          axis.normalize();
          let angle = rotation_sensitivity*Math.sqrt(moveE.movementX*moveE.movementX+moveE.movementY*moveE.movementY);
          let newRot = new THREE.Quaternion().setFromAxisAngle(axis, angle);
          let rotProd = (new THREE.Quaternion()).multiplyQuaternions(newRot, objRot);
          let newEuler = (new THREE.Euler()).setFromQuaternion(rotProd);
          radius.applyQuaternion(newRot);

          $timeout(function() {
            let objRot = object.transform.fields.rotation;
            objRot[0] = newEuler.x;
            objRot[1] = newEuler.y;
            objRot[2] = newEuler.z;
          });
        },
        end: function() {
          let rotation = LAU.Utils.clone(object.transform.fields.rotation);
          object.transform._editorCommitCallback('rotation')(this._initialRotation, rotation);
        }
      }
    }

    //
    // Scale
    function scaleObject(threeObject) {
      let object = threeObject.object.__lauGameObject;
      let scale_sensitivity = 0.1;
      return {
        begin: function() {
          let scale = object.transform.fields.scale;
          this._initialScale = LAU.Utils.clone(scale);
        },
        move: function(moveE) {
          var cameraQuat = camera.getWorldQuaternion();
          var X = new THREE.Vector3(1,0,0).applyQuaternion(cameraQuat);
          var Y = new THREE.Vector3(0,1,0).applyQuaternion(cameraQuat);

          let rot = new THREE.Quaternion().setFromEuler(threeObject.object.parent.rotation);
          X.applyQuaternion(rot);
          Y.applyQuaternion(rot);

          X.multiplyScalar(moveE.movementX * scale_sensitivity);
          Y.multiplyScalar(-moveE.movementY * scale_sensitivity);
          let displacement = X.add(Y);

          $timeout(function() {
            let scale = object.transform.fields.scale;
            scale[0] += displacement.x;
            scale[1] += displacement.y;
            scale[2] += displacement.z;
          });
        },
        end: function() {
          let scale = LAU.Utils.clone(object.transform.fields.scale);
          object.transform._editorCommitCallback('scale')(this._initialScale, scale);
        }
      }
    }

    ////
    // Camera handles
    let translateCamera = {
      move: function(moveE) {
        var cameraQuat = camera.getWorldQuaternion();
        var X = new THREE.Vector3(1,0,0).applyQuaternion(cameraQuat);
        var Y = new THREE.Vector3(0,1,0).applyQuaternion(cameraQuat);

        X.multiplyScalar(-moveE.movementX * translate_sensitivity);
        Y.multiplyScalar(moveE.movementY * translate_sensitivity);

        camera.position.add(X.add(Y));
      }
    }

    let rotateCamera = {
      move: function(moveE) {
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
    }

    // Prevent context menu
    $(renderer.domElement).on('contextmenu', function(e){return false;});
    // Setup mousedown
    renderer.domElement.onmousedown = function(downE) {
      downE.preventDefault();
      downE.stopPropagation();

      mouse.x = downE.layerX*2/$canvas.width-1;
      mouse.y = -downE.layerY*2/$canvas.height+1;

      var eventHandler = null;
      switch(downE.button) {
        case 0: // Left
          let hitObjs = getObjectsUnderCursor();
          if(hitObjs.length == 0)
            return;
          $timeout(function(){
            // Select the clicked game object
            $gom.selectGameObject(hitObjs[0].object.__lauGameObject);
          });

          switch($esm.editCanvasInteraction.mode) {
            case 'translate':
              eventHandler = translateObject(hitObjs[0]);
            break;
            case 'rotate':
              eventHandler = rotateObject(hitObjs[0]);
            break;
            case 'scale':
              eventHandler = scaleObject(hitObjs[0]);
            break;
          }
          break;
        case 1: // Middle
          eventHandler = translateCamera;
        break;
        case 2: // Right
          eventHandler = rotateCamera;
        break;
      }

      $canvas.requestPointerLock();
      if(eventHandler.hasOwnProperty('begin'))
        eventHandler.begin();
      renderer.domElement.addEventListener('mousemove', eventHandler.move);
      renderer.domElement.addEventListener('mouseup', function rendererOnMouseUp() {
        renderer.domElement.removeEventListener('mousemove', eventHandler.move);
        renderer.domElement.removeEventListener('mouseup', rendererOnMouseUp);

        document.exitPointerLock();
        if(eventHandler.hasOwnProperty('end'))
          eventHandler.end();
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
      var width = containerElement.width(), height = containerElement.height();
      camera = new THREE.PerspectiveCamera( 75, width / height, 1, 10000 );
      camera.position.z = 1000;

      renderer = new THREE.WebGLRenderer();
      renderer.setSize( width, height );
      renderer.setClearColor(0x393939, 1.0);
      renderer.domElement.setAttribute('class', 'inner-canvas');
      renderer.sortObjects = false;

      containerElement.append(renderer.domElement);
      $canvas = renderer.domElement;

      initHorizontalGrid();
      initCamera();

      animate();
  }

  return {
    scene: scene,
    createGroup: createGroup,
    createMesh: createMesh,
    createBoundingBox: createBoundingBox,
    getObjectsUnderCursor: getObjectsUnderCursor,
    initCanvas: initCanvas,
  };
}]);
