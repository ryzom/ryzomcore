# naming_service

## options

```
--nsname <post fix>
  sets a service full name post fix
  Examples:
  naming_service --nsname test # will result in the service full name 'naming_service_test' 
--fullnsname <full name>
  sets the service full name, defaults to 'naming_service', takes precedence over --nsname
--shortnsname
  sets the service short name, defaults to 'NS'
--nolog
  don't log to file, takes precedence over setting in the config file
```

## NLNET::IService inherited options

```
--writepid
-A
  path where to run the service, takes precedence over setting in the config file
-C
  config file dir
-L
  log dir
-N <postfix>
  text tat will be inserted between the log file name and the extension
  Examples:
  -N test # will result in a logfile <full name>_test.log 
-Z
  if set to 'u' releases the module manager
```