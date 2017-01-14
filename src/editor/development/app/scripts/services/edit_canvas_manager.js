'use strict';

/**
 * @ngdoc service
 * @name lauEditor.editCanvasManager
 * @description
 * # editCanvasManager
 * Service in the lauEditor.
 */
angular.module('lauEditor').service('editCanvasManager', ['gameObjectManager', '$timeout', 'editorStateManager', '$http', function ($gom, $timeout, $esm, $http) {
  ////
  // Public fields
  var scene = new THREE.Scene();

  ////
  // Public functions
  function createGroup() {
    return new THREE.Group();
  }

  function createMesh(modelPath, callback) {
    let loader = new THREE.JSONLoader();
    loader.load('http://localhost:9002/'+modelPath,
      function onLoad(geometry, materials) {
        var material = new THREE.MeshFaceMaterial( materials );
        let obj = new THREE.Mesh(geometry, material);
        callback(obj);
    });

    // TODO modify THREE.js to return the XMLHttpRequest object so I can
    // cancel() the request if I change the model while still loading a model
  }

  function createAxesHandle() {
    let axesMeshes = [
      new THREE.Mesh(axisCylinderGeometry, axisCylinderMaterials[0]),
      new THREE.Mesh(axisCylinderGeometry, axisCylinderMaterials[1]),
      new THREE.Mesh(axisCylinderGeometry, axisCylinderMaterials[2])
    ];

    axesMeshes[0].rotation.z = -90*Math.PI/180.0;
    axesMeshes[2].rotation.x = 90*Math.PI/180.0;

    let offset = 4.0/2; // Cylinder height/2
    axesMeshes[0].position.x = 4.0/2;
    axesMeshes[1].position.y = 4.0/2;
    axesMeshes[2].position.z = 4.0/2;

    // Whenever the __lauGameObject field is requested, return the field
    // originally defined in its parent, the axes object.
    function defineLauGameObjectGetter(axis) {
      Object.defineProperty(axis, '__lauGameObject', {
        get: function() {
          return this.parent.__lauGameObject;
        }
      });
    }

    let axes = new THREE.Group();
    for(let i = 0; i < axesMeshes.length; ++i) {
      defineLauGameObjectGetter(axesMeshes[i]);
      axes.add(axesMeshes[i]);
    }
    return axes;
  }

  function getObjectsUnderCursor() {
    var vector = new THREE.Vector3( mouse.x, mouse.y, 0.5 ).unproject( camera );
    var raycaster = new THREE.Raycaster( camera.position, vector.sub( camera.position ).normalize() );

    function recurse_(objs) {
      let objsUnderCursor = [];
      for(let i in objs) {
        if(objs.hasOwnProperty(i)) {
          let handle = objs[i].transform.canvasHandle;
          var intersect = raycaster.intersectObject(handle, true);

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
  var axisCylinderGeometry = new THREE.CylinderGeometry( 0.1, 0.1, 4, 32 );
  var axisCylinderMaterials = [
    new THREE.MeshBasicMaterial({
        color: 0xff0000, depthTest: false,
        depthWrite: false, transparent: true
    }),
    new THREE.MeshBasicMaterial({
        color: 0x00ff00, depthTest: false,
        depthWrite: false, transparent: true
    }),
    new THREE.MeshBasicMaterial({
        color: 0x0000ff, depthTest: false,
        depthWrite: false, transparent: true
    }),
  ];

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
    var rotation_pivot_distance = 20.00;
    var translate_sensitivity = 0.10;
    var scroll_sensitivity = 0.05;

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
            object.transform.propagatePositionChange();
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
      let rootObject = threeObject.object;
      while(rootObject.parent.type != 'Scene')
        rootObject = rootObject.parent;
      let radius = (new THREE.Vector3()).subVectors(threeObject.point, rootObject.position);
      return {
        begin: function() {
          let rotation = object.transform.fields.rotation;
          this._initialRotation = LAU.Utils.clone(rotation);
        },
        move: function(moveE) {
          var cameraQuat = camera.getWorldQuaternion();
          var X = new THREE.Vector3(1,0,0).applyQuaternion(cameraQuat);
          var Y = new THREE.Vector3(0,1,0).applyQuaternion(cameraQuat);

          let objRot = new THREE.Quaternion().setFromEuler(rootObject.rotation);

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
            object.transform.propagateRotationChange();
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
      let rootObject = threeObject.object;
      while(rootObject.parent.type != 'Scene')
        rootObject = rootObject.parent;
      return {
        begin: function() {
          let scale = object.transform.fields.scale;
          this._initialScale = LAU.Utils.clone(scale);
        },
        move: function(moveE) {
          var cameraQuat = camera.getWorldQuaternion();
          var X = new THREE.Vector3(1,0,0).applyQuaternion(cameraQuat);
          var Y = new THREE.Vector3(0,1,0).applyQuaternion(cameraQuat);

          let rot = new THREE.Quaternion().setFromEuler(rootObject.rotation);
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
            object.transform.propagateScaleChange();
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
        var pivot = new THREE.Vector3(0,0,-rotation_pivot_distance);
        var rotationAxis = new THREE.Vector3().crossVectors(
          new THREE.Vector3(0,0,-1.0).normalize(),
          new THREE.Vector3(moveE.movementX, -moveE.movementY, -1.0).normalize()
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
    // Setup scropp
    renderer.domElement.onmousewheel= function(scrollE) {
      let lookDir = camera.getWorldDirection().normalize();
      let scrollFactor = scrollE.wheelDelta * scroll_sensitivity;
      lookDir.multiplyScalar(scrollFactor);
      camera.position.add(lookDir);
      rotation_pivot_distance -= scrollFactor;
    };
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
      camera = new THREE.PerspectiveCamera( 50, width / height, 0.1, 2000 );
      camera.position.z = 10;

      renderer = new THREE.WebGLRenderer({antialias: true});
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
    createAxesHandle: createAxesHandle,
    getObjectsUnderCursor: getObjectsUnderCursor,
    initCanvas: initCanvas,
  };
}]);
