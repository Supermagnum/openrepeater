<?php
/* 
 *	Settings Page for Module 
 *	
 *	This is included into a full page wrapper to be displayed. 
 */
?>

<!-- BEGIN FORM CONTENTS -->
	<fieldset>

		<legend>Airports</legend>
	
			<div id="stationsWrap">
			<?php 
			$idNum = 1; // This will be replaced by a loop to load exsiting values 
			
			if ($moduleSettings['stationsArray']) {
				ksort($moduleSettings['stationsArray']);
				foreach($moduleSettings['stationsArray'] as $cur_parent_array => $cur_child_array) { ?>
			
			
					<p class="stationRow<?php if ($idNum == 1) { echo ' first'; } else { echo ' additional'; } ?>">
						<span class="num">
							<input type="hidden" name="stationNum[]" value="<?php echo $idNum; ?>">
							<?php echo $idNum; ?>
						</span>
						
						<span>									
							<input id="stationLabel<?=$idNum?>" type="text" name="stationLabel[]" placeholder="Station Label" value="<?php echo $cur_child_array['label']; ?>" class="stationLabel" required>
							<input id="stationICAO<?=$idNum?>" type="text" name="stationICAO[]" placeholder="ICAO"  value="<?php echo $cur_child_array['icao']; ?>" class="stationICAO" required>
						</span>
			
						<?php if ($idNum == 1) { 
							echo '<a href="#" class="addStation" title="Add a station"><i class="icon-plus-sign"></i></a>';
						} else {
							echo '<a href="#" class="removeStation" title="Remove this station"><i class="icon-minus-sign"></i></a>';
						} ?>
						
						<input type="radio" class="defaultStation" name="defaultStation" value="<?=$idNum?>" <?=($idNum==$moduleSettings['defaultStation'])?' checked':'';?>>
					</p>
			
			
				<?php 
				$idNum++;
				}	
			} else {
				echo "there are no stations...";
			}
			?>
			</div>
			<button id="removeDefault">Remove Default Station</button>
			<br>
			<br>
			<br>




		<legend>Module Settings</legend>

		  <div class="control-group">
			<label class="control-label" for="repeaterDTMF_disable">METAR Server</label>
			<div class="controls">
			  <select id="server" name="server">
			  	<?php $server1 = 'TYPE => XML || URL => https://aviationweather.gov || PATH => /adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecent=true&stationString='; ?>
			  	<option value="<?=$server1?>"<?=($moduleSettings['server']==$server1)?' selected':'';?>>aviationweather.gov (preferred)</option>

			  	<?php $server2 = 'TYPE => TXT || URL => https://tgftp.nws.noaa.gov || PATH => /data/observations/metar/stations/'; ?>
			  	<option value="<?=$server2?>"<?=($moduleSettings['server']==$server2)?' selected':'';?>>tgftp.nws.noaa.gov (testing)</option>

			  </select>
			  <span class="help-inline">Choose the server that METAR Info will be queried from.</span>
			</div>
		  </div>

		  <div class="control-group">
			<label class="control-label" for="timeout_min">Module Time Out</label>
			<div class="controls">
			  <input class="input-xlarge" id="timeout_min" type="text" name="timeout_min" value="<?php echo $moduleSettings['timeout_min']; ?>">
			</div>
		    <span class="help-inline">Timeout in minutes</span>
		  </div>

	</fieldset>					
	
<!-- END FORM CONTENTS -->