<?php

class Ticket_Reply{
    private $tReplyId;
    private $ticket;
    private $content;
    private $author;
    private $timestamp;
    private $db;
    
    //////////////////////////////////Methods/////////////////////////////////
    
    public function __construct($db_data) {
        $this->db = $db_data;
    }


    //Set ticket_reply object
    public function set($t,$c,$a){
        $this->ticket = $t;
        $this->content = $c;
        $this->author = $a;
    }
    
    //create ticket by writing private data to DB.
    public function create(){
        $dbl = new DBLayer($this->db);
        $query = "INSERT INTO ticket_reply (Ticket, Content, Author, Timestamp) VALUES (:ticket, :content, :author, now())";
        $values = Array('ticket' => $this->ticket, 'content' => $this->content, 'author' => $this->author);
        $dbl->execute($query, $values);
    }
    
    //return constructed element based on TCategoryId
    public static function constr_TReplyId( $id, $db_data) {
        $instance = new self($db_data);
        $instance->setTReplyId($id);
        return $instance;
    }

    //return constructed element based on TId
    public function load_With_TReplyId( $id) {
        $dbl = new DBLayer($this->db);
        $statement = $dbl->execute("SELECT * FROM ticket_reply WHERE TReplyId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->tReplyId = $row['TReplyId'];
        $this->ticket = $row['Ticket'];
        $this->content = $row['Content'];
        $this->author = $row['Author'];
        $this->timestamp = $row['Timestamp'];       
    }
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer($this->db);
        $query = "UPDATE ticket SET Ticket = :ticket, Content = :content, Author = :author, Timestamp = :timestamp WHERE TReplyId=:id";
        $values = Array('id' => $this->tReplyId, 'timestamp' => $this->timestamp, 'ticket' => $this->ticket, 'content' => $this->content, 'author' => $this->author);
        $statement = $dbl->execute($query, $values);
    }
    
    
    //////////////////////////////////Getters/////////////////////////////////
    public function getTicket(){
        return $this->ticket;
    }
   
   
    public function getContent(){
        return $this->content;
    }
    
    public function getAuthor(){
        return $this->author;
    }
    
    public function getTimestamp(){
        return $this->timestamp;
    }
    
    
    public function getTReplyId(){
        return $this->tReplyId;
    }
    
    
    ///////////////////////////////////setters////////////////////////////////
    public function setTicket($t){
        $this->ticket = $t;
    }
   
   
    public function setContent($c){
        $this->content = $c;
    }
    
    public function setAuthor($a){
        $this->author =  $a;
    }
    
    public function setTimestamp($t){
        $this->timestamp = $t;
    }
    
    
    public function setTReplyId($i){
        $this->tReplyId = $i;
    }
}