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
<tr><td>{t}Connexion status{/t}</td><td>{$SEAFILE_CON}</td></tr>
  </table>
  </div>
</fieldset>
