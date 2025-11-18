<?php
/*
*  This is a custom form processor, it takes an input array submitted by this module's settings form and does some extra
*  preprocessing before the array gets passed back to the Modules Class to be serialized and saved into the database.
*  data comes into this file as '$inputArray' and MUST be passed back out as '$outputArray' in order for the data to be saved.
*/

### Data comes in as '$inputArray' ###	

// Process submitted post varialbes
foreach($inputArray as $key=>$value){  
	if(in_array($key, array("stationNum", "stationLabel", "stationICAO"))){
		// Process through submitted station sub arrays and store for later nesting
		$stationsPostArray[$key]=$value;
		
	} else {
		// Process non-array based variables normally and add to options array.
		$moduleOptions[$key]=$value;			
	}
}

// LOOP: Process saved post sub arrays into nested array and update stations
foreach($stationsPostArray['stationNum'] as $key=>$value){
	$stationsNested[$value] = [
		'icao' => $stationsPostArray['stationICAO'][$key],
		'label' => $stationsPostArray['stationLabel'][$key]
	];
}

// add nested station array into options array.
$moduleOptions['stationsArray'] = $stationsNested; 

// Pass NEW array back out as $outputArray
$outputArray = $moduleOptions;

### Data MUST leave as '$outputArray' ###
?>