<?php

class Sync{
    
    /**
     *
     * Function syncdata
     *
     * @takes        Nothing
     * @return      array $values
     *
     * Info: Runs functions to finish syncing data when shard is offline
     *
     */
    static public function syncdata () {

        global $LIBDBHOST;
        global $LIBDBPORT;
        global $LIBDBNAME;
        global $LIBDBUSERNAME;
        global $LIBDBPASSWORD;
        
        global $SHARDDBHOST;
        global $SHARDDBPORT;
        global $SHARDDBNAME; 
        global $SHARDDBUSERNAME;
        global $SHARDDBPASSWORD;
        
        try {
            $dbl = new PDO("mysql:host=$LIBDBHOST;port=$LIBDBPORT;dbname=$LIBDBNAME", $LIBDBUSERNAME, $LIBDBPASSWORD);
            $dbl->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
            $statement = $dbl->prepare("SELECT * FROM ams_querycache");
            $statement->execute();
            $rows = $statement->fetchAll();

            $dbs = new PDO("mysql:host=$SHARDDBHOST;port=$SHARDDBPORT;dbname=$SHARDDBNAME", $SHARDDBUSERNAME, $SHARDDBPASSWORD);
            $dbs->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
            
            foreach ($rows as $record) {
    
                switch($record['type']) {
                    case 'createPermissions':
                    case 'user_edit':
                    case 'createUser': 
                        $query = json_decode($record['query']);
                        //make connection with and put into shard db
                        $statement = $dbs->prepare("INSERT INTO user (Login, Password, Email) VALUES (?, ?, ?)");
                        $statement->execute($query);
                        
                        $statement = $dbl->prepare("DELETE FROM ams_querycache WHERE SID=:SID");
                        $query = array('SID' => $record['SID']);
                        $statement->execute($query);
                
                        
                }
            }
            print('Syncing completed');
        }
        catch (PDOException $e) {
            print('Something went wrong!');
            print_r($e);
        }

    }
}
