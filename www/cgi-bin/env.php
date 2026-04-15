<?php
echo "Content-Type: text/plain\r\n\r\n";
foreach($_SERVER as $key => $value) {
    echo "$key: $value\n";
}
