/******************************************************************************
 *
 * Name: hwxfsleep.c - ACPI Hardware Sleep/Wake External Interfaces
 *
 *****************************************************************************/

/******************************************************************************
 *
 * 1. Copyright Notice
 *
 * Some or all of this work - Copyright (c) 1999 - 2012, Intel Corp.
 * All rights reserved.
 *
 * 2. License
 *
 * 2.1. This is your license from Intel Corp. under its intellectual property
 * rights.  You may have additional license terms from the party that provided
 * you this software, covering your right to use that party's intellectual
 * property rights.
 *
 * 2.2. Intel grants, free of charge, to any person ("Licensee") obtaining a
 * copy of the source code appearing in this file ("Covered Code") an
 * irrevocable, perpetual, worldwide license under Intel's copyrights in the
 * base code distributed originally by Intel ("Original Intel Code") to copy,
 * make derivatives, distribute, use and display any portion of the Covered
 * Code in any form, with the right to sublicense such rights; and
 *
 * 2.3. Intel grants Licensee a non-exclusive and non-transferable patent
 * license (with the right to sublicense), under only those claims of Intel
 * patents that are infringed by the Original Intel Code, to make, use, sell,
 * offer to sell, and import the Covered Code and derivative works thereof
 * solely to the minimum extent necessary to exercise the above copyright
 * license, and in no event shall the patent license extend to any additions
 * to or modifications of the Original Intel Code.  No other license or right
 * is granted directly or by implication, estoppel or otherwise;
 *
 * The above copyright and patent license is granted only if the following
 * conditions are met:
 *
 * 3. Conditions
 *
 * 3.1. Redistribution of Source with Rights to Further Distribute Source.
 * Redistribution of source code of any substantial portion of the Covered
 * Code or modification with rights to further distribute source must include
 * the above Copyright Notice, the above License, this list of Conditions,
 * and the following Disclaimer and Export Compliance provision.  In addition,
 * Licensee must cause all Covered Code to which Licensee contributes to
 * contain a file documenting the changes Licensee made to create that Covered
 * Code and the date of any change.  Licensee must include in that file the
 * documentation of any changes made by any predecessor Licensee.  Licensee
 * must include a prominent statement that the modification is derived,
 * directly or indirectly, from Original Intel Code.
 *
 * 3.2. Redistribution of Source with no Rights to Further Distribute Source.
 * Redistribution of source code of any substantial portion of the Covered
 * Code or modification without rights to further distribute source must
 * include the following Disclaimer and Export Compliance provision in the
 * documentation and/or other materials provided with distribution.  In
 * addition, Licensee may not authorize further sublicense of source of any
 * portion of the Covered Code, and must include terms to the effect that the
 * license from Licensee to its licensee is limited to the intellectual
 * property embodied in the software Licensee provides to its licensee, and
 * not to intellectual property embodied in modifications its licensee may
 * make.
 *
 * 3.3. Redistribution of Executable. Redistribution in executable form of any
 * substantial portion of the Covered Code or modification must reproduce the
 * above Copyright Notice, and the following Disclaimer and Export Compliance
 * provision in the documentation and/or other materials provided with the
 * distribution.
 *
 * 3.4. Intel retains all right, title, and interest in and to the Original
 * Intel Code.
 *
 * 3.5. Neither the name Intel nor any other trademark owned or controlled by
 * Intel shall be used in advertising or otherwise to promote the sale, use or
 * other dealings in products derived from or relating to the Covered Code
 * without prior written authorization from Intel.
 *
 * 4. Disclaimer and Export Compliance
 *
 * 4.1. INTEL MAKES NO WARRANTY OF ANY KIND REGARDING ANY SOFTWARE PROVIDED
 * HERE.  ANY SOFTWARE ORIGINATING FROM INTEL OR DERIVED FROM INTEL SOFTWARE
 * IS PROVIDED "AS IS," AND INTEL WILL NOT PROVIDE ANY SUPPORT,  ASSISTANCE,
 * INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL WILL NOT PROVIDE ANY
 * UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY DISCLAIMS ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGEMENT AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * 4.2. IN NO EVENT SHALL INTEL HAVE ANY LIABILITY TO LICENSEE, ITS LICENSEES
 * OR ANY OTHER THIRD PARTY, FOR ANY LOST PROFITS, LOST DATA, LOSS OF USE OR
 * COSTS OF PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, OR FOR ANY INDIRECT,
 * SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THIS AGREEMENT, UNDER ANY
 * CAUSE OF ACTION OR THEORY OF LIABILITY, AND IRRESPECTIVE OF WHETHER INTEL
 * HAS ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES.  THESE LIMITATIONS
 * SHALL APPLY NOTWITHSTANDING THE FAILURE OF THE ESSENTIAL PURPOSE OF ANY
 * LIMITED REMEDY.
 *
 * 4.3. Licensee shall not export, either directly or indirectly, any of this
 * software or system incorporating such software without first obtaining any
 * required license or other approval from the U. S. Department of Commerce or
 * any other agency or department of the United States Government.  In the
 * event Licensee exports any such software from the United States or
 * re-exports any such software from a foreign destination, Licensee shall
 * ensure that the distribution and export/re-export of the software is in
 * compliance with all laws, regulations, orders, or other restrictions of the
 * U.S. Export Administration Regulations. Licensee agrees that neither it nor
 * any of its subsidiaries will export/re-export any technical data, process,
 * software, or service, directly or indirectly, to any country for which the
 * United States government or any agency thereof requires an export license,
 * other governmental approval, or letter of assurance, without first obtaining
 * such license, approval or letter.
 *
 *****************************************************************************/

#include "acpi.h"
#include "accommon.h"

#define _COMPONENT          ACPI_HARDWARE
        ACPI_MODULE_NAME    ("hwxfsleep")

/* Local prototypes */

static ACPI_STATUS
AcpiHwSleepDispatch (
    UINT8                   SleepState,
    UINT32                  FunctionId);

/*
 * Dispatch table used to efficiently branch to the various sleep
 * functions.
 */
#define ACPI_SLEEP_FUNCTION_ID          0
#define ACPI_WAKE_PREP_FUNCTION_ID      1
#define ACPI_WAKE_FUNCTION_ID           2

/* Legacy functions are optional, based upon ACPI_REDUCED_HARDWARE */

static ACPI_SLEEP_FUNCTIONS         AcpiSleepDispatch[] =
{
    {ACPI_HW_OPTIONAL_FUNCTION (AcpiHwLegacySleep),    AcpiHwExtendedSleep},
    {ACPI_HW_OPTIONAL_FUNCTION (AcpiHwLegacyWakePrep), AcpiHwExtendedWakePrep},
    {ACPI_HW_OPTIONAL_FUNCTION (AcpiHwLegacyWake),     AcpiHwExtendedWake}
};


/*
 * These functions are removed for the ACPI_REDUCED_HARDWARE case:
 *      AcpiSetFirmwareWakingVector
 *      AcpiSetFirmwareWakingVector64
 *      AcpiEnterSleepStateS4bios
 */

#if (!ACPI_REDUCED_HARDWARE)
/*******************************************************************************
 *
 * FUNCTION:    AcpiSetFirmwareWakingVector
 *
 * PARAMETERS:  PhysicalAddress     - 32-bit physical address of ACPI real mode
 *                                    entry point.
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Sets the 32-bit FirmwareWakingVector field of the FACS
 *
 ******************************************************************************/

ACPI_STATUS
AcpiSetFirmwareWakingVector (
    UINT32                  PhysicalAddress)
{
    ACPI_FUNCTION_TRACE (AcpiSetFirmwareWakingVector);


    /* Set the 32-bit vector */

    AcpiGbl_FACS->FirmwareWakingVector = PhysicalAddress;

    /* Clear the 64-bit vector if it exists */

    if ((AcpiGbl_FACS->Length > 32) && (AcpiGbl_FACS->Version >= 1))
    {
        AcpiGbl_FACS->XFirmwareWakingVector = 0;
    }

    return_ACPI_STATUS (AE_OK);
}

ACPI_EXPORT_SYMBOL (AcpiSetFirmwareWakingVector)


#if ACPI_MACHINE_WIDTH == 64
/*******************************************************************************
 *
 * FUNCTION:    AcpiSetFirmwareWakingVector64
 *
 * PARAMETERS:  PhysicalAddress     - 64-bit physical address of ACPI protected
 *                                    mode entry point.
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Sets the 64-bit X_FirmwareWakingVector field of the FACS, if
 *              it exists in the table. This function is intended for use with
 *              64-bit host operating systems.
 *
 ******************************************************************************/

ACPI_STATUS
AcpiSetFirmwareWakingVector64 (
    UINT64                  PhysicalAddress)
{
    ACPI_FUNCTION_TRACE (AcpiSetFirmwareWakingVector64);


    /* Determine if the 64-bit vector actually exists */

    if ((AcpiGbl_FACS->Length <= 32) || (AcpiGbl_FACS->Version < 1))
    {
        return_ACPI_STATUS (AE_NOT_EXIST);
    }

    /* Clear 32-bit vector, set the 64-bit X_ vector */

    AcpiGbl_FACS->FirmwareWakingVector = 0;
    AcpiGbl_FACS->XFirmwareWakingVector = PhysicalAddress;
    return_ACPI_STATUS (AE_OK);
}

ACPI_EXPORT_SYMBOL (AcpiSetFirmwareWakingVector64)
#endif


/*******************************************************************************
 *
 * FUNCTION:    AcpiEnterSleepStateS4bios
 *
 * PARAMETERS:  None
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Perform a S4 bios request.
 *              THIS FUNCTION MUST BE CALLED WITH INTERRUPTS DISABLED
 *
 ******************************************************************************/

ACPI_STATUS
AcpiEnterSleepStateS4bios (
    void)
{
    UINT32                  InValue;
    ACPI_STATUS             Status;


    ACPI_FUNCTION_TRACE (AcpiEnterSleepStateS4bios);


    /* Clear the wake status bit (PM1) */

    Status = AcpiWriteBitRegister (ACPI_BITREG_WAKE_STATUS, ACPI_CLEAR_STATUS);
    if (ACPI_FAILURE (Status))
    {
        return_ACPI_STATUS (Status);
    }

    Status = AcpiHwClearAcpiStatus ();
    if (ACPI_FAILURE (Status))
    {
        return_ACPI_STATUS (Status);
    }

    /*
     * 1) Disable/Clear all GPEs
     * 2) Enable all wakeup GPEs
     */
    Status = AcpiHwDisableAllGpes ();
    if (ACPI_FAILURE (Status))
    {
        return_ACPI_STATUS (Status);
    }
    AcpiGbl_SystemAwakeAndRunning = FALSE;

    Status = AcpiHwEnableAllWakeupGpes ();
    if (ACPI_FAILURE (Status))
    {
        return_ACPI_STATUS (Status);
    }

    ACPI_FLUSH_CPU_CACHE ();

    Status = AcpiHwWritePort (AcpiGbl_FADT.SmiCommand,
                (UINT32) AcpiGbl_FADT.S4BiosRequest, 8);

    do {
        AcpiOsStall(1000);
        Status = AcpiReadBitRegister (ACPI_BITREG_WAKE_STATUS, &InValue);
        if (ACPI_FAILURE (Status))
        {
            return_ACPI_STATUS (Status);
        }
    } while (!InValue);

    return_ACPI_STATUS (AE_OK);
}

ACPI_EXPORT_SYMBOL (AcpiEnterSleepStateS4bios)

#endif /* !ACPI_REDUCED_HARDWARE */


/*******************************************************************************
 *
 * FUNCTION:    AcpiHwSleepDispatch
 *
 * PARAMETERS:  SleepState          - Which sleep state to enter/exit
 *              FunctionId          - Sleep, WakePrep, or Wake
 *
 * RETURN:      Status from the invoked sleep handling function.
 *
 * DESCRIPTION: Dispatch a sleep/wake request to the appropriate handling
 *              function.
 *
 ******************************************************************************/

static ACPI_STATUS
AcpiHwSleepDispatch (
    UINT8                   SleepState,
    UINT32                  FunctionId)
{
    ACPI_STATUS             Status;
    ACPI_SLEEP_FUNCTIONS    *SleepFunctions = &AcpiSleepDispatch[FunctionId];


#if (!ACPI_REDUCED_HARDWARE)

    /*
     * If the Hardware Reduced flag is set (from the FADT), we must
     * use the extended sleep registers
     */
    if (AcpiGbl_ReducedHardware ||
        AcpiGbl_FADT.SleepControl.Address)
    {
        Status = SleepFunctions->ExtendedFunction (SleepState);
    }
    else
    {
        /* Legacy sleep */

        Status = SleepFunctions->LegacyFunction (SleepState);
    }

    return (Status);

#else
    /*
     * For the case where reduced-hardware-only code is being generated,
     * we know that only the extended sleep registers are available
     */
    Status = SleepFunctions->ExtendedFunction (SleepState);
    return (Status);

#endif /* !ACPI_REDUCED_HARDWARE */
}


/*******************************************************************************
 *
 * FUNCTION:    AcpiEnterSleepStatePrep
 *
 * PARAMETERS:  SleepState          - Which sleep state to enter
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Prepare to enter a system sleep state.
 *              This function must execute with interrupts enabled.
 *              We break sleeping into 2 stages so that OSPM can handle
 *              various OS-specific tasks between the two steps.
 *
 ******************************************************************************/

ACPI_STATUS
AcpiEnterSleepStatePrep (
    UINT8                   SleepState)
{
    ACPI_STATUS             Status;
    ACPI_OBJECT_LIST        ArgList;
    ACPI_OBJECT             Arg;
    UINT32                  SstValue;


    ACPI_FUNCTION_TRACE (AcpiEnterSleepStatePrep);


    /* _PSW methods could be run here to enable wake-on keyboard, LAN, etc. */

    Status = AcpiGetSleepTypeData (SleepState,
                    &AcpiGbl_SleepTypeA, &AcpiGbl_SleepTypeB);
    if (ACPI_FAILURE (Status))
    {
        return_ACPI_STATUS (Status);
    }

    /* Execute the _PTS method (Prepare To Sleep) */

    ArgList.Count = 1;
    ArgList.Pointer = &Arg;
    Arg.Type = ACPI_TYPE_INTEGER;
    Arg.Integer.Value = SleepState;

    Status = AcpiEvaluateObject (NULL, METHOD_PATHNAME__PTS, &ArgList, NULL);
    if (ACPI_FAILURE (Status) && Status != AE_NOT_FOUND)
    {
        return_ACPI_STATUS (Status);
    }

    /* Setup the argument to the _SST method (System STatus) */

    switch (SleepState)
    {
    case ACPI_STATE_S0:
        SstValue = ACPI_SST_WORKING;
        break;

    case ACPI_STATE_S1:
    case ACPI_STATE_S2:
    case ACPI_STATE_S3:
        SstValue = ACPI_SST_SLEEPING;
        break;

    case ACPI_STATE_S4:
        SstValue = ACPI_SST_SLEEP_CONTEXT;
        break;

    default:
        SstValue = ACPI_SST_INDICATOR_OFF; /* Default is off */
        break;
    }

    /*
     * Set the system indicators to show the desired sleep state.
     * _SST is an optional method (return no error if not found)
     */
    AcpiHwExecuteSleepMethod (METHOD_PATHNAME__SST, SstValue);
    return_ACPI_STATUS (AE_OK);
}

ACPI_EXPORT_SYMBOL (AcpiEnterSleepStatePrep)


/*******************************************************************************
 *
 * FUNCTION:    AcpiEnterSleepState
 *
 * PARAMETERS:  SleepState          - Which sleep state to enter
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Enter a system sleep state
 *              THIS FUNCTION MUST BE CALLED WITH INTERRUPTS DISABLED
 *
 ******************************************************************************/

ACPI_STATUS
AcpiEnterSleepState (
    UINT8                   SleepState)
{
    ACPI_STATUS             Status;


    ACPI_FUNCTION_TRACE (AcpiEnterSleepState);


    if ((AcpiGbl_SleepTypeA > ACPI_SLEEP_TYPE_MAX) ||
        (AcpiGbl_SleepTypeB > ACPI_SLEEP_TYPE_MAX))
    {
        ACPI_ERROR ((AE_INFO, "Sleep values out of range: A=0x%X B=0x%X",
            AcpiGbl_SleepTypeA, AcpiGbl_SleepTypeB));
        return_ACPI_STATUS (AE_AML_OPERAND_VALUE);
    }

    Status = AcpiHwSleepDispatch (SleepState, ACPI_SLEEP_FUNCTION_ID);
    return_ACPI_STATUS (Status);
}

ACPI_EXPORT_SYMBOL (AcpiEnterSleepState)


/*******************************************************************************
 *
 * FUNCTION:    AcpiLeaveSleepStatePrep
 *
 * PARAMETERS:  SleepState          - Which sleep state we are exiting
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Perform the first state of OS-independent ACPI cleanup after a
 *              sleep. Called with interrupts DISABLED.
 *              We break wake/resume into 2 stages so that OSPM can handle
 *              various OS-specific tasks between the two steps.
 *
 ******************************************************************************/

ACPI_STATUS
AcpiLeaveSleepStatePrep (
    UINT8                   SleepState)
{
    ACPI_STATUS             Status;


    ACPI_FUNCTION_TRACE (AcpiLeaveSleepStatePrep);


    Status = AcpiHwSleepDispatch (SleepState, ACPI_WAKE_PREP_FUNCTION_ID);
    return_ACPI_STATUS (Status);
}

ACPI_EXPORT_SYMBOL (AcpiLeaveSleepStatePrep)


/*******************************************************************************
 *
 * FUNCTION:    AcpiLeaveSleepState
 *
 * PARAMETERS:  SleepState          - Which sleep state we are exiting
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Perform OS-independent ACPI cleanup after a sleep
 *              Called with interrupts ENABLED.
 *
 ******************************************************************************/

ACPI_STATUS
AcpiLeaveSleepState (
    UINT8                   SleepState)
{
    ACPI_STATUS             Status;


    ACPI_FUNCTION_TRACE (AcpiLeaveSleepState);


    Status = AcpiHwSleepDispatch (SleepState, ACPI_WAKE_FUNCTION_ID);
    return_ACPI_STATUS (Status);
}

ACPI_EXPORT_SYMBOL (AcpiLeaveSleepState)
