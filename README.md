# Cereal, the real-time serial data visualization utility

## To launch the precompiled `cereal` binary:
1. Enter `cereal` in the commad line in the project directory containing `cereal`. Alternatively you can add `cereal` to `$PATH` to allow it to be launched from any directory. 

## To build your own version of `cereal`:

1. Enter `qmake cereal.pro` in the command line in the project directory. Note, you must have `qmake` installed for this command to work. To install `qmake` and other qt5 utilities and libraries enter `sudo apt-get install qt5-default` in the command line. Also install `libqt5charts5-dev` using `sudo apt-get install libqt5charts5-dev`.

2. `qmake` will generate a Makefile that can be used to compile `cereal`. Enter `make` in the project directory to build the application.

3. Launch `cereal` by typing `cereal` in the project directory.

## How to use `cereal`

Please refer to the GitHub wiki found [here](https://github.com/yeetypete/cereal/wiki) for information on how to use `cereal`. An example of a serial signal is provided for you in the cereal_generator directory. It can be compiled using the Arduino IDE.
