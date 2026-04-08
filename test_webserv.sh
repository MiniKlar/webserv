#!/bin/bash

# ============================================================
#  WEBSERV — Test Suite
#  Auteurs  : à compléter
#  Port     : 8080
#  Routes   : / /static /old-page /uploads /cgi-bin
#  CGI      : .php (php-cgi)  |  .bash (/bin/bash)
#
#  Usage    : ./test_webserv.sh
#  Prérequis: ./webserv lancé sur localhost:8080
# ============================================================

HOST="http://localhost:8080"
PASS=0
FAIL=0
TOTAL=0

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

# ── affichage ────────────────────────────────────────────────

banner() {
    echo ""
    echo -e "${CYAN}${BOLD}"
    echo "  ██╗    ██╗███████╗██████╗ ███████╗███████╗██████╗ ██╗   ██╗"
    echo "  ██║    ██║██╔════╝██╔══██╗██╔════╝██╔════╝██╔══██╗██║   ██║"
    echo "  ██║ █╗ ██║█████╗  ██████╔╝███████╗█████╗  ██████╔╝██║   ██║"
    echo "  ██║███╗██║██╔══╝  ██╔══██╗╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝"
    echo "  ╚███╔███╔╝███████╗██████╔╝███████║███████╗██║  ██║ ╚████╔╝ "
    echo "   ╚══╝╚══╝ ╚══════╝╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  "
    echo -e "${RESET}"
    echo -e "  ${BOLD}Test Suite — HTTP/1.1 — localhost:8080${RESET}"
    echo ""
}

section() {
    echo ""
    echo -e "${CYAN}${BOLD}  ── $1 $( printf '─%.0s' $(seq 1 $((40 - ${#1}))) )${RESET}"
}

check() {
    local label="$1" expected="$2" actual="$3" hint="$4"
    TOTAL=$((TOTAL + 1))
    if [ "$actual" = "$expected" ]; then
        echo -e "  ${GREEN}✔${RESET}  $label ${YELLOW}[$actual]${RESET}"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}✘${RESET}  $label"
        echo -e "      ${RED}attendu: $expected  |  reçu: $actual${RESET}"
        [ -n "$hint" ] && echo -e "      ${YELLOW}→ $hint${RESET}"
        FAIL=$((FAIL + 1))
    fi
}

check_any() {
    local label="$1" actual="$2"
    shift 2
    local accepted="$*"
    TOTAL=$((TOTAL + 1))
    for code in $accepted; do
        if [ "$actual" = "$code" ]; then
            echo -e "  ${GREEN}✔${RESET}  $label ${YELLOW}[$actual]${RESET}"
            PASS=$((PASS + 1))
            return
        fi
    done
    echo -e "  ${RED}✘${RESET}  $label"
    echo -e "      ${RED}attendu: [$accepted]  |  reçu: $actual${RESET}"
    FAIL=$((FAIL + 1))
}

check_body() {
    local label="$1" pattern="$2" body="$3" hint="$4"
    TOTAL=$((TOTAL + 1))
    if echo "$body" | grep -qi "$pattern"; then
        echo -e "  ${GREEN}✔${RESET}  $label"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}✘${RESET}  $label"
        echo -e "      ${RED}pattern '$pattern' absent du body${RESET}"
        [ -n "$hint" ] && echo -e "      ${YELLOW}→ $hint${RESET}"
        FAIL=$((FAIL + 1))
    fi
}

# ── vérification que le serveur tourne ───────────────────────

banner

echo -e "  Vérification du serveur sur ${BOLD}$HOST${RESET} ..."
if ! curl -s -o /dev/null --connect-timeout 2 "$HOST/" ; then
    echo ""
    echo -e "  ${RED}${BOLD}✘  Impossible de joindre le serveur.${RESET}"
    echo -e "  ${YELLOW}→ Lance d'abord : ./webserv <config>${RESET}"
    echo ""
    exit 1
fi
echo -e "  ${GREEN}${BOLD}✔  Serveur en ligne.${RESET}"

# ── setup ────────────────────────────────────────────────────

mkdir -p ./www/uploads ./www/cgi-bin 2>/dev/null

if [ ! -f "./www/cgi-bin/test.php" ]; then
    echo -e "\n  ${YELLOW}⚠  Création de ./www/cgi-bin/test.php${RESET}"
    cat > ./www/cgi-bin/test.php << 'PHPEOF'
<?php
header("Content-Type: text/html");
echo "<html><body>";
echo "<h1>CGI PHP OK</h1>";
echo "<p>Method: " . $_SERVER['REQUEST_METHOD'] . "</p>";
echo "<p>Query: " . $_SERVER['QUERY_STRING'] . "</p>";
echo "</body></html>";
PHPEOF
    chmod +x ./www/cgi-bin/test.php
fi

if [ ! -f "./www/cgi-bin/test.bash" ]; then
    echo -e "  ${YELLOW}⚠  Création de ./www/cgi-bin/test.bash${RESET}"
    cat > ./www/cgi-bin/test.bash << 'BASHEOF'
#!/bin/bash
echo "Content-Type: text/html"
echo ""
echo "<html><body>"
echo "<h1>CGI BASH OK</h1>"
echo "<p>Method: $REQUEST_METHOD</p>"
echo "<p>Query: $QUERY_STRING</p>"
echo "</body></html>"
BASHEOF
    chmod +x ./www/cgi-bin/test.bash
fi

echo "fichier test webserv" > /tmp/ws_test_upload.txt

# ============================================================
#  1. GET STATIQUE
# ============================================================
section "1. GET — Pages statiques"

CODE=$(curl -s -o /dev/null -w "%{http_code}" "$HOST/")
check "GET /" "200" "$CODE" \
    "Vérifie que ./www/default_index.html existe"

CODE=$(curl -s -o /dev/null -w "%{http_code}" "$HOST/default_index.html")
check "GET /default_index.html" "200" "$CODE" \
    "Le fichier ./www/default_index.html est introuvable"

CODE=$(curl -s -o /dev/null -w "%{http_code}" "$HOST/fichier_inexistant_xyz_42")
check "GET ressource inexistante → 404" "404" "$CODE" \
    "Toute ressource inconnue doit retourner 404"

BODY=$(curl -s "$HOST/fichier_inexistant_xyz_42")
check_body "Page 404 est en HTML" "<html\|<!DOCTYPE" "$BODY" \
    "Servir ./www/default_error.html pour les erreurs 404"

# ============================================================
#  2. METHODES
# ============================================================
section "2. Méthodes HTTP — contrôle d'accès"

CODE=$(curl -s -o /dev/null -w "%{http_code}" -X PATCH "$HOST/")
check "PATCH sur / → 405" "405" "$CODE" \
    "PATCH est une méthode connue non autorisée → 405, pas 501 (RFC 7231 §6.5.5)"

CODE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$HOST/")
check "DELETE sur / (route GET only) → 405" "405" "$CODE" \
    "La route / n'autorise que GET → 405"

CODE=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$HOST/static")
check "POST sur /static (route GET only) → 405" "405" "$CODE" \
    "La route /static n'autorise que GET → 405"

CODE=$(curl -s -o /dev/null -w "%{http_code}" -X FOOBAR "$HOST/")
check_any "Méthode FOOBAR inconnue → 400 ou 501" "$CODE" "400" "501"

# ============================================================
#  3. UPLOAD
# ============================================================
section "3. POST — Upload de fichier (/uploads)"

CODE=$(curl -s -o /dev/null -w "%{http_code}" \
    -X POST -F "file=@/tmp/ws_test_upload.txt" "$HOST/uploads")
check_any "POST /uploads → 200 ou 201" "$CODE" "200" "201"

echo -e "  ${YELLOW}⚠  Test 413 ignoré : client_max_body_size = 1G dans la config${RESET}"

# ============================================================
#  4. DELETE
# ============================================================
section "4. DELETE — Suppression de fichier (/uploads)"

FNAME="ws_delete_test_$(date +%s).txt"
echo "à supprimer" > /tmp/$FNAME
curl -s -X POST -F "file=@/tmp/$FNAME" "$HOST/uploads" > /dev/null 2>&1

CODE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$HOST/uploads/$FNAME")
check_any "DELETE /uploads/<fichier> → 200 ou 204" "$CODE" "200" "204"

CODE=$(curl -s -o /dev/null -w "%{http_code}" \
    -X DELETE "$HOST/uploads/fichier_inexistant_xyz.txt")
check "DELETE fichier inexistant → 404" "404" "$CODE" \
    "Un DELETE sur un fichier absent doit retourner 404"

rm -f /tmp/$FNAME

# ============================================================
#  5. CGI PHP
# ============================================================
section "5. CGI — PHP (.php via php-cgi)"

CODE=$(curl -s -o /dev/null -w "%{http_code}" "$HOST/cgi-bin/test.php")
check "GET /cgi-bin/test.php → 200" "200" "$CODE" \
    "500 = php-cgi crashe. Vérifie: 'which php-cgi' et chmod +x sur le script"

BODY=$(curl -s "$HOST/cgi-bin/test.php")
check_body "Body PHP contient du HTML" "<html\|<!DOCTYPE" "$BODY" \
    "Le CGI doit écrire Content-Type: puis ligne vide puis body"

CODE=$(curl -s -o /dev/null -w "%{http_code}" "$HOST/cgi-bin/test.php?name=42&test=ok")
check "GET /cgi-bin/test.php?query → 200" "200" "$CODE" \
    "QUERY_STRING doit être passé en variable d'environnement"

CODE=$(curl -s -o /dev/null -w "%{http_code}" \
    -X POST -H "Content-Type: application/x-www-form-urlencoded" \
    --data "key=value" "$HOST/cgi-bin/test.php")
check "POST /cgi-bin/test.php → 200" "200" "$CODE" \
    "Le body POST doit arriver sur stdin du CGI avec CONTENT_LENGTH"

# ============================================================
#  6. CGI BASH
# ============================================================
section "6. CGI — Bash (.bash via /bin/bash)"

CODE=$(curl -s -o /dev/null -w "%{http_code}" "$HOST/cgi-bin/test.bash")
check "GET /cgi-bin/test.bash → 200" "200" "$CODE" \
    "Vérifie chmod +x et le shebang #!/bin/bash"

BODY=$(curl -s "$HOST/cgi-bin/test.bash")
check_body "Body Bash contient du HTML" "<html\|CGI BASH" "$BODY" \
    "Le script doit écrire Content-Type:, ligne vide, puis body"

CODE=$(curl -s -o /dev/null -w "%{http_code}" "$HOST/cgi-bin/test.bash?foo=bar")
check "GET /cgi-bin/test.bash?query → 200" "200" "$CODE" \
    "QUERY_STRING accessible via \$QUERY_STRING dans le script"

# ============================================================
#  7. HEADERS
# ============================================================
section "7. Headers de réponse HTTP"

HEADERS=$(curl -s -D - -o /dev/null "$HOST/")

TOTAL=$((TOTAL + 1))
if echo "$HEADERS" | grep -qi "^HTTP/"; then
    STATUS=$(echo "$HEADERS" | grep -i "^HTTP/" | tr -d '\r')
    echo -e "  ${GREEN}✔${RESET}  Status line : ${YELLOW}$STATUS${RESET}"
    PASS=$((PASS + 1))
else
    echo -e "  ${RED}✘${RESET}  Status line HTTP absente"
    echo -e "      ${YELLOW}→ La réponse doit commencer par 'HTTP/1.1 200 OK'${RESET}"
    FAIL=$((FAIL + 1))
fi

TOTAL=$((TOTAL + 1))
if echo "$HEADERS" | grep -qi "^Content-Type:"; then
    CT=$(echo "$HEADERS" | grep -i "^Content-Type:" | tr -d '\r')
    echo -e "  ${GREEN}✔${RESET}  ${YELLOW}$CT${RESET}"
    PASS=$((PASS + 1))
else
    echo -e "  ${RED}✘${RESET}  Header Content-Type absent"
    echo -e "      ${YELLOW}→ Ajoute 'Content-Type: text/html' dans buildResponse()${RESET}"
    FAIL=$((FAIL + 1))
fi

TOTAL=$((TOTAL + 1))
if echo "$HEADERS" | grep -qi "^Content-Length:\|^Transfer-Encoding:"; then
    CL=$(echo "$HEADERS" | grep -i "^Content-Length:\|^Transfer-Encoding:" | tr -d '\r')
    echo -e "  ${GREEN}✔${RESET}  ${YELLOW}$CL${RESET}"
    PASS=$((PASS + 1))
else
    echo -e "  ${RED}✘${RESET}  Content-Length et Transfer-Encoding absents"
    echo -e "      ${YELLOW}→ Ajoute 'Content-Length: <taille>' dans tes headers${RESET}"
    FAIL=$((FAIL + 1))
fi

# ============================================================
#  8. AUTOINDEX
# ============================================================
section "8. Directory listing (autoindex on — /static)"

CODE=$(curl -s -o /dev/null -w "%{http_code}" "$HOST/static/")
check "GET /static/ → 200" "200" "$CODE" \
    "500 = crash dans la génération du listing. Vérifie opendir/readdir"

BODY=$(curl -s "$HOST/static/")
check_body "Listing contient des <a href>" "<a " "$BODY" \
    "La page de listing doit avoir des balises <a href> pour chaque fichier"

# ============================================================
#  9. REDIRECTION
# ============================================================
section "9. Redirection HTTP (/old-page → /)"

CODE=$(curl -s -o /dev/null -w "%{http_code}" --max-redirs 0 "$HOST/old-page")
check_any "GET /old-page → 301 ou 302" "$CODE" "301" "302"

TOTAL=$((TOTAL + 1))
LOCATION=$(curl -s -D - -o /dev/null "$HOST/old-page" | grep -i "^Location:" | tr -d '\r')
if [ -n "$LOCATION" ]; then
    echo -e "  ${GREEN}✔${RESET}  Header présent : ${YELLOW}$LOCATION${RESET}"
    PASS=$((PASS + 1))
else
    echo -e "  ${RED}✘${RESET}  Header Location absent dans la réponse 301"
    echo -e "      ${YELLOW}→ Une redirection doit avoir 'Location: <url>' dans ses headers${RESET}"
    FAIL=$((FAIL + 1))
fi

CODE=$(curl -s -o /dev/null -w "%{http_code}" -L "$HOST/old-page")
check "Suivi redirection → 200" "200" "$CODE" \
    "Après le 301, on doit atterrir sur une page 200"

# ============================================================
#  10. SECURITE
# ============================================================
section "10. Sécurité & cas limites"

TOTAL=$((TOTAL + 1))
BODY=$(curl -s "$HOST/../../../etc/passwd")
if echo "$BODY" | grep -q "root:"; then
    echo -e "  ${RED}✘${RESET}  PATH TRAVERSAL — /etc/passwd accessible !"
    echo -e "      ${RED}FAILLE CRITIQUE : normalise les paths avec realpath() avant open()${RESET}"
    FAIL=$((FAIL + 1))
else
    echo -e "  ${GREEN}✔${RESET}  Path traversal bloqué"
    PASS=$((PASS + 1))
fi

LONG_PATH=$(python3 -c "print('a'*8000)" 2>/dev/null || printf '%8000s' | tr ' ' 'a')
CODE=$(curl -s -o /dev/null -w "%{http_code}" "$HOST/$LONG_PATH")
check_any "URI trop longue → 414 ou 400 ou 404" "$CODE" "414" "400" "404"

CODE=$(curl -s -o /dev/null -w "%{http_code}" "$HOST//")
check_any "Double slash // → 200 ou 301 ou 400" "$CODE" "200" "301" "400"

# ============================================================
#  11. STRESS TEST
# ============================================================
section "11. Stress test — 100 requêtes concurrentes"

for i in $(seq 1 100); do
    curl -s -o /dev/null "$HOST/" &
done
wait

CODE=$(curl -s -o /dev/null -w "%{http_code}" "$HOST/")
check "Serveur toujours UP après 100 requêtes" "200" "$CODE" \
    "Fuite de fd, poll mal géré, ou crash silencieux"

# ============================================================
#  12. TIMEOUT — REQUÊTE BLOQUANTE
# ============================================================
section "12. Timeout / requêtes bloquantes"

TOTAL=$((TOTAL + 1))
timeout 3 curl -s "$HOST/" > /dev/null

if [ $? -eq 124 ]; then
    echo -e "  ${RED}✘${RESET}  Requête bloquée (timeout)"
    echo -e "      ${YELLOW}→ Ton serveur ne répond pas correctement (poll mal géré ?)${RESET}"
    FAIL=$((FAIL + 1))
else
    echo -e "  ${GREEN}✔${RESET}  Pas de blocage"
    PASS=$((PASS + 1))
fi

# ============================================================
#  RÉSUMÉ FINAL
# ============================================================
echo ""
echo -e "${BOLD}  ════════════════════════════════════════${RESET}"
echo -e "${BOLD}  RÉSULTATS${RESET}"
echo -e "${BOLD}  ════════════════════════════════════════${RESET}"
echo ""

PERCENT=$(( PASS * 100 / TOTAL ))

# Barre de progression
BAR_FULL=40
BAR_DONE=$(( PERCENT * BAR_FULL / 100 ))
BAR_LEFT=$(( BAR_FULL - BAR_DONE ))
BAR="${GREEN}$(printf '█%.0s' $(seq 1 $BAR_DONE))${RESET}$(printf '░%.0s' $(seq 1 $BAR_LEFT))"
echo -e "  $BAR  ${BOLD}$PERCENT%${RESET}"
echo ""
echo -e "  ${GREEN}✔ Réussis : ${BOLD}$PASS${RESET}  /  ${RED}✘ Échoués : ${BOLD}$FAIL${RESET}  /  Total : ${BOLD}$TOTAL${RESET}"
echo ""

if [ "$FAIL" -eq 0 ]; then
    echo -e "  ${GREEN}${BOLD}🎉  Tous les tests passent — prêt pour la soutenance !${RESET}"
else
    echo -e "  ${YELLOW}Les ✘ avec une flèche → indiquent où chercher dans le code.${RESET}"
fi
echo ""

rm -f /tmp/ws_test_upload.txt