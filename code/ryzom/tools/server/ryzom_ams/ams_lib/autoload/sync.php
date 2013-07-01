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

        global $cfg;
        
        try {
            $dbl = new DBLayer($cfg['db']['lib']);
            $statement = $dbl->executeWithoutParams("SELECT * FROM ams_querycache");
            $rows = $statement->fetchAll();
            $dbs = new DBLayer($cfg['db']['shard']);
            foreach ($rows as $record) {
    
                switch($record['type']) {
                    case 'createPermissions':
                    case 'user_edit':
                    case 'createUser': 
                        $decode = json_decode($record['query']);
                        $query = array('login' => $decode[0], 'pass' => $decode[1], 'mail' => $decode[2] );
                        //make connection with and put into shard db & delete from the lib
                        $dbs->execute("INSERT INTO user (Login, Password, Email) VALUES (:login, :pass, :mail)",$query);              
                        $dbl->execute("DELETE FROM ams_querycache WHERE SID=:SID",array('SID' => $record['SID']));
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
