#pragma COMPACT

#include <reg52.h>
#include <math.h>
#include <intrins.h>
// #include <absacc.h>

#define TIMER0          Timer0() interrupt 1 using 1
#define TIMER1          Timer1() interrupt 3 using 2
#define BOOL            bit
#define BYTE            unsigned char
#define WORD            unsigned int
#define DWORD           unsigned long int
#define SINT            char
#define INT             int
#define LINT            long int
#define TRUE            1
#define FALSE           0
#define FLOAT           float
#define PI              3.14159265359
#define VOID            void

#define CLOCKTIMERPERIOD          (-250)
#define SCANINTERVAL              0xfd
#define DAYSCOUNT                 365
#define TIMECHANGED               0x07
#define SECONDCHANGED             0x01
#define MINUTCHANGED              0x02
#define HOURCHANGED               0x04
#define DAYCHANGED                0x08
#define YEARCHANGED               0x10
#define TIMENOTCHANGED            0x00
#define KEYSDELAY                 30

#define keyNOKEY                  0x00
#define keyKEY1                   0x01
#define keyKEY2                   0x02
#define keyKEY3                   0x04
#define keyKEY4                   0x08
#define keyKEY5                   0x10
#define keyKEY6                   0x20
#define keyTIMEOUT                0x40

#define dmBLANK                   0x00
#define dmTIME                    0x01
#define dmEDITTIME                0x02
#define dmEDITANGLE               0x03
#define dmEDITDATE                0x04
#define dmEDITNUMBER              0x05

#define tfTIME1                   0x80
#define tfTIME2                   0x40
#define tfTIME3                   0x20
#define tfTIME4                   0x02
#define tfTIME5                   0x04
#define tfTIME6                   0x01
#define tfTIMES                   (tfTIME1 | tfTIME2 | tfTIME3 | tfTIME4 | tfTIME5 | tfTIME6)
#define tfNOTTIMES                (~tfTIMES)

#include <TABLES.H>

#define VIDEOOUT                  P1

typedef struct tagTIME {
    BYTE Hour;
	BYTE Minute;
    } TIME;

typedef struct tagDATE {
    BYTE Month;
    BYTE Day;
	} DATE;

sbit Digit1        = P3^4;
sbit Digit2        = P3^1;
sbit Digit3        = P3^2;
sbit Digit4        = P3^3;
sbit Flags         = P3^5;
sbit Keys          = P3^0;
sbit Spkr          = P2^5;

BYTE Video[5];
BOOL Flasher            = FALSE;
BYTE VideoStatus        = 0x00;
BYTE DotsMask           = 0x00;
BYTE VideoScanState     = 0;
BYTE TimeChanged        = 0x00;
WORD CurrentDay         = 0;
TIME CurrentTime        = {0,0};
BYTE CurrentSecond      = 0;
WORD CurrentSecondParts = 0;
WORD CurrentYear        = 0;
BYTE DelayCounter       = 0;
BYTE SpkrCounter        = 0;

FLOAT HorizontalPos     = 35.13;
FLOAT VerticalPos       = 36.75;
BYTE KeyPressed         = keyNOKEY;
BYTE CurrentKeyDown     = keyNOKEY;
BYTE KeysDelayConter    = KEYSDELAY;
TIME Times[7];

code BYTE TimeFlags[6] = { tfTIME1, tfTIME2, tfTIME3, tfTIME4, tfTIME5, tfTIME6};

TIMER0
{
    if (SpkrCounter & 0x01) Spkr = ~Spkr;
    else Spkr = 1;
    CurrentSecondParts += 12;
	if ( CurrentSecondParts >= 32000){
	   Flasher = FALSE;
	   CurrentSecondParts -= 32000;
	   CurrentSecond++;
       if (SpkrCounter > 0) SpkrCounter--;
       if (DelayCounter > 0) DelayCounter--;
	   if (CurrentSecond >= 60){
	      CurrentSecond = 0;
		  CurrentTime.Minute++;
		  if ( CurrentTime.Minute >= 60){
		     CurrentTime.Minute = 0;
			 CurrentTime.Hour++;
			 if (CurrentTime.Hour >= 24) {
			    CurrentTime.Hour = 0;
                CurrentDay++;
                if (CurrentDay >= DAYSCOUNT) {
				   TimeChanged |= MINUTCHANGED | DAYCHANGED | HOURCHANGED | YEARCHANGED | SECONDCHANGED;
                   CurrentDay = 0;
				   CurrentYear++;
				   }
				else TimeChanged |= MINUTCHANGED | DAYCHANGED | HOURCHANGED | SECONDCHANGED;
				}
			 else TimeChanged |= MINUTCHANGED | HOURCHANGED | SECONDCHANGED;
			 }
		  else TimeChanged |= MINUTCHANGED | SECONDCHANGED;
	      }
	   else TimeChanged |= SECONDCHANGED;
	   }
	else {
        if (CurrentSecondParts > 8000) Flasher = TRUE;
        };

}

TIMER1
{
    BYTE DotMask = 0x00;
    switch (VideoStatus & 0xc0){
        case 0x40:
             DotMask = 0x20;
             break;
        case 0x80:
             if (Flasher) DotMask = 0x20;
             else DotMask = 0x00;
             break;
        default:
             DotMask = 0x00;
        };

	switch ( VideoScanState){
	case 0:
         Keys = 1;
         if ((Flasher) || (( VideoStatus & 0x01) == 0x00)) VIDEOOUT = Video[0];
         else VIDEOOUT = DECODE[10];
		 Digit1 = 0;
		 VideoScanState = 1;
		 break;
	case 1:
	     Digit1 = 1;
         if ((Flasher) || (( VideoStatus & 0x02) == 0x00)) VIDEOOUT = Video[1];
         else VIDEOOUT = DECODE[10];
		 Digit2 = 0;
		 VideoScanState = 2;
		 break;
	case 2:
         Digit2 = 1;
         if ((Flasher) || (( VideoStatus & 0x04) == 0x00)) VIDEOOUT = Video[2] ^ DotMask;
         else VIDEOOUT = DECODE[10] ^ DotMask;
		 Digit3 = 0;
		 VideoScanState = 3;
		 break;
	case 3:
	     Digit3 = 1;
         if ((Flasher) || (( VideoStatus & 0x08) == 0x00)) VIDEOOUT = Video[3];
         else VIDEOOUT = DECODE[10];
		 Digit4 = 0;
		 VideoScanState = 4;
		 break;
    case 4:
   	     Digit4 = 1;
         if ((Flasher) || (( VideoStatus & 0x10) == 0x00)) VIDEOOUT = Video[4];
         else VIDEOOUT = 0xff;// Video[4] | tfTIMES;
		 Flags = 0;
		 VideoScanState = 5;
         break;
    default:
            VideoScanState = 0;
            Flags = 1;
            VIDEOOUT = 0xff;
            Keys = 0;
            {
                register BYTE KeysStatus = 0x00;
                do {
                   BYTE Sample1, Sample2, Sample3;
                   _nop_();
                   Sample1 = VIDEOOUT;
                   _nop_();
                   Sample2 = VIDEOOUT;
                   _nop_();
                   Sample3 = VIDEOOUT;
                   if ((Sample1 == Sample2) || (Sample1 == Sample3)) KeysStatus = Sample1;
                   else if (Sample2 == Sample3) KeysStatus = Sample2;
                   } while (KeysStatus == 0x00);

                switch ( KeysStatus){
                    case ~0x04:
                         CurrentKeyDown = keyKEY1;
                         break;
                    case ~0x08:
                         CurrentKeyDown = keyKEY2;
                         break;
                    case ~0x10:
                         CurrentKeyDown = keyKEY3;
                         break;
                    case ~0x20:
                         CurrentKeyDown = keyKEY4;
                         break;
                    case ~0x40:
                         CurrentKeyDown = keyKEY5;
                         break;
                    case ~0x80:
                         CurrentKeyDown = keyKEY6;
                         break;
                    case ~0x00:
                         CurrentKeyDown = keyNOKEY;
                         break;
                    default:
                            CurrentKeyDown = keyNOKEY;
                    };
                }
            if (CurrentKeyDown == keyNOKEY) KeysDelayConter = 0;
            else if ( KeysDelayConter > 0) KeysDelayConter--;
                 else if ( KeyPressed == keyNOKEY) {
                         KeysDelayConter = KEYSDELAY;
                         KeyPressed = CurrentKeyDown;
                         }
	};
	TL1 = 0x00;
	TH1 = SCANINTERVAL;
}

BYTE ReadKey( VOID)
{
    register Key;
    while (KeyPressed == keyNOKEY);
    Key = KeyPressed;
    KeyPressed = keyNOKEY;
    return Key;
}

BYTE WaitKey( BYTE ATime)
{
    if (ATime == 0) return ReadKey();
    else {
        DelayCounter = ATime;
        while ((KeyPressed == keyNOKEY) && (DelayCounter > 0));
        if (KeyPressed == keyNOKEY) return keyTIMEOUT;
        else return ReadKey();
        }
}

FLOAT Round( FLOAT X)
{
    if ((fabs(X - floor( X))) < (fabs( X - ceil(X))))
	return ( floor( X));
    else return ( ceil( X));
}

VOID SeparateTime( FLOAT FTime, TIME *STime)
{
    STime->Hour = ( BYTE)floor( FTime);
	STime->Minute = ( BYTE)Round(( FTime - STime.Hour) * 60.0);
}

VOID GetSunAngleAndTimeEq( BYTE AYearNo, WORD ADayNo, FLOAT *SunAngle, FLOAT *TimeEq)
{
    register INT SunAngle1 = -2295;
	register INT TimeEq1 = 3;

	if ((( AYearNo & 0x03) == 0x00) && ( ADayNo > 58)) ADayNo--;

	while (ADayNo > 0){
	   SunAngle1 += ( SUNANGLEDELTAS[ADayNo] - 72);
	   TimeEq1 += ( TIMEEQDELTAS[ADayNo] - 1);
	   ADayNo--;
	   }

	*SunAngle = (( FLOAT)( SunAngle1)) / 100.0;
	*TimeEq = (( FLOAT)( TimeEq1));
}

VOID CalcTimes( BYTE AYearNo, WORD ADayNo)
{
    FLOAT ll;
    FLOAT gw1;
    FLOAT m1;
	FLOAT mn;
	FLOAT si;
	FLOAT cs;
	FLOAT SunAngle;
	FLOAT TimeEq;

	GetSunAngleAndTimeEq( AYearNo, ADayNo, &SunAngle, &TimeEq);
	ll = floor( VerticalPos / 30.0) * 30.0;
	gw1 = HorizontalPos * PI / 180.0;
	m1 = SunAngle * PI / 180.0;
	mn = 12.0 + TimeEq / 60.0 + ( ll - VerticalPos) / 15.0;
	si = sin( gw1) * sin(m1);
	cs = cos( gw1) * cos(m1);
	SeparateTime( mn - 3.819718634 * acos((-0.3255 - si) / cs), Times[0]);
	SeparateTime( mn - 3.819718634 * acos((-0.0144 - si) / cs), Times[1]);
	SeparateTime( mn + 2.0 / 60.0, Times[2]);
	SeparateTime( mn + 3.819718634 * acos(( sin( atan(1.0 / ( 1.0 + tan( gw1 - m1 )))) - si) / cs), Times[3]);
	SeparateTime( mn + 3.819718634 * acos(( -0.0144 - si) / cs), Times[4]);
	SeparateTime( mn + 3.819718634 * acos((-0.309 - si) / cs), Times[5]);
    Times[6] = Times[0];
    Times[6].Hour += 24;
}

VOID DisplayTime( TIME *ATime)
{
    VideoStatus = 0x40;
	Video[0] = DECODE[ ATime.Minute % 10];
	Video[1] = DECODE[ ATime.Minute / 10];
	Video[2] = DECODE[ ATime.Hour % 10];
	if (ATime.Hour < 10) Video[3] = DECODE[10];
	else Video[3] = DECODE[ ATime.Hour / 10];
    Video[4] = 0xff;
}
BYTE GetTimeFlagAndNextTime( TIME *ATime, TIME *ANextTime, BOOL SoundTime)
{
    register SINT I;
    register SINT II;
    BOOL Found = FALSE;
    for(I = 5; I >= 0; I--)
        if ( ((ATime->Hour > Times[I].Hour) || ((ATime->Hour == Times[I].Hour) && (ATime->Minute >= Times[I].Minute)))){
            if (SoundTime) if ((ATime->Hour == Times[I].Hour) && (ATime->Minute == Times[I].Minute)) SpkrCounter = 60;
            Found = TRUE;
            break;
            }
    if (Found) II = I;
    else II = 5;
    *ANextTime = Times[I + 1];
    return TimeFlags[II];
}

BOOL DisplayActiveTime( TIME *ATime, BOOL SoundTime)
{
    TIME NextTime;
    BYTE TimeFlag;
    // BYTE TimeStart;
    TimeFlag = GetTimeFlagAndNextTime( ATime, &NextTime, SoundTime);
    DisplayTime( ATime);
    Video[4] ^= TimeFlag;
    if ( NextTime.Minute >= 15) NextTime.Minute -= 15;
    else {
        if (NextTime.Hour > 0){
            NextTime.Hour --;
            NextTime.Minute = NextTime.Minute + 45;
            }
        else {
            NextTime.Hour = 0;
            NextTime.Minute = 0;
            }
        };

    if ((ATime.Hour > NextTime.Hour) || ((ATime.Hour == NextTime.Hour) && (ATime.Minute >= NextTime.Minute))) VideoStatus = 0x90;
    else VideoStatus = 0xa0;
}

VOID DisplayDate( DATE *ADate)
{
    VideoStatus = 0x40;
	Video[0] = DECODE[ADate->Day % 10];
	Video[1] = DECODE[ADate->Day / 10];
	Video[2] = DECODE[ADate->Month % 10];
	Video[3] = DECODE[ADate->Month / 10];
    Video[4] = 0xff;
}

VOID ConvertDayNoToDate( WORD ADayNo, DATE *ADate)
{
    register BYTE DayModified;

    if ((( CurrentYear & 0x03) == 0x00) && ( ADayNo > 58)){
        ADayNo--;
        DayModified = TRUE;
        }
    else DayModified = FALSE;
    ADate->Month = 1;
    while ( ADayNo >= MonthOffsets[ ADate.Month]) ADate->Month++;
    ADate->Day = ( BYTE)(( WORD)( ADayNo) - ( WORD)( MonthOffsets[ ADate->Month - 1])) + 1;
    if (( DayModified) && (ADate->Month == 2)) ADate->Day++;
}

VOID ConvertDateToDayNo( DATE *Date, WORD *CurrentDay)
{
    if ((( CurrentYear & 0x03) == 0x00) && ( Date->Month > 2)) *CurrentDay = Date->Day + MonthOffsets[Date->Month - 1];
    else *CurrentDay = Date->Day + MonthOffsets[Date->Month - 1] - 1;
}

BOOL EditDate( DATE *Date)
{
    DATE EditDate;
    BOOL DigitNo = 1;

    EditDate = *Date;

    while (TRUE){
        DisplayDate( &EditDate);
        if (DigitNo) VideoStatus = 0x4C;
        else VideoStatus = 0x43;
        switch ( ReadKey()){
            case keyKEY1:
                 if (DigitNo) if (EditDate.Month < 12) EditDate.Month++;
                              else EditDate.Month = 1;
                 else if (EditDate.Day < MonthDaysCount[EditDate.Month - 1]) EditDate.Day++;
                      else EditDate.Day = 1;
                 break;
            case keyKEY2:
                 if (DigitNo){
                    DigitNo = FALSE;
                    if (EditDate.Day > MonthDaysCount[EditDate.Month - 1]) EditDate.Day = MonthDaysCount[EditDate.Month - 1];
                    }
                 else DigitNo = TRUE;
                 break;
            case keyKEY5:
                 *Date = EditDate;
                 return TRUE;
            case keyKEY6:
                 return FALSE;
            }
        }
}

BOOL EditTime( TIME *Time)
{
    TIME EditTime;
    BOOL DigitNo = 1;

    EditTime = *Time;

    while (TRUE){
        DisplayTime( &EditTime);
        if (DigitNo) VideoStatus = 0x4C;
        else VideoStatus = 0x43;
        switch ( ReadKey()){
            case keyKEY1:
                 if (DigitNo) if (EditTime.Hour < 23) EditTime.Hour++;
                              else EditTime.Hour = 0;
                 else if (EditTime.Minute < 59) EditTime.Minute++;
                      else EditTime.Minute = 0;
                 break;
            case keyKEY2:
                 DigitNo = ~DigitNo;
                 break;
            case keyKEY5:
                 *Time = EditTime;
                 return TRUE;
            case keyKEY6:
                 return FALSE;
            }
        }
}
/*
BOOL EditInt( INT *Value, BOOL VisibleDot)
{
    BYTE Digits[4];
    BYTE DigitNo = 3;

    {
        INT WV = *Value;
        register BYTE I;
        for (I = 0; I < 4; I++){
            Digits[I] = WV % 10;
            WV = WV / 10;
            }
        }

    if ( VisibleDot) VideoStatus = 0x40;
    else VideoStatus = 0x00;

    while ( TRUE){
        {
            register BYTE I;
            for (I = 0; I <4; I++) Video[I] = DECODE[Digits[I]];
            }
        Video[4] = 0xff;
        VideoStatus = ((VideoStatus & 0xc0) | ((0x01 << DigitNo) & 0x0f));
        switch ( ReadKey()){
            case keyKEY1:
                 if (Digits[DigitNo] < 9) Digits[DigitNo]++;
                 else Digits[DigitNo] = 0;
                 break;
            case keyKEY2:
                 DigitNo = ((DigitNo - 1) & 0x03);
                 break;
            case keyKEY5:
                 *Value = (INT)Digits[0] + (INT)(Digits[1]) * 10 + (INT)(Digits[2]) * 100 + ((INT)(Digits[3])) * 1000;
                 return TRUE;
            case keyKEY6:
                 return FALSE;
            }
        }
}
*/

BOOL EditInt( INT *Value, BOOL VisibleDot)
{
    BYTE Digits[4];
    BYTE DigitNo = 3;
    INT TempValue = *Value;
    BOOL Signal;

    if (VisibleDot) {
        if (TempValue < 0) {
            TempValue = - TempValue;
            Signal = TRUE;
            }
        else Signal = FALSE;
        DigitNo = 4;
        }
    else {
        DigitNo = 3;
        Signal = FALSE;
        }

    {
        register BYTE I;
        for (I = 0; I < 4; I++){
            Digits[I] = TempValue % 10;
            TempValue = TempValue / 10;
            }
        }

    while ( TRUE){
        {
            register BYTE I;
            for (I = 0; I <4; I++) Video[I] = DECODE[Digits[I]];
            Video[4] = 0xff;
            if (VisibleDot){
                VideoStatus = 0x40;
                if (Signal) Video[4] ^= 0x08;
                else Video[4] ^= 0x10;
                }
            else VideoStatus = 0x00;
            }

        VideoStatus = ((VideoStatus & 0xc0) | ((0x01 << DigitNo) & 0x1f));

        switch ( ReadKey()){
            case keyKEY1:
                 if (DigitNo < 4){
                     if (Digits[DigitNo] < 9) Digits[DigitNo]++;
                     else Digits[DigitNo] = 0;
                     }
                 else Signal = ~Signal;
                 break;
            case keyKEY2:
                 if (DigitNo > 0) DigitNo --;
                 else {
                     if (VisibleDot) DigitNo = 4;
                     else DigitNo = 3;
                     };
                 break;
            case keyKEY5:
                 TempValue = (INT)Digits[0] + (INT)(Digits[1]) * 10 + (INT)(Digits[2]) * 100 + ((INT)(Digits[3])) * 1000;
                 if (Signal) TempValue = - TempValue;
                 *Value = TempValue;
                 return TRUE;
            case keyKEY6:
                 return FALSE;
            }
        }
}

main()
{
    BYTE I;
    IP = 0x02;               /* set high intrrupt priorery to timer0 */
    TL0 = CLOCKTIMERPERIOD;  /* set timer0 period */
    TH0 = CLOCKTIMERPERIOD;  /* set timer0 reload period */
    TL1 = 0x00;              /* set scan timer1 interval LO*/
    TH1 = SCANINTERVAL;      /* set scan timer1 interval HI*/
    TMOD = 0x12 ;            /* select mode 2 for timer0 & mode 1 for timer1*/
    TR0 = 1;                 /* start timer0 */
    TR1 = 1;                 /* start timer1 */
    ET0 = 1;                 /* enable timer0 interrupt */
    ET1 = 1;                 /* enable timer1 interrupt */
    EA = 1;                  /* global interrupt enable */

    TimeChanged |= SECONDCHANGED | MINUTCHANGED | HOURCHANGED | DAYCHANGED | YEARCHANGED;

    while(1){
        KeyPressed = keyNOKEY;
        do {
           if (TimeChanged != 0){
              if (( TimeChanged & DAYCHANGED) != 0x00){
                 TimeChanged &= ~DAYCHANGED;
                 CalcTimes( CurrentYear, CurrentDay);
                 }
              if (( TimeChanged & MINUTCHANGED) != 0x00){
		         TimeChanged &= ~MINUTCHANGED;
                 DisplayActiveTime( &CurrentTime, TRUE);
 			     }
              }
           } while (KeyPressed == keyNOKEY);

        switch (ReadKey()){
            case keyKEY1:
                 TimeChanged |= MINUTCHANGED;
                 {
		            DATE Date;
                    ConvertDayNoToDate( CurrentDay, &Date);
                    DisplayDate( &Date);
                    }
                 WaitKey(10);
                 break;
            case keyKEY2:
                 TimeChanged |= MINUTCHANGED;
                 for (I = 0; I < 6; I++){
                     DisplayTime( &Times[I]);
                     Video[4] ^= TimeFlags[I];
                     if (WaitKey(10) == keyTIMEOUT) break;
                     }
                  break;
            case keyKEY3:
                 {
                     INT Year = CurrentYear;
                     if (EditInt( &Year, FALSE)) {
		                 DATE Date;
                         ConvertDayNoToDate( CurrentDay, &Date);
                         if (EditDate( &Date)) {
                             TIME Time;
                             Time = CurrentTime;
                             if (EditTime( &Time)){
                                 CurrentSecondParts = 0;
                                 CurrentSecond = 0;
                                 CurrentTime = Time;
                                 ConvertDateToDayNo( &Date, &CurrentDay);
                                 CurrentYear = Year;
                                 }
                             }
                         }
                    }
                 TimeChanged |= SECONDCHANGED | MINUTCHANGED | HOURCHANGED | DAYCHANGED | YEARCHANGED;
                 break;
            case keyKEY4:
                 {
                     INT HPos = HorizontalPos * 100;
                     if (EditInt( &HPos, TRUE)){
                         INT VPos = VerticalPos * 100;
                         if (EditInt( &VPos, TRUE)){
                             HorizontalPos = ((FLOAT)(HPos)) / 100.0;
                             VerticalPos = ((FLOAT)(VPos)) / 100.0;
                             }
                         }
                     }
                 TimeChanged |= SECONDCHANGED | MINUTCHANGED | HOURCHANGED | DAYCHANGED | YEARCHANGED;
                 break;
            case keyKEY5:
                 TimeChanged |= MINUTCHANGED;
                 {
                     TIME NextTime;
                     GetTimeFlagAndNextTime( &CurrentTime, &NextTime, FALSE);
                     if (NextTime.Hour >= 24) NextTime.Hour -= 24;
                     DisplayActiveTime( &NextTime, FALSE);
                     VideoStatus = 0x40;
                     WaitKey(10);
                     }
                 break;
            case keyKEY6:
                 SpkrCounter = 10;
                 break;
            };
        };
}