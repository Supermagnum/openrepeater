// Module JavaScript for UI elements

// Variables
var maxSysStations = 9;
var minReqStations = 1;

// Sub Functions

function htmlTemplate(n) {
	var html = '<p class="stationRow additional">'+
	'<span class="num">'+
	'  <input type="hidden" name="stationNum[]" value="' + n +'">' + n +
	'</span>'+
	'<span>'+
	'  <input id="stationLabel' + n +'" type="text" name="stationLabel[]" placeholder="Station Label" class="stationLabel" Value="Station '+n+'" required>'+
	'  <input id="stationICAO' + n +'" type="text" name="stationICAO[]" placeholder="ICAO" class="stationICAO" required>'+
	'</span>'+
	'<a href="#" class="removeStation" title="Remove this station"><i class="icon-minus-sign"></i></a>'+
	'</p>';
	return html;
}


function updateStationCount(n) {
	if (n == 1) {
		$('#stationCount').html(n + ' Station'); // singular
	} else {
		$('#stationCount').html(n + ' Stations'); // plural
	}
}

function checkMaxStation(currentStations, sysMax) {
	if (currentStations >= sysMax) {
		alert("Sorry, but you cannot add anymore stations because you already have the maximum number of stations the system will allow.");
		return false;
	}
	return true;
}

function checkMinStation(currentStations, minStations) {
	if (currentStations <= minStations) {
		alert("Sorry, but you cannot remove any more stations because this system requires a minimum of " + minStations + " station(s).");
		return false;
	}
	return true;
}



// Main Scripts
$(window).load(function() {
	$(function() {
		var stationsDiv = $('#stationsWrap');
		var i = $('#stationsWrap p').size();
		updateStationCount(i);

		$('.addStation').live('click', function(e) {
			e.preventDefault();
			if (checkMaxStation(i, maxSysStations)) {
				i++;
				$(htmlTemplate(i)).appendTo(stationsDiv);
				updateStationCount(i);
			}
			return false;
		});

		$('.removeStation').live('click', function(e) {
			e.preventDefault();
			if (checkMinStation(i, minReqStations)) {
				$(this).parents('p').remove();
				i--;
				updateStationCount(i);
			}
			return false;
		});

		$('#removeDefault').live('click', function(e) {
			e.preventDefault();
			$('.defaultStation').prop('checked', false);
			$('.radio span').removeClass('checked');
		});
	});
}); //]]>