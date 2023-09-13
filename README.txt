This is a very simple model of a process manager spawning a child process which in turn spawns subprocesses, both of which print output, which the process manager captures to a file.

The process manager captures output by connecting its child's standard output to a pipe, a standard method, as described here:

http://www.microhowto.info/howto/capture_the_output_of_a_child_process_in_c.html

The model process manager is slightly different, in that it spawns using posix_spawnp, and so does the connecting using spawn file actions.

The only explicit flushing anywhere here is in the app, which flushes its C stdout buffer before spawning the subprocess, so that output from both is correctly interleaved at the file descriptor.

To build, do:

    ./build.sh

To run the app directly, do:

    PATH=build:$PATH build/simple_app

It should produce output like:

    in app
    in echo
    in app
    in echo
    in app
    in echo

The "in app" lines are printed by the app, and the "in echo" lines are printed by subprocesses.

To run the manager, do:

    PATH=build:$PATH build/simple_manager simple_app

This will capture the output of the app, and write it to a file called journal.txt in the current working directory.

The journal should contain the same output as running the app directly.

It is also possible to run this app under a real process manager, systemd.

To install the service definition as user service:

    ln --symbolic --relative simple-app.service ~/.config/systemd/user/simple-app.service
    systemctl --user daemon-reload
    systemctl --user status simple-app

This service definition assumes that this repository is checked out in the user's home directory; modify the paths in it accordingly if that is not the case.

To start the service:

    systemctl --user start simple-app

To inspect the journal:

    journalctl --user --unit simple-app

For me, with systemd 239 on a Rocky 8.7 machine, the journal contains the output of the app, but not the subprocess:

    -- Logs begin at Tue 2023-09-12 17:47:31 BST, end at Wed 2023-09-13 17:23:29 BST. --
    Sep 13 17:22:27 myhost systemd[859344]: Starting Simple App...
    Sep 13 17:22:27 myhost systemd[859344]: Started Simple App.
    Sep 13 17:22:27 myhost simple_app[921892]: in app
    Sep 13 17:22:28 myhost simple_app[921892]: in app
    Sep 13 17:22:29 myhost simple_app[921892]: in app
    Sep 13 17:22:30 myhost simple_app[921892]: in app
    Sep 13 17:22:31 myhost simple_app[921892]: in app
    Sep 13 17:22:32 myhost simple_app[921892]: in app
    Sep 13 17:22:33 myhost simple_app[921892]: in app
    Sep 13 17:22:34 myhost simple_app[921892]: in app
    Sep 13 17:22:35 myhost simple_app[921892]: in app
    Sep 13 17:22:36 myhost simple_app[921892]: in app
    Sep 13 17:22:37 myhost simple_app[921892]: in app
    Sep 13 17:22:38 myhost simple_app[921892]: in app
    Sep 13 17:22:39 myhost simple_app[921892]: in app
    Sep 13 17:22:40 myhost simple_app[921892]: in app
    Sep 13 17:22:41 myhost simple_app[921892]: in app
    Sep 13 17:22:42 myhost simple_app[921892]: in app
    Sep 13 17:22:42 myhost systemd[859344]: Stopping Simple App...
    Sep 13 17:22:42 myhost systemd[859344]: Stopped Simple App.

This suggests that systemd captures output differently to the simple process manager.
