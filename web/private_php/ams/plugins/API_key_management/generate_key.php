<?php

/**
 * Class for API_Key_management plugin 
 * Contains the function to generate random Tokken
 * 
 * @author shubham meena mentored by Matthew Lagoe 
 */

 class generate_key {
    
    /**
     * Static function to generate random token which is registerd with the user 
     * to allow public access using this random token
     * It return different types of tokkens according to the parameters pass through it
     * like length , if standard chracter requires, if special character requires etc
     */
    public static function randomToken( $len = 64, $output = 5, $standardChars = true, $specialChars = true, $chars = array() ) {
        $out = '';
         $len = intval( $len );
         $outputMap = array( 1 => 2, 2 => 8, 3 => 10, 4 => 16, 5 => 10 );
         if ( !is_array( $chars ) ) {
            $chars = array_unique( str_split( $chars ) );
        } 
        if ( $standardChars ) {
            $chars = array_merge( $chars, range( 48, 57 ), range( 65, 90 ), range( 97, 122 ) );
        } 
        if ( $specialChars ) {
            $chars = array_merge( $chars, range( 33, 47 ), range( 58, 64 ), range( 91, 96 ), range( 123, 126 ) );
        } 
        array_walk( $chars, function( &$val ) {
                if ( !is_int( $val ) ) {
                    $val = ord( $val ); } 
            } 
            );
         if ( is_int( $len ) ) {
            while ( $len ) {
                $tmp = ord( openssl_random_pseudo_bytes( 1 ) );
                 if ( in_array( $tmp, $chars ) ) {
                    if ( !$output || !in_array( $output, range( 1, 5 ) ) || $output == 3 || $output == 5 ) {
                        $out .= ( $output == 3 ) ? $tmp : chr( $tmp );
                    } 
                    else {
                        $based = base_convert( $tmp, 10, $outputMap[$output] );
                         $out .= ( ( ( $output == 1 ) ? '00' : ( ( $output == 4 ) ? '0x' : '' ) ) . ( ( $output == 2 ) ? sprintf( '%03d', $based ) : $based ) );
                         } 
                    $len--;
                     } 
                } 
            } 
        return ( empty( $out ) ) ? false : $out;
         }     
    }
