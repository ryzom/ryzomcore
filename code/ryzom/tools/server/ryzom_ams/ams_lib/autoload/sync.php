<?php
/**
* handler for performing changes when shard is back online after being offline.
* the sync class is responsible for the syncdata function, which will synchronise the website with the shard
* (when the shard is offline, users can still change their password, email or even register)
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class Sync{
    
    /**
     * performs the actions listed in the querycache.
     * All entries in the querycache will be read and performed depending on their type.
     * This is done because the shard could have been offline and we want changes made on the website (which is still online) to eventually hit the shard.
     * These changes are: createPermissions, createUser, change_pass, change_mail
     */
    static public function syncdata ($display = true) {

        try {
            $dbl = new DBLayer("lib");
            $statement = $dbl->executeWithoutParams("SELECT * FROM ams_querycache");
            $rows = $statement->fetchAll();           
            foreach ($rows as $record) {
                
                $db = new DBLayer($record['db']);
                switch($record['type']) {
                    case 'createPermissions':
                        $decode = json_decode($record['query']);
                        $values = array('username' => $decode[0]);
                        //make connection with and put into shard db & delete from the lib
                        $sth = $db->execute("SELECT UId FROM user WHERE Login= :username;", $values);
                        $result = $sth->fetchAll();
                        foreach ($result as $UId) {
                            $ins_values = array('id' => $UId['UId']);
                            $db->execute("INSERT INTO permission (UId, ClientApplication, AccessPrivilege) VALUES (:id, 'r2', 'OPEN');", $ins_values);
                            $db->execute("INSERT INTO permission (UId, ClientApplication, AccessPrivilege) VALUES (:id , 'ryzom_open', 'OPEN');", $ins_values);
                        }
                        break;
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
            if ($display == true) {
                print('Syncing completed');
            }
        }
        catch (PDOException $e) {
            if ($display == true) {
                print('Something went wrong! The shard is probably still offline!');
                print_r($e);
            }
        }

    }
}
