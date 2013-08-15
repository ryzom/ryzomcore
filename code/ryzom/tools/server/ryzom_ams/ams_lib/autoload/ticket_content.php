<?php

class Ticket_Content{
    
    private $tContentId;
    private $content;
    
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    //return constructed element based on TCategoryId
    public static function constr_TContentId( $id) {
        $instance = new self();
        $instance->setTContentId($id);
        return $instance;
    } 

    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
    
    public function __construct() {
    }

    //Creates a ticket_content entry in the DB
    public function create() {
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO ticket_content (Content) VALUES (:content)";
        $values = Array('content' => $this->content);
        $this->tContentId = $dbl->executeReturnId($query, $values); ;
    }
    
    //return constructed element based on TContentId
    public function load_With_TContentId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_content WHERE TContentId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->tContentId = $row['TContentId'];
        $this->content = $row['Content'];
    }
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer("lib");
        $query = "UPDATE ticket_content SET Content = :content WHERE TContentId=:id";
        $values = Array('id' => $this->tContentId, 'content' => $this->content);
        $statement = $dbl->execute($query, $values);
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getContent(){
        if ($this->content == ""){
            $this->load_With_TContentId($this->tContentId);
        }
        return $this->content;
    }
    
    
    public function getTContentId(){
        return $this->tContentId;
    }
    
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
    
    public function setContent($c){
        $this->content = $c;
    }
   
    public function setTContentId($c){
        $this->tContentId = $c;
    }
    
}