/*
 * Copyright © 1998-2012 Apple Inc.  All rights reserved.
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

#define _NUM_VERSION_ 1

#import "USBLoggerController.h"

@implementation LoggerEntry
#define NUM_FRESH_ENTRIES 20000
static NSMutableArray * freshEntries = nil;
static int remainingFreshEntries = 0;

+ (void)initialize {
    freshEntries = [[NSMutableArray alloc] initWithCapacity:NUM_FRESH_ENTRIES];
    [self replenishFreshEntries];
}

+ (void)replenishFreshEntries {
    LoggerEntry *temp;
    int i;
    
    [freshEntries removeAllObjects];
    
    for (i=0; i<NUM_FRESH_ENTRIES; i++) {
        temp = [[LoggerEntry alloc] init];
        [freshEntries addObject:temp];
        [temp release];
    }
    
    remainingFreshEntries = NUM_FRESH_ENTRIES;
}

+ (LoggerEntry *)cachedFreshEntry {
    if (remainingFreshEntries <= 0) {
        [self replenishFreshEntries];
    }
    remainingFreshEntries--;
    return [freshEntries objectAtIndex: remainingFreshEntries];
}

- init {
    return [self initWithText:nil level:-1];
}

- initWithText:(NSString *)text level:(int)level {
    if (self = [super init]) {
        _text = [text retain];
        _level = level;
    }
    return self;
}

- (void)setText:(NSString *)text level:(int)level {
    [_text release];
    _text = [text retain];
    _level = level;
}

- (NSString *)text {
    return _text;
}

- (int)level {
    return _level;
}

@end

@implementation USBLoggerController

- init {
    if (self = [super init]) {
        _outputLines = [[NSMutableArray alloc] init];
        _currentFilterString = nil;
        _outputBuffer = [[NSMutableString alloc] init];
        _bufferLock = [[NSLock alloc] init];
        _outputLock = [[NSLock alloc] init];;
    }
    return self;
}

- (void)dealloc {
    if (_logger != nil) {
        [_logger invalidate];
        [_logger release];
    }
    [_outputLines release];
    [_currentFilterString release];
    [_outputBuffer release];
    [_bufferLock release];
    [_outputLock release];
    [super dealloc];
}

- (void)awakeFromNib {
    [LoggerOutputTV setFont:[NSFont fontWithName:@"Monaco" size:10]];
    [FilterProgressIndicator setUsesThreadedAnimation:YES];
    
    if ([[[NSUserDefaults standardUserDefaults] objectForKey:@"USBLoggerLoggingLevel"] intValue] != 0) {
        [LoggingLevelPopUp selectItemAtIndex:[[[NSUserDefaults standardUserDefaults] objectForKey:@"USBLoggerLoggingLevel"] intValue]-1];
    }
    
    _klogKextisPresent = [self isKlogKextPresent];
    _klogKextIsCorrectRevision = [self isKlogCorrectRevision];
    
    _refreshTimer = [[NSTimer scheduledTimerWithTimeInterval: (NSTimeInterval) LOGGER_REFRESH_INTERVAL
                                                      target:                         self
                                                    selector:                       @selector(handlePendingOutput:)
                                                    userInfo:                       nil
                                                     repeats:                        YES] retain];

    [self setupRecentSearchesMenu];
}

- (void)setupRecentSearchesMenu {
    // we can only do this if we're running on 10.3 or later (where FilterTextField is an NSSearchField instance)
    if ([FilterTextField respondsToSelector: @selector(setRecentSearches:)]) {
        NSMenu *cellMenu = [[NSMenu alloc] initWithTitle:@"Search Menu"];
        NSMenuItem *recentsTitleItem, *norecentsTitleItem, *recentsItem, *separatorItem, *clearItem;
        id searchCell = [FilterTextField cell];

        [FilterTextField setRecentsAutosaveName:@"logger_output_filter"];
        [searchCell setMaximumRecents:10];

        recentsTitleItem = [[NSMenuItem alloc] initWithTitle:@"Recent Searches" action: nil keyEquivalent:@""];
        [recentsTitleItem setTag:NSSearchFieldRecentsTitleMenuItemTag];
        [cellMenu insertItem:recentsTitleItem atIndex:0];
        [recentsTitleItem release];
        norecentsTitleItem = [[NSMenuItem alloc] initWithTitle:@"No recent searches" action: nil keyEquivalent:@""];
        [norecentsTitleItem setTag:NSSearchFieldNoRecentsMenuItemTag];
        [cellMenu insertItem:norecentsTitleItem atIndex:1];
        [norecentsTitleItem release];
        recentsItem = [[NSMenuItem alloc] initWithTitle:@"Recents" action: nil keyEquivalent:@""];
        [recentsItem setTag:NSSearchFieldRecentsMenuItemTag];
        [cellMenu insertItem:recentsItem atIndex:2];
        [recentsItem release];
        separatorItem = (NSMenuItem *)[NSMenuItem separatorItem];
        [separatorItem setTag:NSSearchFieldRecentsTitleMenuItemTag];
        [cellMenu insertItem:separatorItem atIndex:3];
        clearItem = [[NSMenuItem alloc] initWithTitle:@"Clear" action: nil keyEquivalent:@""];
        [clearItem setTag:NSSearchFieldClearRecentsMenuItemTag];
        [cellMenu insertItem:clearItem atIndex:4];
        [clearItem release];
        [searchCell setSearchMenuTemplate:cellMenu];
        [cellMenu release];
    }
}

- (IBAction)ChangeLoggingLevel:(id)sender
{
    if (_logger != nil) {
        [_logger setDebuggerOptions:-1 setLevel:true level:[[sender selectedItem] tag] setType:false type:0];
    }
    [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithInt:[[sender selectedItem] tag]] forKey:@"USBLoggerLoggingLevel"];
}

- (IBAction)ClearOutput:(id)sender
{
    [_outputLock lock];
    [_outputLines removeAllObjects];
    [LoggerOutputTV setString:@""];
    [_outputLock unlock];
}

#define DATEUNITS (NSCalendarUnitMonth | NSCalendarUnitDay | NSCalendarUnitHour | NSCalendarUnitMinute | NSCalendarUnitSecond)

- (NSString *)getMonthName:(NSInteger)monthNumber
{
    switch (monthNumber)
    {
        case 1:
            return @"Januari";

        case 2:
            return @"Februari";

        case 3:
            return @"March";

        case 4:
            return @"April";

        case 5:
            return @"May";

        case 6:
            return @"June";

        case 7:
            return @"July";

        case 8:
            return @"August";

        case 9:
            return @"September";

        case 10:
            return @"October";

        case 11:
            return @"November";

        case 12:
            return @"December";
    }

    // Should not happend...
    return @"";
}

- (IBAction)MarkOutput:(id)sender
{
    NSDate *currentDate;
    NSDateComponents *dateComponents;
    NSCalendar *calendar;
    NSString *dateString;

    currentDate = [NSDate date];
    calendar = [NSCalendar autoupdatingCurrentCalendar];

    dateComponents = [calendar components:DATEUNITS fromDate:currentDate];

    dateString = [NSString stringWithFormat:@"\n\t\t**** %@ %d %d:%d:%d ****\n\n",
                    [self getMonthName:[dateComponents month]],
                    (int)[dateComponents day],
                    (int)[dateComponents hour],
                    (int)[dateComponents minute],
                    (int)[dateComponents second]];

    [self appendOutput:dateString atLevel:[NSNumber numberWithInt:0]];
}

/*- (IBAction)SaveOutput:(id)sender
{
    NSSavePanel *sp = [NSSavePanel savePanel];
    int result;
    
    [sp setRequiredFileType:@"txt"];
    result = [sp runModalForDirectory:NSHomeDirectory() file:@"USB Log"];
    if (result == NSOKButton) {
        NSString *finalString;
        
        [_outputLock lock];
        
        finalString = [LoggerOutputTV string];
        
        if (![finalString writeToFile:[sp filename] atomically:YES encoding:NSUTF8StringEncoding error:NULL])
            NSBeep();
        
        [_outputLock unlock];
    }
}*/

- (IBAction)SaveOutput:(id)sender
{
    NSSavePanel *sp = [NSSavePanel savePanel];

    [sp setAllowedFileTypes:[NSArray arrayWithObjects:@"txt", nil]];
    [sp setDirectoryURL:[NSURL URLWithString:NSHomeDirectory()]];
    [sp setNameFieldStringValue:@"USB Log"];
    [sp setExtensionHidden:NO];

    [sp beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger returnCode)
    {
        if (returnCode == NSModalResponseOK)
        {
            NSString *finalString;
            
            [_outputLock lock];
            
            finalString = [LoggerOutputTV string];
                
            if (![finalString writeToURL:[sp URL] atomically:YES encoding:NSUTF8StringEncoding error:NULL])
            {
                NSBeep();
            }

            [_outputLock unlock];
        }
    }];
}

/* Run alert panel replacement */
-(int)runAlertPanel:(NSString *)title message:(NSString *)message firstButton:(NSString *)first secondButton:(NSString *)second thirdButton:(NSString *)third
{
    NSAlert *alert = [[NSAlert alloc] init];

    int result = 0;

    if (message != nil)
    {
        [alert setInformativeText:message];
    }

    if (title != nil)
    {
        [alert setMessageText:title];
    }

    if (first != nil)
    {
        [alert addButtonWithTitle:first];
    }

    if (second != nil)
    {
        [alert addButtonWithTitle:second];
    }

    if (third != nil)
    {
        [alert addButtonWithTitle:third];
    }

    result = [alert runModal];

    [alert release];

    return result;
}

- (IBAction)Start:(id)sender
{
    NSButton *dmpCheckBox = DumpCheckBox;

    if (!_klogKextisPresent) {
        int result = [self runAlertPanel:@"Missing Kernel Extension" message:@"The required kernel extension \"KLog.kext\" is not installed. Would you like to install it now?" firstButton:@"Install" secondButton:@"Cancel" thirdButton:nil];

        if (result == NSAlertFirstButtonReturn)
        {
            //try to install
            if ([self installKLogKext] != YES)
            {
                // error occured while installing, so return
                return;
            } else {
				_klogKextisPresent = YES;
				_klogKextIsCorrectRevision = YES;
			}
        } else {
            // user does not want to install KLog.kext, so return
            return;
        }
    } else if (!_klogKextIsCorrectRevision) {
        int result = [self runAlertPanel:@"Wrong revision for Kernel Extension" message:@"The required kernel extension \"KLog.kext\" is not the right revision. Would you like to upgrade it now?" firstButton:@"Upgrade" secondButton:@"Cancel" thirdButton:nil];

        if (result == NSAlertFirstButtonReturn)
        {
            //try to install
            if ([self removeAndinstallKLogKext] != YES)
            {
                // error occured while installing, so return
                return;
            } else  {
                [self runAlertPanel:@"Need to Restart" message:@"The required kernel extension \"KLog.kext\" was installed.  Please quit and restart." firstButton:@"OK" secondButton:nil thirdButton:nil];

                _klogKextIsCorrectRevision = NO;

				return;
			}
        } else {
            // user does not want to install KLog.kext, so return
            return;
		}
	}
    
    if ([dmpCheckBox state] == NSOnState)
    {
        NSSavePanel *sp;
        NSDate *currentDate;
        NSDateComponents *dateComponents;
        NSCalendar *calendar;
        NSString *dateString;
        
        currentDate = [NSDate date];
        calendar = [NSCalendar autoupdatingCurrentCalendar];
        
        dateComponents = [calendar components:DATEUNITS fromDate:currentDate];
        
        dateString = [NSString stringWithFormat:@"%@ %d %d:%d:%d",
                      [self getMonthName:[dateComponents month]],
                      (int)[dateComponents day],
                      (int)[dateComponents hour],
                      (int)[dateComponents minute],
                      (int)[dateComponents second]];
        
        sp = [NSSavePanel savePanel];
        [sp setAllowedFileTypes:[NSArray arrayWithObjects:@"txt", nil]];
        
        [sp setDirectoryURL:[NSURL URLWithString:NSHomeDirectory()]];
        [sp setNameFieldStringValue:@"USB Log"];
        [sp setExtensionHidden:NO];
        [sp beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger returnCode){
            if (returnCode == NSModalResponseOK)
            {
                NSString *theFileName;
                theFileName = [[sp URL] path];
        
                _dumpingFile = fopen ([theFileName cStringUsingEncoding:NSUTF8StringEncoding],"w");
                if (_dumpingFile == NULL)
                {
                    [self appendOutput:[NSString stringWithFormat:@"%@: Error - unable to open the file %@\n\n",currentDate,theFileName] atLevel:[NSNumber numberWithInt:0]];
                } else {
                    [self appendOutput:[NSString stringWithFormat:@"%@: Saving output to file %@\n\n", dateString,theFileName] atLevel:[NSNumber numberWithInt:0]];
                }
                
                [self actuallyStartLogging];
            }
        }];
    } else {
        [self actuallyStartLogging];
    }
}

- (void) actuallyStartLogging
{
    if (_logger == nil) {
        _logger = [[USBLogger alloc] initWithListener:self level:[[LoggingLevelPopUp selectedItem] tag]];
        
    }
    [_logger beginLogging];
    
    [DumpCheckBox setEnabled:NO];
    [StartStopButton setAction:@selector(Stop:)];
    [StartStopButton setTitle:@"Stop"];
}

- (IBAction)Stop:(id)sender
{
    if (_dumpingFile != NULL) {
        fclose(_dumpingFile);
        _dumpingFile = NULL;
    }
    
    if (_logger != nil) {
        [_logger invalidate];
        [_logger release];
        _logger = nil;
    }
    
    [StartStopButton setAction:@selector(Start:)];
    [StartStopButton setTitle:@"Start"];
    [DumpCheckBox setEnabled:YES];
}

- (IBAction)ToggleDumping:(id)sender
{
}

- (IBAction)FilterOutput:(id)sender {
    NSRange endMarker;
    NSScroller *scroller = [[LoggerOutputTV enclosingScrollView] verticalScroller];
    BOOL isScrolledToEnd = (![scroller isEnabled] || [scroller floatValue] == 1);
    
    NSEnumerator *lineEnumerator = [_outputLines objectEnumerator];
    LoggerEntry *thisEntry;
    NSString *text;
    NSMutableString *finalOutput = [[NSMutableString alloc] init];
    
    [_currentFilterString release];
    if (![[sender stringValue] isEqualToString:@""]) {
        _currentFilterString = [[sender stringValue] retain];
    } else {
        _currentFilterString = nil;
    }
    
    [_outputLock lock];

    [LoggerOutputTV setString:@""];
    
    //endMarker = NSMakeRange([[LoggerOutputTV string] length], 0);
    
    [FilterProgressIndicator startAnimation:self];
    while (thisEntry = [lineEnumerator nextObject]) {
        text = [thisEntry text];
        if (_currentFilterString == nil || [text rangeOfString:_currentFilterString options:NSCaseInsensitiveSearch].location != NSNotFound) {
            [finalOutput appendString:text];
            //[LoggerOutputTV replaceCharactersInRange:endMarker withString:text];
            //endMarker.location += [text length];
        }
    }

    [LoggerOutputTV replaceCharactersInRange:NSMakeRange(0, [[LoggerOutputTV string] length]) withString:finalOutput];
    [FilterProgressIndicator stopAnimation:self];
    
    if (isScrolledToEnd) {
        endMarker = NSMakeRange([[LoggerOutputTV string] length], 0);
        [LoggerOutputTV scrollRangeToVisible:endMarker];
    }
    [LoggerOutputTV setNeedsDisplay:YES];
    [_outputLock unlock];
    [finalOutput release];
}

- (BOOL)runProcessAsAdministrator:(NSString*)scriptPath
                    withArguments:(NSArray *)arguments
                           output:(NSString **)output
                 errorDescription:(NSString **)errorDescription
{
    NSString *allArgs = [arguments componentsJoinedByString:@" "];
    NSString *fullScript = [NSString stringWithFormat:@"'%@' %@", scriptPath, allArgs];
    NSDictionary *errorInfo = [NSDictionary new];
    NSString *script = [NSString stringWithFormat:@"do shell script \"%@\" with administrator privileges", fullScript];
    NSAppleScript *appleScript = [[NSAppleScript new] initWithSource:script];
    NSAppleEventDescriptor *eventResult = [appleScript executeAndReturnError:&errorInfo];
    NSString *errorMessage = nil;
    NSNumber *errorNumber = nil;

    // Check errorInfo
    if (!eventResult)
    {
        if (errorDescription != nil)
        {
            // Describe common errors
            *errorDescription = nil;

            if ([errorInfo valueForKey:NSAppleScriptErrorNumber])
            {
                errorNumber = (NSNumber *)[errorInfo valueForKey:NSAppleScriptErrorNumber];

                if (errorNumber != nil)
                {
                    if ([errorNumber integerValue] == -128)
                    {
                        *errorDescription = @"The administrator password is required to do this.";
                    }
                }
            }
            
            // Set error message from provided message
            if (*errorDescription == nil)
            {
                if ([errorInfo valueForKey:NSAppleScriptErrorMessage])
                {
                    *errorDescription = (NSString *)[errorInfo valueForKey:NSAppleScriptErrorMessage];
                }
            }
        } else {
            if ([errorInfo valueForKey:NSAppleScriptErrorNumber])
            {
                errorNumber = (NSNumber *)[errorInfo valueForKey:NSAppleScriptErrorNumber];

                if (errorNumber != nil)
                {
                    if ([errorNumber integerValue] == -128)
                    {
                        errorMessage = @"The administrator password is required to do this.";
                    }
                }
            }

            // Set error message from provided message
            if (errorMessage == nil)
            {
                if ([errorInfo valueForKey:NSAppleScriptErrorMessage])
                {
                    errorMessage = (NSString *)[errorInfo valueForKey:NSAppleScriptErrorMessage];
                }
            }
            
            [self runAlertPanel:@"Authorization error" message:errorMessage firstButton:@"OK" secondButton:nil thirdButton:nil];
        }

        return NO;
    } else {
        if (output != nil)
        {
            // Set output to the AppleScript's output
            *output = [eventResult stringValue];
        }
    }

    return YES;
}

- (BOOL)isKlogKextPresent {
    return [[NSFileManager defaultManager] fileExistsAtPath:@"/System/Library/Extensions/KLog.kext"];
}

- (BOOL)isKlogCorrectRevision {
    NSBundle *klogBundle = [NSBundle bundleWithPath:@"/System/Library/Extensions/KLog.kext"];

    if (klogBundle == nil)
    {
        return NO;
    }

    NSDictionary *plist = [klogBundle infoDictionary];
    uint32_t version = [[plist valueForKey:@"CFBundleNumericVersion"] intValue];

    if ((version < 0x03600000) && (version != 0))
    {
        return NO;
    }

    return YES;
}

- (BOOL)installKLogKext {
    NSString *sourcePath = [[NSBundle mainBundle] pathForResource:@"KLog" ofType:@"kext"];
    NSString *destPath = [NSString pathWithComponents:[NSArray arrayWithObjects:@"/",@"System",@"Library",@"Extensions",@"KLog.kext",nil]];
    NSString *permRepairPath = [[NSBundle mainBundle] pathForResource:@"SetKLogPermissions" ofType:@"sh"];
    NSArray *args;

    if ([[NSFileManager defaultManager] fileExistsAtPath:sourcePath] == NO)
    {
        [self runAlertPanel:@"Missing Source File" message:@"\"KLog.kext\" could not be installed because it is missing from the application bundle." firstButton:@"Okay" secondButton:nil thirdButton:nil];
        
        return NO;
    }
    
    sourcePath = [NSString stringWithFormat:@"'%@'", sourcePath];
    destPath = [NSString stringWithFormat:@"'%@'", destPath];

    args = [NSArray arrayWithObjects:@"-Rf", sourcePath, destPath, nil];
    
    if ([self runProcessAsAdministrator:@"/bin/cp" withArguments:args output:nil errorDescription:nil] == NO)
    {
        return NO;
    }

    permRepairPath = [NSString stringWithFormat:@"'%@'", permRepairPath];

    args = [NSArray arrayWithObjects:permRepairPath, nil];
    
    if ([self runProcessAsAdministrator:@"/bin/sh" withArguments:args output:nil errorDescription:nil] == NO)
    {
        return NO;
    }
    
    args = [NSArray arrayWithObjects:destPath, nil];
    
    if ([self runProcessAsAdministrator:@"/sbin/kextload" withArguments:args output:nil errorDescription:nil] == NO)
    {
        return NO;
    }
    
    return YES;
}

- (BOOL)removeAndinstallKLogKext {
    NSString *              sourcePath = [[NSBundle mainBundle] pathForResource:@"KLog" ofType:@"kext"];
    NSString *              destPath = [NSString pathWithComponents:[NSArray arrayWithObjects:@"/",@"System",@"Library",@"Extensions",@"KLog.kext",nil]];
    NSString *              permRepairPath = [[NSBundle mainBundle] pathForResource:@"SetKLogPermissions" ofType:@"sh"];
    NSArray *args;

    if ([[NSFileManager defaultManager] fileExistsAtPath:sourcePath] == NO)
    {
        [self runAlertPanel:@"Missing Source File" message:@"\"KLog.kext\" could not be installed because it is missing from the application bundle." firstButton:@"Okay" secondButton:nil thirdButton:nil];
        
        return NO;
    }
    
    sourcePath = [NSString stringWithFormat:@"'%@'", sourcePath];
    destPath = [NSString stringWithFormat:@"'%@'", destPath];

    args = [NSArray arrayWithObjects:destPath, @"/private/tmp", nil];
    
    if ([self runProcessAsAdministrator:@"/bin/mv" withArguments:args output:nil errorDescription:nil] == NO)
    {
        return NO;
    }

    args = [NSArray arrayWithObjects:@"-Rf", sourcePath, destPath, nil];

    if ([self runProcessAsAdministrator:@"/bin/cp" withArguments:args output:nil errorDescription:nil] == NO)
    {
        return NO;
    }

    args = [NSArray arrayWithObjects:permRepairPath, nil];

    if ([self runProcessAsAdministrator:@"/bin/sh" withArguments:args output:nil errorDescription:nil] == NO)
    {
        return NO;
    }

    args = [NSArray arrayWithObjects:destPath, nil];

    if ([self runProcessAsAdministrator:@"/sbin/kextload" withArguments:args output:nil errorDescription:nil] == NO)
    {
        return NO;
    }
    
    return YES;
}

- (NSArray *)logEntries {
    return _outputLines;
}

- (NSArray *)displayedLogLines {
    return [[LoggerOutputTV string] componentsSeparatedByString:@"\n"];
}

- (void)scrollToVisibleLine:(NSString *)line {
    NSRange textRange = [[LoggerOutputTV string] rangeOfString:line];
    NSWindowController *outputCtrl = LoggerOutputTV;
    
    if (textRange.location != NSNotFound) {
        [LoggerOutputTV scrollRangeToVisible:textRange];
        [LoggerOutputTV setSelectedRange:textRange];
        [[outputCtrl window] makeFirstResponder:LoggerOutputTV];
        [[outputCtrl window] makeKeyAndOrderFront:self];
    }
}

- (void)handlePendingOutput:(NSTimer *)timer {
    if ([_bufferLock tryLock]) {
        if ([_outputLock tryLock]) {
            if ([_outputBuffer length] > 0) {
                NSRange endMarker = NSMakeRange([[LoggerOutputTV string] length], 0);
                NSScroller *scroller = [[LoggerOutputTV enclosingScrollView] verticalScroller];
                BOOL isScrolledToEnd = (![scroller isEnabled] || [scroller floatValue] == 1);
                
                [LoggerOutputTV replaceCharactersInRange:endMarker withString:_outputBuffer];
                
                if (isScrolledToEnd) {
                    endMarker.location += [_outputBuffer length];
                    [LoggerOutputTV scrollRangeToVisible:endMarker];
                }
                
                [_outputBuffer setString:@""];
                
                [LoggerOutputTV setNeedsDisplay:YES];
            }
            [_outputLock unlock];
        }
        [_bufferLock unlock];
    }
}

- (void)appendOutput:(NSString *)aString atLevel:(NSNumber *)level {
    LoggerEntry *entry = [[LoggerEntry alloc] initWithText:aString level:[level intValue]];
    
    [_outputLock lock];
    [_outputLines addObject:entry];
    [_outputLock unlock];
    
    [entry release];
    
    if (_dumpingFile != NULL) {
        fprintf(_dumpingFile, "@%s", [aString cStringUsingEncoding:NSUTF8StringEncoding]);
        fflush(_dumpingFile);
    }
    
    [_bufferLock lock];
    if (_currentFilterString == nil || [aString rangeOfString:_currentFilterString options:NSCaseInsensitiveSearch].location != NSNotFound) {
        [_outputBuffer appendString:aString];
    }
    [_bufferLock unlock];
}

- (void)appendLoggerEntry:(LoggerEntry *)entry {
    NSString *text = [entry text];
    [_outputLock lock];
    [_outputLines addObject:entry];
    [_outputLock unlock];
    
    if (_dumpingFile != NULL) {
        fprintf(_dumpingFile, "@%s", [text cStringUsingEncoding:NSUTF8StringEncoding]);
        fflush(_dumpingFile);
    }
    
    [_bufferLock lock];
    if (_currentFilterString == nil || [text rangeOfString:_currentFilterString options:NSCaseInsensitiveSearch].location != NSNotFound) {
        [_outputBuffer appendString:text];
    }
    [_bufferLock unlock];
}

- (void)usbLoggerTextAvailable:(NSString *)text forLevel:(int)level {
    LoggerEntry *entry = [LoggerEntry cachedFreshEntry];
    [entry setText:text level:level];
    
    [self performSelectorOnMainThread:@selector(appendLoggerEntry:) withObject:entry waitUntilDone:NO];
}

@end
