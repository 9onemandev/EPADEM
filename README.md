# EPAMK
The application is used to assess the configuration of the water supply network in conditions where geodetic elevations of the earth's surface are known approximately.

![image](https://github.com/9onemandev/EPADEM/assets/163633812/182c243c-5c71-4514-9b8c-e1dea8e2c4c3)

The program changes the values of geodetic elevations of nodes at each iteration and recalculates the hydraulic calculation.
Changes are instead made within a user-specified range (GE deviation).
As a result of the calculation, a report is generated, which the program analyzes and if the pressure in the node is less than the minimum pressure specified by the user (Min. pressure value, m), the network configuration is accepted as unsatisfactory.
Ultimately, the total number of all iterations and the number of unsatisfactory attempts are compared and the percentage of reliability of the selected network configuration is determined.

![image](https://github.com/9onemandev/EPADEM/assets/163633812/e6f8f744-0b28-4277-aba9-d5dd5c7d8206)

Settings for the number of CPU cores allows you to speed up the simulation process.
"Don`t delete temp file" checkbox will allow to save all generated inp and rpt files. So it is possible to take a look inside each iteration of  modeling.
They will be in app folder in format (1_5.inp), where 1 is thread id and 5 - is iteration number.
But be careful! In case of big inp file size and big number of iterations you can generate tons of GB of data on your PC.

The Monte Carlo method was used. Distribution of random variables - normal.

There is no license restrictions. Feel free to use it as you want.
Also i`m not resposible in case of some errors in code and produced results.

One important thing!
Your inp file should contain report section in format like shown in report section.txt.

Compiled app is in release folder (also contains epanet). Code for this app is in src folder.
