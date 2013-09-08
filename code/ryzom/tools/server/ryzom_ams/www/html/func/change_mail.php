<?php

function change_mail(){
	
    try{
        //if logged in
        if(WebUsers::isLoggedIn()){
            
            if(isset($_POST['target_id'])){
		
                
                if(  ($_POST['target_id'] == $_SESSION['id']) || Ticket_User::isMod($_SESSION['ticket_user']) ){
                    if($_POST['target_id'] == $_SESSION['id']){
                        $target_username = $_SESSION['user'];
                    }else{
			$webUser = new WebUsers($_POST['target_id']);
                        $target_username = $webUser->getUsername();
                    }
                    
                    $webUser = new WebUsers($_POST['target_id']);
		    $reply = $webUser->checkEmail($_POST['NewEmail']);
		    
		    global $SITEBASE;
                    require_once($SITEBASE . 'inc/settings.php');
                    $result = settings();
		    
		    if ( $reply != "success" ){
			$result['EMAIL_ERROR'] = 'TRUE';
		    }else{
			$result['EMAIL_ERROR'] = 'FALSE';
		    }
		    $result['prevNewEmail'] = filter_var($_POST["NewEmail"], FILTER_SANITIZE_EMAIL);
		    
                    if ($reply== "success"){
                        $status = WebUsers::setEmail($target_username, filter_var($_POST["NewEmail"], FILTER_SANITIZE_EMAIL) );
                        if($status == 'ok'){
                            $result['SUCCESS_MAIL'] = "OK";
                        }else if($status == 'shardoffline'){
                             $result['SUCCESS_MAIL'] = "SHARDOFF";
                        }
                        $result['permission'] = $_SESSION['ticket_user']->getPermission();
                        $result['no_visible_elements'] = 'FALSE';
			$result['username'] = $_SESSION['user'];
                        $result['target_id'] = $_POST['target_id'];
                        if(isset($_GET['id'])){
                            if(Ticket_User::isMod($_SESSION['ticket_user']) && ($_POST['target_id'] != $_SESSION['id'])){
                                $result['isMod'] = "TRUE";
                            }
                        }
                        helpers :: loadtemplate( 'settings', $result);
                        exit;
                         
                    }else{
			$result['EMAIL'] = $reply;
                        $result['permission'] = $_SESSION['ticket_user']->getPermission();
                        $result['no_visible_elements'] = 'FALSE';
                        $result['username'] = $_SESSION['user'];
                        $result['target_id'] = $_POST['target_id'];
                        if(isset($_GET['id'])){
                            if(Ticket_User::isMod($_SESSION['ticket_user']) && ($_POST['target_id'] != $_SESSION['id'])){
                                $result['isMod'] = "TRUE";
                            }
                        }
                        helpers :: loadtemplate( 'settings', $result);
                        exit;
                    }
                    
                }else{
                    //ERROR: permission denied!
		    $_SESSION['error_code'] = "403";
                    header("Location: index.php?page=error");
                    exit;
                }
        
            }else{
                //ERROR: The form was not filled in correclty
		header("Location: index.php?page=settings");
		exit;
            }    
        }else{
            //ERROR: user is not logged in
	    header("Location: index.php");
	    exit;
        }
                  
    }catch (PDOException $e) {
         //go to error page or something, because can't access website db
         print_r($e);
         exit;
    }
    
}


