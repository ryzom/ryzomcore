<?php

class MyCrypt{
    
    private $config;
    
    function __construct($cryptinfo) {
        $this->config = $cryptinfo;
    }
    

    public function encrypt($data) {
        
        self::check_methods($this->config['enc_method'], $this->config['hash_method']);
        $iv = self::hashIV($this->config['key'], $this->config['hash_method'], openssl_cipher_iv_length($this->config['enc_method']));
        $infostr = sprintf('$%s$%s$', $this->config['enc_method'], $this->config['hash_method']);
        return $infostr . openssl_encrypt($data, $this->config['enc_method'], $this->config['key'], false, $iv);
    }

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

    private static function hashIV($key, $method, $iv_size) {
        $myhash = hash($method, $key, TRUE);
        while( strlen($myhash) < $iv_size ) {
            $myhash .= hash($method, $myhash, TRUE);
        }
        return substr($myhash, 0, $iv_size);
    }

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