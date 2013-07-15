<?php

class Support_Group{
    
    private $sGroupId;
    private $name;
    private $tag;
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    //return all groups
    public static function getGroups() {
        $dbl = new DBLayer("lib");
        $statement = $dbl->executeWithoutParams("SELECT * FROM support_group ORDER BY Name ASC");
        $rows = $statement->fetchAll();
        $result = Array();
        foreach($rows as $group){

            $instanceGroup = new self();
            $instanceGroup->set($group);
            $result[] = $instanceGroup;
        }
        return $result; 
    }
    
    //wrapper for creating a support group
    public static function createSupportGroup( $name, $tag) {
        
        if(strlen($name) < 21 && strlen($name) > 4 &&strlen($tag) < 8  && strlen($tag) > 1 ){
            $notExists = self::supportGroup_NotExists($name, $tag);
            if ( $notExists == "SUCCESS" ){
                $sGroup = new self();
                $sGroup->setName($name);
                $sGroup->setTag($tag);
                $sGroup->create();
                return "SUCCESS";
            }else{
                //return NAME_TAKEN  or TAG_TAKEN
                return $notExists;
            }
        }else{
            //RETURN ERROR that indicates SIZE
            return "SIZE_ERROR";
        } 
    }

    //check if group exists
    public static function supportGroup_NotExists( $name, $tag) {
        $dbl = new DBLayer("lib");
        //check if name is already used
        if(  $dbl->execute("SELECT * FROM support_group WHERE Name = :name",array('name' => $name))->rowCount() ){
            return "NAME_TAKEN";
        }
        else if(  $dbl->execute("SELECT * FROM support_group WHERE Tag = :tag",array('tag' => $tag))->rowCount() ){
            return "TAG_TAKEN";
        }else{
            return "SUCCESS";
        } 
    }

    //return constructed element based on SGroupId
    public static function constr_SGroupId( $id) {
        $instance = new self();
        $instance->setSGroup($id);
        return $instance;
    }
    
    //returns list of all users that are enlisted to a support group
    public static function getAllUsersOfSupportGroup($id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_log INNER JOIN ticket_user ON ticket_log.Author = ticket_user.TUserId and ticket_log.Ticket=:id", array('id' => $ticket_id));
        $row = $statement->fetchAll();
        $result = Array();
        foreach($row as $log){
            $instance = new self();
            $instance->set($log);
            $result[] = $instance;
        }
        return $result; 
    }
     
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
     
    public function __construct() {
    }
    
    //set values
    public function set($values) {
        $this->setSGroupId($values['SGroupId']);
        $this->setName($values['Name']);
        $this->setTag($values['Tag']);
    }
    
    public function create() {
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO support_group (Name, Tag) VALUES (:name, :tag)";
        $values = Array('name' => $this->name, 'tag' => $this->tag);
        $dbl->execute($query, $values);
    } 

    //Load with sGroupId
    public function load_With_SGroupId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_log WHERE TLogId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->set($row);
    }
    
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer("lib");
        $query = "UPDATE ticket_log SET Timestamp = :timestamp, Query = :query, Author = :author, Ticket = :ticket WHERE TLogId=:id";
        $values = Array('id' => $this->getTLogId(), 'timestamp' => $this->getTimestamp(), 'query' => $this->getQuery(), 'author' => $this->getAuthor(), 'ticket' => $this->getTicket() );
        $statement = $dbl->execute($query, $values);
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getSGroupId(){
        return $this->sGroupId;
    }
    
    public function getName(){
        return $this->name;
    }
    
    public function getTag(){
        return $this->tag;
    }
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
     
    public function setSGroupId($id){
        $this->sGroupId = $id;
    }
    
    public function setName($n){
        $this->name = $n;
    }
    
    public function setTag($t){
        $this->tag = $t;
    }
   
    
}