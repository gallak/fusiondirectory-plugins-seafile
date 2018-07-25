<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2018  FusionDirectory
  Copyright (C) 2018  Antoine Gallavardin

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
  \brief   seafile plugin

  This class provides the functionality to read and write all informations 
  for creating / editing / deleting remote seafiel account.

 */
 
 


class seafileAccount extends simplePlugin
{
  var $displayHeader  = TRUE;
  var $objectclasses  = array('fdSeafileAccount');
  var $myclient	= '';
  
  var $seafile_id='';

  private $mainSectionAttrs = array();
  
   /*!
  *  \brief quota_to_image : it display a progress bar for showing capacity of remote seafile storage
  */
  static private function quota_to_image($use, $quota)
  {
      $usage = round(($use / $quota) * 100);
      return "<img src='progress.php?x=100&amp;y=17&amp;p=$usage'>";
  }
  
  /*!
  *  \brief generateRandomString : return a random string.Whn creating an accoutn , a password is mandatory.
  *  the password could be pushed by the normal way of FusionDirectory
  */  
  static private function generateRandomString($length = 10) {
    $characters = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890&$)àç_è-("é&';
    $charactersLength = strlen($characters);
    $randomString = '';
    for ($i = 0; $i < $length; $i++) {
        $randomString .= $characters[rand(0, $charactersLength - 1)];
    }
    return $randomString;
}
  

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Seafile'),
      'plDescription'   => _('Seafile settings'),
      'plIcon'          => 'geticon.php?context=applications&icon=seafile&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=seafile&size=16',
      'plDepends'       => array('mailAccount'),
      'plSelfModify'    => FALSE,
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
          new DisplayLDAPAttribute (
	    _('Seafile User Id'),
	    _('Seafile User name'),
            'fdSeafileAccountUser',
             TRUE,
             ''
          ),
          new SelectAttribute (
            _('Server'), _('Specify the Seafile where the user will be hosted on'),
            'fdSeafileAccountServer', TRUE,
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

    #
    # Fetch Seafile Server
    #
	global $config;

	# Some attribute are fixed
	# mail is the Seafile ID	
	$this->attributesAccess['fdSeafileAccountUser']->setValue($this->attrs['mail'][0]);
	// Select all seafile server available in LDAP tree
	$seafileServers = objects::ls('server', array('cn' => 1,
									'fdSeafileServiceURI' => 1,
									'fdSeafileServiceTechnicalAccount' => '*',
									'fdSeafileServiceTechnicalPassword' => '*'),
									 NULL,
									 '(objectClass=fdSeafileService)');
	
	// define array for droplist
	$profilesdn    = array();
	$profileslabel = array();

   
   
	// filling the drop list of seafile server
	foreach( $seafileServers as $key => $value ) { 
		$profilesdn[] = $key;
		$profileslabel[] = $value['cn'];
	}   
    // setting in object the seafile server choosed
    $this->attributesAccess['fdSeafileAccountServer']->setChoices($profilesdn, $profileslabel);
	
	// if mail address and seafile server selected
	if (isset($this->attrs['fdSeafileAccountServer'][0]) && isset($this->attrs['mail'][0])){	
		// parsing the seafile server URL
		$urlParsed=parse_url($seafileServers[$this->attrs['fdSeafileAccountServer'][0]]['fdSeafileServiceURI']);
		$technicalAccount=$seafileServers[$this->attrs['fdSeafileAccountServer'][0]]['fdSeafileServiceTechnicalAccount'][0];
		$technicalPassword=$seafileServers[$this->attrs['fdSeafileAccountServer'][0]]['fdSeafileServiceTechnicalPassword'][0];
		// load Seafile Object
		try {
			$myclient=new SeafileAPI([
					'url' 		=> $urlParsed['scheme']."://".$urlParsed['host'],
					'user' 		=> $technicalAccount,
					'password' 	=> $technicalPassword,
					'port' 		=> $urlParsed['port']
				]);

			// try to get remote account
			$info=$myclient->getAccount($this->attrs['mail'][0]);
			// warning : storage are fetch in octets and chown and push in Mo
			$usage= $info->usage / 1000 / 1000;
			$total= $info->total / 1000 / 1000;
			// set SMARTY value
			$smarty->assign('SEAFILE_USAGE',$usage);
			$smarty->assign('SEAFILE_TOTAL',$total);
			$smarty->assign('SEAFILE_BAR',$this->quota_to_image($usage, $total));
			
			//fetch Quota Value
			$this->attributesAccess['fdSeafileAccountQuota']->setValue($total);
			///fetch account status
			$this->attributesAccess['fdSeafileAccountIsActive']->setValue($info->is_active);
			//fetch account status
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
	
	
	
	// fetch server information
	$seafileServers = objects::ls('server', array('cn' => 1,'fdSeafileServiceURI' => 1, 'fdSeafileServiceTechnicalAccount' => '*', 'fdSeafileServiceTechnicalPassword' => '*'), $this->attrs['fdSeafileAccountServer'], '(objectClass=fdSeafileService)');
	$urlParsed=parse_url($seafileServers[$this->attrs['fdSeafileAccountServer']]['fdSeafileServiceURI']);
	$technicalAccount=$seafileServers[$this->attrs['fdSeafileAccountServer']]['fdSeafileServiceTechnicalAccount'][0];
	$technicalPassword=$seafileServers[$this->attrs['fdSeafileAccountServer']]['fdSeafileServiceTechnicalPassword'][0];


	try {
		
		
		// create connection on seafile server
		$myclient=new SeafileAPI([
				'url' => $urlParsed['scheme']."://".$urlParsed['host'],
				'user' => $technicalAccount,
				'password' => $technicalPassword,
				'port' => $urlParsed['port']
				]);

			
			try {
				//try to fetch the account give the information if we have to create it before
				$info=$myclient->getAccount($this->attrs['fdSeafileAccountUser']);
				//account exist so we modify it
				
				try {
					// fetching information from form and compare it
					$change_array=[];
					if ( (integer) $this->attrs['fdSeafileAccountQuota'] != $info->total / 1000 / 1000 ) {
						$change_array['storage'] = (integer) $this->attrs['fdSeafileAccountQuota'];
					}

					$is_active = 0;
					$is_staff = 0;
					
					if ($this->attrs['fdSeafileAccountIsActive'] == "TRUE") { $is_active = 1; }
					if ($this->attrs['fdSeafileAccountIsStaff'] == "TRUE") { $is_staff = 1; }
					
					if ( $info->is_active != $is_active) { $change_array['is_active'] = (integer) $is_active;}
					if ( $info->is_staff != $is_staff) { $change_array['is_staff'] = (integer) $is_staff;}
						
					//fecthing information from user tab 
					$userTab = $this->parent->by_object['user'];
					if ( isset($userTab->displayName) && $userTab->displayName != $info->name){ 
						$change_array['name'] = $userTab->displayName;
					}
					if ( isset($userTab->ou) && $userTab->ou != $info->department ){ 
						$change_array['department'] = $userTab->ou;
					}
										
					if ( $userTab->attributesAccess['userPassword']->getClear() != ""){ 
						$change_array['password'] = $userTab->attributesAccess['userPassword']->getClear();
					}					
					
					$myclient->modifyAccount($this->attrs['fdSeafileAccountUser'],$change_array);
				}catch (Exception $e ){
					msg_dialog::display(_("Seafile error"), sprintf(_("Unable to modify account %s :  %s"),attrs['fdSeafileAccountUser'],$e->getMessage()), ERROR_DIALOG);
				}
			}catch (Exception $e ){
				
				if ($myclient->seafile_code == "404"){
					// account didn't exist we have to create it
					try {
						// Create thie account
						$myclient->createAccount($this->attrs['fdSeafileAccountUser'],$this->generateRandomString(20));
						// modify the account with the new information
						// create modification array
						// array wich contains all change to apply
						$change_array=[];
						if ( isset($this->attrs['fdSeafileAccountQuota'])){ 
							$change_array['storage'] = (integer) $this->attrs['fdSeafileAccountQuota'];
						}
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
						// set inforation from usertab
						$userTab = $this->parent->by_object['user'];
						if ( isset($userTab->displayName)){ 
							$change_array['name'] = $userTab->displayName;
						}
						if ( isset($userTab->ou)){ 
							$change_array['department'] = $userTab->ou;
						}
						if ( $userTab->attributesAccess['userPassword']->getClear() != ""){ 
							$change_array['password'] = $userTab->attributesAccess['userPassword']->getClear();
						}	
						// send modification
						$myclient->modifyAccount($this->attrs['fdSeafileAccountUser'],$change_array);
						
					}catch (Exception $e ){
						msg_dialog::display(_("Seafile error"), sprintf(_("Unable to create account %s :  %s"),attrs['fdSeafileAccountUser'],$e->getMessage()), ERROR_DIALOG);
					}		
				}else{
					//error or fetching information
					msg_dialog::display(_("Seafile error"), sprintf(_("unable to fetch account information  of %s on %s  seafile server"),$this->attrs['fdSeafileAccountUser'][0],$urlParsed['host']),ERROR_DIALOG);
				}
			}

		}catch (Exception $e ){	
				msg_dialog::display(_("Seafile error"), sprintf(_("Unable to connect to Seafile Server"),
				$e->getMessage()), ERROR_DIALOG);			
		}
}



  function post_remove()
  {
	
	global $config;
	
	$seafileServers = objects::ls('server', array('cn' => 1,'fdSeafileServiceURI' => 1, 'fdSeafileServiceTechnicalAccount' => '*', 'fdSeafileServiceTechnicalPassword' => '*'), $this->attributesAccess['fdSeafileAccountServer']->getValue(), '(objectClass=fdSeafileService)');
	
	$urlParsed=parse_url($seafileServers[$this->attributesAccess['fdSeafileAccountServer']->getValue()]['fdSeafileServiceURI']);
	$technicalAccount=$seafileServers[$this->attributesAccess['fdSeafileAccountServer']->getValue()]['fdSeafileServiceTechnicalAccount'][0];
	$technicalPassword=$seafileServers[$this->attributesAccess['fdSeafileAccountServer']->getValue()]['fdSeafileServiceTechnicalPassword'][0];

	
	try {
		
		$myclient=new SeafileAPI([
				'url' => $urlParsed['scheme']."://".$urlParsed['host'],
				'user' => $technicalAccount,
				'password' => $technicalPassword,
				'port' => $urlParsed['port']
				]);
		try {
				$myclient->deleteAccount($this->attributesAccess['fdSeafileAccountUser']->getValue());
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

}

?>
