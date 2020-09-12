 Example Task : 
"
Implement a software with two threads. The first thread reads out the values from one CSV file and sends them to the other thread once it read 10 data points.
 The Second thread get the arithmetic average value of these data points and store them with the UNIX timestamp in milliseconds in a 2nd csv file.

"

Explanation:
This program consists of 2 threads
1st Thread "thReadData" is using method named "ReadCSV" which is storing input data from user in array "DataArr"

2nd Thread "thWriteCSV" is using method named "addData2File" which is taking average of stored data & then writing data with timestamp into a CSV file.

Mutex and conditonal variable is used to achieve parallelism between threads. for further explanation i have also added comments inside main code file. 




  
