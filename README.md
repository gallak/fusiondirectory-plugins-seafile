# fusiondirectory-plugins-seafile
FusionDirectory plugin for managing Seafile Account

## Introduction
This plugin allow to add / modify / delete an account on external seafile Server ( Pro or community Edition )

you should have a FusionDirectory Installation and the plugin mail installed.
For the moment e-mail address is used as identifier but it could be independant ( for the futur)

## Next plan :
debugging (of course)

### User side
+ add independant identifier
+ add more information on accoutn status ( like number of librairie, file, share, group)

### Groupe side
+ add group support


### server side
+ add flag for avoid chnage password ( authentification could be delegate to LDAP / CAS / shibboleth)
+ a kind of dashboard ( total size / number of librairies / file / share / group)

## additional informations
Webservice of seafile is very complete and evolve a lot, so it could be difficult to follow all evolution

## special thanks 
+ to ben@netcap.fr for the seafile library for php
+ to opensides Team for FusionDirectory ( just a charm to use)

