<?php

function change_password(){
	
    try{
        //if logged in
        if(WebUsers::isLoggedIn()){
            
            if(isset($_POST['target_id'])){
                $adminChangesOther = false;
                //if target_id is the same as session id or is admin
                if(  ($_POST['target_id'] == $_SESSION['id']) ||  WebUsers::isAdmin()  ){
                    if($_POST['target_id'] == $_SESSION['id']){
                        $target_username = $_SESSION['user'];
                    }else{
                        $target_username = WebUsers::getUsername($_POST['target_id']);
                        //isAdmin is true when it's the admin, but the target_id != own id
                        $adminChangesOther = true;
                        $_POST["CurrentPass"] = "dummypass";
                    }
                    $id = $_POST['target_id'];
                    
                    $webUser = new WebUsers();
                    $params = Array( 'user' => $target_username, 'CurrentPass' => $_POST["CurrentPass"], 'NewPass' => $_POST["NewPass"], 'ConfirmNewPass' => $_POST["ConfirmNewPass"], 'adminChangesOther' => $adminChangesOther);
                    $result = $webUser->check_change_password($params);
                    if ($result == "success"){
                        //edit stuff into db
                        $hashpass = crypt($_POST["NewPass"], WebUsers::generateSALT());
                        print('success!');
                        exit;
                         
                    }else{
                        
                        $result['prevCurrentPass'] = $_POST["CurrentPass"];
                        $result['prevNewPass'] = $_POST["NewPass"];
                        $result['prevConfirmNewPass'] = $_POST["ConfirmNewPass"];
                        $result['permission'] = $_SESSION['permission'];
                        $result['no_visible_elements'] = 'FALSE';
                        $result['target_id'] = $_POST['target_id'];
                        if(isset($_GET['id'])){
                            if(WebUsers::isAdmin() && ($_POST['target_id'] != $_SESSION['id'])){
                                $result['isAdmin'] = "TRUE";
                            }
                        }
                        helpers :: loadtemplate( 'settings', $result);
                        exit;
                    }
                    
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

