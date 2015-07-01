Server
******

The server is a module created to perform editor-related tasks that would otherwise be too complex to perform if it was written in JavaScript, such as C++ file parsing, filesystem monitoring and shell commands (e.g. for generating builds). It is written in Python, which have many libraries that simplify these tasks and many others.

============
Folder structure
============
Any source code related to the server is located inside `$REPO_ROOT$/src/server`.

.. _server-rpc:

====
RPCs
====

.. _server-events:

======
Socket events
======

