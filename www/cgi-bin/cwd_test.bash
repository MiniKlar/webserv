#!/bin/bash
printf "Content-type: text/plain\r\n\r\n"
echo "--- CGI CWD TEST ---"
echo "Current directory: $(pwd)"
echo "Files in current directory:"
ls -1
