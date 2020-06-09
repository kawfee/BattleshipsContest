# BattleshipsContest
Code for running battleships contests

We are initially going to create a controller that can communicate with one or more clients via UNIX domain sockets to run a set of mock battleships contests.
* The controller starts one or more player AIs (the controller's clients)
* The controller communicates all necessary information to the AIs via UNIX domain sockets and reads responses.
* If a client "breaks the contest rules", the client forfeits the current round.
** Failure to respond within a certain amount of time (command-line parameterizable)
** Return an invalid response.

Why UNIX domain sockets? Because inter-process communication over UNIX domain sockets is 
* Portable
* Crazy faster than TCP for same-host IPC
