ALTER TABLE `plugins` ADD UNIQUE(`Name`);
UPDATE `plugins` SET `Info` = '{"PluginName":"Achievements","Description":"Returns the achievements of a user with respect to the character","Version":"1.0.0","TemplatePath":"..\\/..\\/..\\/private_php\\/ams\\/plugins\\/Achievements\\/templates\\/index.tpl","Type":"Manual","":null}' WHERE `Name` = 'Achievements';
