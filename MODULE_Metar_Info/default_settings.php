<?php
/*
* This is the file that gets called for this module when OpenRepeater first
* installs the mdoule. It adds the default settings for the module defined in
* the PHP array below into the database in the moduleOptions field in the
* modules table. This information is serialized in the database. The variables
* that should be defined here are the same variables that are used in the
* settings form. Again, it is what is stored in the database as options and
* not the variables that the SVXLink code expects to see in the generated 
* config file for the module.
*/

$default_settings = [
    'timeout_min' => '2',
	'server' => 'TYPE => XML || URL => https://aviationweather.gov || PATH => /adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecent=true&stationString=',

	'stationsArray' => [
		'1' => [
			'icao' => 'ESSB',
			'label' => 'Stockholm, Sweden',
		],
		'2' => [
			'icao' => 'EDDP',
			'label' => 'Leipzig, Germany',
		],
		'3' => [
			'icao' => 'SKSM',
			'label' => 'Santa Marta, Colombia',
		],
		'4' => [
			'icao' => 'EDDS',
			'label' => 'Stuttgart, Germany',
		],
		'5' => [
			'icao' => 'EDDM',
			'label' => 'Munich, Germany',
		],
		'6' => [
			'icao' => 'EDDF',
			'label' => 'Frankfurt/Main, Germany',
		],
		'7' => [
			'icao' => 'KJAC',
			'label' => 'Jackson, WY , United States',
		],
		'8' => [
			'icao' => 'KTOL',
			'label' => 'Toledo, OH , United States',
		],
		'9' => [
			'icao' => 'KMRB',
			'label' => 'Martinsburg, WV',
		],
		
	],
    
];
?>