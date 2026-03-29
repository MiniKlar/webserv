#!/bin/bash

# Le serveur HTTP s'attend à recevoir des en-têtes valides en premier,
# suivis d'une ligne vide, puis du corps (payload).

# 1. En-têtes HTTP
printf "Content-type: text/html\r\n\r\n"

# 2. Corps de la réponse (HTML)
echo "<!DOCTYPE html>"
echo "<html lang=\"fr\">"
echo "<head>"
echo "    <meta charset=\"UTF-8\">"
echo "    <title>Test CGI</title>"
echo "</head>"
echo "<body>"
echo "    <h1>Succès du CGI !</h1>"
echo "    <p>Ce fichier a été exécuté avec succès par votre serveur via CGI.</p>"
echo "    <hr>"
echo "    <h2>Variables d'environnement :</h2>"
echo "    <ul>"
echo "        <li><b>REQUEST_METHOD :</b> $REQUEST_METHOD</li>"
echo "        <li><b>CONTENT_LENGTH :</b> $CONTENT_LENGTH</li>"
echo "        <li><b>SCRIPT_NAME :</b> $SCRIPT_NAME</li>"
echo "    </ul>"
echo "</body>"
echo "</html>"
