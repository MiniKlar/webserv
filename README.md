*This project has been created as part of the 42 curriculum by banne, lomont.*

# webserv

## Description
`webserv` is a small HTTP/1.1 web server written in C++98.

The goal of the project is to understand how a real web server works under the hood by implementing core web features without external frameworks. This server parses a configuration file, opens listening sockets, handles multiple clients, and serves responses according to route configuration.

Main implemented capabilities include:
- Static file serving
- Route-based method restrictions (`GET`, `POST`, `DELETE`)
- File upload and deletion on configured locations
- CGI execution for `.php` and `.bash`
- Custom error pages
- Autoindex directory listing
- HTTP redirections

## Instructions
### 1. Prerequisites
- Linux
- `make`
- C++ compiler compatible with C++98 (`g++-15` by default in the Makefile)
- `curl` (for manual checks and test script)
- `php-cgi` (for PHP CGI routes)
- `/bin/bash` (for Bash CGI routes)

If needed, you can override the compiler:
```bash
make CXX=g++
```

### 2. Build
From the repository root:
```bash
make
```

This generates the executable:
- `./webserv`

Optional build with AddressSanitizer:
```bash
make SANITIZE=1
```

### 3. Run
Run with explicit config file:
```bash
./webserv webserv.conf
```

Or run with default config (when no argument is provided):
```bash
./webserv
```

Default listening port in `webserv.conf`:
- `0.0.0.0:8080`

### 4. Quick checks
Open in browser:
- `http://localhost:8080/`

Run the provided test suite (server must already be running):
```bash
./test_webserv.sh
```

### 5. Clean
```bash
make fclean
```

## Project Layout
- `srcs/`: C++ source files
- `srcs/includes/`: headers
- `www/`: static pages, CGI scripts, uploads folder
- `webserv.conf`: default server configuration
- `test_webserv.sh`: integration test script

## Resources
Classic references used for this topic:
- RFC 7230: Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing
- RFC 7231: Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content
- MDN Web Docs: HTTP overview and status codes
- Nginx documentation (routing/location behavior and server configuration ideas)
- CGI/1.1 specification (RFC 3875)
- `man` pages: `socket`, `bind`, `listen`, `accept`, `epoll`, `fork`, `execve`, `stat`

AI usage in this project:
- AI was used as a support tool for:
  - drafting and improving project documentation (this `README.md`)
  - checking wording clarity in tests and error hints
  - discussing debugging hypotheses for routing/CGI behaviors
- Final design and implementation decisions, code integration, and validation were performed manually by the project author.

## Notes
- The test script targets `localhost:8080`.
- For CGI PHP tests, make sure `php-cgi` is installed and executable.
- Upload/delete behavior depends on the configured location and filesystem permissions.
