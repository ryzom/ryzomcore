<?php
/**
* This function is beign used to load info that's needed for the logout page.
* it will just unset & destroy the session
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function logout(){
    session_unset();
    session_destroy();
}
