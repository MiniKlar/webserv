#!/bin/bash

# En-têtes HTTP requis pour la réponse CGI
printf "Content-type: text/html\r\n\r\n"

echo "<!DOCTYPE html>"
echo "<html lang=\"fr\">"
echo "<head>"
echo "    <meta charset=\"UTF-8\">"
echo "    <title>Test CGI POST</title>"
echo "    <style>body { font-family: sans-serif; padding: 20px; } pre { background: #eee; padding: 10px; border-radius: 5px; }</style>"
echo "</head>"
echo "<body>"
echo "    <h1>Test du script CGI avec un body (POST)</h1>"
echo "    <ul>"
echo "        <li><b>REQUEST_METHOD :</b> $REQUEST_METHOD</li>"
echo "        <li><b>CONTENT_LENGTH :</b> $CONTENT_LENGTH</li>"
echo "        <li><b>CONTENT_TYPE :</b> $CONTENT_TYPE</li>"
echo "    </ul>"
echo "    <hr>"
echo "    <h2>Corps de la requête (Body) :</h2>"
echo "    <pre>"

# Lecture du body sur l'entrée standard (STDIN)
# Pour éviter que le script ne bloque si le serveur n'a pas bien fermé le pipe,
# on lit exactement la quantité d'octets indiquée par CONTENT_LENGTH.
if [ "$REQUEST_METHOD" = "POST" ]; then
    if [ -n "$CONTENT_LENGTH" ] && [ "$CONTENT_LENGTH" -gt 0 ]; then
        head -c "$CONTENT_LENGTH"
    else
        echo "[Aucun body reçu ou CONTENT_LENGTH manquant/zéro]"
    fi
else
    echo "[Veuillez envoyer une requête POST pour simuler un body]"
fi

echo "    </pre>"
echo "</body>"
echo "</html>"
