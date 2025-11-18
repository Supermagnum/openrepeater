<?php
/* 
 *	Settings Page for Module 
 *	
 *	This is included into a full page wrapper to be displayed. 
 */
?>

<?php
	$ports = $this->Database->get_ports();
	$pttOptionHTML = '';
	foreach($ports as $curPort) {
		$pttOptionHTML .= '<option value="' . $curPort['txGPIO'] . '">PTT: ' . $curPort['portLabel'] . ' (' . $curPort['txGPIO'] . ')</option>';
		
	}
	
	// Hidden DIV to pass module settings array to JavaScript as json array
	echo '<div id="moduleSettingsArray" style="display:none;">' . json_encode($moduleSettings) . '</div>';

?>

<!-- BEGIN FORM CONTENTS -->

	<fieldset>
		<legend>Module Settings</legend>
		  <div class="control-group">
			<label class="control-label" for="mode">Fan Mode</label>
			<div class="controls">
			  <select id="mode" name="mode">
				  <option value="FOLLOW_PTT">Follow PTT</option>
				  <option value="COUNT_DOWN">Delay Timer</option>
			  </select>

			</div>
		    <span class="help-inline">Select the the mode you want to use to control the fan.</span>
		  </div>
	
		  <div class="control-group countDownOptions">
			<label class="control-label" for="hysteresis">Pre-Delay</label>
			<div class="controls">
				  <input class="input-xlarge" id="hysteresis" type="number" name="hysteresis" value="<?php echo $moduleSettings['hysteresis']; ?>">
			</div>
		    <span class="help-inline">This is a hysteresis delay in seconds until the fan turns on after PTT becomes active. This is useful to prevent the fans from coming on from brief keyups and IDs.</span>
		  </div>

		  <div class="control-group countDownOptions">
			<label class="control-label" for="delay">Post-Delay</label>
			<div class="controls">
			  <input class="input-xlarge" id="delay" type="number" name="delay" value="<?php echo $moduleSettings['delay']; ?>">
			</div>
		    <span class="help-inline">Delay in seconds until fan turns off after PTT drops.</span>
		  </div>

		  <div class="control-group">
			<label class="control-label" for="ptt1_gpio">PTT GPIO Pin(s)</label>
			<div class="controls">
			  <select id="ptt1_gpio" name="ptt1_gpio"><?=$pttOptionHTML?></select>
			  <select id="ptt2_gpio" name="ptt2_gpio"><?=$pttOptionHTML?></select>
			</div>
		    <span class="help-inline">The GPIO based PTT(s) that are monitored to trigger the fan circuit.</span>
		  </div>

		  <div class="control-group">
			<label class="control-label" for="fan_gpio">Fan GPIO Pin</label>
			<div class="controls">
			  <input class="input-xlarge" id="fan_gpio" type="number" name="fan_gpio" value="<?php echo $moduleSettings['fan_gpio']; ?>">
			</div>
		    <span class="help-inline">GPIO pin used to control fan circuit.</span>
		  </div>

	
	
		  <div class="control-group">
			<label class="control-label" for="fan_gpio_active_state"> Fan Active High or Low State: </label>
			<div class="controls">
			  <input type="radio" name="fan_gpio_active_state" value="high" <?php if ($moduleSettings['fan_gpio_active_state'] == "high") { echo 'checked="checked"'; } ?>><span> Active High </span>
			  <input type="radio" name="fan_gpio_active_state" value="low" <?php if ($moduleSettings['fan_gpio_active_state'] == "low") { echo 'checked="checked"'; } ?>><span> Active Low </span>
			</div>
			  <span class="help-inline">This setting is dependent upon they hardware/circuit design your are using. Active High enables the fan gpio pin with +3.3 volts. Active Low enables by setting the GPIO pin to ground (0 volts).</span>
		  </div>
	
	</fieldset>					
	
<!-- END FORM CONTENTS -->