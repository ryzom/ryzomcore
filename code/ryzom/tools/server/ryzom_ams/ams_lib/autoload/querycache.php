<?php

class Querycache{
    
    private $SID;
    private $type;
    private $query;
    private $db;
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    
    
     ////////////////////////////////////////////Methods////////////////////////////////////////////////////
    public function __construct() {
    }
    
    //set values
    public function set($values) {
        $this->setSID($values['SID']);
        $this->setType($values['type']);
        $this->setQuery($values['query']);
        $this->setDb($values['db']);
    }
    
    
    //return constructed element based on SID
    public function load_With_SID( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ams_querycache WHERE SID=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->set($row);
    } 
    
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer("lib");
        $query = "UPDATE ams_querycache SET type= :t, query = :q, db = :d WHERE SID=:id";
        $values = Array('id' => $this->getSID(), 't' => $this->getType(), 'q' => $this->getQuery(), 'd' => $this->getDb());
        $statement = $dbl->execute($query, $values);
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getSID(){
        return $this->SID;
    }
   
   
    public function getType(){
        return $this->type;
    }
    
    
    public function getQuery(){
        return $this->query;
    }
    
    public function getDb(){
        return $this->db;
    }
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
    
    public function setSID($s){
        $this->SID = $s;
    }
   
   
    public function setType($t){
        $this->type = $t;
    }
    
    public function setQuery($q){
        $this->query= $q;
    }
    
    public function setDb($d){
        $this->db= $d;
    }
    
}