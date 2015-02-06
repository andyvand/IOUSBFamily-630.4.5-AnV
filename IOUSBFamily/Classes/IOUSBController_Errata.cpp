/*
 * Copyright © 1998-2010 Apple Inc.  All rights reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <IOKit/system.h>
#include <IOKit/IOPlatformExpert.h>

#include "../Headers/IOUSBController.h"
#include "../Headers/IOUSBControllerV2.h"
#include "../Headers/IOUSBControllerV3.h"
#include "../Headers/IOUSBLog.h"

#define super IOUSBBus
#define self this

/*
 This table contains the list of errata that are necessary for known
 problems with particular silicon.  The format is vendorID, deviceID,
 lowest revisionID needing errata, highest rev needing errata, errataBits.
 The result of all matches is ORed together, so more than one entry may
 match.  Typically for a given errata a list of chips revisions that
 this applies to is supplied.
 */
static ErrataListEntry  errataList[] = {
    
    {0x1095, 0x0670, 0, 0x0004,	kErrataCMDDisableTestMode | kErrataOnlySinglePageTransfers | kErrataRetryBufferUnderruns},	// CMD 670 & 670a (revs 0-4)
    {0x1045, 0xc861, 0, 0x001f, kErrataLSHSOpti},																			// Opti 1045
    {0x11C1, 0x5801, 0, 0xffff, kErrataDisableOvercurrent | kErrataLucentSuspendResume | kErrataNeedsWatchdogTimer},		// Lucent USS 302
    {0x11C1, 0x5802, 0, 0xffff, kErrataDisableOvercurrent | kErrataLucentSuspendResume | kErrataNeedsWatchdogTimer},		// Lucent USS 312
    {0x106b, 0x0019, 0, 0xffff, kErrataDisableOvercurrent | kErrataNeedsWatchdogTimer},										// Apple KeyLargo - all revs
    {0x106b, 0x0019, 0, 0, 	kErrataLucentSuspendResume },																	// Apple KeyLargo - USB Rev 0 only
    {0x106b, 0x0026, 0, 0xffff, kErrataDisableOvercurrent | kErrataLucentSuspendResume | kErrataNeedsWatchdogTimer},		// Apple Pangea, all revs
    {0x106b, 0x003f, 0, 0xffff, kErrataDisableOvercurrent | kErrataNeedsWatchdogTimer},										// Apple Intrepid, all revs
    {0x1033, 0x0035, 0, 0xffff, kErrataDisableOvercurrent | kErrataNECOHCIIsochWraparound | kErrataNECIncompleteWrite },	// NEC OHCI
    {0x1033, 0x00e0, 0, 0xffff, kErrataDisableOvercurrent | kErrataNECIncompleteWrite},										// NEC EHCI
    {0x1131, 0x1561, 0x30, 0x30, kErrataNeedsPortPowerOff },																// Philips, USB 2
    {0x11C1, 0x5805, 0x11, 0x11, kErrataAgereEHCIAsyncSched },																// Agere, Async Schedule bug

    //Slice
	{0x8086, 0x2442, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },	//
	{0x8086, 0x2482, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },	// ICH Intel 845 UHCI #1
	{0x8086, 0x2484, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },	// ICH UHCI #2
	{0x8086, 0x2487, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },	// ICH UHCI #3

	{0x8086, 0x24d2, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },	// ICH Intel 865 UHCI #1
	{0x8086, 0x24d4, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },	// ICH UHCI #2
	{0x8086, 0x24d7, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },	// ICH UHCI #3
	{0x8086, 0x24de, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },	// ICH UHCI #4
	{0x8086, 0x24dd, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataNeedsOvercurrentDebounce },									// Intel 865

	{0x8086, 0x24c2, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },	// ICH Intel 855 UHCI #1
	{0x8086, 0x24c4, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },	// ICH UHCI #2
	{0x8086, 0x24c7, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },	// ICH UHCI #3
	{0x8086, 0x24ce, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },	// ICH UHCI #4
	{0x8086, 0x24cd, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataNeedsOvercurrentDebounce },									// Intel 855

	{0x8086, 0x2658, 0x03, 0x04, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },	// ICH6 UHCI #1
	{0x8086, 0x2659, 0x03, 0x04, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },	// ICH6 UHCI #2
	{0x8086, 0x265A, 0x03, 0x04, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },	// ICH6 UHCI #3
	{0x8086, 0x265B, 0x03, 0x04, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },	// ICH6 UHCI #4
	{0x8086, 0x265C, 0x03, 0x04, kErrataICH6PowerSequencing | kErrataNeedsOvercurrentDebounce },									// ICH6 EHCI
	
	{0x8086, 0x2688, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },	// ESB UHCI #1
	{0x8086, 0x2689, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },	// ESB UHCI #2
	{0x8086, 0x268A, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },	// ESB UHCI #3
	{0x8086, 0x268B, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },	// ESB UHCI #4
	{0x8086, 0x268C, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataNeedsOvercurrentDebounce },									// ESB EHCI
	
	{0x8086, 0x27C8, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },	// ICH7 UHCI #1
	{0x8086, 0x27C9, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },	// ICH7 UHCI #2
	{0x8086, 0x27CA, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },	// ICH7 UHCI #3
	{0x8086, 0x27CB, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },	// ICH7 UHCI #4
	{0x8086, 0x27CC, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataICH7ISTBuffer  | kErrataNeedsOvercurrentDebounce },			// ICH7 EHCI

	{0x8086, 0x2830, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },   // ICH8 UHCI #1
	{0x8086, 0x2831, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },   // ICH8 UHCI #2
	{0x8086, 0x2832, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },   // ICH8 UHCI #3
	{0x8086, 0x2834, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },   // ICH8 UHCI #4
	{0x8086, 0x2835, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },   // ICH8 UHCI #5
	{0x8086, 0x2836, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataNeedsOvercurrentDebounce },			// ICH8 EHCI #1
	{0x8086, 0x283a, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataNeedsOvercurrentDebounce },			// ICH8 EHCI #2

    //Slice
	{0x8086, 0x2934, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },   // ICH9 UHCI #1
	{0x8086, 0x2935, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },   // ICH9 UHCI #2
	{0x8086, 0x2936, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },   // ICH9 UHCI #3
	{0x8086, 0x2937, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },   // ICH9 UHCI #4
	{0x8086, 0x2938, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },   // ICH9 UHCI #5
	{0x8086, 0x2939, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },   // ICH9 UHCI #5
	{0x8086, 0x293a, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataNeedsOvercurrentDebounce },			// ICH9 EHCI #1
	{0x8086, 0x293c, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataNeedsOvercurrentDebounce },			// ICH9 EHCI #2

	{0x8086, 0x8114, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable  },   // Poulsbo UHCI #1
	{0x8086, 0x8115, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable  },   // Poulsbo UHCI #2
	{0x8086, 0x8116, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable  },   // Poulsbo UHCI #3
	{0x8086, 0x8117, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataNeedsOvercurrentDebounce }	,			// Pouslbo EHCI #2

	{0x10de, 0x0aa6, 0x00, 0xff, kErrataNoCSonSplitIsoch | kErrataMissingPortChangeInt  | kErrataUse32bitEHCI},			// MCP79 EHCI #1
	{0x10de, 0x0aa9, 0x00, 0xff, kErrataNoCSonSplitIsoch | kErrataMissingPortChangeInt  | kErrataUse32bitEHCI},			// MCP79 EHCI #2
	{0x10de, 0x0aa5, 0x00, 0xff, kErrataOHCINoGlobalSuspendOnSleep },								// MCP79 OHCI #1
	{0x10de, 0x0aa7, 0x00, 0xff, kErrataOHCINoGlobalSuspendOnSleep },								// MCP79 OHCI #2

	{0x8086, 0x3a34, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },   // ICH10 UHCI #1
	{0x8086, 0x3a35, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },   // ICH10 UHCI #2
	{0x8086, 0x3a36, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },   // ICH10 UHCI #3
	{0x8086, 0x3a37, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },   // ICH10 UHCI #4
	{0x8086, 0x3a38, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },   // ICH10 UHCI #5
	{0x8086, 0x3a39, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable | kErrataUHCISupportsResumeDetectOnConnect },   // ICH10 UHCI #6
	{0x8086, 0x3a3a, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataNeedsOvercurrentDebounce },			// ICH10 EHCI #1
	{0x8086, 0x3a3c, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataNeedsOvercurrentDebounce },			// ICH10 EHCI #2
	
	{0x8086, 0x3b36, 0x00, 0xff, kErrataDontUseCompanionController },   // P55 UHCI #1 (Ibex Peak)
	{0x8086, 0x3b37, 0x00, 0xff, kErrataDontUseCompanionController },   // P55 UHCI #2 (Ibex Peak)
	{0x8086, 0x3b38, 0x00, 0xff, kErrataDontUseCompanionController },   // P55 UHCI #3 (Ibex Peak)
	{0x8086, 0x3b39, 0x00, 0xff, kErrataDontUseCompanionController },   // P55 UHCI #4 (Ibex Peak)
	{0x8086, 0x3b3b, 0x00, 0xff, kErrataDontUseCompanionController },   // P55 UHCI #5 (Ibex Peak)
	{0x8086, 0x3b3e, 0x00, 0xff, kErrataDontUseCompanionController },   // P55 UHCI #6 (Ibex Peak)
	{0x8086, 0x3b3f, 0x00, 0xff, kErrataDontUseCompanionController },   // P55 UHCI #7 (Ibex Peak)
	{0x8086, 0x3b34, 0x00, 0xff, kErrataDontUseCompanionController },	// P55 EHCI #1 (Ibex Peak)
	{0x8086, 0x3b3c, 0x00, 0xff, kErrataDontUseCompanionController },	// P55 EHCI #2 (Ibex Peak)
	
	{0x8086, 0x1c27, 0x00, 0xff, kErrataDontUseCompanionController },   // CPT UHCI #1
	{0x8086, 0x1c28, 0x00, 0xff, kErrataDontUseCompanionController },   // CPT UHCI #2
	{0x8086, 0x1c29, 0x00, 0xff, kErrataDontUseCompanionController },   // CPT UHCI #3
	{0x8086, 0x1c2a, 0x00, 0xff, kErrataDontUseCompanionController },   // CPT UHCI #4
	{0x8086, 0x1c2c, 0x00, 0xff, kErrataDontUseCompanionController },   // CPT UHCI #5
	{0x8086, 0x1c2e, 0x00, 0xff, kErrataDontUseCompanionController },   // CPT UHCI #6
	{0x8086, 0x1c2f, 0x00, 0xff, kErrataDontUseCompanionController },   // CPT UHCI #7
	{0x8086, 0x1c26, 0x00, 0xff, kErrataDontUseCompanionController },	// CPT EHCI #1
	{0x8086, 0x1c2d, 0x00, 0xff, kErrataDontUseCompanionController },	// CPT EHCI #2

    //Slice
 	{0x8086, 0x3a64, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },   // ICH10R UHCI #1
 	{0x8086, 0x3a65, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },   // ICH10R UHCI #2
 	{0x8086, 0x3a66, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },   // ICH10R UHCI #3
 	{0x8086, 0x3a67, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },   // ICH10R UHCI #4
 	{0x8086, 0x3a68, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },   // ICH10R UHCI #5
 	{0x8086, 0x3a69, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataUHCISupportsOvercurrent | kErrataNeedsOvercurrentDebounce | kErrataSupportsPortResumeEnable },   // ICH10R UHCI #6
 	{0x8086, 0x3a6a, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataNeedsOvercurrentDebounce },			// ICH10R EHCI #1
 	{0x8086, 0x3a6c, 0x00, 0xff, kErrataICH6PowerSequencing | kErrataNeedsOvercurrentDebounce },				// ICH10R EHCI #2

 	{0x10de, 0x03f2, 0x00, 0xff, kErrataNoCSonSplitIsoch | kErrataMissingPortChangeInt | kErrataMCP79IgnoreDisconnect | kErrataUse32bitEHCI },			// MCP EHCI #1
 	{0x10de, 0x036d, 0x00, 0xff, kErrataNoCSonSplitIsoch | kErrataMissingPortChangeInt | kErrataMCP79IgnoreDisconnect | kErrataUse32bitEHCI },			// MCP EHCI #1
 	{0x10de, 0x00e8, 0x00, 0xff, kErrataNoCSonSplitIsoch | kErrataMissingPortChangeInt | kErrataMCP79IgnoreDisconnect | kErrataUse32bitEHCI},			// MCP EHCI #2
 	{0x10de, 0x077c, 0x00, 0xff, kErrataNoCSonSplitIsoch | kErrataMissingPortChangeInt | kErrataMCP79IgnoreDisconnect | kErrataUse32bitEHCI},			// MCP78S EHCI #2
 	{0x10de, 0x055f, 0x00, 0xff, kErrataNoCSonSplitIsoch | kErrataMissingPortChangeInt | kErrataMCP79IgnoreDisconnect | kErrataUse32bitEHCI},			// MCP Ar4er EHCI #2
 	{0x10de, 0x005b, 0x00, 0xff, kErrataNoCSonSplitIsoch | kErrataMissingPortChangeInt | kErrataMCP79IgnoreDisconnect | kErrataUse32bitEHCI},			// MCP EHCI #2
 	{0x10de, 0x026e, 0x00, 0xff, kErrataNoCSonSplitIsoch | kErrataMissingPortChangeInt | kErrataMCP79IgnoreDisconnect | kErrataUse32bitEHCI},			// ASUS Z53T EHCI #2
 	{0x10de, 0x055e, 0x00, 0xff, kErrataOHCINoGlobalSuspendOnSleep | kErrataMCP79IgnoreDisconnect },														// MCP Ar4er OHCI #1
 	{0x10de, 0x03f1, 0x00, 0xff, kErrataOHCINoGlobalSuspendOnSleep | kErrataMCP79IgnoreDisconnect },														// MCP OHCI #1
 	{0x10de, 0x036c, 0x00, 0xff, kErrataOHCINoGlobalSuspendOnSleep | kErrataMCP79IgnoreDisconnect },														// MCP OHCI #1
 	{0x10de, 0x00e7, 0x00, 0xff, kErrataOHCINoGlobalSuspendOnSleep | kErrataMCP79IgnoreDisconnect },														// MCP OHCI #2
 	{0x10de, 0x077b, 0x00, 0xff, kErrataOHCINoGlobalSuspendOnSleep | kErrataMCP79IgnoreDisconnect },														// MCP78S OHCI #2
 	{0x10de, 0x005a, 0x00, 0xff, kErrataOHCINoGlobalSuspendOnSleep | kErrataMCP79IgnoreDisconnect },														// MCP OHCI #2
 	{0x10de, 0x026d, 0x00, 0xff, kErrataOHCINoGlobalSuspendOnSleep | kErrataMCP79IgnoreDisconnect },

	{0x10de, 0x0d9d, 0x00, 0xff, kErrataIgnoreRootHubPowerClearFeature },			// MCP89 EHCI #1,2
	{0x10de, 0x0d9c, 0x00, 0xff, kErrataIgnoreRootHubPowerClearFeature },			// MCP89 OHCI #1,2

	{0x12d8, 0x400f, 0x00, 0x01, kErrataDisablePCIeLinkOnSleep}			// Pericom
	
};

#define errataListLength (sizeof(errataList)/sizeof(ErrataListEntry))

UInt32 IOUSBController::GetErrataBits(UInt16 vendorID, UInt16 deviceID, UInt16 revisionID)
{
    ErrataListEntry			*entryPtr;
    UInt32					i, errata = 0;
	
    for(i = 0, entryPtr = errataList; i < errataListLength; i++, entryPtr++)
    {
        if (vendorID == entryPtr->vendID &&
            deviceID == entryPtr->deviceID &&
            revisionID >= entryPtr->revisionLo &&
            revisionID <= entryPtr->revisionHi)
        {
            // we match, add this errata to our list
            errata |= entryPtr->errata;
        }
    }
	
	
	//USBError(1, "Errata bits for controller 0x%x/0x%x(rev 0x%x) are 0x%x", vendorID, deviceID, revisionID, errata);

    return errata;
}       


