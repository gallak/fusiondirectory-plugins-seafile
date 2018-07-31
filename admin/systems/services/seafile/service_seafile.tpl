<fieldset id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <legend><span><label for="{$attributes.fdSeafileWebService.htmlid}">{$section}</label></span></legend>


 <div>
  <table>
    <tr>
      <td>{t}Server state{/t}</td><td> {$SEAFILE_STATUS}</td>
   </tr>
   <tr>
      <td>{t}Server version{/t}</td><td>{$SEAFILE_VERSION}</td>
   <tr>
      <td rowspan={$SEAFILE_FEATURES|@count}>{t}Server features{/t}<td>
      {foreach from=$SEAFILE_FEATURES item=features}
            <td>{$features}</td>
      {/foreach}
    </tr>
	<tr>
		<td>{t}Connexion status{/t}</td><td>{$SEAFILE_CON}</td>
	</tr>
	<tr>
		<td>{t}Statistics{/t}</td><td>{t}Users{/t}: {t}from Database{/t} ({$SEAFILE_NB_USER_DB}) / {t}from LDAP{/t} ({$SEAFILE_NB_USER_LDAP}) / {t}Common{/t} ({$SEAFILE_NB_COMMON})</td>
	<tr>
		<td></td><td>{t}Groups{/t} :{$SEAFILE_NB_GROUPS} </td>		
	</tr>
	<tr>
		<td></td><td>{t}Libraries{/t} : {$SEAFILE_NB_LIBRARIES} {t}libraries corresponding to{/t} {$SEAFILE_TOTAL_NB_FILE} {t}files{/t} {t}and{/t} {$SEAFILE_TOTAL_STORAGE} {t}Mo{/t}</td>		
	</tr>
  </table>
  </div>
</fieldset>
