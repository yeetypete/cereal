# Project Proposal: Cereal, the real-time serial data visualization utility

## Project Outline:
It is common for Arduino and other embedded systems projects to send collected runtime data to a computer over a serial communication link. Because the collected data is often based on a live event, it is useful to be able to visualize the seraial data in real-time. Currently the most common way of visualizing real-time serial data is by displaying incoming data as newline separated strings printed to the terminal. We propose to expand on this functionality by creating Cereal, a real-time plotting utility for serial data.

Cereal will be launched from the command line by typing `cereal` along with any additional arguments, including the serial port, baud rate, delimiter character, etc. A new window will be launched next to the terminal displaying a live plot of the incoming data. Axis data assignments can be specified from the terminal window and the axes will automatically shrink and expand to match the data bounds.

While the plot is running in a new window, all interactivity with the plot will be done through the command line using a vim style interface. Commands will include setting threshold and manual data collection triggers, and exporting the collected data to a csv file.

The C++ QT5 cross platform application framework, specifically QT Charts will be used to create the plotting portion of Cereal. If necessary, the ncurses C library will be used to allow for non-blocking printing to the terminal window. 

## Team Roles:
We have discussed our project with Prof. Carruthers and have been given permission to work on this project as a team of two. The distribution of tasks are as follows:

Dylan:
1. The command line portion of Cereal, including optional arguments.
2. The command line interface for interacting with the real-time plot.
3. Documentation

Peter:
1. Using QT Charts to enable the live-plotting of incoming serial data.
2. Publishing cereal as an installable package.
3. Documentation
