<?php

function change_password(){
	
    try{
        //if logged in
        if(WebUsers::isLoggedIn()){
            
            if(isset($_POST['target_id'])){
                $adminChangesOther = false;
                //if target_id is the same as session id or is admin
                if(  ($_POST['target_id'] == $_SESSION['id']) ||  Ticket_User::isMod($_SESSION['ticket_user'])  ){
                    if($_POST['target_id'] == $_SESSION['id']){
                        $target_username = $_SESSION['user'];
                    }else{
			$webUser = new WebUsers($_POST['target_id']);
                        $target_username = $webUser->getUsername();
                        //isAdmin is true when it's the admin, but the target_id != own id
                        $adminChangesOther = true;
                        $_POST["CurrentPass"] = "dummypass";
                    }
                    
                    $webUser = new WebUsers($_POST['target_id']);
                    $params = Array( 'user' => $target_username, 'CurrentPass' => $_POST["CurrentPass"], 'NewPass' => $_POST["NewPass"], 'ConfirmNewPass' => $_POST["ConfirmNewPass"], 'adminChangesOther' => $adminChangesOther);
                    $result = $webUser->check_change_password($params);
                    if ($result == "success"){
                        //edit stuff into db
                        global $SITEBASE;
                        require_once($SITEBASE . 'inc/settings.php');
                        $succresult = settings();
                        $hashpass = crypt($_POST["NewPass"], WebUsers::generateSALT());
                        $status = WebUsers::setPassword($target_username, $hashpass);
                        if($status == 'ok'){
                            $succresult['SUCCESS_PASS'] = "OK";
                        }else if($status == 'shardoffline'){
                             $succresult['SUCCESS_PASS'] = "SHARDOFF";
                        }
                        $succresult['permission'] = $_SESSION['ticket_user']->getPermission();
                        $succresult['no_visible_elements'] = 'FALSE';
                        $succresult['username'] = $_SESSION['user'];
                        $succresult['target_id'] = $_POST['target_id'];
                        helpers :: loadtemplate( 'settings', $succresult);
                        exit;
                         
                    }else{
			
                        $result['prevCurrentPass'] = filter_var($_POST["CurrentPass"], FILTER_SANITIZE_STRING);
                        $result['prevNewPass'] = filter_var($_POST["NewPass"], FILTER_SANITIZE_STRING);
                        $result['prevConfirmNewPass'] = filter_var($_POST["ConfirmNewPass"], FILTER_SANITIZE_STRING);
                        $result['permission'] =  $_SESSION['ticket_user']->getPermission();
                        $result['no_visible_elements'] = 'FALSE';
                        $result['username'] = $_SESSION['user'];
                        $result['target_id'] = $_POST['target_id'];

                        global $SITEBASE;
                        require_once($SITEBASE . 'inc/settings.php');
                        $settings = settings();
                        
                        $result = array_merge($result,$settings);
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


