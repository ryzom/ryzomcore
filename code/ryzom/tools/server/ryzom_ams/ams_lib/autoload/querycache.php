<?php

/**
* class for storing changes when shard is offline.
* @todo make sure that the querycache class is being used by the sync class and also for inserting the queries themselfs into it.
* Atm this class isn't used yet if I remember correctly
* @author Daan Janssens, mentored by Matthew Lagoe
*/

class Querycache{
    
    private $SID;  /**< The queries ID */ 
    private $type;  /**< The type of query*/ 
    private $query;  /**< The query itself (json encoded) */ 
    private $db;  /**< the db where the query  should be performed */ 
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
    
    
    /**
    * A constructor.
    * Empty constructor
    */
    public function __construct() {
    }
    
    /**
    * sets the object's attributes.
    * @param $values should be an array of the form array('SID' => sid, 'type' => type, 'query' => query, 'db' => db).
    */
    public function set($values) {
        $this->setSID($values['SID']);
        $this->setType($values['type']);
        $this->setQuery($values['query']);
        $this->setDb($values['db']);
    }
    
    
    /**
    * loads the object's attributes.
    * loads the object's attributes by giving a SID as parameter
    * @param $id the id of the querycaches row
    */
    public function load_With_SID( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ams_querycache WHERE SID=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->set($row);
    } 
    
    
    /**
    * updates the entry.
    */
    public function update(){
        $dbl = new DBLayer("lib");
        $query = "UPDATE ams_querycache SET type= :t, query = :q, db = :d WHERE SID=:id";
        $values = Array('id' => $this->getSID(), 't' => $this->getType(), 'q' => $this->getQuery(), 'd' => $this->getDb());
        $statement = $dbl->execute($query, $values);
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    /**
    * get SID attribute of the object.
    */
    public function getSID(){
        return $this->SID;
    }
   
    /**
    * get type attribute of the object.
    */
    public function getType(){
        return $this->type;
    }
    
    /**
    * get query attribute of the object.
    */
    public function getQuery(){
        return $this->query;
    }
    
    /**
    * get db attribute of the object.
    */
    public function getDb(){
        return $this->db;
    }
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
    
    /**
    * set SID attribute of the object.
    * @param $s integer id 
    */
    public function setSID($s){
        $this->SID = $s;
    }
   
    /**
    * set type attribute of the object.
    * @param $t type of the query, could be changePassword, changePermissions, changeEmail, createUser
    */
    public function setType($t){
        $this->type = $t;
    }
    
    /**
    * set query attribute of the object.
    * @param $q query string
    */
    public function setQuery($q){
        $this->query= $q;
    }
    
    /**
    * set db attribute of the object.
    * @param $d the name of the database in the config global var that we want to use.
    */
    public function setDb($d){
        $this->db= $d;
    }
    
}