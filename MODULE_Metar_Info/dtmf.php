<?php
/*
* This is the file that gets called for this module when OpenRepeater displays the DTMF commands. This file is optional,
* but highly recommended if your module has DTMF commands. 
*/

$module_id = $cur_mod_loop['moduleKey'];
$options = unserialize($cur_mod_loop['moduleOptions']);

$sub_subcommands = "AIRPORTS
DTMF command to choose to play METAR information for saved airports.\n\n";

if ($options['stationsArray']) {
	ksort($options['stationsArray']);
	foreach($options['stationsArray'] as $cur_parent_array_key => $cur_child_array) {
		if ($options['defaultStation'] == $cur_parent_array_key) {
			$sub_subcommands .= '<strong>';
			$sub_subcommands .= $cur_parent_array_key.'#		'.$cur_child_array['label'] . ' ('.$cur_child_array['icao'].')';
			$sub_subcommands .= ' - PLAYS BY DEFAULT</strong>'. "\n";
		} else {
			$sub_subcommands .= $cur_parent_array_key.'#		'.$cur_child_array['label'] . ' ('.$cur_child_array['icao'].')'. "\n";
		}
	}
}

$sub_subcommands .= "\n#		Deactivate Relay Module\n";

if (isset($options['defaultStation'])) {
	$sub_subcommands .= "\n<em>NOTE: The default station above will play once the module is activated. Once that station is completed, additional stations may be played using their commands above.</em>";
} else {
	$sub_subcommands .= "\n<em>NOTE: There is no default station selected to play. Once the module is activated, you must also choose a station to play.</em>";
}

?>