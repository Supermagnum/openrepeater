<?php
/*
*  This is a custom form processor, it takes an input array submitted by this module's settings form and does some extra
*  preprocessing before the array gets passed back to the Modules Class to be serialized and saved into the database.
*  data comes into this file as '$inputArray' and MUST be passed back out as '$outputArray' in order for the data to be saved.
*/

### Data comes in as '$inputArray' ###	

$moduleOptions = $inputArray;

// Build array to update GPIO Pins DB table for pin registration with OS
$gpio_array[] = [
	'gpio_num' => $moduleOptions['fan_gpio'],
	'direction' => 'out',
	'active' => $moduleOptions['fan_gpio_active_state'],
	'description' => 'TxFan: Fan GPIO',
];

// Update GPIO pins table with new pins.
$this->update_gpios('TxFan',$gpio_array);		

// Pass NEW array back out as $outputArray
$outputArray = $moduleOptions;

### Data MUST leave as '$outputArray' ###
?>