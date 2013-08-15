<?php

class In_Support_Group{
    
    private $user;
    private $group;
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
  
    //check if user is in in_support_group
    public static function userExistsInSGroup( $user_id, $group_id) {
        $dbl = new DBLayer("lib");
        //check if name is already used
        if(  $dbl->execute(" SELECT * FROM `in_support_group` WHERE `User` = :user_id and `Group` = :group_id ", array('user_id' => $user_id, 'group_id' => $group_id) )->rowCount() ){
            return true;
        }else{
            return false;
        } 
    }
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
     
    public function __construct() {
    }
    
    //set values
    public function set($values) {
        $this->setUser($values['User']);
        $this->setGroup($values['Group']);
    }
    
    public function create() {
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO `in_support_group` (`User`,`Group`) VALUES (:user, :group)";
        $values = Array('user' => $this->user, 'group' => $this->group);
        $dbl->execute($query, $values);
    }
    
    //delete entry
    public function delete() {
        $dbl = new DBLayer("lib");
        $query = "DELETE FROM `in_support_group` WHERE `User` = :user_id and `Group` = :group_id";
        $values = array('user_id' => $this->getUser() ,'group_id' => $this->getGroup());
        $dbl->execute($query, $values);
    }

    //Load with sGroupId
    public function load( $user_id, $group_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM `in_support_group` WHERE `Group` = :group_id", Array('group_id' => $group_id));
        $row = $statement->fetch();
        $this->set($row);
    }
    

    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getUser(){
        return $this->user;
    }
    
    public function getGroup(){
        return $this->group;
    }
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////

    public function setUser($u){
        $this->user = $u;
    }
    
    public function setGroup($g){
        $this->group = $g;
    }
   
    
}