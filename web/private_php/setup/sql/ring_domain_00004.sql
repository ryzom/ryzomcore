-- An account name is up to 64 characters (`user`.`Login` of the NeL database); the login script copies it here at the first login
ALTER TABLE `ring_users`
  CHANGE `user_name` `user_name` VARCHAR(64) NOT NULL DEFAULT '';
