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
