Standard Components
********************

LauEngine comes with a set of pre-defined components that implement elements
commonly required by games, like a Transform, renderers etc. This section
documents all these common components.

======
Lau::Transform
======

The Transform component is required by all components that perform any kind of
rendering, since it specifies the position, scale and rotation of the object
being rendered in the 3D space.

.. warning::
    Although a game object doesn't have to have a Transform component, you must
    make sure never to have more than one Transform per game object.

------------
Public members
------------

.. cpp:member:: Eigen::Vector3f position

   3D position of the game object with respect to the world reference.

.. cpp:member:: Eigen::Quaternionf rotation

   Game object orientation with respect to the world reference, defined as a
   quaternion.

.. cpp:member:: Eigen::Vector3f position

   3D position of the game object with respect to the world reference.

TODO exemplo pegando um transform e usando ele (mostrar namespace)

----
Mesh
----
The Mesh component is responsible for managing mesh data, but not for rendering
it (see :ref:`Mesh Renderer <mesh-renderer-component>` for that).  It loads
mesh files, which are stored in the internal lmf file (currently, the same as
`ThreeJS geometry format 4
<https://github.com/mrdoob/three.js/wiki/JSON-Geometry-format-4>`_), pushes it
to the GPU through a `Vertex Buffer Object
<http://www.songho.ca/opengl/gl_vbo.html>`_, and deletes that buffer when the
Mesh component is destroyed.

.. warning::

   You can only have one Mesh component per game object.

TODO exemplo pegando um mesh e usando ele (mostrar namespace)

.. _mesh-renderer-component:
----
Mesh Renderer
----

The Mesh Renderer is responsible for applying a shader program to an already
existing Mesh component in the current game object. If the game object has no
meshes, nothing will be drawn.

.. warning::

   You can only have one Mesh Renderer component per game object.

TODO exemplo pegando um mesh_renderer e usando ele (mostrar namespace)
