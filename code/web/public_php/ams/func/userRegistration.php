<?php
/**
 * This function is beign used to change the users emailaddress info.
 * It will first check if the user who executed this function is the person of whom the emailaddress is or if it's a mod/admin. If this is not the case the page will be redirected to an error page.
 * The emailaddress will be validated first. If the checking was successful the email will be updated and the settings template will be reloaded. Errors made by invalid data will be shown
 * also after reloading the template.
 * @author Daan Janssens, mentored by Matthew Lagoe
 */
function userRegistration()
{
    
    try {
        //if logged in
        if (WebUsers::isLoggedIn()) {
            
            $dbl = new DBLayer("lib");
            $dbl->update("settings", Array('Value' => $_POST['userRegistration']), "`Setting` = 'userRegistration'");
                        
            $result['target_id'] = $_GET['id'];
            global $SITEBASE;
            require_once($SITEBASE . '/inc/settings.php');
            $pageElements = settings();
            $pageElements = array_merge(settings(), $result);
            $pageElements['permission'] = unserialize($_SESSION['ticket_user'])->getPermission();
            // pass error and reload template accordingly
            helpers :: loadtemplate( 'settings', $pageElements);
            throw new SystemExit();

        } else {
            //ERROR: user is not logged in
            header("Location: index.php");
            throw new SystemExit();
        }
        
    }
    catch (PDOException $e) {
        //go to error page or something, because can't access website db
        print_r($e);
        throw new SystemExit();
    }
    
}