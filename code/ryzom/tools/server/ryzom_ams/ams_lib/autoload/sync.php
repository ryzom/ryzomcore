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

        try {
            $dbl = new DBLayer("lib");
            $statement = $dbl->executeWithoutParams("SELECT * FROM ams_querycache");
            $rows = $statement->fetchAll();           
            foreach ($rows as $record) {
                
                $db = new DBLayer($record['db']);
                switch($record['type']) {
                    case 'createPermissions':
                    case 'change_pass':
                        $decode = json_decode($record['query']);
                        $values = array('user' => $decode[0], 'pass' => $decode[1]);
                        //make connection with and put into shard db & delete from the lib
                        $db->execute("UPDATE user SET Password = :pass WHERE Login = :user",$values);              
                        break;
                    case 'change_mail':
                        $decode = json_decode($record['query']);
                        $values = array('user' => $decode[0], 'mail' => $decode[1]);
                        //make connection with and put into shard db & delete from the lib
                        $db->execute("UPDATE user SET Email = :mail WHERE Login = :user",$values);              
                        break;
                    case 'createUser': 
                        $decode = json_decode($record['query']);
                        $values = array('login' => $decode[0], 'pass' => $decode[1], 'mail' => $decode[2] );
                        //make connection with and put into shard db & delete from the lib
                        $db->execute("INSERT INTO user (Login, Password, Email) VALUES (:login, :pass, :mail)",$values);              
                        break;
                }
                $dbl->execute("DELETE FROM ams_querycache WHERE SID=:SID",array('SID' => $record['SID']));
            }
            print('Syncing completed');
        }
        catch (PDOException $e) {
            print('Something went wrong!');
            print_r($e);
        }

    }
}
