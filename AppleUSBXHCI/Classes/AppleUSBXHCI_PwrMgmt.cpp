/*
 *  AppleUSBXHCI_PwrMgmt.cpp
 *
 *  Copyright © 2011 Apple Inc. All Rights Reserved.
 *
 */

//================================================================================================
//
//   Headers
//
//================================================================================================
//
#include "../../IOUSBFamily/Headers/IOUSBRootHubDevice.h"
#include "../../IOUSBFamily/Headers/IOUSBHubPolicyMaker.h"

#include "AppleUSBXHCIUIM.h"

//================================================================================================
//
//   Local Definitions
//
//================================================================================================
//
#ifndef XHCI_USE_KPRINTF 
#define XHCI_USE_KPRINTF 0
#endif

#if XHCI_USE_KPRINTF
#undef USBLog
#undef USBError
void kprintf(const char *format, ...) __attribute__((format(printf, 1, 2)));
#define USBLog( LEVEL, FORMAT, ARGS... )  if ((LEVEL) <= XHCI_USE_KPRINTF) { kprintf( FORMAT "\n", ## ARGS ) ; }
#define USBError( LEVEL, FORMAT, ARGS... )  { kprintf( FORMAT "\n", ## ARGS ) ; }
#endif

#if (DEBUG_REGISTER_READS == 1)
#define Read32Reg(registerPtr, ...) Read32RegWithFileInfo(registerPtr, __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#define Read32RegWithFileInfo(registerPtr, function, file, line, ...) (															\
	fTempReg = Read32Reg(registerPtr, ##__VA_ARGS__),																			\
	fTempReg = (fTempReg == (typeof (*(registerPtr))) -1) ?																		\
		(kprintf("AppleUSBXHCI[%p]::%s Invalid register at %s:%d %s\n", this,function,file, line,#registerPtr), -1) : fTempReg,	\
	(typeof(*(registerPtr)))fTempReg)


#define Read64Reg(registerPtr, ...) Read64RegWithFileInfo(registerPtr, __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#define Read64RegWithFileInfo(registerPtr, function, file, line, ...) (															\
	fTempReg = Read64Reg(registerPtr, ##__VA_ARGS__),																			\
	fTempReg = (fTempReg == (typeof (*(registerPtr))) -1) ?																		\
		(kprintf("AppleUSBXHCI[%p]::%s Invalid register at %s:%d %s\n", this,function,file, line,#registerPtr), -1) : fTempReg,	\
	(typeof(*(registerPtr)))fTempReg)
#endif

#define super							IOUSBControllerV3
#define _controllerCanSleep				_expansionData->_controllerCanSleep

#pragma mark •••••••• Power Management ••••••••

//================================================================================================
//
//   CheckSleepCapability
//
//================================================================================================
//
void											
AppleUSBXHCI::CheckSleepCapability(void)
{
	// *****************
	// This stuff used to be done in initForPM. I could not coalesce the two methods, but I need the _controllerCanSleep calculation
	// earlier than that time, so I will do the calculaton here instead of there
	// *****************
	// assume that sleep is OK at first
	USBLog(2, "AppleUSBXHCI[%p]::CheckSleepCapability - assuming that I can sleep", this);
	_controllerCanSleep = true;
	_hasPCIPwrMgmt = false;
	
	//   We need to determine which XHCI controllers don't survive sleep.  These fall into 2 categories:
	//
	//   1.  ExpressCards
	//	 2.  PCI Cards that lose power (right now because of a bug in the PCI Family, USB PCI cards do not prevent
	//	     sleep, so even cards that don't support the PCI Power Mgmt stuff get their power removed.
	//
	//  So here, we look at all those cases and set the _unloadUIMAcrossSleep boolean to true.  As it turns out,
	//  if a controller does not have the "AAPL,clock-id" property, then it means that it cannot survive sleep.  We
	//  might need to refine this later once we figure how to deal with PCI cards that can go into PCI sleep mode.
	//  An exception is the B&W G3, that does not have this property but can sleep.  Sigh...
	
	//  Now, look at PCI cards.  Note that the onboard controller's provider is an IOPCIDevice so we cannot use that
	//  to distinguish between USB PCI cards and the on board controller.  Instead, we use the existence of the
	//  "AAPL,clock-id" property in the provider.  If it does not exist, then we are a XHCI controller on a USB PCI card.
	//
	if ( !_device->getProperty("AAPL,clock-id") )
	{
		// Added check for Thunderbolt attached controllers too here, as them may support D3 but may not be tagged in
		// the IO registry as built-in. If we don't do this then _hasPCIPowerMgmt flag would be set to false and on
		// system sleep it would be considered equal to a restart and we tear down the stack on every sleep.
		if (_device->getProperty("built-in") || (_device->getProperty(kIOPCITunnelledKey) == kOSBooleanTrue))
		{			
			// rdar://5769508 - if we are on a built in PCI device, then assume the system supports D3cold
			if (_device->hasPCIPowerManagement(kPCIPMCPMESupportFromD3Cold) && (_device->enablePCIPowerManagement(kPCIPMCSPowerStateD3) == kIOReturnSuccess))
			{
				_hasPCIPwrMgmt = true;
				setProperty("Card Type","Built-in");
                setProperty("ResetOnResume", false);
			}
		}
		else
		{
			// rdar://5856545 - on older machines without the built-in property, we need to use the "default" case in the IOPCIDevice code
			if (_device->hasPCIPowerManagement() && (_device->enablePCIPowerManagement() == kIOReturnSuccess))
			{
				_hasPCIPwrMgmt = true;
				setProperty("Card Type","Built-in");
                setProperty("ResetOnResume", false);
			}
		}
		
        /* AnV - Force power management */
        if (_sleepFix)
        {
            USBError(1, "AppleUSBXHCI[%p]::CheckSleepCapability - forced ON",this);
            _hasPCIPwrMgmt = true;
            _controllerCanSleep = true;
            setProperty("Card Type","Built-in");
            setProperty("ResetOnResume", false);
        } else {
            if (!_hasPCIPwrMgmt)
            {
                USBError(1, "AppleUSBXHCI[%p]::CheckSleepCapability - controller will be unloaded across sleep",this);
                _controllerCanSleep = false;
                setProperty("Card Type","PCI");
            }
        }
	}
	else
	{
		// old Apple ASICs come in here
		setProperty("Card Type","Built-in");

        /* AnV - Force power management */
        if (_sleepFix)
        {
            USBError(1, "AppleUSBXHCI[%p]::CheckSleepCapability - forced ON",this);
            _hasPCIPwrMgmt = true;
            _controllerCanSleep = true;
            setProperty("ResetOnResume", false);
        }
	}

    /* AnV - Disable controller sleep */
    if (_noSleepForced)
    {
        USBError(1, "AppleUSBXHCI[%p]::CheckSleepCapability - forced OFF",this);
        _hasPCIPwrMgmt = false;
        _controllerCanSleep = false;
        setProperty("ResetOnResume", true);
    }
}

//================================================================================================
//
//   ResetControllerState 
//
//      Called on system wake from Hibernate
//
//================================================================================================
//
IOReturn				
AppleUSBXHCI::ResetControllerState(void)
{
    IOReturn status = kIOReturnSuccess;
    int      i = 0;

    USBLog(2, "AppleUSBXHCI[%p]::ResetControllerState (num interrupts: %d, num primary: %d, inactive:%d, unavailable:%d, is controller available:%d lost register access:%d)", this, (int)_numInterrupts, (int)_numPrimaryInterrupts, (int)_numInactiveInterrupts, (int)_numUnavailableInterrupts, (int)_controllerAvailable, (int)_lostRegisterAccess);
    USBLog(2, "AppleUSBXHCI[%p]::ResetControllerState _myPowerState: %d _stateSaved %d", this, (uint32_t)_myPowerState, _stateSaved);

    if(_lostRegisterAccess)
    {
        return(kIOReturnNotResponding);
    }
    else
    {
        // Confirm that controller is Halted
        status = StopUSBBus();

        if (_resetControllerFix == true)
        {
            EnableInterruptsFromController(false);
            IOSleep(1U);	// drain primary interrupts
            if (_expansionData &&
                _controllerCanSleep &&
                _device)
            {
                _device->enablePCIPowerManagement(kPCIPMCSPowerStateD0);
            }
        }

        if( status != kIOReturnSuccess )
        {
            USBLog(1, "AppleUSBXHCI[%p]::ResetControllerState  StopUSBBus returned: %x", this, (uint32_t)status);
            return status;
        }
        
        _uimInitialized = false;
    }

	return(kIOReturnSuccess);
}

IOReturn AppleUSBXHCI::InitializePorts(void)
{
	for (uint8_t port = 0U; port < _rootHubNumPorts; ++port) {
		uint32_t portSC = Read32Reg(&_pXHCIRegisters->PortReg[port].PortSC);
		if (portSC & kXHCIPortSC_WRC)
			Write32Reg(&_pXHCIRegisters->PortReg[port].PortSC, (portSC & (kXHCIPortSC_PP | XHCI_PS_PIC_SET(3U) | (kXHCIPortSC_WCE | kXHCIPortSC_WDE | kXHCIPortSC_WOE))) | kXHCIPortSC_WRC);
	}
	return kIOReturnSuccess;
}

//================================================================================================
//
//   RestartControllerFromReset 
//
//      Called on system wake from Hibernate after ResetControllerState
//
//      Called after ::UIMInitialize and if _myPowerState == 0 from super class
//
//================================================================================================
//
IOReturn				
AppleUSBXHCI::RestartControllerFromReset(void)
{
    USBLog(2, "AppleUSBXHCI[%p]::RestartControllerFromReset (num interrupts: %d, num primary: %d, inactive:%d, unavailable:%d, is controller available:%d lost register access:%d)", this, (int)_numInterrupts, (int)_numPrimaryInterrupts, (int)_numInactiveInterrupts, (int)_numUnavailableInterrupts, (int)_controllerAvailable, (int)_lostRegisterAccess);
    
    USBLog(2, "AppleUSBXHCI[%p]::RestartControllerFromReset _myPowerState: %d _stateSaved %d", this, (uint32_t)_myPowerState, _stateSaved);

    // TODO:: Similar to UIMInitialize, we have to collapse both together.
    if (!_uimInitialized)
    {
        IOReturn status = ResetController();

        if( status != kIOReturnSuccess )
        {
            USBLog(2, "AppleUSBXHCI[%p]::RestartControllerFromReset ResetController returned 0x%x", this, status);
            return status;
        }
        		
        UInt8    portIndex = 0, deviceIndex = 0;

		// If the WRC is set, clear it
		for ( portIndex=1; portIndex<=_rootHubNumPorts; portIndex++ )
		{
			UInt32		portSC = Read32Reg(&_pXHCIRegisters->PortReg[portIndex-1].PortSC);
			
			if (_lostRegisterAccess)
			{
				return kIOReturnNoDevice;
			}
			
			if (portSC & kXHCIPortSC_WRC)
			{
				portSC |= (UInt32) kXHCIPortSC_WRC;
				Write32Reg(&_pXHCIRegisters->PortReg[portIndex-1].PortSC, portSC);
				IOSync();
			}
			
			if ((portSC & kXHCIPortSC_CCS) && !(portSC & kXHCIPortSC_CSC))
			{
				// Intel Errata (rdar://10403564):  After a HRST, if we have a connection but no connection status change, then we need to fake it
				USBLog(1, "AppleUSBXHCI[%p]::UIMInitialize - PortReg[%d].PortSC: 0x%08x, has a CCS but no CSC", this, portIndex-1, (uint32_t)portSC);
				_synthesizeCSC[portIndex-1] = true;
			}
		}

        TakeOwnershipFromBios();
        EnableXHCIPorts();
        DisableComplianceMode();
        InitializePorts();

        _stateSaved = false;
        
        //
        // Deallocate all rings and reset all slots
        //
        UInt16 slot, endp;
        XHCIRing *ring = NULL;

        for( slot = 0; slot < _numDeviceSlots; slot++ )
        {
            if( _slots[slot].buffer != NULL )
            {
                for( endp = 1; endp < kXHCI_Num_Contexts; endp++ )
                {
                    // All other endpoints
                    ring = GetRing(slot, endp, 0);
                    
                    if(IsStreamsEndpoint(slot, endp))
                    {
                        if( _slots[slot].maxStream[endp] != 0 )
                        {
                            for( UInt16 streamsID = 1; streamsID <= _slots[slot].maxStream[endp]; streamsID++ )
                            {
                                XHCIRing *streamsRing = GetRing(slot, endp, streamsID);
                                
                                if(streamsRing != NULL)
                                {
                                    // DeallocRing - frees all IOUSBCommands
                                    DeallocRing(streamsRing);
                                }
                            }
                        }
                    }

                    if( ring != NULL )
                    {
                        // TODO:: Do we have any outstanding IOUSBCommands
                        
                        // DeallocRing - frees the array for holding IOUSBCommands
                        DeallocRing(ring);
                        IOFree(ring, sizeof(XHCIRing)* (_slots[slot].maxStream[endp]+1));
                        _slots[slot].potentialStreams[endp] = 0;
                        _slots[slot].maxStream[endp] = 0;
                        _slots[slot].rings[endp] = NULL;
                    }                        
                    
                }
                
                // Do this first to mark its deleted.
                _slots[slot].deviceContext = NULL;    
                _slots[slot].deviceContext64 = NULL;
				
                // Relase the output context
                _slots[slot].buffer->complete();
                _slots[slot].buffer->release();
                _slots[slot].buffer = 0;
                _slots[slot].deviceContextPhys = 0;
            }
        } // end of deallocate slot and endpoint 
        
        
        // Initialise some state variables
		for ( deviceIndex = 0; deviceIndex < kMaxDevices; deviceIndex++ )
		{
			_devHub[deviceIndex] = 0;
			_devPort[deviceIndex] = 0;
			_devMapping[deviceIndex] = 0;
			_devEnabled[deviceIndex] = false;
		}
        
		for ( portIndex = 0 ; portIndex < kMaxPorts; portIndex++ )
		{
			_prevSuspend[portIndex] = false;
			_suspendChangeBits[portIndex] = false;
		}
        
		UInt32 configReg = Read32Reg(&_pXHCIRegisters->Config);
		if (_lostRegisterAccess)
		{
			return kIOReturnNoDevice;
		}
        configReg =  (configReg & ~kXHCINumDevsMask) | _numDeviceSlots;
		
		Write32Reg(&_pXHCIRegisters->Config, configReg);
        
        // Turn on all device notifications, they'll be logged in PollEventring
		Write32Reg(&_pXHCIRegisters->DNCtrl, 0xFFFF);
        
        USBLog(3, "AppleUSBXHCI[%p]::RestartControllerFromReset - DCBAA - pPhysical[%p] pLogical[%p]", this, (void*)_DCBAAPhys, _DCBAA);
        
        Write64Reg(&_pXHCIRegisters->DCBAAP, _DCBAAPhys);
        
        USBLog(3, "AppleUSBXHCI[%p]::RestartControllerFromReset - CMD Ring - pPhysical[%p] pLogical[%p], num CMDs: %d", this, (void*)_CMDRingPhys, _CMDRing, _numCMDs);
        
        InitCMDRing();
        
        USBLog(3, "AppleUSBXHCI[%p]::RestartControllerFromReset - _ERSTMax  %d", this, _ERSTMax);

        USBLog(3, "AppleUSBXHCI[%p]::RestartControllerFromReset - Event Ring %d - pPhysical[%p] pLogical[%p], num Events: %d", this, kPrimaryInterrupter, (void*)_events[kPrimaryInterrupter].EventRingPhys, _events[kPrimaryInterrupter].EventRing, _events[kPrimaryInterrupter].numEvents);        
        
        InitEventRing(kPrimaryInterrupter, true);

        if (_useSingleInt == false)
        {
            USBLog(3, "AppleUSBXHCI[%p]::RestartControllerFromReset - Event Ring %d - pPhysical[%p] pLogical[%p], num Events: %d", this, kTransferInterrupter, (void*)_events[kTransferInterrupter].EventRingPhys, _events[kTransferInterrupter].EventRing, _events[kTransferInterrupter].numEvents);
        
            InitEventRing(kTransferInterrupter, true);
        }
        
        if(_numScratchpadBufs != 0)
        {
            _DCBAA[0] = _SBAPhys;
        }
        
		_CCEPhysZero = 0;
		_CCEBadIndex = 0;
		_EventChanged = 0;
		_IsocProblem = 0;

		_stateSaved = false;
		_fakedSetaddress = false;
		
		_filterInterruptActive = false;
		_frameNumber64 = 0;
		_numInterrupts = 0;
		_numPrimaryInterrupts = 0;
		_numInactiveInterrupts = 0;
		_numUnavailableInterrupts = 0;
        
        
        _uimInitialized = true;
    }
    
    return(kIOReturnSuccess);
}

//================================================================================================
//
//   SaveAnInterrupter 
//
//      Save an interruptor set (hardware registers)
//
//================================================================================================
//
void AppleUSBXHCI::SaveAnInterrupter(int IRQ)
{
 	_savedInterrupter[IRQ].ERSTSZ = Read32Reg(&_pXHCIRuntimeReg->IR[IRQ].ERSTSZ);
	_savedInterrupter[IRQ].ERSTBA = Read64Reg(&_pXHCIRuntimeReg->IR[IRQ].ERSTBA);
	_savedInterrupter[IRQ].ERDP = Read64Reg(&_pXHCIRuntimeReg->IR[IRQ].ERDP);
	_savedInterrupter[IRQ].IMAN = Read32Reg(&_pXHCIRuntimeReg->IR[IRQ].IMAN);
	_savedInterrupter[IRQ].IMOD = Read32Reg(&_pXHCIRuntimeReg->IR[IRQ].IMOD);
   
}

//================================================================================================
//
//   RestoreAnInterruptor 
//
//      Restore an interruptor set (hardware registers)
//
//================================================================================================
//
void AppleUSBXHCI::RestoreAnInterrupter(int IRQ)
{
    Write32Reg(&_pXHCIRuntimeReg->IR[IRQ].ERSTSZ, _savedInterrupter[IRQ].ERSTSZ);
    Write64Reg(&_pXHCIRuntimeReg->IR[IRQ].ERSTBA, _savedInterrupter[IRQ].ERSTBA);
    Write64Reg(&_pXHCIRuntimeReg->IR[IRQ].ERDP, _savedInterrupter[IRQ].ERDP);
    Write32Reg(&_pXHCIRuntimeReg->IR[IRQ].IMAN, _savedInterrupter[IRQ].IMAN);
    Write32Reg(&_pXHCIRuntimeReg->IR[IRQ].IMOD, _savedInterrupter[IRQ].IMOD);
}

//================================================================================================
//
//   SaveControllerStateForSleep 
//
//      Called on the way to system Sleep and Hibernate
//
//================================================================================================
//
IOReturn				
AppleUSBXHCI::SaveControllerStateForSleep(void)
{
    USBLog(2, "AppleUSBXHCI[%p]::SaveControllerStateForSleep  _myPowerState: %d", this, (uint32_t)_myPowerState);
    
    IOReturn status = kIOReturnSuccess;
	UInt32	 CMD, count=0;
        
	// Confirm that controller is Halted
	status = StopUSBBus();
	
	if( status != kIOReturnSuccess )
    {
        USBLog(1, "AppleUSBXHCI[%p]::SaveControllerStateForSleep  StopUSBBus returned: %x", this, (uint32_t)status);
		return status;
	}

	_savedRegisters.USBCMD = Read32Reg(&_pXHCIRegisters->USBCMD);
	_savedRegisters.DNCtrl = Read32Reg(&_pXHCIRegisters->DNCtrl);
	_savedRegisters.DCBAAP = Read64Reg(&_pXHCIRegisters->DCBAAP);
	_savedRegisters.Config = Read32Reg(&_pXHCIRegisters->Config);

	if (_lostRegisterAccess)
	{
		return kIOReturnNoDevice;
	}
    
	SaveAnInterrupter(kPrimaryInterrupter);

    if (_useSingleInt == false)
    {
        SaveAnInterrupter(kTransferInterrupter);
    }
    
    // Section 4.23.2 of XHCI doesn't require us to save/restore the CRCR state.
	//_savedRegisters.CRCR = Read64Reg(&_pXHCIRegisters->CRCR);
	
	CMD = Read32Reg(&_pXHCIRegisters->USBCMD);
	if (_lostRegisterAccess)
	{
		return kIOReturnNoDevice;
	}
	
	CMD |= kXHCICMDCSS;
	Write32Reg(&_pXHCIRegisters->USBCMD, CMD);
	
	count = 0;
	UInt32 usbSts = Read32Reg(&_pXHCIRegisters->USBSTS);
	while((!_lostRegisterAccess) && (usbSts & kXHCISSS))
	{
		IOSleep(1);
		if(count++ >100)
		{
			USBLog(1, "AppleUSBXHCI[%p]::SaveControllerStateForSleep - Controller not saved state after 100ms", this);
			return (kIOReturnInternalError);
		}
		
		usbSts = Read32Reg(&_pXHCIRegisters->USBSTS);
	}
    
    if (_lostRegisterAccess)
    {
        return kIOReturnNoDevice;
    }
	
	
	usbSts = Read32Reg(&_pXHCIRegisters->USBSTS);
	if (_lostRegisterAccess)
	{
		return kIOReturnNoDevice;
	}
	
	if(usbSts & kXHCISRE)
	{
        Write32Reg(&_pXHCIRegisters->USBSTS, kXHCISRE);

        if (_device)
			_device->enablePCIPowerManagement(kPCIPMCSPowerStateD0);
	}
	
	_stateSaved = true;
	USBLog(2, "AppleUSBXHCI[%p]::SaveControllerStateForSleep - state saved", this);
	PrintRuntimeRegs();
    
    return kIOReturnSuccess;
}


uint16_t AppleUSBXHCI::PortNumberCanonicalToProtocol(uint16_t canonical, uint8_t* pProtocol)
{
	if (canonical + 1U >= _v3ExpansionData->_rootHubPortsSSStartRange &&
		canonical + 1U < _v3ExpansionData->_rootHubPortsSSStartRange + _v3ExpansionData->_rootHubNumPortsSS) {
		if (pProtocol)
			*pProtocol = kUSBDeviceSpeedSuper;
		return canonical - _v3ExpansionData->_rootHubPortsSSStartRange + 2U;
	}
	if (canonical + 1U >= _v3ExpansionData->_rootHubPortsHSStartRange &&
		canonical + 1U < _v3ExpansionData->_rootHubPortsHSStartRange + _v3ExpansionData->_rootHubNumPortsHS) {
		if (pProtocol)
			*pProtocol = kUSBDeviceSpeedHigh;
		return canonical - _v3ExpansionData->_rootHubPortsHSStartRange + 2U;
	}
	return 0U;
}

IOUSBHubPolicyMaker* AppleUSBXHCI::GetHubForProtocol(uint8_t protocol)
{
	if (protocol == kUSBDeviceSpeedHigh && _rootHubDevice)
		return _rootHubDevice->GetPolicyMaker();
	if (protocol == kUSBDeviceSpeedSuper && _expansionData && _rootHubDeviceSS)
		return _rootHubDeviceSS->GetPolicyMaker();
	return 0;
}

bool AppleUSBXHCI::RHCheckForPortResume(uint16_t port, uint8_t protocol, uint32_t havePortSC)
{
	AbsoluteTime deadline;
	uint32_t portSC;
    
	if (havePortSC == UINT32_MAX) {
		portSC = Read32Reg(&_pXHCIRegisters->PortReg[port].PortSC);
	} else
		portSC = havePortSC;
	if (_rhPortBeingResumed[port]) {
		/*
		 * Check if port has reached U0 state.
		 */
		if (XHCI_PS_PLS_GET(portSC) == kXHCIPortSC_PLS_U0)
			_rhPortBeingResumed[port] = false;
		return false;
	}
	if (XHCI_PS_PLS_GET(portSC) != kXHCIPortSC_PLS_Resume)
		return false;
	/*
	 * Device-initiated resume
	 */
	_rhPortBeingResumed[port] = true;
	if (protocol == kUSBDeviceSpeedSuper) {
		Write32Reg(&_pXHCIRegisters->PortReg[port].PortSC,
				   (portSC & (kXHCIPortSC_PP | XHCI_PS_PIC_SET(3U) | (kXHCIPortSC_WCE | kXHCIPortSC_WDE | kXHCIPortSC_WOE))) | kXHCIPortSC_LWS | XHCI_PS_PLS_SET(kXHCIPortSC_PLS_U0) | kXHCIPortSC_PLC);
		return true;
	}
	Write32Reg(&_pXHCIRegisters->PortReg[port].PortSC, (portSC & (kXHCIPortSC_PP | XHCI_PS_PIC_SET(3U) | (kXHCIPortSC_WCE | kXHCIPortSC_WDE | kXHCIPortSC_WOE))) | kXHCIPortSC_PLC);
	if (_rhResumePortTimerThread[port]) {
		clock_interval_to_deadline(20U, kMillisecondScale, (AbsoluteTime *)&deadline);
		thread_call_enter1_delayed(_rhResumePortTimerThread[port],
								   reinterpret_cast<thread_call_param_t>(static_cast<size_t>(port)),
								   deadline);
	}
	return true;
}

void AppleUSBXHCI::SantizePortsAfterPowerLoss(void)
{
	for (uint8_t port = 0U; port < _rootHubNumPorts; ++port) {
		uint32_t portSC = GetPortSCForWriting(port);
		Write32Reg(&_pXHCIRegisters->PortReg[port].PortSC, portSC | (kXHCIPortSC_WCE | kXHCIPortSC_WDE | kXHCIPortSC_WOE));
	}
}

/* AnV - Add properties if no DSDT injection */
void AppleUSBXHCI::SetPropsForBookkeeping(void)
{
	/*
	 * Note:
	 *   IOUSBController::CreateRootHubDevice copies these properties
	 *   from _device to the root hub devices, and also to IOResources.
	 *   The copying to IOResources takes place once (globally).
	 *   IOUSBRootHubDevice::RequestExtraWakePower then uses the global
	 *   values on IOResources to do power-accounting for all root hubs
	 *   in the system (!).  So whatever value given here for extra current
	 *   must cover 400mA of extra current for each SS root hub port in
	 *   the system.
	 *   Since there may be more than one xHC, must account for them all.
	 *   Set a high value of 255 to (hopefully) cover everything.
	 *   Additionally, iPhone/iPad ask for 1600mA of extra power on high-speed
	 *   ports, so we allow for that as well.
	 */
	/*
	 * Note: Only set defaults if none were injected via DSDT
	 */
    /* AnV - Fix for Intel Panther Point too... */
	if (!OSDynamicCast(OSNumber, _device->getProperty(kAppleMaxPortCurrent)))
		_device->setProperty(kAppleMaxPortCurrent, 0x0834, 32U);
	if (!OSDynamicCast(OSNumber, _device->getProperty(kAppleCurrentExtra)))
		_device->setProperty(kAppleCurrentExtra, 0x0A8C, 32U);
	if (!OSDynamicCast(OSNumber, _device->getProperty(kAppleMaxPortCurrentInSleep)))
		_device->setProperty(kAppleMaxPortCurrentInSleep, 0x0834, 32U);
	if (!OSDynamicCast(OSNumber, _device->getProperty(kAppleCurrentExtraInSleep)))
		_device->setProperty(kAppleCurrentExtraInSleep, 0x0A8C, 32U);
}

//================================================================================================
//
//   RestoreControllerStateFromSleep 
//
//      Called on system wake from Sleep
//
//================================================================================================
//
IOReturn				
AppleUSBXHCI::RestoreControllerStateFromSleep(void)
{
  int slot, endp;
	UInt32 CMD, STS, count=0;
  UInt32 portIndex;
	volatile UInt32 val = 0;
	volatile UInt32 * addr;

  USBLog(2, "AppleUSBXHCI[%p]::RestoreControllerStateFromSleep _myPowerState: %d _stateSaved %d", this, (uint32_t)_myPowerState, _stateSaved);
	PrintRuntimeRegs();

  UInt32		sts = Read32Reg(&_pXHCIRegisters->USBSTS);
	if (_lostRegisterAccess)
	{
    USBLog(2, "AppleUSBXHCI[%p]::RestoreControllerStateFromSleep : no device", this);
		return kIOReturnNoDevice;
	}


	// At this point, interrupts are disabled, and we are waking up. If the Port Change Detect bit is active
	// then it is likely that we are responsible for the system issuing the wakeup
	if (sts & kXHCIPCD)
	{
		UInt32			port;

		for (port=0; port < _rootHubNumPorts; port++)
		{
			UInt32	portSC = Read32Reg(&_pXHCIRegisters->PortReg[port].PortSC);
			if (_lostRegisterAccess)
			{
				return kIOReturnNoDevice;
			}

			if (portSC & kXHCIPortSC_CSC)
			{
				if (portSC & kXHCIPortSC_PED)
				{
					USBError(1, "USB (XHCI):Port %d on bus 0x%x - connect status changed but still enabled. clearing enable bit: portSC(0x%x)\n", (int)port+1, (uint32_t)_busNumber, (uint32_t)portSC);
					portSC = GetPortSCForWriting(port+1);
					portSC |= (UInt32)kXHCIPortSC_PEC;
					Write32Reg(&_pXHCIRegisters->PortReg[port].PortSC, portSC);
				}
				else
				{
					IOLog("USB (%s):Port %d on bus 0x%x connected or disconnected: portSC(0x%x)\n", _rootHubDevice ? _rootHubDevice->getName() : "XHCI", (int)port+1, (uint32_t)_busNumber, (uint32_t)portSC);
					USBLog(5, "AppleUSBXHCI[%p]::RestoreControllerStateFromSleep  Port %d on bus 0x%x (%s)- connected or disconnected, calling EnsureUsability()", this, (int)port+1, (uint32_t)_busNumber, _rootHubDevice ? _rootHubDevice->getName() : "XHCI");
					EnsureUsability();
				}
			}
			else if ( ((portSC & kXHCIPortSC_LinkState_Mask) >> kXHCIPortSC_LinkState_Shift) == kXHCIPortSC_PLS_Resume)
			{
				if (_resetControllerFix == true)
        {
          uint8_t protocol = kUSBDeviceSpeedHigh;
          IOUSBHubPolicyMaker* pm = 0;
          uint16_t portNum = PortNumberCanonicalToProtocol(port, &protocol);
          if (portNum)
            pm = GetHubForProtocol(protocol);
          /*
           * Note: This message causes HID device that generated PME to
           *   do a full system wakeup.  Without it, the display remains down.
           */
          if (pm)
            pm->message(kIOUSBMessageRootHubWakeEvent, this, reinterpret_cast<void*>(portNum - 1U));

          RHCheckForPortResume(port, protocol, portSC);
        } else {

          USBLog(5, "AppleUSBXHCI[%p]::RestoreControllerStateFromSleep  Port %d on bus 0x%x - has remote wakeup from some device", this, (int)port+1, (uint32_t)_busNumber);

          // Because of how XHCI works, the root hub driver might not be able to detect that there was a remote wakeup
          // on a port if the upper level driver issues a Resume before the root hub interrupt timer runs
          // Let the hub driver know that from here to make sure we get the log

          if (_rootHubDevice && _rootHubDevice->GetPolicyMaker())
          {
            _rootHubDevice->GetPolicyMaker()->message(kIOUSBMessageRootHubWakeEvent, this, (void *)(uintptr_t) port);
          }
          else
          {
            IOLog("\tUSB (XHCI):Port %d on bus 0x%x has remote wakeup from some device\n", (int)port+1, (uint32_t)_busNumber);
          }

          // Clear the PLC bit if set
          portSC	= GetPortSCForWriting(port+1);
          if (_lostRegisterAccess)
          {
            return kIOReturnNoDevice;
          }

          portSC |= (UInt32)kXHCIPortSC_PLC;
          Write32Reg(&_pXHCIRegisters->PortReg[port].PortSC, portSC);
        }
			}
			else if (portSC & kXHCIPortSC_PED)
			{
				USBLog(5, "AppleUSBXHCI[%p]::RestoreControllerStateFromSleep  Port %d on bus 0x%x is enabled but not handling portSC of 0x%x", this, (int)port+1, (uint32_t)_busNumber, (uint32_t)portSC);
			}

			// In EHCI, we do an "else" check here for a port that is enabled but not suspended.
      // However, it seems that in XHCI when we get here we are already in resume, so a
			// check for being in PLS of U3 does not make sense
		}
	}

  // Restore XHCI run time registers
	if(_stateSaved)
	{
    // Step 4
    USBLog(2, "AppleUSBXHCI[%p]::RestoreControllerStateFromSleep - restoring saved state", this);
		Write32Reg(&_pXHCIRegisters->USBCMD, _savedRegisters.USBCMD);
		Write32Reg(&_pXHCIRegisters->DNCtrl, _savedRegisters.DNCtrl);

    // Section 4.23.2 of XHCI doesn't require us to save/restore the CRCR state.
		Write64Reg(&_pXHCIRegisters->DCBAAP, _savedRegisters.DCBAAP);
		Write32Reg(&_pXHCIRegisters->Config, _savedRegisters.Config);

    RestoreAnInterrupter(kPrimaryInterrupter);

    if (_useSingleInt == false)
    {
      RestoreAnInterrupter(kTransferInterrupter);
    }

    // Step 5
		CMD = Read32Reg(&_pXHCIRegisters->USBCMD);
		if (_lostRegisterAccess)
		{
			return kIOReturnNoDevice;
		}

		CMD |= kXHCICMDCRS;
		Write32Reg(&_pXHCIRegisters->USBCMD, CMD);

		count = 0;
		STS = Read32Reg(&_pXHCIRegisters->USBSTS);

		while((!_lostRegisterAccess) && (STS & kXHCIRSS))
		{
			IOSleep(1);
			if(count++ > 500)
			{
				USBLog(1, "AppleUSBXHCI[%p]::RestoreControllerStateFromSleep - Controller state not restored  after 500ms", this);
				break ;
			}
			STS = Read32Reg(&_pXHCIRegisters->USBSTS);
		}

		if (_lostRegisterAccess)
		{
			return kIOReturnNoDevice;
		}

    // Step 6
    InitCMDRing();

		STS = Read32Reg(&_pXHCIRegisters->USBSTS);
		if (_lostRegisterAccess)
		{
			return kIOReturnNoDevice;
		}

		if(STS & kXHCISRE)
		{
      if (_resetControllerFix == true)
      {
        Write32Reg(&_pXHCIRegisters->USBSTS, kXHCISRE);

        _uimInitialized = false;
        RestartControllerFromReset();
        SantizePortsAfterPowerLoss();

        return kIOReturnSuccess;
      } else {

        int i;
        USBLog(1, "AppleUSBXHCI[%p]::RestoreControllerStateFromSleep - Error restoring controller state USBSTS = 0x%x", this, STS);
        Write32Reg(&_pXHCIRuntimeReg->IR[kPrimaryInterrupter].IMAN, 0);	// Disable the interrupt.

        if (_useSingleInt == false)
        {
          Write32Reg(&_pXHCIRuntimeReg->IR[kTransferInterrupter].IMAN, 0);	// Disable the interrupt.
        }

        Write32Reg(&_pXHCIRegisters->USBCMD, 0);  		// this sets r/s to stop
        IOSync();

        STS = Read32Reg(&_pXHCIRegisters->USBSTS);
        for (i=0; (!_lostRegisterAccess) && (i < 500) && !(STS & kXHCIHCHaltedBit); i++)
        {
          IOSleep(1);
          STS = Read32Reg(&_pXHCIRegisters->USBSTS);
        }

        if (_lostRegisterAccess)
        {
          return kIOReturnNoDevice;
        }

        if (i >= 500)
        {
          USBError(1, "AppleUSBXHCI[%p]::RestoreControllerStateFromSleep - could not get chip to halt within 500 ms",  this);
          return(kIOReturnInternalError);
        }
      }
		}

		_stateSaved = false;
	}

	STS = Read32Reg(&_pXHCIRegisters->USBSTS);
	if( (STS & kXHCIHCHaltedBit) || (STS & kXHCIHSEBit) )
	{
    // TODO :: We need to tell the super that controller we could not recover due to HSE error, so we need
    // it to call ResetControllerState
    IOSync();
    IOSleep(50);
	}

  DisableComplianceMode();

  // Restart all endpoints
  for(slot = 0; slot<_numDeviceSlots; slot++)
	{
		if(_slots[slot].buffer != NULL)
		{
			for(endp = 1; endp<kXHCI_Num_Contexts; endp++)
			{
				XHCIRing *ring;
				ring = GetRing(slot, endp, 0);
				if( (ring != NULL) && (ring->TRBBuffer != NULL) )
				{
          if(IsStreamsEndpoint(slot, endp))
					{
            USBLog(5, "AppleUSBXHCI[%p]::RestoreControllerStateFromSleep - restart stream ep=%d", this, endp);
						RestartStreams(slot, endp, 0);
					}
					else
					{
            USBLog(5, "AppleUSBXHCI[%p]::RestoreControllerStateFromSleep - slot=%d doorbell4ep=%d", this, slot, endp);
            StartEndpoint(slot, endp);
					}
				}
			}
		}
	}

  // Deal with any port with CAS (Cold Attach Status) set
	for (portIndex=0; portIndex < _rootHubNumPorts; portIndex++)
	{
    UInt32	portSC;

		portSC = Read32Reg(&_pXHCIRegisters->PortReg[portIndex].PortSC);
		if (_lostRegisterAccess)
		{
			return kIOReturnNoDevice;
		}

		if (portSC & kXHCIPortSC_CAS)
		{
			USBLog(5, "AppleUSBXHCI[%p]::RestoreControllerStateFromSleep  Port %d on bus 0x%x has CAS bit set (0x%08x), issuing a Warm Reset", this, (int)portIndex+1, (uint32_t)_busNumber, (uint32_t)portSC);

			portSC = GetPortSCForWriting(portIndex+1);
			if (_lostRegisterAccess)
			{
				return kIOReturnNoDevice;
			}

			portSC |= (UInt32)kXHCIPortSC_WPR;
			Write32Reg(&_pXHCIRegisters->PortReg[portIndex].PortSC, portSC);
			IOSync();
			IOSleep(50);
		}
	}

  return kIOReturnSuccess;
}

//================================================================================================
//
//   DozeController 
//
//      
//
//================================================================================================
//
enum  {
	SWAXHCIReg		=	0x40,						// offset in config space
	SWAXHCIValue	=	0x800						// value (bit 11) to mask in/out
};



IOReturn				
AppleUSBXHCI::DozeController(void)
{
	
	// Previous Host Controllers would halt the controller when going to doze
	// That is not allowed with XHCI controllers.
    USBLog(2, "AppleUSBXHCI[%p]::DozeController _myPowerState: %d, _externalDeviceCount %d", this, (uint32_t)_myPowerState, (int)_v3ExpansionData->_externalDeviceCount);
	
	if (_resetControllerFix == true)
    {
        if (!_v3ExpansionData->_externalDeviceCount && (_errataBits & kXHCIErrataPPT))
        {
            uint16_t xhcc = _device->configRead16(0x40U);
            if (xhcc == UINT16_MAX)
            {
                return kIOReturnSuccess;
            }

            xhcc &= ~PCI_XHCI_INTEL_XHCC_SWAXHCIP_SET(3U);
            /*
             * Set SWAXHCI to clear if 1) software, 2) MMIO, 3) xHC exits idle
             */
            xhcc |= PCI_XHCI_INTEL_XHCC_SWAXHCIP_SET(0U);
            xhcc |= PCI_XHCI_INTEL_XHCC_SWAXHCI;
            _device->configWrite16(0x40U, xhcc);
        }
    }


    return kIOReturnSuccess;
}



//================================================================================================
//
//   WakeControllerFromDoze 
//
//      
//
//================================================================================================
//
IOReturn				
AppleUSBXHCI::WakeControllerFromDoze(void)
{
	int				i;
	UInt32			port;
	bool			somePortNeedsToResume = false;
	
	// Previous Host Controllers would halt the controller when going to doze
	// That is not allowed with XHCI controllers.
    USBLog(2, "AppleUSBXHCI[%p]::WakeControllerFromDoze _myPowerState: %d", this, (uint32_t)_myPowerState);

    if (_resetControllerFix == true)
    {
        if (!_v3ExpansionData->_externalDeviceCount && (_errataBits & kXHCIErrataPPT))
        {
            uint16_t xhcc = _device->configRead16(0x40U);
            /*
             * Clear SWAXHCI if it's still on
             */
            if (xhcc != UINT16_MAX && (xhcc & PCI_XHCI_INTEL_XHCC_SWAXHCI))
                _device->configWrite16(0x40U, xhcc & ~PCI_XHCI_INTEL_XHCC_SWAXHCI);
        }
    } else {
        
	// check to see if we have a pending resume on any port and if so, wait for 20ms
	for (port = 0; port < _rootHubNumPorts; port++)
	{
		UInt32 PLS, portSC;
		portSC = Read32Reg(&_pXHCIRegisters->PortReg[port].PortSC);
		if (_lostRegisterAccess)
		{
			return kIOReturnNoDevice;
		}
		
		PLS = (UInt32)(portSC & kXHCIPortSC_LinkState_Mask) >> kXHCIPortSC_LinkState_Shift;
		if (PLS == kXHCIPortSC_PLS_Resume)
		{
			USBLog(5, "AppleUSBXHCI[%p]::WakeControllerFromDoze - port %d appears to be resuming from a remote wakeup", this, (int)port+1);
			_rhPortBeingResumed[port] = true;
			somePortNeedsToResume = true;
		}
	}
	
	if ( somePortNeedsToResume )
	{
		// Now, wait the 20ms for the resume and then call RHResumeAllPorts to finish
		IOSleep(20);
		
		RHCompleteResumeOnAllPorts();
	}
    }
	
	return kIOReturnSuccess;
}


IOReturn				
AppleUSBXHCI::EnableInterruptsFromController(bool enable)
{
    UInt32      CMD = Read32Reg(&_pXHCIRegisters->USBCMD);

    USBLog(2, "AppleUSBXHCI[%p]::EnableInterruptsFromController (num interrupts: %d, num primary: %d, inactive:%d, unavailable:%d, is controller available:%d lost register access:%d)", this, (int)_numInterrupts, (int)_numPrimaryInterrupts, (int)_numInactiveInterrupts, (int)_numUnavailableInterrupts, (int)_controllerAvailable, (int)_lostRegisterAccess);
	
	if (_lostRegisterAccess)
	{
		return kIOReturnNoDevice;
	}

	if (enable)
	{
		USBLog(2, "AppleUSBXHCI[%p]::EnableInterruptsFromController - enabling interrupts, USBCMD(%p) INTE(%s)", this, (void*)(UInt64)CMD,
               (CMD & kXHCICMDINTE) ? "true":"false" );
        
		RestartUSBBus();
	}
	else
	{
		CMD &= ~(kXHCICMDINTE | kXHCICMDEWE);  //it was CMD &= ~kXHCICMDINTE; 
		Write32Reg(&_pXHCIRegisters->USBCMD, CMD);
		IOSync();
		USBLog(2, "AppleUSBXHCI[%p]::EnableInterruptsFromController - interrupts disabled, USBCMD(%p) INTE(%s)", this, (void*)(UInt64)CMD,
               (CMD & kXHCICMDINTE | kXHCICMDEWE) ? "true":"false" );
	}
	
	return kIOReturnSuccess;
}

IOReturn
AppleUSBXHCI::QuiesceAllEndpoints ( )
{   
    IOReturn ret = kIOPMAckImplied;
    int slot, endp;
    USBLog(5, "AppleUSBXHCI[%p]::QuiesceAllEndpoints", this);

    if (_resetControllerFix == true)
    {
        for(slot = 0; slot<_numDeviceSlots; slot++)
        {
            if(_slots[slot].buffer != NULL)
            {
                for(endp = 1; endp<kXHCI_Num_Contexts; endp++)
                {
                    XHCIRing *ring;
                    ring = GetRing(slot, endp, 0);
                    if( (ring != NULL) && (ring->TRBBuffer != NULL) )
                    {
                        USBLog(1, "AppleUSBXHCI[%p]::QuiesceAllEndpoints calling QuiesceEndpoint = %d, %d ", this, slot, endp);
                        QuiesceEndpoint(slot, endp);
                    }
                }
            }
        }

        return kIOReturnSuccess;
    } else {

    // radar://10439459 - Clear CSC bit if not set, only for PPT
    if( (_errataBits & kXHCIErrataPPT) != 0 )
    {
        int portIndex = 0;
        
        for (portIndex=0; portIndex < _rootHubNumPorts; portIndex++ )
        {
            UInt32		portSC = Read32Reg(&_pXHCIRegisters->PortReg[portIndex].PortSC);
			if (_lostRegisterAccess)
			{
				return kIOReturnNoDevice;
			}
            
            // If the CSC is not set on the way to sleep, clear it
            if ( !(portSC & kXHCIPortSC_CSC) )
            {
                portSC = GetPortSCForWriting(portIndex+1);
				if (_lostRegisterAccess)
				{
					return kIOReturnNoDevice;
				}
				
                portSC |= (UInt32) kXHCIPortSC_CSC;
                Write32Reg(&_pXHCIRegisters->PortReg[portIndex].PortSC, portSC);
                IOSync();
            }
        }
    }
    
    // Stop all endpoints
    for(slot = 0; slot<_numDeviceSlots; slot++)
    {
        if(_slots[slot].buffer != NULL)
        {
            for(endp = 1; endp<kXHCI_Num_Contexts; endp++)
            {
                XHCIRing *ring;
                ring = GetRing(slot, endp, 0);
                if( (ring != NULL) && (ring->TRBBuffer != NULL) )
                {
                    USBLog(1, "AppleUSBXHCI[%p]::QuiesceAllEndpoints calling QuiesceEndpoint = %d, %d ", this, slot, endp);
                    QuiesceEndpoint(slot, endp);
                }
            }
        }
    }
    
    int commandRingRunning = (int)Read64Reg(&_pXHCIRegisters->CRCR);
	if (_lostRegisterAccess)
	{
		return kIOReturnNoDevice;
	}
    
    if( commandRingRunning & kXHCI_CRR )
    {
        
        int count = 0;
        // Stop the command ring
        Write64Reg(&_pXHCIRegisters->CRCR, (_CMDRingPhys & ~kXHCICRCRFlags_Mask) | kXHCI_CS);
        
        _waitForCommandRingStoppedEvent = true;
        while (_waitForCommandRingStoppedEvent)
        {
            USBLog(1, "AppleUSBXHCI[%p]::QuiesceAllEndpoints waiting for command ring stop, count = %d", this, count);
            PollForCMDCompletions(kPrimaryInterrupter);
			if (_lostRegisterAccess)
			{
				return kIOReturnNoDevice;
			}
			
            count++;
            if ( count > 100 )
                break;
            IOSleep(1);
        }
    }
    }

    return (ret);
}

IOReturn AppleUSBXHCI::CommandStop(void)
{
	int lowCRCr = Read64Reg(&_pXHCIRegisters->CRCR);
	if (!(lowCRCr & kXHCI_CRR))
		return kIOReturnSuccess;

	stopPending = true;

	Write64Reg(&_pXHCIRegisters->CRCR, kXHCI_CS);
	for (int32_t count = 0; stopPending && count < 100; ++count) {
		if (count)
			IOSleep(1U);
		PollForCMDCompletions(0);
	}
	if (stopPending) {
		stopPending = false;
		IOLog("%s: Timeout waiting for command ring to stop, 100ms\n", __FUNCTION__);
		return kIOReturnTimeout;
	}
	return kIOReturnSuccess;
}

void
AppleUSBXHCI::ControllerSleep ( void )
{
    USBLog(5, "AppleUSBXHCI[%p]::ControllerSleep", this);
    if(_myPowerState == kUSBPowerStateLowPower)
        WakeControllerFromDoze();
    
    QuiesceAllEndpoints();

    if (_resetControllerFix == true)
    {
        CompleteSuspendOnAllPorts();
        CommandStop();
    }

    EnableInterruptsFromController(false);
    IOSleep(1U);	// drain primary interrupts

    SaveControllerStateForSleep();
}


/* AnV - add suspend code */
IOReturn
AppleUSBXHCI::CompleteSuspendOnAllPorts(void)
{
    uint32_t wait, portSC, changePortSC;
    wait = 0U;
    
	for (uint8_t port = 0U; port < _rootHubNumPorts; ++port) {
		changePortSC = 0U;
        portSC = Read32Reg(&_pXHCIRegisters->PortReg[port].PortSC);
        
        if ((_errataBits & kXHCIErrataPPT) &&
			!(portSC & kXHCIPortSC_CSC))
			changePortSC = kXHCIPortSC_CSC;
        
        if ((portSC & kXHCIPortSC_PED) &&
			(XHCI_PS_PLS_GET(portSC) < kXHCIPortSC_PLS_U3)) {
			changePortSC |= kXHCIPortSC_LWS | XHCI_PS_PLS_SET(kXHCIPortSC_PLS_U3);
			wait = 15U;
		}

        if (changePortSC)
            Write32Reg(&_pXHCIRegisters->PortReg[port].PortSC, (portSC & (kXHCIPortSC_PP | kXHCIPortSC_PEC | kXHCIPortSC_WRC | kXHCIPortSC_OCC | kXHCIPortSC_PRC | kXHCIPortSC_PLC | kXHCIPortSC_CEC)) | changePortSC);
    }

    if (wait)
		IOSleep(wait);

    return kIOReturnSuccess;
}

//================================================================================================
//
//   powerChangeDone
//
//================================================================================================
//
void
AppleUSBXHCI::powerChangeDone ( unsigned long fromState)
{
	unsigned long newState = getPowerState();
	
	USBLog((fromState == newState) || !_controllerAvailable ? 7 : 4, "AppleUSBXHCI[%p]::powerChangeDone from state (%d) to state (%d) _controllerAvailable(%s)", this, (int)fromState, (int)newState, _controllerAvailable ? "true" : "false");
    
	if (_wakingFromHibernation)
	{
        USBLog(5, "AppleUSBXHCI[%p]::powerChangeDone - _wakingFromHibernation is set", this);
	}
    
	super::powerChangeDone(fromState);
}
