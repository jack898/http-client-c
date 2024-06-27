# Cross-platform HTTP Client
A simple HTTP client, compatible with both Windows and Linux*. Makes an HTTP GET request to user-specified host and port using HTTP 1.0,
and outputs the HTTP response (or timeout error) after 5 seconds. Mostly just a learning project for me but feel free to fork as desired!

# Installation/Building
* Download the files (client.c, client.h, Makefile)
* Open a terminal instance in this directory
* Type "make" and the httpClient executable will be built

# Usage
./httpClient [host] [port]
  - Example: ./httpClient example.com 80

# Acknowledgments
[Beej's Network guide](https://beej.us/guide/bgnet/): This guide was super helpful to learn the C sockets library and network programming

# Possible future additions (or ideas, if you want to practice/learn!)
* Add support for HTTP/1.1
* Allow user to specify request URL path





**Tested on Windows 10 Home and Ubuntu 22.04.3 LTS*
