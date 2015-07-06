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

-----
Adding support to new targets
-----

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

