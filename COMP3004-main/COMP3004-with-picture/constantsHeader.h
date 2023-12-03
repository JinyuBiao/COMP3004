#ifndef CONSTANTSHEADER_H
#define CONSTANTSHEADER_H

//global constants

#define DEFAULT_CURRSTEP -1;
#define DEFAULT_CPRCOUNT 0;
#define DEFAULT_SHOCKCOUNT 0;
#define DEFAULT_PREVIOUSCPR -1;
const bool DEFAULT_AED_WORKING = false;
#define DEFAULT_WAVE_GRAPH_X 0.0;
#define DEFAULT_CPR_TIME 0;
#define DEFAULT_ANALYZING_TIME 0;
#define DEFAULT_WAIT_PAD_TIME 0;

const float CPR_COMPRESSION_LEVEL_A_INCH = 0.5;
const float CPR_COMPRESSION_LEVEL_B_INCH = 1;
const float CPR_COMPRESSION_LEVEL_C_INCH = 2;

const int CPR_PATIENT_HEALTHY_POSSIBILITY_FACTOR = 3;
const int CPR_PATIENT_DIE_POSSIBILITY_FACTOR = 3;
const int CPR_NOT_ENOUGH_PATIENT_HEALTHY_POSSIBILITY_FACTOR = 4;
const int CPR_IMPOSSIBLE = 2;

const int CPR_BAR_MULTIPLIAR = 40;

const int ENERGY_NORMALOPERATION_J = 50;
const int ENERGY_J_1 = 120;
const int ENERGY_J_2 = 150;
const int ENERGY_J_3 = 200;

const int BATTERY_FULL_ENERGY_J = 18135;


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
