<fieldset id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <legend><span><label for="{$attributes.fdSeafileAccount.htmlid}">{$section}</label></span></legend>


 <div>
  <table>
    <tr>
      <td>{t}Quota status{/t} : {$SEAFILE_USAGE} / {$SEAFILE_TOTAL} Mo </td><td>{$SEAFILE_BAR}</td>
    </tr>
    <tr>
       <td>{t}Librairies status{/t}</td><td>{t}number of existing librairie{/t} : {$EXISTING_LIBRARY} / {t}number of deleted librairie{/t} : {$DELETED_LIBRARY}</td>
    </tr>
  </table>
  </div>
</fieldset>
