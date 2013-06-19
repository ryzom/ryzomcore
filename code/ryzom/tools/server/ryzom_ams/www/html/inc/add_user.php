<?php

function add_user(){
     $result = Users :: check_Register();
     //print_r($result);
     // if all are good then create user
     if ( $result == "success"){
          $edit = array(
             'name' => $_POST["Username"],
              'pass' => $_POST["Password"],
              'mail' => $_POST["Email"],
              'init' => $_POST["Email"],
              'unhashpass' => $_POST["Password"],
              'status' => 1,
              'access' => REQUEST_TIME
              );
          //header( 'Location: email_sent.php' );
          write_user( $edit );
          exit;
     }else{
          // pass error
          $result['prevUsername'] = $_POST["Username"];
          $result['prevPassword'] = $_POST["Password"];
          $result['prevConfirmPass'] = $_POST["ConfirmPass"];
          $result['prevEmail'] = $_POST["Email"];
          $result['no_visible_elements'] = 'TRUE';
          helpers :: loadtemplate( 'register', $result);
          exit;
     }
}


function write_user(){


     // add user locally here
     print('Awesome');
     }

