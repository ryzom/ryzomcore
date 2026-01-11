<?php
/**
* This function is beign used to change the users emailaddress info.
* It will first check if the user who executed this function is the person of whom the emailaddress is or if it's a mod/admin. If this is not the case the page will be redirected to an error page.
* The emailaddress will be validated first. If the checking was successful the email will be updated and the settings template will be reloaded. Errors made by invalid data will be shown
* also after reloading the template.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function change_mail(){
	
    try{
        //if logged in
        if(WebUsers::isLoggedIn()){
            
            if(isset($_POST['target_id'])){
		
                //check if the user who executed this function is the person of whom the emailaddress is or if it's a mod/admin.
                if(  ($_POST['target_id'] == $_SESSION['id']) || Ticket_User::isMod(unserialize($_SESSION['ticket_user'])) ){
                    if($_POST['target_id'] == $_SESSION['id']){
			//if the email is of the executing user himself
                        $target_username = $_SESSION['user'];
                    }else{
			//if its from someone else.
			$webUser = new WebUsers($_POST['target_id']);
                        $target_username = $webUser->getUsername();
                    }
                    
                    $webUser = new WebUsers($_POST['target_id']);
		    //check if emailaddress is valid.
		    $reply = $webUser->checkEmail($_POST['NewEmail']);
		    
		    global $SITEBASE;
                    require_once($SITEBASE . '/inc/settings.php');
                    $result = settings();
		    
		    if ( $reply != "success" ){
			$result['EMAIL_ERROR'] = 'TRUE';
		    }else{
			$result['EMAIL_ERROR'] = 'FALSE';
		    }
		    $result['prevNewEmail'] = filter_var($_POST["NewEmail"], FILTER_SANITIZE_EMAIL);
		    
                    if ($reply== "success"){
			//if validation was successful, update the emailaddress
                        $status = WebUsers::setEmail($target_username, filter_var($_POST["NewEmail"], FILTER_SANITIZE_EMAIL) );
                        if($status == 'ok'){
                            $result['SUCCESS_MAIL'] = "OK";
                        }else if($status == 'shardoffline'){
                             $result['SUCCESS_MAIL'] = "SHARDOFF";
                        }
                        $result['permission'] = unserialize($_SESSION['ticket_user'])->getPermission();
                        $result['no_visible_elements'] = 'FALSE';
			$result['username'] = $_SESSION['user'];
                        $result['target_id'] = $_POST['target_id'];
                        if(isset($_GET['id'])){
                            if(Ticket_User::isMod(unserialize($_SESSION['ticket_user'])) && ($_POST['target_id'] != $_SESSION['id'])){
                                $result['isMod'] = "TRUE";
                            }
                        }
                        helpers :: loadtemplate( 'settings', $result);
                        throw new SystemExit();
                         
                    }else{
			$result['EMAIL'] = $reply;
                        $result['permission'] = unserialize($_SESSION['ticket_user'])->getPermission();
                        $result['no_visible_elements'] = 'FALSE';
                        $result['username'] = $_SESSION['user'];
                        $result['target_id'] = $_POST['target_id'];
                        if(isset($_GET['id'])){
                            if(Ticket_User::isMod(unserialize($_SESSION['ticket_user'])) && ($_POST['target_id'] != $_SESSION['id'])){
                                $result['isMod'] = "TRUE";
                            }
                        }
                        $result['CEMAIL_ERROR'] = true;
                        helpers :: loadtemplate( 'settings', $result);
                        throw new SystemExit();
                    }
                    
                }else{
                    //ERROR: permission denied!
		    $_SESSION['error_code'] = "403";
                    header("Location: index.php?page=error");
                    throw new SystemExit();
                }
        
            }else{
                //ERROR: The form was not filled in correctly
		header("Location: index.php?page=settings");
		throw new SystemExit();
            }    
        }else{
            //ERROR: user is not logged in
	    header("Location: index.php");
	    throw new SystemExit();
        }
                  
    }catch (PDOException $e) {
         //go to error page or something, because can't access website db
         print_r($e);
         throw new SystemExit();
    }
    
}

