<?php
/**
* This function is beign used to load info that's needed for the register page.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function register(){
    global $TOS_URL;
    
    $pageElements['TOS_URL'] = $TOS_URL;
    return $pageElements;
}
