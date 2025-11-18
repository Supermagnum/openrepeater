<?php
/*
* This is the file that gets called for this module when OpenRepeater rebuilds the configuration files for SVXLink.
* Settings for the config file are created as a PHP associative array, when the file is called it will convert it into
* the requiried INI format and write the config file to the appropriate location with the correct naming.
*/

$options = unserialize($cur_mod['moduleOptions']);
$base_gpio_path = '/sys/class/gpio/gpio';

// Build Config
$module_config_array['Module'.$cur_mod['svxlinkName']] = [
	'NAME' => $cur_mod['svxlinkName'],
	'ID' => $cur_mod['svxlinkID'],
	'PLUGIN_NAME' => 'Tcl',				
];

# MODE: Select the operating mode "FOLLOW_PTT" or "COUNT_DOWN"
$module_config_array['Module'.$cur_mod['svxlinkName']] += [
	'MODE' => $options['mode'],
	'HYSTERESIS_TRIGGER' => $options['hysteresis'],
	'DELAY' => $options['delay'],
];

# Path in the file system where the digital inputs can be monitored
# 2 paths are required, if there is only 1 PTT, assign them the same GPIO.
$module_config_array['Module'.$cur_mod['svxlinkName']] += [
	'PTT_PATH_1' => $base_gpio_path . trim($options['ptt1_gpio']) . '/value',
];

if(trim($options['ptt2_gpio']) != '') {
	$module_config_array['Module'.$cur_mod['svxlinkName']] += [
		'PTT_PATH_2' => $base_gpio_path . trim($options['ptt2_gpio']) . '/value',
	];	
} else {
	// Set PTT2 to same value as PTT1
	$module_config_array['Module'.$cur_mod['svxlinkName']] += [
		'PTT_PATH_2' => $base_gpio_path . trim($options['ptt1_gpio']) . '/value',
	];	
}


# Fan GPIO
$module_config_array['Module'.$cur_mod['svxlinkName']] += [
	'FAN_GPIO' => $base_gpio_path . trim($options['fan_gpio']) . '/value',
];

?>
