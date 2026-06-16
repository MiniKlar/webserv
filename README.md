# webserv

> An HTTP/1.1 web server written from scratch in C++98, configured the nginx way.

`webserv` parses an nginx-style configuration file, opens listening sockets, and
serves multiple clients concurrently using non-blocking I/O multiplexed with
`epoll`. It handles static files, file uploads, deletions, CGI execution, custom
error pages, autoindex and HTTP redirections — without any external framework.

*A 42 school project by [lomont](mailto:omontloris@gmail.com) and
[banne](mailto:baidyanne9@gmail.com).*

---

## 🇬🇧 English

### Principle

A web server is, at its core, a program that listens on a TCP socket, accepts
client connections, reads raw HTTP requests, and writes back HTTP responses.
`webserv` implements that loop in C++98:

1. It reads a configuration file (nginx-inspired `server { ... }` blocks).
2. It opens a listening socket for each `listen` directive (`bind` + `listen`).
3. It registers every socket in an `epoll` instance and runs a single-threaded
   event loop (`epoll_wait`). Every file descriptor is non-blocking, so a slow
   or stalled client never blocks the others.
4. For each ready connection it parses the HTTP request, matches it against the
   right `location`, produces a response (static file, upload, deletion, CGI
   output, redirection or error page) and writes it back.

The whole point of the project is to understand what really happens under the
hood of a real web server, by reimplementing it.

### Features

- **HTTP/1.1** request parsing and response generation.
- **Methods**: `GET`, `POST`, `DELETE` — restrictable per `location`.
- **Static file serving** with configurable `root` and `index` files.
- **File upload** (`POST`) and **deletion** (`DELETE`) on configured locations
  via `upload_store`.
- **CGI execution** through `fork` + `execve`, dispatched by file extension
  (`cgi_pass`). The repo ships Bash (`/bin/bash`) and PHP (`php-cgi`) scripts.
- **Autoindex**: automatic directory listing when `autoindex on`.
- **Custom error pages** via `error_page`.
- **HTTP redirections** via `return` (e.g. `return 301 /`).
- **client_max_body_size** enforcement (supports unit suffixes such as `1G`).
- **Non-blocking I/O** multiplexed with a single `epoll` event loop.

### Configuration

The server is driven by an nginx-style configuration file. Supported directives:

| Scope     | Directive              | Description                                            |
|-----------|------------------------|--------------------------------------------------------|
| server    | `listen`               | Interface and port to bind (`host:port`).              |
| server    | `client_max_body_size` | Max request body size (accepts `K`/`M`/`G` suffixes).  |
| server    | `error_page`           | Map status codes to a custom error page.               |
| location  | `methods`              | Allowed HTTP methods for this route.                   |
| location  | `root`                 | Filesystem root for this route.                        |
| location  | `index`                | Default index file(s).                                 |
| location  | `autoindex`            | `on`/`off` directory listing.                          |
| location  | `cgi_pass`             | Bind a file extension to a CGI interpreter.            |
| location  | `upload_store`         | Destination directory for uploaded files.              |
| location  | `return`               | HTTP redirection (status + target).                    |

Minimal example (extract from [`webserv.conf`](webserv.conf)):

```nginx
server {
    listen 0.0.0.0:8080;
    client_max_body_size 1G;

    location / {
        methods GET;
        root /www;
        index default_index.html;
        autoindex on;
    }

    location /uploads {
        methods GET POST DELETE;
        root /www;
        upload_store /www/uploads/;
    }

    location /cgi-bin {
        methods GET POST;
        root /www;
        cgi_pass .bash /bin/bash;
        cgi_pass .php /usr/bin/php-cgi;
    }
}

error_page 500 502 503 504 /www/default_error.html;
error_page 403 404 413 /www/default_error.html;
```

### Build & Run

**Prerequisites**: Linux, `make`, a C++98-capable compiler (`g++-15` by default),
plus `php-cgi` and `/bin/bash` for the CGI routes.

Build from the repository root:

```bash
make
```

This produces the `./webserv` executable. You can override the compiler or build
with AddressSanitizer:

```bash
make CXX=g++       # use another compiler
make SANITIZE=1    # build with -fsanitize=address
```

Run with a configuration file (falls back to `webserv.conf` when no argument is
given):

```bash
./webserv webserv.conf
# or simply
./webserv
```

The default config listens on `0.0.0.0:8080`. Test it:

```bash
# In a browser
http://localhost:8080/

# With curl
curl http://localhost:8080/
curl -X POST --data-binary @file.txt http://localhost:8080/uploads/file.txt
curl -X DELETE http://localhost:8080/uploads/file.txt
curl http://localhost:8080/cgi-bin/test.php
```

A provided integration script exercises the running server:

```bash
./test_webserv.sh         # server must already be running
```

Clean up with `make fclean`.

### What I learned

- **Sockets & TCP/IP**: creating, binding and listening on sockets, accepting
  connections, and the lifecycle of a client connection.
- **The HTTP protocol**: parsing request lines, headers and bodies, building
  valid responses, and handling status codes (RFC 7230/7231).
- **Non-blocking I/O & multiplexing**: driving many simultaneous connections
  from a single thread with `epoll`, never blocking on a single client.
- **Configuration parsing**: tokenizing and interpreting an nginx-style config
  into server/location structures.
- **CGI**: spawning interpreters with `fork`/`execve`, wiring environment
  variables and pipes, and streaming their output back as an HTTP response.
- **Server architecture**: structuring the read/process/write cycle, error
  handling and resource management in C++98.

---

## 🇫🇷 Français

### Principe

Un serveur web est, fondamentalement, un programme qui écoute sur une socket
TCP, accepte des connexions clientes, lit des requêtes HTTP brutes et renvoie
des réponses HTTP. `webserv` implémente cette boucle en C++98 :

1. Il lit un fichier de configuration (blocs `server { ... }` à la nginx).
2. Il ouvre une socket d'écoute pour chaque directive `listen` (`bind` +
   `listen`).
3. Il enregistre chaque socket dans une instance `epoll` et exécute une boucle
   d'événements mono-thread (`epoll_wait`). Tous les descripteurs sont non
   bloquants : un client lent ne bloque jamais les autres.
4. Pour chaque connexion prête, il parse la requête HTTP, la fait correspondre à
   la bonne `location`, produit une réponse (fichier statique, upload,
   suppression, sortie CGI, redirection ou page d'erreur) et la renvoie.

Tout l'intérêt du projet est de comprendre ce qui se passe réellement sous le
capot d'un vrai serveur web, en le réimplémentant.

### Fonctionnalités

- **HTTP/1.1** : parsing des requêtes et génération des réponses.
- **Méthodes** : `GET`, `POST`, `DELETE` — restreignables par `location`.
- **Service de fichiers statiques** avec `root` et fichiers `index`
  configurables.
- **Upload** (`POST`) et **suppression** (`DELETE`) de fichiers sur les
  locations configurées via `upload_store`.
- **CGI** via `fork` + `execve`, dispatché par extension de fichier
  (`cgi_pass`). Le dépôt fournit des scripts Bash (`/bin/bash`) et PHP
  (`php-cgi`).
- **Autoindex** : listing automatique de répertoire quand `autoindex on`.
- **Pages d'erreur personnalisées** via `error_page`.
- **Redirections HTTP** via `return` (ex. `return 301 /`).
- **client_max_body_size** : limite de taille du corps (suffixes `1G`, etc.).
- **I/O non bloquantes** multiplexées avec une seule boucle `epoll`.

### Configuration

Le serveur est piloté par un fichier de configuration façon nginx. Directives
supportées :

| Portée    | Directive              | Description                                              |
|-----------|------------------------|---------------------------------------------------------|
| server    | `listen`               | Interface et port à écouter (`host:port`).              |
| server    | `client_max_body_size` | Taille max du corps de requête (suffixes `K`/`M`/`G`). |
| server    | `error_page`           | Associe des codes de statut à une page d'erreur.        |
| location  | `methods`              | Méthodes HTTP autorisées pour cette route.              |
| location  | `root`                 | Racine sur le disque pour cette route.                  |
| location  | `index`                | Fichier(s) index par défaut.                            |
| location  | `autoindex`            | Listing de répertoire `on`/`off`.                       |
| location  | `cgi_pass`             | Associe une extension à un interpréteur CGI.            |
| location  | `upload_store`         | Répertoire de destination des fichiers uploadés.        |
| location  | `return`               | Redirection HTTP (statut + cible).                      |

Exemple minimal (extrait de [`webserv.conf`](webserv.conf)) :

```nginx
server {
    listen 0.0.0.0:8080;
    client_max_body_size 1G;

    location / {
        methods GET;
        root /www;
        index default_index.html;
        autoindex on;
    }

    location /uploads {
        methods GET POST DELETE;
        root /www;
        upload_store /www/uploads/;
    }

    location /cgi-bin {
        methods GET POST;
        root /www;
        cgi_pass .bash /bin/bash;
        cgi_pass .php /usr/bin/php-cgi;
    }
}

error_page 500 502 503 504 /www/default_error.html;
error_page 403 404 413 /www/default_error.html;
```

### Compilation & Lancement

**Prérequis** : Linux, `make`, un compilateur compatible C++98 (`g++-15` par
défaut), ainsi que `php-cgi` et `/bin/bash` pour les routes CGI.

Compiler depuis la racine du dépôt :

```bash
make
```

Cela produit l'exécutable `./webserv`. On peut surcharger le compilateur ou
compiler avec AddressSanitizer :

```bash
make CXX=g++       # utiliser un autre compilateur
make SANITIZE=1    # compiler avec -fsanitize=address
```

Lancer avec un fichier de configuration (revient à `webserv.conf` si aucun
argument n'est fourni) :

```bash
./webserv webserv.conf
# ou simplement
./webserv
```

La configuration par défaut écoute sur `0.0.0.0:8080`. Pour tester :

```bash
# Dans un navigateur
http://localhost:8080/

# Avec curl
curl http://localhost:8080/
curl -X POST --data-binary @file.txt http://localhost:8080/uploads/file.txt
curl -X DELETE http://localhost:8080/uploads/file.txt
curl http://localhost:8080/cgi-bin/test.php
```

Un script d'intégration fourni teste le serveur en fonctionnement :

```bash
./test_webserv.sh         # le serveur doit déjà tourner
```

Nettoyer avec `make fclean`.

### Ce que ça m'a apporté

- **Sockets & TCP/IP** : création, `bind` et écoute des sockets, acceptation des
  connexions, et cycle de vie d'une connexion cliente.
- **Le protocole HTTP** : parsing de la ligne de requête, des en-têtes et du
  corps, construction de réponses valides, gestion des codes de statut
  (RFC 7230/7231).
- **I/O non bloquantes & multiplexage** : gérer de nombreuses connexions
  simultanées depuis un seul thread avec `epoll`, sans jamais bloquer sur un
  client.
- **Parsing de configuration** : tokeniser et interpréter une configuration
  façon nginx en structures server/location.
- **CGI** : lancer des interpréteurs avec `fork`/`execve`, câbler les variables
  d'environnement et les pipes, et renvoyer leur sortie comme réponse HTTP.
- **Architecture serveur** : structurer le cycle lecture/traitement/écriture, la
  gestion des erreurs et des ressources en C++98.

---

## Project layout / Arborescence

- `srcs/` — C++ source files / fichiers source C++
- `srcs/includes/` — headers / en-têtes
- `www/` — static pages, CGI scripts, uploads / pages statiques, scripts CGI, uploads
- `webserv.conf` — default configuration / configuration par défaut
- `test_webserv.sh` — integration test script / script de test d'intégration

## References / Références

- RFC 7230 & 7231 — HTTP/1.1 message syntax, routing, semantics and content
- RFC 3875 — The Common Gateway Interface (CGI/1.1)
- MDN Web Docs — HTTP overview and status codes
- Nginx documentation — configuration and `location` behavior
- `man` pages: `socket`, `bind`, `listen`, `accept`, `epoll`, `fork`, `execve`, `stat`
