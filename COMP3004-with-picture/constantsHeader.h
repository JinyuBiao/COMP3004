#ifndef CONSTANTSHEADER_H
#define CONSTANTSHEADER_H

//global constants

const int DEFAULT_CURRSTEP = -1;
const int DEFAULT_CPRCOUNT = 0;
const int DEFAULT_PREVIOUSCPR = -1;
const bool DEFAULT_AED_WORKING = false;
const float DEFAULT_WAVE_GRAPH_X = 0.0;
const int DEFAULT_CPR_TIME = 0;
const int DEFAULT_ANALYZING_TIME = 0;
const int DEFAULT_WAIT_PAD_TIME = 0;

#define ANALYZING_TIME 6    //the end time for analyzing
#define CPR_TIME 10         //the end time for cpr,
                            //actual time is CPR_TIME*2 seconds
#define WAIT_PAD_TIME 5     //the end time for waiting pad
#define MAIN_PROCESS_TIME_INTERVAL 2000
                            //the time interval for main process timer,
                            //1000 = 1s
#define ENOUGH_CPR_COUNT 4  //the enough number of cpr performed to possibly bring patient back to healthy, if they do not receive this number of cprs, they never get healthy and may die
#define CPR_BAR_DROP_RATE 250 //cpr bar dropping rate


#endif // CONSTANTSHEADER_H
