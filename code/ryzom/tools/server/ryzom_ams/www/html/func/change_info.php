<?php

function change_info(){
    
 try{
        //if logged in
        if(WebUsers::isLoggedIn()){
            
            if(isset($_POST['target_id'])){
		
                
                if(  ($_POST['target_id'] == $_SESSION['id']) ||  WebUsers::isAdmin()  ){
                    if($_POST['target_id'] == $_SESSION['id']){
                        $target_username = $_SESSION['user'];
                    }else{
                        $target_username = WebUsers::getUsername($_POST['target_id']);
                    }
                    
                    $webUser = new WebUsers();
                    //use current info to check for changes
                    $current_info = $webUser->getInfo($_POST['target_id']);
                    //TODO: XSS filtering
                    
                    //make the query that will update the data.
                    $updated = false;
                    $values = Array();
                    $values['user'] = $target_username;
                    $query = "UPDATE ams_user SET ";
                    if(($_POST['FirstName'] != "") && ($_POST['FirstName'] != $current_info['FirstName'])){
                        $query = $query . "FirstName = :fName ";
                        $updated = true;
                        $values['fName'] = $_POST['FirstName'];
                    }
                    if(($_POST['LastName'] != "") && ($_POST['LastName'] != $current_info['LastName'])){
			if($updated){
			 $query = $query . ", LastName = :lName ";
			}else{
			 $query = $query . "LastName = :lName ";
			}
                        $updated = true;
                        $values['lName'] = $_POST['LastName'];
                    }
                    //TODO: add the other fields too
                    $query = $query . "WHERE Login = :user";

                    //if some field is update then:
                    if($updated){
                        global $cfg;
                        //execute the query in the web DB.
                        $dbw = new DBLayer($cfg['db']['web']);
                        $dbw->execute($query,$values);  
                    }

                    global $SITEBASE;
                    require_once($SITEBASE . 'inc/settings.php');
                    $result = settings();
                    if($updated){
                        $result['info_updated'] = "OK";
                    }
                    $result['permission'] = $_SESSION['permission'];
                    $result['username'] = $_SESSION['user'];
                    $result['no_visible_elements'] = 'FALSE';
                    $result['target_id'] = $_POST['target_id'];
                    if(isset($_GET['id'])){
                        if(WebUsers::isAdmin() && ($_POST['target_id'] != $_SESSION['id'])){
                            $result['isAdmin'] = "TRUE";
                        }
                    }
                    helpers :: loadtemplate( 'settings', $result);
                    exit;
                    
                }else{
                    //ERROR: permission denied!
                }
        
            }else{
                //ERROR: The form was not filled in correclty
            }    
        }else{
            //ERROR: user is not logged in
            exit;
        }
                  
    }catch (PDOException $e) {
         //go to error page or something, because can't access website db
         print_r($e);
         exit;
    }   
}