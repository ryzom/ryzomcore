<?php

function change_password(){
	
    try{
            if(isset($_SESSION["user"])){
                $webUser = new WebUsers();
                $params = Array( 'user' => $_SESSION["user"], 'CurrentPass' => $_POST["CurrentPass"], 'NewPass' => $_POST["NewPass"], 'ConfirmNewPass' => $_POST["ConfirmNewPass"]);
                $result = $webUser->check_change_password($params);
                if ($result == "success"){
                    //edit stuff into db
                }else{
                    $result['prevCurrentPass'] = $_POST["CurrentPass"];
                    $result['prevNewPass'] = $_POST["NewPass"];
                    $result['prevConfirmNewPass'] = $_POST["ConfirmNewPass"];
                    $result['permission'] = $_SESSION['permission'];
                    $result['no_visible_elements'] = 'FALSE';
                    helpers :: loadtemplate( 'settings', $result);
                    exit;
                }

            }	
	}catch (PDOException $e) {
	     //go to error page or something, because can't access website db
	     print_r($e);
	     exit;
	}
	
}

