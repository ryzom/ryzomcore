<?php
/**
* Handles the linkage of users being in a support group.
* Moderators and Admins can be part of a support group, this class offers functionality to check if a link between a user and group is existing.
* @author Daan Janssens, mentored by Matthew Lagoe
* 
*/
class In_Support_Group{
    
    private $user; /**< The id of the user being in a support group */ 
    private $group; /**< The id of the support group*/ 
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    /**
    * Check if user is in in_support_group.
    * @param $user_id the id of the user.
    * @param $group_id the id of the support group.
    * @return true is returned in case the user is in the support group, else false is returned.
    */
    public static function userExistsInSGroup( $user_id, $group_id) {
        $dbl = new DBLayer("lib");
        //check if name is already used
        if(  $dbl->select("in_support_group", array('user_id' => $user_id, 'group_id' => $group_id), "`User` = :user_id and `Group` = :group_id")->rowCount() ){
            return true;
        }else{
            return false;
        } 
    }
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
     
    /**
    * A constructor.
    * Empty constructor
    */
    public function __construct() {
    }
    
    
    /**
    * sets the object's attributes.
    * @param $values should be an array of the form array('User' => user_id, 'Group' => support_groups_id).
    */
    public function set($values) {
        $this->setUser($values['User']);
        $this->setGroup($values['Group']);
    }
    
    
    /**
    * creates a new 'in_support_group' entry.
    * this method will use the object's attributes for creating a new 'in_support_group' entry in the database.
    */
    public function create() {
        $dbl = new DBLayer("lib");
        $dbl->insert("`in_support_group`", Array('User' => $this->user, 'Group' => $this->group);
    }
    
    
    /**
    * deletes an existing 'in_support_group' entry.
    * this method will use the object's attributes for deleting an existing 'in_support_group' entry in the database.
    */
    public function delete() {
        $dbl = new DBLayer("lib");
        $dbl->delete("`in_support_group`", array('user_id' => $this->getUser() ,'group_id' => $this->getGroup(), "`User` = :user_id and `Group` = :group_id");
    }

    /*
    public function load($group_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM `in_support_group` WHERE `Group` = :group_id", Array('group_id' => $group_id));
        $row = $statement->fetch();
        $this->set($row);
    }
    */
    

    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    /**
    * get user attribute of the object.
    */
    public function getUser(){
        return $this->user;
    }
    
    
    /**
    * get group attribute of the object.
    */
    public function getGroup(){
        return $this->group;
    }
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////

    /**
    * set user attribute of the object.
    * @param $u integer id of the user
    */
    public function setUser($u){
        $this->user = $u;
    }
    
    
    /**
    * set group attribute of the object.
    * @param $g integer id of the support group
    */
    public function setGroup($g){
        $this->group = $g;
    }
   
    
}
