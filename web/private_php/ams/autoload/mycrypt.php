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
        $iv = self::hashIV($this->config['key'], $this->config['hash_method'], openssl_cipher_iv_length($this->config['enc_method']));
        $infostr = sprintf('$%s$%s$', $this->config['enc_method'], $this->config['hash_method']);
        return $infostr . openssl_encrypt($data, $this->config['enc_method'], $this->config['key'], false, $iv);
    }

    /**
    * decrypts by using the given enc_method and hash_method.
    * @param $edata the encrypted string that we want to decrypt
    * @return the decrypted string.
    */
    public function decrypt($edata) {
        $e_arr = explode('$', $edata);
        if( count($e_arr) != 4 ) {
            Throw new Exception('Given data is missing crucial sections.');
        }
        $this->config['enc_method'] = $e_arr[1];
        $this->config['hash_method'] = $e_arr[2];
        self::check_methods($this->config['enc_method'], $this->config['hash_method']);
        $iv = self::hashIV($this->config['key'], $this->config['hash_method'], openssl_cipher_iv_length($this->config['enc_method']));
        return openssl_decrypt($e_arr[3], $this->config['enc_method'], $this->config['key'], false, $iv);
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
        } else if( ! in_array($enc, openssl_get_cipher_methods()) ) {
            Throw new Exception('Encryption method ' . $enc . ' not supported.');
        } else if( ! in_array(strtolower($hash), hash_algos()) ) {
            Throw new Exception('Hashing method ' . $hash . ' not supported.');
        }
    }



}