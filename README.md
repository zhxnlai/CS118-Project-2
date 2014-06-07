Introduction
===============

This project is a *Simple Window-based Reliable Data Transfer in C/C++*. It utilizes Go-Back-N protocol to achieve reliable data transfer on top of UDP socket and uses [json11](https://github.com/dropbox/json11) for packet serialization.

Requirement:
===
This project requires the complier to support C++11. Here is [a guide](http://ubuntuhandbook.org/index.php/2013/08/install-gcc-4-8-via-ppa-in-ubuntu-12-04-13-04/) to update gcc on ubuntu.

Usage:
===
In the root directory, run `make` command in the terminal.

This will generate two folders, *server_folder* and *client_folder*, as well as two programs *server* and *client* inside them.

Inside *server_folder*, run

~~~
> server < portnumber > [ CWnd Pl PC ]
~~~

This will run the *server* program and wait for requests on the `portnumber` specified by the user.

Inside *client_folder*, run

~~~
> client < server hostname >< server portnumber >< filename > [ Pl PC ]
~~~

This will run the *client* program which first try to handshake with server and request the file in *server_folder* specified by `filename`. 

The parameters in [ ] are optional. `Cwnd` is the window size which is default to 5. `Pl` is the probability for packet loss and `Pc` is the probability for packet corruption from 0 to 1.

Contributors:
===

Zhixuan Lai

Zhetian Sun

