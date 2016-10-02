Sketch Structure Information
--------------------------------
The sketch contains two directories to support the included out of the box functionality.


Scripts
---------------------------------
The scripts directory contains batch files that are executed as a result of different events during the development cycle.  The following envents are captured and handled by different
batch files in the scripts directory.

Prebuild Event - This event is launched prior to compilation of the code.  The batch file prebuild.bat is executed.  This batch file is responsible for executting all batch files with
the name pre-build## where ## is a two digit number.  The pre-build## batch files are executed from higher to lower, so pre-build99.bat is executed prior to pre-build98.bat.  Adding
additional scripts to satisfy your development needs is as simple as creating a new file named pre-build## and chosing the proper number so the batch file is executed when needed.

Postbuild Event - This event is launched after a successful compilation.  The batch file postbuild.bat is executed.  This batch file is responsible for excecuting all batch files eith
the name post-build## where ## is a two digit number.  The post-build## batch files are executed from higher to lower, so post-build99.bat is executed prior to post-build98.bat.  Adding
additional scripts to satisfy your development needs is as simple as creating a new file named post-build## and chosing the proper number so the batch file is executed when needed.

Deployment Event - This event is launched when a request to deploy to the board is given to Atmel Studio.  The batch file localdeploy.bat is executed.  This batch file is responsible
for executing all batch files with the name local-deploy##.bat where ## is a two digit number.  The local-deploy## batch files are executed from higher to lower, so local-deploy99.bat 
is executed prior to local-deploy98.bat.  Adding additional scripts to satisfy your development needs is as simple as creating a new file named local-deploy##.bat and chosing the
proper number so the batch file is executed when needed.

Sucessful Deployment - This event is launched when deployment to a board is successful.  The batch file deploysuccess.bat is executed.  This batch file is responsible for executing all
batch files with the name deploy-success##.bat where ## is a two digit number.  The deploy-success##.bat batch files are executed from higher to lower, so deploy-success99.bat is
executed prior to deploy-success98.bat.  Adding additional scripts to satisfy your development needs is as simple as creating a new file named deploy-success##.bat and chosing the
proper number so the batch file is executed when needed.

Failed Deployment - This event is launched when deployment to a board is fails.  The batch file deployfail.bat is executed.  This batch file is responsible for executing all
batch files with the name deploy-fail##.bat where ## is a two digit number.  The deploy-fail##.bat batch files are executed from higher to lower, so deploy-fail99.bat is
executed prior to deploy-fail98.bat.  Adding additional scripts to satisfy your development needs is as simple as creating a new file named deploy-fail##.bat and chosing the
proper number so the batch file is executed when needed.


Script Description
---------------------------------------

[]prebuild.bat
Executed to handle the prebuild event. Responsible for executing in reverse order all batch files with the name pattern pre-build##.bat.

[]pre-build99.bat
Responsible for saving the project name, output file, device name and configuration mode (DEBUG|RELEASE) to the files project.txt, output.txt, device.txt and config.txt respectively.
This files are used by different scripts while handling different events.

[]pre-build98.bat
Responsible for dynamically building the file Utility\builddate.cpp.  This provides an optional function to display from within your sketch the last time stamp of the last compilation
of the sketch.  This is usefull to verify sketch version when debugging or troubleshooting.  This function is optional and there is no penalty if not invoke.  The text for the time 
stamp can be stored in RAM, EEMEM or PROGMEM depending on the value of the variable BUILDINFO_RAM, BUILDINFO_EEMEM, BUILDINFO_PROGMEM defined in the file Utility\Buildinfo.h.  The default
behavior is to build it in RAM.

[]pre-build00.bat
This script is responsible for starting the capture of key performance indicators of the development process.  This script captures how many times the code is compiled and minimalistic
state information of the development environment such as, time stamp of the request to start compilation, number of different files in the development environment.  Two files are populated
with this information.  startcompilercounter.txt counts the number of times a request to start compilation has been made and buildlog.txt captures the rest of the information.

[]postbuild.bat
Executed after a sucessfull compilation.  Responsible for exectuing in reverse order all batch files eith the name pattern post-build##.bat

[]post-build99.bat
This script captures the size of the program to feed future functionality for metrics analysis.  The program information is saved in the file memoryusage.txt.

[]post-build98.bat
This script capturs the number of times a request for compilation is scucessfull.  The counter is kept in the file endcompilecounter.txt

[]post-build00.bat
This script displays metric information about the development process.  Attempts to compile, failed compilation, sucessfull compilations, deployment attempts, deployment failures.  It
also updates the buildlog.txt file to indicate the build was sucessfull.

[]local-deploy.bat
Executed to handle the deploy event.  Responsible for executing in reverse order all batch files with the name pattern local-deploy##.bat.

[]local-deploy99.bat
Future usage.

[]local-deploy99.bat
This script is responsible for deploying the program to the device using information from targetboard.xml and the %TEX_HOME%\boards.xml file.  If the deployment is sucessfull it launches
the deploysucess.bat script otherwise it launches the deployfail.bat scripts.


[]deploysucess.bat
Executed to handle the deploy sucessful event.  Responsible for executing in reverse order all batch files with the name pattern deploy-success##.bat

[]deploy-sucess99.bat
This script is responsible for generating code to display the number of times the code has been deployed to the target device.  The function is optional and follows the same parameters as
the time stamp previously explained (see pre-build99.bat).  This file updates the buildlog.txt to indicate a sucessful deployment event has taken place.

[]deployfail.bat
Executed to handle a deployment failure event.  Responsible for executing in reverse order all batch files with the name pattern deploy-fail##.bat

[]deploy-fail99.bat
This script is responisble for updating the buildlog.txt file to indicate a deployment failure has taken place.
