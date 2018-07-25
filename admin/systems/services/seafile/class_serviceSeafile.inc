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

class serviceSeafile extends simpleService
{
  var $objectclasses = array('fdSeafileService');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Seafile service'),
      'plDescription' => _('Seafile service').' ('._('Services').')',
      'plIcon'        => 'geticon.php?context=applications&icon=seafile&size=16',
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }


  static function getAttributesInfo ()
  {
      return array (
       'SeafileServerProperties' => array(
          'name' => _('Seafile Server Properties'),
          'attrs' => array(
            new StringAttribute(
              _('fdSeafileServiceURI'),
              _('URI of Seafile Web service'),
              'fdSeafileServiceURI',
              TRUE),
            new StringAttribute(
              _('fdSeafileServiceTechnicalAccount'),
              _('Tecnical user for managing Seafile Host'),
              'fdSeafileServiceTechnicalAccount',
              TRUE),
            new PasswordAttribute(
              _('fdSeafileServiceTechnicalPassword'),
              _('Password of Technical user for managing Seafile Host'),
              'fdSeafileServiceTechnicalPassword',
	      TRUE),
          ),
  	),
     'StatusSeafileService' => array(
        'name' => _('Status of Seafile Service'),
        'class' => array('fullwidth'),
        'template' => get_template_path('service_seafile.tpl', TRUE, dirname(__FILE__)),
        ), 
    );
  }


  function execute()
  {
	$smarty = get_smarty();

	if ($this->attrs['fdSeafileServiceURI'][0])	{

	try {
		$urlParsed=parse_url($this->attrs['fdSeafileServiceURI'][0]);
		$myclient=new SeafileAPI([
				'url' => $urlParsed['scheme']."://".$urlParsed['host'],
				'user' => $this->attrs['fdSeafileServiceTechnicalAccount'][0],
				'password' => $this->attrs['fdSeafileServiceTechnicalPassword'][0],
				'port' => $urlParsed['port']
				]);
		if ($myclient->ping()){           
			$cap=$myclient->getServerCapabilities();
			$smarty->assign('SEAFILE_STATUS',"ALIVE");
			$smarty->assign('SEAFILE_VERSION',$cap->version);
			$smarty->assign('SEAFILE_FEATURES',$cap->features);
			$smarty->assign('SEAFILE_CON',$myclient->seafile_status."(".$myclient->seafile_code.")");
		}
	
	
		} catch (Exception $e ){	
			msg_dialog::display(_("Seafile error"), sprintf(_("Error on seafile settings: %s"),
            $e->getMessage()), ERROR_DIALOG);
		}
	}

  return parent::execute();
  }



}

?>

