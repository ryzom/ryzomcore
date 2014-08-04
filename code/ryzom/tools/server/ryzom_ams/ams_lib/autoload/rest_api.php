<?php

/**
 * REST API class  
 * 
 * Request for the given url using cURL  
 * and send the AccessToken for authentication 
 * to make public access for the user
 * 
 * @author Shubham Meena, mentored by Matthew Lagoe 
 */

class Rest_Api {
    
    /**
     * Makes a request using cURL with authentication headers and returns the response.
     * 
     * @param  $url where request is to be sent
     * @param  $applicationKey user generated key
     * @param  $host host for the website
     * @return URL response.
     */
    public function request( $url , $applicationKey, $host )
     {
        // Check the referer is the host website
        $referer = $_SERVER['HTTP_REFERER'];
         $referer_parse = parse_url( $referer );
         if ( $referer_parse['host'] == $host ) {
            
            // Initialize the cURL session with the request URL
            $session = curl_init( $url );
            
             // Tell cURL to return the request data
            curl_setopt( $session, CURLOPT_RETURNTRANSFER, true );
            
             // Set the HTTP request authentication headers
            $headers = array( 
                'AppKey: ' . $applicationKey,
                 'Timestamp: ' . date( 'Ymd H:i:s', time() )
                 );
             curl_setopt( $session, CURLOPT_HTTPHEADER, $headers );
            
             // Execute cURL on the session handle
            $response = curl_exec( $session );
            
             return $response;
             } 
        else {
            return null;
             } 
        } 
    } 
