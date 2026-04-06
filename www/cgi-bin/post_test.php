#!/usr/bin/php
<?php
// En-têtes HTTP requis pour la réponse CGI
echo "Content-type: text/html\r\n\r\n";
?>
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <title>Test CGI PHP POST</title>
    <style>body { font-family: sans-serif; padding: 20px; } pre { background: #eee; padding: 10px; border-radius: 5px; }</style>
</head>
<body>
    <h1>Test du script CGI avec un body (POST) en PHP</h1>
    <ul>
        <li><b>REQUEST_METHOD :</b> <?php echo isset($_SERVER['REQUEST_METHOD']) ? $_SERVER['REQUEST_METHOD'] : 'Non définie'; ?></li>
        <li><b>CONTENT_LENGTH :</b> <?php echo isset($_SERVER['CONTENT_LENGTH']) ? $_SERVER['CONTENT_LENGTH'] : 'Non définie'; ?></li>
        <li><b>CONTENT_TYPE :</b> <?php echo isset($_SERVER['CONTENT_TYPE']) ? $_SERVER['CONTENT_TYPE'] : 'Non définie'; ?></li>
    </ul>
    <hr>
    <h2>Corps de la requête (Body) :</h2>
    <pre>
<?php
$method = isset($_SERVER['REQUEST_METHOD']) ? $_SERVER['REQUEST_METHOD'] : '';

if ($method === 'POST') {
    // Lecture directe depuis l'entrée standard STDIN.
    $body = file_get_contents('php://stdin');

    // Fallback php://input au cas où le script est lancé par php-cgi classique
    if (empty($body)) {
        $body = file_get_contents('php://input');
    }

    if (!empty($body)) {
        echo htmlspecialchars($body);
    } else {
        echo "[Aucun body reçu, PHP n'a rien lu dans STDIN ou php://input]";
    }
} else {
    echo "[Veuillez envoyer une requête POST pour simuler un body]";
}
?>
    </pre>
</body>
</html>
