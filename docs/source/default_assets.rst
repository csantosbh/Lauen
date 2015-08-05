Default Assets
**************

The default_assets folder contain all the internal engine code that is shipped
with every game. It performs window management, content loading, game object
instantiation, scene loading, automatic game state serialization and
deserialization, and much more.

=====
Components
=====

Every component must inherit from the base ``Component`` class. It defines the
lowest common denominator interface that all components must have, with the
following methods:

.. cpp:function:: int getId()

   Returns the :ref:`unique numeric id <define-unique-component-id>` associated
   with the component type. Every component of that particular type will return
   the same id whenever this function is called.

.. cpp:function:: void setId(int id)

   Sets the id of this component. Do not call this function; it is used by the
   component factory to set up its id.

.. cpp:function:: virtual void update(float dt)

   Update step. This is component specific and must be implemented by the child
   classes.

   :param dt: The fixed update time step.

-----
Static methods
-----

.. cpp:function:: int getComponentId<T>()

   Returns the unique, numeric id associated with the component of type ``T``.
   All components of this type will return this value when their ``getId()``
   method is called.

=====
Factories
=====

Factories are :ref:`dynamically generated <cpy-templates>` functions that
deserialize game objects and components. They are implemented in the
``Factories.cpy`` template file, which is parsed into ``Factories.cpp`` by the
engine server.

The ``Factories`` class contains factory methods for Components and GameObjects:

.. cpp:function:: shared_ptr<Component> Factories::componentFactory(shared_ptr<GameObject>& gameObj, const rapidjson::Value& serializedComponent)

    Given a serialized version of the component and the game object to which it
    will belong, this function creates an instance of the corresponding
    component class and initializes its public fields with the serialized data.

    :param gameObj: Reference to a shared_ptr pointing to the parent game object.
    :param serializedComponent: A serialized object with values for all public fields of the component.

.. cpp:function:: vector<shared_ptr<GameObject>> Factories::gameObjectFactory(const rapidjson::Document& objects)

    Given a serialized version of an array containing game objects and their
    components, creates a vector of game objects and instantiates each one of
    them, populating them with their components (which are instantiated with
    Factories::componentFactory).

    :param objects: Serialized array of gameobjects, containing their components as well.

The sequence diagram below indicates where and when game objects are
instantiated under normal conditions:

.. uml::

   Window -> Game: init()
   Game -> Factories: gameObjectFactory()
   Factories -> Factories: componentFactory()
   Factories -> GameObject: addComponent()
   Game <- Factories: gameObjects


====
Peekers
====

Peekers are helper classes created with easy component serialization in mind.
Component Peekers are essentially children of the ``Component`` class, with an
internal pointer to an actual component class.

They are implemented as :ref:`CPY files <cpy-templates>`, so that all script
fields can be retrieved in compilation time. Every Component Peeker implement the
following methods:

.. cpp:function:: ComponentPeeker(shared_ptr<T> actualComponent)

   The only available ComponentPeeker constructor. Takes a shared pointer to
   the actual component as a parameter.

   :param actualComponent: Shared pointer to the actual component being wrapped.

.. cpp:function:: const pp::VarDictionary& getCurrentState()

   Returns the current state of the component wrapped by the peeker (that is,
   the current values of all its public fields).

.. cpp:function:: int getInstanceId()

   Returns a unique numeric instance id that's computed sequentially during
   runtime. Every component instance will have a different instance id.


-------
Game Object Peeker
-------
The Game Object peeker is responsible for using these methods in order to
update the Editor state during a preview run. It is implemented as a
specialization of the GameObject class, and is only used in preview mode.

If the game is compiled in preview mode, game object instantiation will occur
as the sequence diagram below illustrates:

.. uml::

   NaClWindow -> Game: init()
   Game -> Factories: gameObjectFactory()
   Factories -> Factories: componentFactory()
   Factories -> GameObjectPeeker: addComponent()
   NaClWindow <- GameObjectPeeker: addComponent()
   NaClWindow -> Editor: broadcasts "component added" event
   Game <- Factories: gameObjects

====
Windows
====

The window manager is responsible for creating windows, listening to resize
events, capturing input data and managing the OpenGL context. There are
currently two types of window classes: Desktop (implemented with `GLFW
<http://www.glfw.org>`_ and compatible with Linux, Windows and Mac) and NaCl
(native client window).

All window management files are located in the ``window`` folder. Internally,
every Window has a pointer to a Game object. In the Desktop window, the game
loop is implemented with `fixed time step
<http://gameprogrammingpatterns.com/game-loop.html>`_ calls to the
``Game::update`` function, and is followed by a call to ``draw(alpha)`` with a
floating point parameter :math:`\alpha \in [0,1)` that can be used to perform
interpolation in the time dimension, avoiding stuttery associated with time
aliasing.

=====
Game class
=====

The Game class is responsible for managing the collection of game objects from
the current scene.

====
Utils
====

The ``utils`` folder contains a collection of utility functions detailed below.

----
IO
----

This module is responsible for performing asynchronous file loading. In order
to use it, you must get the IO singleton with ``IO::getInstance()``, and then
call ``OI::requestFiles()`` with a list of filenames and a callback function:

.. code-block:: c++

   auto& instance = lau::utils::IO::getInstance();
   instance.requestFiles({
     "file1.txt",
     "file2.png",
     "path/to/file3.bin"
   }, [] (std::deque<std::pair<bool, std::vector<uint8_t>>>& input) {
       for(auto& fileStatus: input) {
          if(fileStatus.first) { // .first: true if the file was loaded; false otherwise
              lau << "Buffer size: " << fileStatus.second.size() << endl;
          } else {
              lau << "Could not load this file!" << endl;
          }
       }
   });


The path given to ``getInstance()`` is relative to the folder where the game
executable is located.

.. warning::
   
   When loading text files, make sure to add a ``'\0'`` to the end of the loaded
   buffer if you are going to pass that buffer to some function that assumes
   its existence.

-----
Time
-----

The function ``double lau::utils::time::now()`` returns the number of seconds,
down to microseconds precision, since the Epoch, and is used by the ``Game``
class in the game loop.

=====
OpenGL
=====

-----
VBO
-----

Vertex Buffer Object manager. Handles creation and removal of VBOs from the GPU.

.. cpp:function:: VBO()

   Default constructor. Does not create a VBO on the GPU.

.. cpp:function:: VBO(uint8_t dimensions, std::vector<float>& vertices, std::vector<int>& indices)

   Creates a VBO on the GPU.

   :param dimensions: Number of dimensions of each vertex. Must be 1, 2, 3 or 4.
   :param vertices: Collection of vertices tightly packed.
   :param indices: Collection of vertex indices, used to draw polygon primitives.

.. cpp:function:: void bindVertexToAttribute(GLuint attributeId)

   Given a shader attribute id, bind the vertex buffer to that attribute.

   :param attributeId: TODO linkar com funcao da classe shader que retorna attrids

.. cpp:function:: void bindForDrawing(GLuint attributeId)

   Binds the VBO so it can be rendered by a shader program.

