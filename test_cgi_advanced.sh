#!/bin/bash

# ============================================================
#  Script de Test Avancé pour les CGI (Webserv 42)
# ============================================================

HOST="http://localhost:8080"
CGI_DIR="./www/cgi-bin"
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
RESET='\033[0m'

echo -e "${CYAN}=== PRÉPARATION DES SCRIPTS CGI ===${RESET}"
mkdir -p "$CGI_DIR"

# 1. CGI Environnement (PHP)
cat > "$CGI_DIR/env.php" << 'EOF'
<?php
echo "Content-Type: text/plain\r\n\r\n";
foreach($_SERVER as $key => $value) {
    echo "$key: $value\n";
}
EOF
chmod +x "$CGI_DIR/env.php"

# 2. CGI Status Custom (PHP)
cat > "$CGI_DIR/status.php" << 'EOF'
<?php
echo "Status: 418 I'm a teapot\r\n";
echo "Content-Type: text/plain\r\n\r\n";
echo "I am a teapot!";
EOF
chmod +x "$CGI_DIR/status.php"

# 3. CGI Crash par Segfault (C++)
cat > "$CGI_DIR/crash.cpp" << 'EOF'
int main() {
    char *ptr = 0;
    *ptr = 1; // Segfault forcé
    return 0;
}
EOF
c++ "$CGI_DIR/crash.cpp" -o "$CGI_DIR/crash.cgi" 2>/dev/null
chmod +x "$CGI_DIR/crash.cgi"

# 4. CGI Sans permission
cat > "$CGI_DIR/no_exec.bash" << 'EOF'
#!/bin/bash
echo "Content-Type: text/plain"
echo ""
echo "This should never be printed"
EOF
chmod -x "$CGI_DIR/no_exec.bash"

# 5. CGI Boucle Infinie / Timeout
cat > "$CGI_DIR/infinite.bash" << 'EOF'
#!/bin/bash
while true; do sleep 1; done
EOF
chmod +x "$CGI_DIR/infinite.bash"

# 6. CGI Echo POST (Bash)
cat > "$CGI_DIR/post_test.bash" << 'EOF'
#!/bin/bash
printf "Content-Type: text/plain\r\n\r\n"
# Lire le contenu envoyé par le navigateur (via webserv) sur l'entrée standard
cat
EOF
chmod +x "$CGI_DIR/post_test.bash"

echo -e "Fichiers créés dans $CGI_DIR.\n"

# ============================================================
#  EXECUTION DES TESTS
# ============================================================

echo -e "${CYAN}Test 1 : Variables d'Environnement${RESET}"
echo -e "${YELLOW}Description : Vérifie PATH_INFO, QUERY_STRING et Custom Headers (HTTP_X_SECRET)${RESET}"
curl -s -v "$HOST/cgi-bin/env.php/mon/chemin/extra?user=42&eval=ok" -H "X-Secret: meow" 2> trace.txt > body.txt
echo -e "${CYAN}=== ENVOI ===${RESET}"
awk '/^> / {print substr($0, 3)}' trace.txt
echo -e "${CYAN}=== RECEPTION ===${RESET}"
awk '/^< / {print substr($0, 3)}' trace.txt
echo -e "${CYAN}=== CORPS ===${RESET}"
cat body.txt
echo ""
if grep -q "PATH_INFO" body.txt && grep -q "QUERY_STRING" body.txt && grep -q "HTTP_X_SECRET" body.txt; then
    echo -e "${GREEN}✔ Succès : Les variables d'environnement sont présentes.${RESET}"
else
    echo -e "${RED}✘ Échec : Variables d'environnement manquantes.${RESET}"
fi
echo ""

echo -e "${CYAN}Test 2 : Parsing du Custom Status CGI${RESET}"
echo -e "${YELLOW}Description : Le CGI renvoie 'Status: 418 I'm a teapot'. Ton serveur doit l'utiliser dans la ligne HTTP.${RESET}"
curl -s -v -k "$HOST/cgi-bin/status.php" 2> trace.txt > body.txt
echo -e "${CYAN}=== ENVOI ===${RESET}"
awk '/^> / {print substr($0, 3)}' trace.txt
echo -e "${CYAN}=== RECEPTION ===${RESET}"
awk '/^< / {print substr($0, 3)}' trace.txt
echo -e "${CYAN}=== CORPS ===${RESET}"
cat body.txt
echo ""
HTTP_LINE=$(awk '/^< / {print substr($0, 3)}' trace.txt | head -n 1 | tr -d '\r')
echo "Réponse reçue : $HTTP_LINE"
if echo "$HTTP_LINE" | grep -q "418"; then
    echo -e "${GREEN}✔ Succès : Le serveur a bien parsé le header Status du CGI${RESET}"
else
    echo -e "${RED}✘ Échec : Le serveur doit lire le 'Status: xxx' du CGI au lieu de toujours forcer 200 OK.${RESET}"
fi
echo ""

echo -e "${CYAN}Test 3 : CGI Sans Droits d'Exécution${RESET}"
echo -e "${YELLOW}Description : Fichier présent mais chmod -x. Attendu 403 Forbidden (ou 500).${RESET}"
curl -s -v -k "$HOST/cgi-bin/no_exec.bash" 2> trace.txt > body.txt
echo -e "${CYAN}=== ENVOI ===${RESET}"
awk '/^> / {print substr($0, 3)}' trace.txt
echo -e "${CYAN}=== RECEPTION ===${RESET}"
awk '/^< / {print substr($0, 3)}' trace.txt
echo -e "${CYAN}=== CORPS ===${RESET}"
cat body.txt
echo ""
if grep -qE "HTTP/1.1 (403|500)" trace.txt; then
    echo -e "${GREEN}✔ Succès : Le serveur a bien bloqué l'accès (Erreur 403 ou 500).${RESET}"
else
    echo -e "${RED}✘ Échec : Le serveur n'a pas renvoyé le bon code d'erreur.${RESET}"
fi
echo ""

echo -e "${CYAN}Test 4 : Le CGI crash (Segfault)${RESET}"
echo -e "${YELLOW}Description : Le script fait un Segfault. Ton serveur doit capter que waitpid a un statut d'erreur et renvoyer 500, sans crasher lui-même !${RESET}"
curl -s -v -k "$HOST/cgi-bin/crash.bash" 2> trace.txt > body.txt
echo -e "${CYAN}=== ENVOI ===${RESET}"
awk '/^> / {print substr($0, 3)}' trace.txt
echo -e "${CYAN}=== RECEPTION ===${RESET}"
awk '/^< / {print substr($0, 3)}' trace.txt
echo -e "${CYAN}=== CORPS ===${RESET}"
cat body.txt
echo ""
if grep -qE "HTTP/1.1 (500|502)" trace.txt; then
    echo -e "${GREEN}✔ Succès : Le serveur a bien intercepté le crash du CGI (Erreur 500 ou 502).${RESET}"
else
    echo -e "${RED}✘ Échec : Le serveur n'a pas renvoyé le bon code d'erreur.${RESET}"
fi
echo ""

echo -e "${CYAN}Test 5 : Timeout CGI (Boucle Infinie)${RESET}"
echo -e "${YELLOW}Description : Le CGI tourne à l'infini (while true). Ton serveur gère-t-il les timeouts (504 Gateway Timeout) ?${RESET}"
echo "La requête est lancée avec un Timeout de 5 secondes sur curl..."
curl -s -v --max-time 5 -k "$HOST/cgi-bin/infinite.bash" 2> trace.txt > body.txt
RET=$?
echo -e "${CYAN}=== ENVOI ===${RESET}"
awk '/^> / {print substr($0, 3)}' trace.txt
echo -e "${CYAN}=== RECEPTION ===${RESET}"
awk '/^< / {print substr($0, 3)}' trace.txt
echo -e "${CYAN}=== CORPS ===${RESET}"
cat body.txt
echo ""
if [ $RET -eq 28 ]; then
    echo -e "${RED}✘ Échec : curl a hit son timeout de 5 secondes. Ton webserv est sûrement resté bloqué indéfiniment ou a freezé !${RESET}"
else
    echo -e "${GREEN}✔ Terminé : Ton serveur n'a pas freezé (ou a coupé la connexion correctement).${RESET}"
fi
echo ""

echo -e "${CYAN}Test 6 : Requête POST au CGI${RESET}"
echo -e "${YELLOW}Description : Envoie un body via POST. Webserv doit écrire le body dans l'entrée standard (stdin) du CGI.${RESET}"
curl -s -v -X POST -d "Hello Webserv! Voici mon super body POST" "$HOST/cgi-bin/post_test.bash" 2> trace.txt > body.txt
echo -e "${CYAN}=== ENVOI ===${RESET}"
awk '/^> / {print substr($0, 3)}' trace.txt
echo -e "${CYAN}=== RECEPTION ===${RESET}"
awk '/^< / {print substr($0, 3)}' trace.txt
echo -e "${CYAN}=== CORPS ===${RESET}"
cat body.txt
echo ""
if grep -q "Hello Webserv! Voici mon super body POST" body.txt; then
    echo -e "${GREEN}✔ Succès : Le CGI a bien reçu et renvoyé le body du POST.${RESET}"
else
    echo -e "${RED}✘ Échec : Le CGI n'a pas répliqué le body attendu (problème pipe/stdin).${RESET}"
fi
echo ""

rm -f trace.txt body.txt


# Nettoyage optionnel des binaires compilés
rm -f "$CGI_DIR/crash.cpp" "$CGI_DIR/crash.cgi" "$CGI_DIR/no_exec.bash" "$CGI_DIR/post_test.bash"

echo -e "${CYAN}=== FIN DES TESTS CGI ===${RESET}"