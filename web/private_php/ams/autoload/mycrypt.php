<?php
/**
* Basic encryption/decryption class.
* We use this class atm for encrypting & decrypting the imap passwords.
*/
class MyCrypt{
    
    private $config; /**< array that should contain the enc_method & hash_method & key */
    
    
    /**
    * constructor.
    * loads the config array with the given argument.
    * @param $cryptinfo an array containing the info needed to encrypt & decrypt.(enc_method & hash_method & key)
    */
    function __construct($cryptinfo) {
        $this->config = $cryptinfo;
    }
    
    /**
    * encrypts by using the given enc_method and hash_method.
    * It will first check if the methods are supported, if not it will throw an error, if so it will encrypt the $data
    * @param $data the string that we want to encrypt.
    * @return the encrypted string.
    */
    public function encrypt($data) {

        self::check_methods($this->config['enc_method'], $this->config['hash_method']);
        // A fresh iv per message. The old format derived it from the key, so
        // the same password always produced the same ciphertext; the iv is
        // carried in the result instead, which decrypt() recognises by the
        // extra section.
        $iv = openssl_random_pseudo_bytes(openssl_cipher_iv_length($this->config['enc_method']));
        $infostr = sprintf('$%s$%s$%s$', $this->config['enc_method'], $this->config['hash_method'], bin2hex($iv));
        return $infostr . openssl_encrypt($data, $this->config['enc_method'], $this->config['key'], false, $iv);
    }

    /**
    * decrypts by using the given enc_method and hash_method.
    * @param $edata the encrypted string that we want to decrypt
    * @return the decrypted string.
    */
    public function decrypt($edata) {
        $e_arr = explode('$', $edata);
        if( count($e_arr) != 4 && count($e_arr) != 5 ) {
            Throw new Exception('Given data is missing crucial sections.');
        }
        // The ciphertext used to name its own cipher/hash, which let a stored
        // value ask for a weaker method if openssl still offered one. Always
        // decrypt with the methods from config; the embedded names are only
        // checked so we refuse blobs that were not written by us.
        $blob_enc = $e_arr[1];
        $blob_hash = $e_arr[2];
        if ($blob_enc !== $this->config['enc_method'] || $blob_hash !== $this->config['hash_method']) {
            Throw new Exception('Ciphertext methods do not match configuration.');
        }
        self::check_methods($this->config['enc_method'], $this->config['hash_method']);
        if( count($e_arr) == 5 ) {
            // written by the current encrypt(): the iv travels with the data
            $iv = hex2bin($e_arr[3]);
            $payload = $e_arr[4];
        } else {
            // written before that: the iv was derived from the key
            $iv = self::hashIV($this->config['key'], $this->config['hash_method'], openssl_cipher_iv_length($this->config['enc_method']));
            $payload = $e_arr[3];
        }
        return openssl_decrypt($payload, $this->config['enc_method'], $this->config['key'], false, $iv);
    }

    /**
    * hashes the key by using a hash method specified.
    * @param $key the key to be hashed
    * @param $method the metho of hashing to be used
    * @param $iv_size the size of the initialization vector.
    * @return return the hashed key up till the size of the iv_size param.
    */
    private static function hashIV($key, $method, $iv_size) {
        $myhash = hash($method, $key, TRUE);
        while( strlen($myhash) < $iv_size ) {
            $myhash .= hash($method, $myhash, TRUE);
        }
        return substr($myhash, 0, $iv_size);
    }

    /**
    * checks if the encryption and hash methods are supported
    * @param $enc the encryption method.
    * @param $hash the hash method.
    * @throw Exception in case a method is not supported.
    */
    private static function check_methods($enc, $hash) {
        
        if( ! function_exists('openssl_encrypt') ) {
            Throw new Exception('openssl_encrypt() not supported.');
        } else if( ! in_array(strtolower($enc), array_map('strtolower', openssl_get_cipher_methods())) ) {
            // openssl reports the cipher names in lower case
            Throw new Exception('Encryption method ' . $enc . ' not supported.');
        } else if( ! in_array(strtolower($hash), hash_algos()) ) {
            Throw new Exception('Hashing method ' . $hash . ' not supported.');
        }
    }



}