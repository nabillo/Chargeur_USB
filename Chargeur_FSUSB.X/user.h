/******************************************************************************/
/* User Level #define Macros                                                  */
/******************************************************************************/

#define DEADBAND 2
#define ERROR_STEP 5
#define DELTA 10
#define ADC_TIMEOUT 10

typedef enum State
{
    POWERUP,
    INITIALIZERD,
    BATTERIES_LISTED,
    BATTERY_SELECTED,
    ACTION_LISTED,
    ACTION_SELECTED,
    RUN_SELECTED,
    EDIT_SELECTED,
    DELETE_SELECTED,
    PROGRAM_LISTED,
    PROGRAM_SELECTED,
    LIPO_ALGO_STARTED,
    NIMH_ALGO_STARTED,
    PRECONDITION_SELECTED,
    PRECONDITION_STARTED,
    PRECONDITION_VERIFICATION,
    PRECONDITION_ENDED,
    CC_PHASE_STARTED,
    VBAT_VERIFICATION,
    CC_PHASE_ENDED,
    CV_PHASE_STARTED,
    IBAT_VERIFICATION,
    CV_PHASE_ENDED,
    TRICKLE_STARTED,
    CHARGE_ERROR,
    CHARGE_ENDED
} STATE;

typedef enum Program
{
    CHARGE,
    STORAGE,
    DISCHARGE,
    CYCLE,
    BALANCE
} PROGRAM;

typedef enum Action
{
    RUN,
    EDIT,
    DELETE
} ACTION;

char algo_Name [3][5] = {"LIPO","NIMH","PB"};

typedef enum Result
{
    OK,
    KO,
    INDEFINED
} RESULT;

typedef struct {
    short Kp;
    short Ki;
    short OutMax;
    float Setpoint;
    float Feedback;
    float Error;
    float Integral;
    float Output;
    short Saturated;
 } tPIParams;

/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/

short start_lipo_algo();
short check_Precondition(short *precondition);
short verifie_Precondition(char *end);
short start_Precondition();
short start_CC();
short verifie_Vbat(char *end);
short start_CV();
short verifie_Ibat(char *end);
short stop_CV();
void end_Charge();
short V_Eval(unsigned char channel,signed float *voltage);
short I_V_Set(unsigned char channel);
void CalcPI(tPIParams *PIdata);
short initialize(void);
short list_Battery(void);
void select_Battery(void);
short list_Action(void);
void select_Action(void);
short list_Program(void);
void select_Program(void);
unsigned int calc_Time(long initial);
short check_Lipo(void);
