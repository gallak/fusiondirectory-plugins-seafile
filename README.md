# fusiondirectory-plugins-seafile
FusionDirectory plugin for managing Seafile Account

## Introduction
This plugin allow to add / modify / delete an account on external seafile Server ( Pro or community Edition )

you should have a FusionDirectory Installation and the FusionDirectory mail plugin installed.
For the moment e-mail address is used as identifier but it could be independant (in the the future)

## Next plan :
debugging (of course)

### User functionality
+ add independant identifier
+ add more information on account status ( like number of librairie, file, share, group)

### Group functionality
+ add group support


### Server functionality
+ add flag to avoid the change of password ( authentification could be delegated to LDAP / CAS / shibboleth)
+ some dashboard ( total size / number of librairies / file / share / group)

## Additional informations

The webservice of seafile is very complete and evolve a lot, so it could be difficult to follow all the evolutions

## Special thanks
+ to ben@netcap.fr for the seafile library for php
+ to opensides Team for FusionDirectory ( just a charm to use)

