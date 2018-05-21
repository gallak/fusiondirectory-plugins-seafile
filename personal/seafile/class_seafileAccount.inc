<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hicker
  Copyright (C) 2011-2016  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/


/*!
  \brief   mail plugin

  This class provides the functionality to read and write all attributes
  relevant for mail accounts from/to the LDAP.

  It does syntax checking and displays the formulars required.
  Special handling like sieve or imap actions will be implemented
  by the mailMethods.

 */


class seafileAccount extends simplePlugin
{
  var $displayHeader  = TRUE;
  var $objectclasses  = array('fdSeafileAccount');
  var $myclient	= '';

  private $mainSectionAttrs = array();

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Seafile'),
      'plDescription'   => _('Seafile settings'),
      'plIcon'          => 'geticon.php?context=applications&icon=internet-mail&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=internet-mail&size=16',
      'plSelfModify'    => TRUE,
      'plPriority'      => 4,
      'plObjectType'    => array('user'),
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => array(
        'fdSeafileServer' => array(
          array('serverGeneric', 'cn'),
        )
      )
    );
  }

  /*!
  *  \brief The main function : information about attributes
  */
  static function getAttributesInfo ()
  {
    return array (
      'main' => array (
        'name'  => _('Seafile account'),
        'attrs' => array (
          new StringAttribute (
	    _('Seafile User Id'),
	    _('Seafile User name'),
            'fdSeafileAccountUser', TRUE
          ),
          new SelectAttribute (
            _('Server'), _('Specify the Seafile where the user will be hosted on'),
            'fdSeafileAccountServer', FALSE,
            array()
          ),
          new intAttribute (
            _('Quota size'),
	    _('Define quota size in MB'),'fdSeafileAccountQuota', FALSE,
            0, FALSE
    	),
	new BooleanAttribute(
		_('User is Staff member'),
		_('If checked , the user is an administrator of Seafile'),
		'fdSeafileAccountIsStaff',
		FALSE,
		FALSE
		),	
	new BooleanAttribute(
		_('User is Active'),
		_('If checked, the user is active on seafile server'),
		'fdSeafileAccountIsActive',
		FALSE,
		TRUE
		),
	new StringAttribute(
		_('User name displayed'),
		_('This the user name display on seafile Interface'),
		'fdSeafileAccountName',
		FALSE,
		''#$this->attrs['displayName'][0],
	),
	new StringAttribute(
		_('Department of User'),
		_('This is the name of the departement'),
		'fdSeafileAccountDepartment',
		FALSE,
		''#$this->attrs['ou'][0],
	)
)	
      ),

     'StatusSeafileAccount' => array(
        'name' => _('Status of Seafile Account'),
        'class' => array('fullwidth'),
        'template' => get_template_path('account_seafile.tpl', TRUE, dirname(__FILE__)),
        ),

    );
  }




  function execute()
  {
    $smarty = get_smarty();

    $this->mainSectionAttrs = array_keys($this->attributesInfo['main']['attrs']);
    #
    # Fetch Seafile Server
    #
	global $config;


    $ldap = $config->get_ldap_link();
    $ldap->search('(objectClass=fdSeafileService)', array('cn'));
    $profilesdn    = array();
    $profileslabel = array();
    while ($attrs = $ldap->fetch()) {
       $profilesdn[]    = $attrs['dn'];
       $profileslabel[] = $attrs['cn'][0];
    }
    $this->attributesAccess['fdSeafileAccountServer']->setChoices($profilesdn, $profileslabel);

	if (isset($this->attrs['fdSeafileAccountServer'][0]) && isset($this->attrs['fdSeafileAccountUser'][0])){
		
		$ldap = $config->get_ldap_link();
		$array_dn=ldap_explode_dn($this->attrs['fdSeafileAccountServer'][0],1);
		$ldap->search('(&(objectClass=fdSeafileService)(cn='.$array_dn[0].'))', array('fdSeafileServiceURI','fdSeafileServiceTechnicalAccount','fdSeafileServiceTechnicalPassword'));
		$attrs = $ldap->fetch();
		$urlParsed=parse_url($attrs['fdSeafileServiceURI'][0]);
		try {
			$myclient=new SeafileAPI([
				'url' => $urlParsed['scheme']."://".$urlParsed['host'],
				'user' => $attrs['fdSeafileServiceTechnicalAccount'][0],
				'password' => $attrs['fdSeafileServiceTechnicalPassword'][0],
				'port' => $urlParsed['port']
				]);
    
			$info=$myclient->getAccount($this->attrs['fdSeafileAccountUser'][0]);
			# storage are fetch in octets and chown and push in Mo
			$usage= $info->usage / 1000 / 1000;
			$total= $info->total / 1000 / 1000;
			
			$smarty->assign('SEAFILE_USAGE',$usage);
			$smarty->assign('SEAFILE_TOTAL',$total);
			$this->attributesAccess['fdSeafileAccountQuota']->setValue($total);
			$this->attributesAccess['fdSeafileAccountName']->setValue($info->name);
			$this->attributesAccess['fdSeafileAccountDepartment']->setValue($info->department);
			$this->attributesAccess['fdSeafileAccountIsActive']->setValue($info->is_active);
			$this->attributesAccess['fdSeafileAccountIsStaff']->setValue($info->is_staff);
		}catch (Exception $e ){	
			if ($myclient->seafile_code == "404"){
				msg_dialog::display(_("Seafile error"), sprintf(_("Account %s not exist on %s  seafile server"),$this->attrs['fdSeafileAccountUser'][0],$urlParsed['host']),ERROR_DIALOG);
		}else{	
			msg_dialog::display(_("Seafile error"), sprintf(_("Error on Fetching User inormations: %s"),
            $e->getMessage()), ERROR_DIALOG);
		}
	}
	}
  return parent::execute();
  }


  function prepare_save(){

	parent::prepare_save();
	global $config;
	
	# faire apparaitre l'usage du quota
    $ldap = $config->get_ldap_link();
   # on recupere leCN
    $array_dn=ldap_explode_dn($this->attrs['fdSeafileAccountServer'],1);
    	
	$ldap->search('(&(objectClass=fdSeafileService)(cn='.$array_dn[0].'))', array('fdSeafileServiceURI','fdSeafileServiceTechnicalAccount','fdSeafileServiceTechnicalPassword'));

	$attrs = $ldap->fetch();
	$urlParsed=parse_url($attrs['fdSeafileServiceURI'][0]);
		
	try {
		$myclient=new SeafileAPI([
				'url' => $urlParsed['scheme']."://".$urlParsed['host'],
				'user' => $attrs['fdSeafileServiceTechnicalAccount'][0],
				'password' => $attrs['fdSeafileServiceTechnicalPassword'][0],
				'port' => $urlParsed['port']
				]);
			
		    $change_array=[];
			if ( (integer) $this->attrs['fdSeafileAccountQuota'] != $info->total / 1000 / 1000 ) 
				{ $change_array['storage'] = (integer) $this->attrs['fdSeafileAccountQuota'];}
			if ($this->attrs['fdSeafileAccountName'] != $info->name)
				{ $change_array['name'] = $this->attrs['fdSeafileAccountName'];}
            if ($this->attrs['fdSeafileAccountDepartment'] != $info->department)
                { $change_array['department'] = $this->attrs['fdSeafileAccountDepartment'];}
			if ($this->attrs['fdSeafileAccountIsActive'] == "TRUE") {
				$change_array['is_active'] = (integer) 1;
			}else{
			  	$change_array['is_active'] = (integer) 0;	
			}
			if ($this->attrs['fdSeafileAccountIsStaff'] == "TRUE") {
				$change_array['is_staff']=(integer) 1;
			}else{
				$change_array['is_staff']=(integer) 0;
			}
			try {
				$info=$myclient->getAccount($this->attr['fdSeafileAccountUser']);
			}catch (Exception $e ){
				try {
					$myclient->createAccount(
						$this->attrs['fdSeafileAccountUser'],$this->attrs['fdSeafileAccountUser']);
					}catch (Exception $e ){
					msg_dialog::display(_("Seafile error"), sprintf(_("Unable to create account %s :  %s"),attrs['fdSeafileAccountUser'],$e->getMessage()), ERROR_DIALOG);
					}
			}
			try {
				$myclient->modifyAccount($this->attrs['fdSeafileAccountUser'],$change_array);
			}catch (Exception $e ){
				msg_dialog::display(_("Seafile error"), sprintf(_("Unable to modify account %s :  %s"),attrs['fdSeafileAccountUser'],$e->getMessage()), ERROR_DIALOG);
			}
		}catch (Exception $e ){	
			msg_dialog::display(_("Seafile error"), sprintf(_("Error on Fetching User inormations: %s"),
            $e->getMessage()), ERROR_DIALOG);
		}
}


  function deleteSeafileAccount(){
	global $config;
	$ldap = $config->get_ldap_link();
	$array_dn=ldap_explode_dn($this->attrs['fdSeafileAccountServer'][0],1);
	$ldap->search('(&(objectClass=fdSeafileService)(cn='.$array_dn[0].'))', array('fdSeafileServiceURI','fdSeafileServiceTechnicalAccount','fdSeafileServiceTechnicalPassword'));
	$attrs = $ldap->fetch();
	$urlParsed=parse_url($attrs['fdSeafileServiceURI'][0]);
	try {
		
		$myclient=new SeafileAPI([
				'url' => $urlParsed['scheme']."://".$urlParsed['host'],
				'user' => $attrs['fdSeafileServiceTechnicalAccount'][0],
				'password' => $attrs['fdSeafileServiceTechnicalPassword'][0],
				'port' => $urlParsed['port']
				]);
		try {
			$myclient->deleteAccount($this->attr['fdSeafileAccountUser']);
			 parent::post_remove();
			}catch (Exception $e){
			msg_dialog::display(_("Seafile error"), sprintf(_("Error on deleting user : %s"),
            $e->getMessage()), ERROR_DIALOG);
		}
				
				
	}catch (Exception $e ){	
			msg_dialog::display(_("Seafile error"), sprintf(_("Error on connecting Seafile Server for deleting : %s"),
            $e->getMessage()), ERROR_DIALOG);
		}  
  }

  function post_remove()
  {
	$this->deleteSeafileAccount();
				
    #parent::post_remove();
  }



# function ldap_save($cleanup = TRUE){
#	parent::ldap_save($cleanup);
	
#  }


# function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
#  {
#    global $config;
#    parent::__construct($dn, $object, $parent, $mainTab);
#
    #$this->mainSectionAttrs = array_keys($this->attributesInfo['main']['attrs']);
    #
    #
    # recuperer la liste des serveur Seafile
    #


 #   global $config;

  
  #  $ldap = $config->get_ldap_link();
  #  $ldap->search('(objectClass=fdSeafileService)', array('cn'));
  #  $profilesdn    = array();
  #  $profileslabel = array();
  #  while ($attrs = $ldap->fetch()) {
  #     $profilesdn[]    = $attrs['dn'];
  #     $profileslabel[] = $attrs['cn'][0];
  #  }
  #  $this->attributesAccess['fdSeafileAccountServer']->setChoices($profilesdn, $profileslabel);


#print_r($this->attrs['fdSeafileAccountServer'][0]);

#print_r('ICI'.$this->attributesAccess['fdSeafileAccountServer'].'LA');
   # se connecter avec les info du serveur seafile
   # recuperer le token
   # demander le quota
   # faire apparaitre l'usage du quota
#    $ldap = $config->get_ldap_link();
#    $ldap->search('(dn='.$this->attrs['fdSeafileAccountServer'][0].')', array('fdSeafileServiceURI','fdSeafileServiceTechnicalAccount','fdSeafileServiceTechnicalPassword'));
#    $attrs = $ldap->fetch();

#    print_r($attrs);
#	require_once '/usr/share/fusiondirectory/include/class_seafile.inc';

  #      $myclient=new Client2($attrs['fdSeafileServiceURI'][0]);
#        if ($myclient->ping()){
#                # try to get a token
#                $myclient->generateNewToken($attrs['fdSeafileServiceTechnicalAccount'][0],$attrs['fdSeafileServiceTechnicalPassword'][0]);
#		$token=$myclient->getAuthToken();
#                if ($token !== NULL ){
#                        print_r(_("Token succesfully get"));
#                }else{
#                        print_r(_("Unable to get Token"));
#                }
#        }else{
#
#		print_r("unable to ping server");
 #       }

    

#    $this->attributesAccess['fdSeafileServiceURI']->setChoices(array_keys(mailMethod::getMailServers()));
#    $this->attributesAccess['gosaMailServer']->setSubmitForm('mailServerChanged');
#    $this->mailServerChanged();
#
#    $this->attributesAccess['flag_ownmailbox']->setInLdap(FALSE);
#    $this->attributesAccess['flag_vacation']->setInLdap(FALSE);
#    $this->attributesAccess['flag_localonly']->setInLdap(FALSE);
#
#    /* If mail can be used for login it must be unique */
#    if (in_array('mail', explode(',', $config->get_cfg_value('loginAttribute')))) {
#      $this->attributesAccess['mail']->setUnique('whole');
#    }
#  }


}

?>
