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

* ``build`` Contains our custom build system, responsible for compiling the
  project game into all supported targets.
* ``components`` Contains the flyweights of standard components.
* ``io`` The IOEventHandler submodule sets up some IO related RPCs, and the
  Utils submodule has several utility functions for managing files and folders.
* ``parser`` C++ parsing related utilities.
* ``project`` Holds the current project state (component ids, scenes, etc), and sets up some project related RPCs.

=====
Build system
=====

-----
CPY templates
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

