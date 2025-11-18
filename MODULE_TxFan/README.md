# TX Fan Module
This module is useful to allow the system to activate an external fan for cooling PA units/power supplies/etc that may become heated during long QSO's. 

There are 2 basic opperating modes supported:

1. **FOLLOW PTT**: The follow PTT method monitors the PTT signals (up to 2x) and when the PTT is active and the module timer fires, it will turn on the GPIO assigned to the fan controller.  When the PTT is then released, the GPIO will be turned off when the next timer itteration fires.  If the timer is really short, and the QSO transmitter has a poor signal that can't hold the squelch open this can cause some pretty severe control signal thrashing that could lead to equipment failures.  This can be smoothed out by extending the PTT hold time setting.
2. **COUNTDOWN**: The CountDown method forces the gpio to stay on for a settable number of timer units after the PTT has turned off.  This is useful if there is a need to have no PTT hold time or the fan needs to stay on for some set amount of time after the PTT has ended due to slow migration of heat through heatsinks or etc.  For example, you impiraclly determine the fan needs to stay on for 60 seconds to ensure sufficient cooling has been done.  If the timer is set for minutes accuracy, then the count down would be set to 1.  If the timer is set for seconds, the count down would be set for 60.

## Installation on OpenRepeater
* Download this module from the GitHub repository as a zip file. This can be done by going to the "Releases" section and downloading the packaged ZIP file attachment for the latest release of this module.
* Log into OpenRepeater web interface and go to the Modules section.
* Choose to upload a new module and select the zip file that you downloaded to your local machine. 
* After installed be sure to activate the module. 
* Edit any options for the module on it's settings page and save when done. 
* When ready, rebuild the config to make it active on the live setup. 
