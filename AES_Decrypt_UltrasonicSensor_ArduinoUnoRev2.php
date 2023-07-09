<?php

$data = isset($_GET["enc"]);

echo file_put_contents("test.txt",$_GET["enc"]);

$replaced = str_replace(' ', '+', $_GET["enc"]);


$raw = $replaced;

// Convert the encrypted data from base64 to binary
$encryptedData = $raw;

// Define the encryption key and IV
$key = "aaaaaaaaaaaaaaaa"; // Encryption key (16 characters for AES-128)
$iv = "aaaaaaaaaaaaaaaa"; // Initialization Vector (IV)

// Decrypt the data
$decryptedData = openssl_decrypt($encryptedData, 'aes-128-cbc', $key, OPENSSL_ZERO_PADDING, $iv);

// Output the decrypted data
echo "Decrypted Data: " . base64_decode($decryptedData);

?>
