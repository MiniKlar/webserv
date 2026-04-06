#!/usr/bin/php
<?php
// En-têtes HTTP requis pour la réponse CGI
echo "Content-type: text/html\r\n\r\n";
?>
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <title>Test CGI PHP (GET)</title>
</head>
<body>
    <h1>Succès du CGI (PHP) !</h1>
    <p>Ce fichier a été exécuté avec succès par votre serveur via PHP CGI.</p>
    <hr>
    <h2>Variables d'environnement :</h2>
    <ul>
        <li><b>REQUEST_METHOD :</b> <?php echo isset($_SERVER['REQUEST_METHOD']) ? $_SERVER['REQUEST_METHOD'] : 'Non définie'; ?></li>
        <li><b>QUERY_STRING :</b> <?php echo isset($_SERVER['QUERY_STRING']) ? $_SERVER['QUERY_STRING'] : 'Non définie'; ?></li>
        <li><b>CONTENT_LENGTH :</b> <?php echo isset($_SERVER['CONTENT_LENGTH']) ? $_SERVER['CONTENT_LENGTH'] : 'Non définie'; ?></li>
        <li><b>SCRIPT_NAME :</b> <?php echo isset($_SERVER['SCRIPT_NAME']) ? $_SERVER['SCRIPT_NAME'] : 'Non définie'; ?></li>
    </ul>
</body>
</html>
