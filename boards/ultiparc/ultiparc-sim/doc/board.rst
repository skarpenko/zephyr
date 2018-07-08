.. _ultiparc-sim:

Ultiparc Emulation (SystemC)
############################

Overview
********

The Ultiparc-sim board configuration is used to emulate the Ultiparc (MIPS-I)
architecture.

Programming and Debugging
*************************

Applications for the ``ultiparc-sim`` board configuration can be built and run in
the usual way for emulated boards (see :ref:`build_an_application` and
:ref:`application_run` for more details).

Flashing
========

While this board is emulated and you can't "flash" it, you can use this
configuration to run basic Zephyr applications and kernel tests in the
emulated environment. For example, with the :ref:`synchronization_sample`:

.. zephyr-app-commands::
   :zephyr-app: samples/synchronization
   :host-os: unix
   :board: ultiparc-sim
   :goals: run

This will build an image with the synchronization sample app, boot it using
Ultiparc system model, and display the following console output:

.. code-block:: console

        ***** Booting Zephyr OS v1.12.0-652-g2dc28b4 *****
        threadA: Hello World from ultiparc!
        threadB: Hello World from ultiparc!
        threadA: Hello World from ultiparc!
        threadB: Hello World from ultiparc!
        threadA: Hello World from ultiparc!
        threadB: Hello World from ultiparc!
        threadA: Hello World from ultiparc!
        threadB: Hello World from ultiparc!
        threadA: Hello World from ultiparc!
        threadB: Hello World from ultiparc!

Debugging
=========

Refer to the detailed overview about :ref:`application_debugging`.
