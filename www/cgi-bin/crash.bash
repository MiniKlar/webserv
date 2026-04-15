#!/bin/bash
echo "Content-Type: text/plain"
echo ""

# La ligne suivante envoie un signal de Segfault (SIGSEGV) au script lui-même
kill -SEGV $$