#!/bin/bash

PORT=8080
HOST="localhost"
BASE_URL="http://$HOST:$PORT"
NC_CMD="nc -q 1 $HOST $PORT" # Use -N or -q 1 depending on nc version, we will try standard nc

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Starting 42 Webserv CGI Evaluation Tester...${NC}\n"

# Check if server is up
curl -s $BASE_URL > /dev/null
if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Server is not responding on $BASE_URL. Please start webserv first.${NC}"
    exit 1
fi

chmod +x www/cgi-bin/*.bash

# ------------------------------------------------------------------------------
echo -e "${YELLOW}[1] Environment Variables & Arguments${NC}"
echo "Testing if QUERY_STRING and basic env vars are passed to CGI..."
RES=$(curl -s "$BASE_URL/cgi-bin/test.bash?user=evaluator&project=webserv")
if echo "$RES" | grep -q "user=evaluator&project=webserv"; then
    echo -e "${GREEN}SUCCESS: QUERY_STRING found in CGI output.${NC}"
else
    echo -e "${RED}FAIL: QUERY_STRING not found.${NC}"
    echo "Output was:"
    echo "$RES" | head -n 10
fi
echo ""

# ------------------------------------------------------------------------------
echo -e "${YELLOW}[2] Chunked Request (Server un-chunks -> CGI receives EOF)${NC}"
echo "Sending a chunked POST request via nc..."
# We send "Hello\r\n World" in two chunks: "5" and "6", total "Hello World"
REQ="POST /cgi-bin/post_test.bash HTTP/1.1\r\nHost: $HOST:$PORT\r\nTransfer-Encoding: chunked\r\nContent-Type: text/plain\r\n\r\n5\r\nHello\r\n6\r\n World\r\n0\r\n\r\n"
RES=$(printf "$REQ" | nc -N $HOST $PORT 2>/dev/null || printf "$REQ" | nc $HOST $PORT)

if echo "$RES" | grep -q "Hello World"; then
    echo -e "${GREEN}SUCCESS: CGI received the un-chunked body ('Hello World').${NC}"
elif echo "$RES" | grep -q "5.*Hello"; then
    echo -e "${RED}FAIL: Server passed raw chunk sizes to the CGI instead of unchunking.${NC}"
else
    echo -e "${RED}FAIL: CGI did not return the expected body.${NC}"
    echo "Output was:"
    echo "$RES" | tail -n 15
fi
echo ""

# ------------------------------------------------------------------------------
echo -e "${YELLOW}[3] Output without Content-Length (CGI closes via EOF)${NC}"
echo "Testing CGI that returns no Content-Length header..."
RES=$(curl -i -s "$BASE_URL/cgi-bin/no_cl.bash")
if echo "$RES" | grep -q "rely on EOF"; then
    echo -e "${GREEN}SUCCESS: Body received entirely via EOF.${NC}"
    if echo "$RES" | grep -qi "Transfer-Encoding: chunked"; then
        echo -e "${GREEN}SUCCESS: Server safely translated it to chunked encoding for the client.${NC}"
    elif echo "$RES" | grep -qi "Content-Length:"; then
        echo -e "${GREEN}SUCCESS: Server computed Content-Length before sending.${NC}"
    else
        echo -e "${YELLOW}WARNING: Server sent it as HTTP/1.0 or connection close.${NC}"
    fi
else
    echo -e "${RED}FAIL: Could not read properly CGI output ending with EOF.${NC}"
fi
echo ""

# ------------------------------------------------------------------------------
echo -e "${YELLOW}[4] File Access & Execution Directory (CWD)${NC}"
echo "Testing if CGI is executed in the correct directory (should be cgi-bin)..."
RES=$(curl -s "$BASE_URL/cgi-bin/cwd_test.bash")
if echo "$RES" | grep -q "/cgi-bin"; then
    echo -e "${GREEN}SUCCESS: CGI listed files in cgi-bin (executed in correct relative path).${NC}"
else
    echo -e "${RED}FAIL: CGI does not seem to be executed in the correct directory.${NC}"
    echo "Output was:"
    echo "$RES"
fi
echo ""

# ------------------------------------------------------------------------------
echo -e "${YELLOW}[5] Multiple CGI Support (PHP)${NC}"
echo "Testing if PHP CGI works (assuming post_test.php or env.php is configured)..."
RES=$(curl -s -X POST -d "hello=php" "$BASE_URL/cgi-bin/post_test.php")
if echo "$RES" | grep -qi -E "hello=php|REQUEST_METHOD"; then
    echo -e "${GREEN}SUCCESS: PHP CGI rendered successfully.${NC}"
else
    echo -e "${YELLOW}WARNING/FAIL: PHP CGI failed or not configured.${NC}"
fi
echo ""

echo -e "${YELLOW}Evaluation Tests Completed.${NC}"
