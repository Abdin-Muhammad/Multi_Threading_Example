#include <fstream>
#include <thread>
#include <iostream>
#include <chrono>
#include <inttypes.h>
#include <regex>
#include <ctype.h>
#include <mutex>
#include <condition_variable>

#define inputFileName "battery_data.csv"

#define outputFileName "Avg_battery_data.csv"

#define dataPointsSamples 10    // numbers of samples we want to process

#define voltColumnNum 0         // Position of voltage data in CSV file
#define socColumnNum 1          // Position of SOC data in CSV file

#define numofRows 8000          // given numbers of rows inside CSV file
#define numofCols 2             // given numbers of columns inside CSV file

void ReadCSV();                 // method to read data from CSV file
void addData2File();            // method to write data into CSV file
double calcAvgSOC();            // method to calculate average of voltage data
double calcAvgVolt();           // method to calculate average of SOC data

int readCount=1;              //  variable to count number of read lines

std::mutex m;                   // to generate locking & unlocking
std::condition_variable cv;     // to pass status between threads
bool readingDone = false;
bool writingStart = false;
void readThreadCtrl();          // method to signal the writing thread
void writeThreadCtrl();         // method to signal the reading thread


double DataArr[numofRows][numofCols];           // Array to hold data from CSV file
int traverseVdataFrom=1;                        // iterator for voltage data starting point
int traverseVdataTo=dataPointsSamples;          // iterator for voltage data ending point
int traverseSOCdataFrom=1;                      // iterator for SOC data starting point
int traverseSOCdataTo=dataPointsSamples;        // iterator for SOC data ending point

using namespace std;

int main()
{
	thread thReadData(ReadCSV);                 // Creating a thread to read data from CSV
	thread thWriteCSV(addData2File);            // Creating a thread to write data to CSV file
	thWriteCSV.join();
	thReadData.join();

	return 0;
}


void ReadCSV()
{
  string line;                                  // text line from file
  string tempStr= "";                           // to hold string temporary
  int   colNum=0;
  int   rowNum=0;
  int   lengthOfLine=0;
  int   posOnLine=0;
  int   lastposOnLine=0;
  int   datapoints=0;
ifstream infile (inputFileName);

  if(infile.is_open())
  {
    while (getline(infile,line))        // Run till last line
    {

        posOnLine=0,lastposOnLine=0,colNum=0;
        lengthOfLine=line.length();

       while(posOnLine!=lengthOfLine)
        {
            if(line[posOnLine]==','||posOnLine==lengthOfLine-1)
                {

                    DataArr[rowNum][colNum]=atof(tempStr.append(line,lastposOnLine,posOnLine).c_str()); // parsing data
                    tempStr="";
                    colNum++;
                    if(posOnLine!=lengthOfLine-1)
                    lastposOnLine=posOnLine+1;
                }
            posOnLine++;
        }

        rowNum++;

        datapoints++;
        if(datapoints == dataPointsSamples)
          {
            datapoints =0;
            readCount++;
            readThreadCtrl(); // calling to write as 10 datapoints has been collected
          }
  }

 }

     readThreadCtrl(); // to make sure that all data is written
     infile.close();
     cout<< "Reading Done=" <<rowNum<< endl;

}


double calcAvgVolt()
{
    double avgVolt=0;
    for(;traverseVdataFrom<traverseVdataTo;traverseVdataFrom++)
    {
         avgVolt+=DataArr[traverseVdataFrom][voltColumnNum];
    }
    avgVolt = avgVolt/dataPointsSamples;
    traverseVdataTo+=dataPointsSamples;     // get to next ten values position in DataArr
    return avgVolt;
}

double calcAvgSOC()
{
    double avgSOC=0;
    for(;traverseSOCdataFrom<traverseSOCdataTo;traverseSOCdataFrom++)
    {
         avgSOC+=DataArr[traverseSOCdataFrom][socColumnNum];
    }
    avgSOC = avgSOC/dataPointsSamples;
    traverseSOCdataTo+=dataPointsSamples;
 	return avgSOC;
}

void addData2File()
{

    ofstream writeFile;
    writeFile.open(outputFileName, ios::out | ios::trunc); // Creating a output file in trunc mode
    if(writeFile.is_open())
    {
        writeFile << "VoltAvg" << "," << "SOCAvg" << ","<< "Unix TimeStamp"  << "\n"; // Adding title
    }
    int writeCount=0;
    while(  writeCount!=readCount)          // write till last read count
    {



    this_thread::sleep_for(chrono::milliseconds(20));      // adding sleep to cause delay
    writeThreadCtrl(); // run after getting the signal from read



	double avgV=0;                          // to hold average value
	double avgSOC=0;                        // to hold average value



    if(writeFile.is_open()) // if file is opened successfully
    {
		avgV=calcAvgVolt();
		avgSOC=calcAvgSOC();
		uint64_t now = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count(); // for Timestamp
        writeFile << avgV << "," << avgSOC << "," << now << "\n"; // writing to the CSV file
        avgSOC=0;
        avgV=0;

    }
        writeCount++;


    }
	writeFile.close(); // close file
	cout << "writing Done !!"<< endl;
}


void readThreadCtrl()
{
{
    std::lock_guard<std::mutex> lk(m);
    readingDone = true;
    writingStart = false;

}
    cv.notify_one(); // notify that 10 datapoints reading is done
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, []{return writingStart;}); // wait till writing is started

}

void writeThreadCtrl()
{
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, []{return readingDone;}); // wait till readingDone becomes true

    writingStart = true;
    readingDone = false;

    lk.unlock(); // unlock mutex
    cv.notify_one(); // notify writing is started

}



