// Module JavaScript for UI elements

// Main Scripts
$(window).load(function(){
	$(function() {
        // Get Saved Settings from server
        var moduleSettingsJSON = $('#moduleSettingsArray').text();
		var moduleSettings = JSON.parse(moduleSettingsJSON);
			
		// Mode Functions
		$("#mode").val(moduleSettings.mode);

		function selectMode() {
			if( $("#mode").val() == 'FOLLOW_PTT' ) {
				$(".countDownOptions").hide();
				console.log('hide');
			} else {
				$(".countDownOptions").show();
				console.log('show');
			}				
		}

		$( "#mode" ).change( selectMode );
		selectMode(); // Check on Load

		

		// PTT Pin Functions
		$("#ptt1_gpio").val(moduleSettings.ptt1_gpio);
		$("#ptt2_gpio").val(moduleSettings.ptt2_gpio);

		var numberOfPTTs = $('#ptt1_gpio option').length;

		function setPTTs() {
			if( numberOfPTTs <= 1 ) {
				// Only one PTT available
				$("#ptt2_gpio").hide();
				$("#ptt2_gpio").val( $("#ptt1_gpio").val() ); //set PTT2 to same as PTT1
			} else {
				$("#ptt2_gpio").show();
			}
		}
	
		$( "#ptt1_gpio" ).change( setPTTs );
		setPTTs(); // Check on Load
	
	});

});//]]> 
