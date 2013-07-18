<?php

class WebUsers extends Users{
    
     /**
     * Function checkUserNameExists
     *
     * @takes $username
     * @return string Info: Returns true or false if the user is in the web db.
     */
     protected function checkUserNameExists($username){
            $dbw = new DBLayer("web");
            return $dbw->execute("SELECT * FROM ams_user WHERE Login = :name",array('name' => $username))->rowCount();  
     }
    
    
    /**
     * Function checkEmailExists
     *
     * @takes $username
     * @return string Info: Returns true or false if the user is in the www db.
     */
     protected function checkEmailExists($email){
            $dbw = new DBLayer("web");
            return $dbw->execute("SELECT * FROM ams_user WHERE Email = :email",array('email' => $email))->rowCount();
     }
     
     
    /**
     * Function checkUserPassMatch
     *
     * @takes $username,$password
     * @return string Info: Returns true or false if a login match is found in the web db
     */
     public function checkLoginMatch($username,$password){
        $dbw = new DBLayer("web");
        $statement = $dbw->execute("SELECT * FROM ams_user WHERE Login=:user", array('user' => $username));
        $row = $statement->fetch();
        
        $salt = substr($row['Password'],0,2);
        $hashed_input_pass = crypt($password, $salt);
        if($hashed_input_pass == $row['Password']){
              return $row;
        }else{
              return "fail";
        }	
     }
     
    public function getId($username){
        $dbw = new DBLayer("web");
        $statement = $dbw->execute("SELECT * FROM ams_user WHERE Login=:username", array('username' => $username));
        $row = $statement->fetch();
        return $row['UId'];
    }
    
    public function getUsername($id){
        $dbw = new DBLayer("web");
        $statement = $dbw->execute("SELECT * FROM ams_user WHERE UId=:id", array('id' => $id));
        $row = $statement->fetch();
        return $row['Login'];
    }
    
    public function getEmail($id){
        $dbw = new DBLayer("web");
        $statement = $dbw->execute("SELECT * FROM ams_user WHERE UId=:id", array('id' => $id));
        $row = $statement->fetch();
        return $row['Email'];
    }
    
    public function getInfo($id){
        $dbw = new DBLayer("web");
        $statement = $dbw->execute("SELECT * FROM ams_user WHERE UId=:id", array('id' => $id));
        $row = $statement->fetch();
        $result = Array('FirstName' => $row['FirstName'], 'LastName' => $row['LastName'], 'Gender' => $row['Gender'], 'Country' => $row['Country']);
        return $result;
    }
    
    public function isLoggedIn(){
        if(isset($_SESSION['user'])){
            return true;
        }
        return false;
    }
    
    public function setPassword($user, $pass){
        $reply = WebUsers::setAmsPassword($user, $pass);
        $values = Array('user' => $user, 'pass' => $pass);
         try {
               //make connection with and put into shard db
               $dbw = new DBLayer("web");
               $dbw->execute("UPDATE ams_user SET Password = :pass WHERE Login = :user ",$values);
          }
          catch (PDOException $e) {
            //ERROR: the web DB is offline
          }
        return $reply;
    }
    
     public function setEmail($user, $mail){
        $reply = WebUsers::setAmsEmail($user, $mail);
        $values = Array('user' => $user, 'mail' => $mail);
         try {
               //make connection with and put into shard db
               $dbw = new DBLayer("web");
               $dbw->execute("UPDATE ams_user SET Email = :mail WHERE Login = :user ",$values);
          }
          catch (PDOException $e) {
            //ERROR: the web DB is offline
          }
        return $reply;
    }
    
    public function getUsers(){
        $dbl = new DBLayer("web");
        $data = $dbl->executeWithoutParams("SELECT * FROM ams_user");
        return $data;
    }
    
}