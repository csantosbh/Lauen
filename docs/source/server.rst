Server
******

The server is a module created to perform editor-related tasks that would
otherwise be too complex to perform if it was written in JavaScript, such as
C++ file parsing, filesystem monitoring and shell commands (e.g. for generating
builds). It is written in Python, which have many libraries that simplify these
tasks and many others.

============
Folder structure
============
Any source code related to the server is located inside
``$REPO_ROOT$/src/server``. It is subdivided into the following folders:

* ``build`` Contains our custom :ref:`build system <build-system>`, responsible
  for compiling the project game into all supported targets.
* ``components`` Contains the flyweights of standard components.
* ``io`` The IOEventHandler submodule sets up some IO related RPCs, and the
  Utils submodule has several utility functions for managing files and folders.
* ``parser`` C++ parsing related utilities.
* ``project`` Holds the current project state (component ids, scenes, etc), and sets up some project related :ref:`RPCs <server-rpc>`.

.. _build-system:
=====
Build system
=====
The build system is responsible for generating game binaries. It was created to
automate dependency detection and perform fast, incremental building of the
games. It is used to generate both the final project binaries and the
previewable binaries.

The file ``build/BuildEventHandler.py`` exposes the following functions:

.. function:: BuildProject(platform = 'linux', runGame = True, compilationMode='DEBUG', outputFolder = None)

    Compiles all assets and standard assets and generates a game executable. It
    creates a temporary ``/build/`` folder where object files will be
    generated.

    :param platform: Target platform. Currently supported values are ``linux``, ``windows``, ``nacl`` and ``preview`` (same as nacl, but with preview specific code enabled).
    :param runGame: Runs the game after it is built. Only works with the ``linux`` target.
    :param compilationMode: ``DEBUG`` or ``RELEASE``. Affects compilation flags and linked libraries.
    :param outputFolder: Complete path where the binaries will be generated.


.. function:: ExportGame(platform, buildAndRun, compilationMode, outputFolder)

    Builds and packages the final game, cleaning up temporary files and copying
    assets to the output folder.

    :param platform: Target platform. Currently supported values are ``linux``, ``windows``, ``nacl`` and ``preview`` (same as nacl, but with preview specific code enabled).
    :param buildAndRun: Runs the game after it is built. Only works with the ``linux`` target.
    :param compilationMode:  ``DEBUG`` or ``RELEASE``. Affects compilation flags and linked libraries.
    :param outputFolder: Complete path to output folder.

-----
CPY templates
-----

CPY files are `Mako templates <http://www.makotemplates.org/>`_ of C++ files.
Theywere created to overcome the lack of compile time reflection in C++. They
are rendered by the function ``_renderTemplateSources``, which is called by the
``build/BuildEventHandler.py`` module. The currently implemented CPY files are
the following:

* **Factories.cpy** Contains factories for game objects, components and peekers
  (wrapper classes that handle serialization of components and game objects).

-----
Adding support to new targets
-----

In order to make games compilable to new platforms, follow these steps:

* In the directory ``LauEngine/third_party/cross_compiling``, create a new
  folder with the target name and, inside it, put the object files for all the
  dependency libraries (check out :ref:`dependencies <dependencies>` for a list
  of standard, dynamically linked dependencies).
* Add the new target to the ``build/BuildEventHandler.py`` module:

  * **Create a preprocessor directive** Create a preprocessor directive by
    adding a new key to the dictionary ``platform_preprocessors``.
  * **Setup the required compiler** Cross compilation oftenly requires
    different compilers. Whether the new platform requires a special compiler
    or not, you must add an entry to the ``cxx_compiler`` dictionary specifying
    the required compiler (if it is a non-standard compiler, create a
    :ref:`Config <configpy>` entry to allow users to specify the location for
    their local installation of the required compiler).
  * **Compilation and link flags** Edit the ``_getFlags`` function and add both
    compilation and link flags for the new platform.
  * `(optional)` **Setup the post compilation actions** If the new target
    requires any post compilation steps, like copying dependency libs to the
    build folder, these steps must be specified in the ``_PostExportStep``
    function.
* **Make the new target available on the editor** In the editor, open up the
  view ``views/dialogs/build.html`` and add the new target to the export
  menu. Make sure to use the same alias you used previously to define the new
  target.

======
C++ Parser
======


======
HTTP and WebSocket servers
======
The HTTP server provides the editor with both internal engine data (like HTML
and JavaScript files) and project data (like user assets and NaCl files).

====
IO Utilities
====
documentar funcoes do utils

=====
Project module
=====

.. _configpy:

=====
Project settings and Runtime settings
=====
``Config.py``

.. _server-rpc:

====
RPCs
====

.. _server-events:

======
Socket events
======

