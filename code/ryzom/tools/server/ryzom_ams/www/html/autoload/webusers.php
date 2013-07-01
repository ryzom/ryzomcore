<?php

class WebUsers extends Users{
    
     /**
     * Function checkUserNameExists
     *
     * @takes $username
     * @return string Info: Returns true or false if the user is in the web db.
     */
     protected function checkUserNameExists($username){
            global $cfg;
            $dbw = new DBLayer($cfg['db']['web']);
            return $dbw->execute("SELECT * FROM ams_user WHERE Login = :name",array('name' => $username))->rowCount();  
     }
    
    
    /**
     * Function checkEmailExists
     *
     * @takes $username
     * @return string Info: Returns true or false if the user is in the www db.
     */
     protected function checkEmailExists($email){
            global $cfg;
            $dbw = new DBLayer($cfg['db']['web']);
            return $dbw->execute("SELECT * FROM ams_user WHERE Email = :email",array('email' => $email))->rowCount();
     }
}