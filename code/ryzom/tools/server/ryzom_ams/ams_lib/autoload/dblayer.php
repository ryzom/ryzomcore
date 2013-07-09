<?php
class DBLayer{
    
    private $PDO;
    
    function __construct($db)
    {
        try{
            $dsn  = "mysql:";
            $dsn .= "host=".   $db['host'].";";
            $dsn .= "dbname=". $db['name'].";";
            $dsn .= "port=".   $db['port'].";";
        
            $opt = array(
                PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
                PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC
            );
            $this->PDO = new PDO($dsn,$db['user'],$db['pass'], $opt);
        }catch (PDOException $e) {
            throw $e;
        }
    }
    
    public function executeWithoutParams($query){
        try{
            $statement = $this->PDO->prepare($query);
            $statement->execute();
            return $statement;
        }catch (PDOException $e) {
            throw $e;
        }
    }
    
    public function execute($query,$params){
        try{
            $statement = $this->PDO->prepare($query);
            $statement->execute($params);
            return $statement;
        }catch (PDOException $e) {
            throw $e;
        }
    }
    
    public function executeReturnId($query,$params){
        try{
            $statement = $this->PDO->prepare($query);
            $this->PDO->beginTransaction();
            $statement->execute($params);
            $lastId =$this->PDO->lastInsertId();
            $this->PDO->commit();
            return $lastId;
        }catch (PDOException $e) {
            throw $e;
        }
    }
    
}