<?php
/*
* This is the file that gets called for this module when OpenRepeater rebuilds the configuration files for SVXLink.
* Settings for the config file are created as a PHP associative array, when the file is called it will convert it into
* the requiried INI format and write the config file to the appropriate location with the correct naming.
*/

$options = unserialize($cur_mod['moduleOptions']);

// Retrieve and split server parameters string into array.
$parsing_array = explode('||', $options['server']);
for($i=0; $i < count($parsing_array ); $i++){
    $key_value_array = explode('=>', $parsing_array[$i]);
    $server_array[trim($key_value_array[0])] = trim($key_value_array[1]);
}

// Build Config
$module_config_array['Module'.$cur_mod['svxlinkName']] = [
	'NAME' => $cur_mod['svxlinkName'],
	'ID' => $cur_mod['svxlinkID'],
	'TIMEOUT' => intval($options['timeout_min']) * 60,				
	'TYPE' => $server_array['TYPE'],
	'SERVER' => $server_array['URL'],
	'LINK' => $server_array['PATH'],
	'#LONGMESSAGES' => '1',
	'#REMARKS' => '1',
	'#DEBUG' => '1',
];


if ($options['stationsArray']) {
	ksort($options['stationsArray']);
	foreach($options['stationsArray'] as $cur_parent_array_key => $cur_child_array) {
		$airportArray[$cur_parent_array_key] = $cur_child_array['icao'];

		// Compare default station slected and set ICAO if it matches	
		if ($options['defaultStation'] == $cur_parent_array_key) { $module_config_array['Module'.$cur_mod['svxlinkName']]['STARTDEFAULT'] = $cur_child_array['icao']; }
	}	

	$module_config_array['Module'.$cur_mod['svxlinkName']]['AIRPORTS'] = implode(',', $airportArray);


}
?>