<?php
class DBLayer{
    
    private $PDO;
    
    function __construct($db)
    {
        global $cfg;
        $dsn  = "mysql:";
        $dsn .= "host=".   $cfg['db'][$db]['host'].";";
        $dsn .= "dbname=". $cfg['db'][$db]['name'].";";
        $dsn .= "port=".   $cfg['db'][$db]['port'].";";
    
        $opt = array(
            PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
            PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC
        );
        $this->PDO = new PDO($dsn,$cfg['db'][$db]['user'],$cfg['db'][$db]['pass'], $opt);

    }
    
    public function executeWithoutParams($query){
        $statement = $this->PDO->prepare($query);
        $statement->execute();
        return $statement;
    }
    
    public function execute($query,$params){
        $statement = $this->PDO->prepare($query);
        $statement->execute($params);
        return $statement;
    }
    
    public function executeReturnId($query,$params){
        $statement = $this->PDO->prepare($query);
        $this->PDO->beginTransaction();
        $statement->execute($params);
        $lastId =$this->PDO->lastInsertId();
        $this->PDO->commit();
        return $lastId;
    }
    
}