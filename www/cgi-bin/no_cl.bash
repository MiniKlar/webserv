#!/bin/bash
printf "Content-type: text/plain\r\n\r\n"
printf "This response has no Content-Length header. The server must rely on EOF from the CGI script to know when the body ends."
