<?php

class Ticket_Reply{
    private $tReplyId;
    private $ticket;
    private $content;
    private $author;
    private $timestamp;
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    //return constructed element based on TCategoryId
    public static function constr_TReplyId( $id) {
        $instance = new self();
        $instance->setTReplyId($id);
        return $instance;
    }
    
    //return constructed element based on TCategoryId
    public static function getRepliesOfTicket( $ticket_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_reply INNER JOIN ticket_content ON ticket_reply.Content = ticket_content.TContentId and ticket_reply.Ticket=:id", array('id' => $ticket_id));
        $row = $statement->fetchAll();
        $result = Array();
        foreach($row as $tReply){
            $instanceReply = new self();
            $instanceReply->setTReplyId($tReply['TReplyId']);
            $instanceReply->setTimestamp($tReply['Timestamp']);
            $instanceReply->set($tReply['Ticket'],$tReply['Content'],$tReply['Author']);
            
            $instanceContent = new Ticket_Content();
            $instanceContent->setTContentId($tReply['TContentId']);
            $instanceContent->setContent($tReply['Content']);
            
            $instanceReply->setContent($instanceContent);
            
            $result[] = $instanceReply;
        }
        return $result; 
    }
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
    
    public function __construct() {
    }


    //Set ticket_reply object
    public function set($t,$c,$a){
        $this->setTicket($t);
        $this->setContent($c);
        $this->setAuthor($a);
    }
    
    //create ticket by writing private data to DB.
    public function create(){
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO ticket_reply (Ticket, Content, Author, Timestamp) VALUES (:ticket, :content, :author, now())";
        $values = Array('ticket' => $this->ticket, 'content' => $this->content, 'author' => $this->author);
        $dbl->execute($query, $values);
    }

    //return constructed element based on TId
    public function load_With_TReplyId( $id) {
        $dbl = new DBLayer("lib");
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
        $dbl = new DBLayer("lib");
        $query = "UPDATE ticket SET Ticket = :ticket, Content = :content, Author = :author, Timestamp = :timestamp WHERE TReplyId=:id";
        $values = Array('id' => $this->tReplyId, 'timestamp' => $this->timestamp, 'ticket' => $this->ticket, 'content' => $this->content, 'author' => $this->author);
        $statement = $dbl->execute($query, $values);
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
     
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
    
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
     
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