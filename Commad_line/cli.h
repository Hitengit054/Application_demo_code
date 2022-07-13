#ifndef __CLI_H__  
#define __CLI_H__

#include <stdio.h>
#include <stdint.h>
#include <pthread.h> 
#include <unistd.h>
#include <pthread.h>

 
/**
  This defintion determines the index that is used for the first command
  in a command list.  Typically this will be eiher 0 or 1.
  */
#define COMMAND_START_INDEX                                             0

/**
   This definition determines the size of the input buffer for CLI
   command. It effectively controls the maximum length of a command and
   its parameters.
*/ 

#define MAXIMUM_QCLI_COMMAND_STRING_LENGTH                              (256)

/**
   This definition determines the maximum number of parameters that can be
   provided. Note that this may also include paramters used to navigete
   into groups.
*/

#define MAXIMUM_NUMBER_OF_PARAMETERS                                    (17)

/**
   This definition determines the size of the buffer used for formatted
   messages to the console when using QCLI_Printf.
*/

#define MAXIMUM_PRINTF_LENGTH                                           (256)

#define true                                                            (1)
#define false                                                           (0)

typedef enum
{
   QCLI_STATUS_SUCCESS_E,
   QCLI_STATUS_ERROR_E,
   QCLI_STATUS_USAGE_E
} QCLI_Command_Status_t;

typedef struct QCLI_Parameter_s
{
   char    *String_Value;
   int32_t  Integer_Value;
   uint32_t  Integer_Is_Valid;
} QCLI_Parameter_t;


/**
   @brief Type which represents the format of a function which processes
          commands from the CLI.

   @param Parameter_Count indicates the number of parameters that were
          specified to the CLI for the function.
   @param Parameter_List is the list of parameters specified to the CLI
          for the function.

   @return
    - QCLI_STATUS_SUCCESS_E if the command executed successfully.
    - QCLI_STATUS_ERROR_E if the command encounted a general error. Note
      that the CLI currently doesn't take any action for this error.
    - QCLI_STATUS_USAGE_E indicates that the parameters passed to the CLI
      were not correct for the command.  When this error code is returned,
      the CLI will display the usage message for the command.
*/

typedef QCLI_Command_Status_t (*QCLI_Command_Function_t)(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

/**
   This type represents a group handle.
*/
typedef void *QCLI_Group_Handle_t;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 

typedef struct QCLI_Command_s
{
   QCLI_Command_Function_t  Command_Function; /** The function that will be called when the command is executed from the CLI. */
   uint32_t                 Start_Thread;     /** A flag which indicates if the command should start on its own thread.       */
   const char              *Command_String;   /** The string representation of the function.                                  */
   const char              *Usage_String;     /** The usage string for the command.                                           */
   const char              *Description;      /** The description string for the commmand.                                    */
} QCLI_Command_t;

typedef struct QCLI_Command_Group_s
{
   const char           *Group_String;   /** The string representation of the group. */
   uint32_t              Command_Count; /** The number of commands in the group.    */
   const QCLI_Command_t *Command_List;   /** The list of commands for the group.     */
} QCLI_Command_Group_t;

typedef struct Group_List_Entry_s
{
   const QCLI_Command_Group_t *Command_Group;         /**< The command group information. */
   struct Group_List_Entry_s  *Parent_Group;          /**< the parent group for this subgroup. */
   struct Group_List_Entry_s  *Subgroup_List;         /**< The list of subgroups registerd for this group. */
   struct Group_List_Entry_s  *Next_Group_List_Entry; /**< The next entry in the list. */
} Group_List_Entry_t;


/*
 * This structure reprents the result of a Find_Command() operation.
 */
typedef struct Find_Result_s
{
    /* A flag indicating if the result is a command or a group. */
    int Is_Group;
    union
    {
        /* The entry that was found if it is a command. */
        const QCLI_Command_t *Command;
        /* The entry that was found if it is a group. */
        Group_List_Entry_t   *Group_List_Entry;
    } Data;
} Find_Result_t;


typedef struct Thread_Info_s
{
   pthread_mutex_t       Signal_LOCK;        /**< Wait for Signal LOCK */  
   pthread_cond_t        Thread_Ready_Event; /**< Event which indicates the thread no longer needs this information structure. */
   uint32_t              Command_Index;      /**< The index of the command that will be executed. */
   const QCLI_Command_t *Command;            /**< The command that will be executed. */
   uint32_t              Parameter_Count;    /**< The number of parameters specified for the command. */
   QCLI_Parameter_t     *Parameter_List;     /**< The list of paramters for the command. */
} Thread_Info_t;

typedef struct QCLI_Context_s
{
   Group_List_Entry_t  Root_Group;                                           /**< The root of the group menu structure.                                    */
   Group_List_Entry_t *Current_Group;                                        /**< The current group.                                                       */
   Group_List_Entry_t *Executing_Group;                                      /**< Group of currently executing command. */

   uint32_t            Input_Length;                                         /**< The length of the current console input string.                          */
   char                Input_String[MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1]; /**< Buffer containing the current console input string.                      */
   QCLI_Parameter_t    Parameter_List[MAXIMUM_NUMBER_OF_PARAMETERS + 1];     /**< List of parameters for input command.                                    */

   uint32_t            Thread_Count;                                         /**< THe number of command threads that are currently running.                */
   Thread_Info_t       Thread_Info;                                          /**< Information structure for passing information to command threads.        */
   pthread_mutex_t     CLI_Mutex;                                            /**< The Mutex used to protect shared resources of the module.                */

   char                Printf_Buffer[MAXIMUM_PRINTF_LENGTH];                 /**< The buffer used for formatted output strings.                            */
   QCLI_Group_Handle_t Current_Printf_Group;                                 /**< The group handle that was last passed to QCLI_Printf().                  */
   uint32_t            Printf_New_Line;                                      /**< Indicates that a newline should be displayed if a printf changes groups. */
} QCLI_Context_t;


int32_t Memcmpi(const void *Source1, const void *Source2, uint32_t size);


#endif
