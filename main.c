#include  "sys.h"
#include  "ports.h"
#include  "font.h"


#define xPos 1600      // middle of dispaly
#define yPos 1600      // middle of dispaly
#define nsteps 2048   // was 240 to be consistent with the old circle angle definition
#define stepDelay 50 //32
#define motionDelay 15  // how fast the beam will get there before enabling15
#define settlingDelay 10  // let the beam finish its move before turning on8
#define glowDelayOn 2  // let the beam finish its move before turning on8 2
#define glowDelayOff 3 // wait for beam to get bright before moving7 3
#define circleSpeed 65535   // angular step; bigger makes circles draw faster and more coarsely
#define lineStride  1  // linear step; bigger makes lines draw faster and more coarsely



#define OurBigKern 2  // this font's kern size for 40-up scales
#define OurLilKern 3  // this font's kern size for 30-down scales
#define OurBigGap 4   // this font's inter-row gap for 40-up scales
#define OurLilGap 10  // this font's inter-row gap for 30-down scales
#define OurChrHt 20   // this font's character height

// A draw list is a list of items to be drawn. These comprise:
#define listend 0 //                                         end of list
#define text 1 // size, zero,     stringptr, xpos, ypos   text item, no menu
#define menu 2  // size, function, stringptr, xpos, ypos   text item with menu
#define field 3 // size, function, stringptr, xpos, ypos   changeable field text
#define seg 4  // size, function, segptr,    xpos, ypos   segment (special character)

int thisX, thisY;    // circle position
int i;

// the sin/cos lookup table outputs are signed ints in range from -65536 to +65536
int sintab[nsteps];
int costab[nsteps];  // scaled int angle lookup tables, filled in during init
int tabXSize[nsteps];
int tabYSize[nsteps]; 
int tabXStart[nsteps];
int tabYStart[nsteps]; 

// timekeeping variables
static uint8_t Century = 20;   // century range 19-20
static uint8_t Years = 22;     // years range 0-99
static uint8_t Mons = 1;       // months range 1-12
static uint8_t WDay = 0;       // day of week, 0=Sunday
static uint8_t Days = 1;       // days range 1-31
static uint8_t Hrs = 0;        // hours range 0-23
static uint8_t Mins = 0;       // minutes range 0-59            
static uint8_t Secs = 0;       // seconds range 0-59

int ScrX;            // screensaver position table index 0..ScrTabL
int XSaver;          // current X offset of screensaver
int YSaver;          // current Y offset of screensaver


uint8_t theClock = 4;        // which clock face to show

// Text string variables
char *StrPtr;        // points to text string location
char TheChr;         // current character ASCII code
int ChrXPos;         // X position of LL corner of this character
int ChrYPos;         // Y position
uint8_t NewL;           // newline seen by GetWid for Center

// String formatting variables
int Scale;          // Font scale factor * 20 pixels high
int ChrHt;          // scaled character height in pixels (uppercase)
int ChrWid;         // scaled character width in pixels (variable)
int RowGap;         // scaled row gap in pixels (vert. kern)
int Kern;           // scaled space between chars in pixels
int DispHt;         // height of entire draw list display
int ChrCnt;         // String character count from GetWid
int StrWid;         // scaled width of string in pixels from GetWid

// The current segment being displayed
const int * TheSeg;        // pointer to our segment data in Fonts
int XCenter;        // X coord of circle center
int YCenter;        // Y coord of circle center
int XSize;          // Width of circle
int YSize;          // Height of circle
int XStart;         // X start of a line
int YStart;         // Y start of a line
int XEnd;           // X end of a line
int YEnd;           // Y end of a line
uint8_t Shape;          // shape code: lin/cir
int FirstO;         // first octant to display
int LastO;          // last octant to display

// The text, menu and field items are similar in format.
// xpos and ypos may be zero for the centering code to figure out.
// Segments are not centered.
// Any items after the first segment item will not be centered.
// They need a real xpos and ypos specified.



// Each item of a drawlist is one of these:
struct item {
  int type;        // see list above
  int scale;       // scale factor
  int func;        // function to execute (mod or menu numeration) if used, 0 if not
  char *string;     // the string to display
  int xpos;        // where it goes, or used when calculating where it goes
  int ypos;
};




struct item TheList[20];    // the list above is copied into here to allow modification

struct item * whichList;

int lastHour = 0;

const char janStr[] = "Jan";
const char febStr[] = "Feb";
const char marStr[] = "Mar";
const char aprStr[] = "Apr";
const char mayStr[] = "May";
const char junStr[] = "Jun";
const char julStr[] = "Jul";
const char augStr[] = "Aug";
const char sepStr[] = "Sep";
const char octStr[] = "Oct";
const char novStr[] = "Nov";
const char decStr[] = "Dec";

const char* const JanStr[] = {janStr, febStr, marStr, aprStr, mayStr, junStr,
                              julStr, augStr, sepStr, octStr, novStr, decStr};
const char januaryStr[] = "January";
const char februaryStr[] = "February";
const char marchStr[] = "March";
const char aprilStr[] = "April";
const char juneStr[] = "June";
const char julyStr[] = "July";
const char augustStr[] = "August";
const char septemberStr[] = "September";
const char octoberStr[] = "October";
const char novemberStr[] = "November";
const char decemberStr[] = "December";

const char* const JanuaryStr[] = {januaryStr, februaryStr, marchStr, aprilStr, mayStr, juneStr,
                              julyStr, augustStr, septemberStr, octoberStr, novemberStr, decemberStr};
// Weekday strings
const char sunStr[] = "Sun\n";
const char monStr[] = "Mon\n";
const char tueStr[] = "Tue\n";
const char wedStr[] = "Wed\n";
const char thuStr[] = "Thu\n";
const char friStr[] = "Fri\n";
const char satStr[] = "Sat\n";

const char* const SunStr[] = {monStr, tueStr, wedStr, thuStr, friStr, satStr,sunStr};

const char sundayStr[] = "Sunday\n";
const char mondayStr[] = "Monday\n";
const char tuesdayStr[] = "Tuesday\n";
const char wednesdayStr[] = "Wednesday\n";
const char thursdayStr[] = "Thursday\n";
const char fridayStr[] = "Friday\n";
const char saturdayStr[] = "Saturday\n";

const char* const SundayStr[] = {mondayStr, tuesdayStr, wednesdayStr, thursdayStr, fridayStr, saturdayStr, sundayStr};

// Other time related strings
char WeekDayStr[12];    // weekday string with NL
char WDayStr[12];    // weekday string with NL
char MontStr[12];   // month abbreviation with built-in NL
char MonthStr[12];   // month abbreviation with built-in NL


// Time strings - These are updated by makeTimeStrings()
char CenStr[] = "00";
char YrsStr[] = "00";
char MonStr[] = "00";
char DayStr[] = "00\n";
char HrsStr[] = "00";
char AMPMStr[] = "AM";
char Hrs12Str[] = "00";
char MinStr[] = "00";
char SecStr[] = "00\n";

// Some fixed strings
char DotLn[] = ".\n";
char Dot[] = ".";
char BlankLn[] = "\n";
char ColStr[]  = ":";
char DashStr[] = "-";
char SlaStr[]  = "/";
char SpaStr[]  = " ";
char DoneStr[] = "Exit\n"; // done with this menu
char OnStr[]   = "On \n";  // for on/off options like DST
char OffStr[]  = "Off\n";  // both are 5 bytes for easy copying

// The splash screen draw list
char scopeClock[] = "CRT ARM Clock\n";
char nm[]    = "DS3231\n";
char manuf[]   = "HELLO, WORLD!\n";
char manufCountry[] = "HOLODOK\n";
char copyR[]   = "2022\n";
char versionNo[]  = "Version 1.3\n";

char romanI[]    = "I";
char romanII[]   = "II";
char romanIII[]  = "III";
char romanIV[] = " IV ";
char romanV[]    = "V";
char romanVI[]   = "VI";
char romanVII[]  = "VII";
char romanVIII[] = "VIII";
char romanIX[]   = "IX";
char romanX[]    = "X";
char romanXI[]   = "XI";
char romanXII[]  = "XII";

char o1[]    = "1";
char o2[]   = "2";
char o3[]  = " 3 ";
char o4[] = "4";
char o5[]    = "5";
char o6[]   = "6";
char o7[]  = " 7 ";
char o8[] = " 8  ";
char o9[]   = "9";
char o10[]    = "10";
char o11[]   = "11";
char o12[]  = "12";


void readRTCtime()
{

  Secs  = 59;  // do some BCD to binary
  Mins  = 34;
  Hrs   = 8;
  Days  = 16;
  Mons  = 7;
  Years = 22;
  WDay = 4;
}


void SetScale() 
{
  ChrHt = Scale * OurChrHt;
  if (Scale < 40) 
  {
    Kern = Scale * OurLilKern;
    RowGap = Scale * OurLilGap;
  }
  else
  {
    Kern = Scale * OurBigKern;
    RowGap = Scale * OurBigGap;
  }
}

// DoSeg displays one segment.
// It returns after drawing the segment.
// Entered with the display blanked.
// Exits with the display blanked.

// Uses some math to draw each dot exactly once, with the number of dots calculated 
// to produce a good even display brightness. The dots are close enough together that they 
// are smeared together into a line or arc. 
// 
// Scaling:
// The DACs are 12 bits, giving a field of 4096 square. We use 2500 for the active field, 
// centered at (0,0). 
// with the rest being extra room for the x and y position controls to work.  
//
void DoSeg() 
{
  int len, xmotion, ymotion, motion, xstart, ystart;
  if (Shape) {
    // draw a circle
    int xcen = XCenter * Scale + ChrXPos + XSaver + xPos;  // X center position
    int ycen = YCenter * Scale + ChrYPos + YSaver + yPos;  // Y center position
    int xrad = (XSize * Scale) / 2;         // X size: radius is diameter/2 
    int yrad = (YSize * Scale) / 2;         // Y size
    int firstAngle = FirstO     * (nsteps >> 3);
    int lastAngle = (LastO + 1) * (nsteps >> 3); 
    int bigness = (xrad > yrad ? xrad : yrad );
    int stride = (circleSpeed) / bigness;   // stride is scaled by 256 to allow finer resolution of step size
    xstart = ((costab[firstAngle] * xrad)>>16) + xcen;
    ystart = ((sintab[firstAngle] * yrad)>>16) + ycen;
    xmotion = abs(thisX - xstart);
    ymotion = abs(thisY - ystart);
    motion = (xmotion > ymotion ? xmotion : ymotion);   // how far to move from previous segment

    // go to the start point with beam off
    *(volatile uint16_t *)&(DAC0_DAT0L) = xstart;
    *(volatile uint16_t *)&(DAC1_DAT0L) = ystart;

    // wait for the beam to reach the start point
   _WAIT_us(motion/motionDelay + settlingDelay);
    BLANK_ON;        // start making photons
   _WAIT_us(glowDelayOn);        // wait for glow to start

    // draw the circle with the beam on, sride is 24.8 bits to allow fine rate control
    for (i=(firstAngle<<8); i<(lastAngle<<8); i+=stride) {
      thisX = ((costab[(i>>8) % nsteps] * xrad) >> 16) + xcen;
      thisY = ((sintab[(i>>8) % nsteps] * yrad) >> 16) + ycen;
      _WAIT_NOP(stepDelay);  
      *(volatile uint16_t *)&(DAC0_DAT0L) = thisX;
      *(volatile uint16_t *)&(DAC1_DAT0L) = thisY;
    }
   _WAIT_us(glowDelayOff);        // wait for glow to start
    BLANK_OFF;        // done, hide dot now
  }
  else {
    // draw a line 
    xstart = XStart * Scale + ChrXPos + XSaver + xPos;  // X start position
    ystart = YStart * Scale + ChrYPos + YSaver + yPos;  // Y start position
    int xlen = ((XEnd - XStart) * Scale);         // X size
    int ylen = ((YEnd - YStart) * Scale);         // Y size
    if (xlen == 0) 
    {
      len = abs(ylen);   //save ourselves a square root and two muls when not needed
    }
    else 
    {
      if (ylen == 0)
      {
        len = abs(xlen);
      }
      else
      {
        len = (int)sqrt(xlen * xlen + ylen * ylen);
      }
    }
    if (len <= 0) len = lineStride; 
    int xinc = ((xlen<<8)/len);
    int yinc = ((ylen<<8)/len);
    xmotion = abs(thisX - xstart);
    ymotion = abs(thisY - ystart);
    motion = (xmotion > ymotion ? xmotion : ymotion);   // how far to move from previous segment
    *(volatile uint16_t *)&(DAC0_DAT0L) = xstart;
    *(volatile uint16_t *)&(DAC1_DAT0L) = ystart;
    
   _WAIT_us(motion/motionDelay + settlingDelay);
    BLANK_ON;        // start making photons
   _WAIT_us(glowDelayOn);        // wait for glow to start
    
 
    for (i=0; i<(len); i += lineStride) {
      thisX = ((i*xinc)>>(8)) + xstart;
      thisY = ((i*yinc)>>(8)) + ystart;
      _WAIT_NOP(stepDelay); 
      *(volatile uint16_t *)&(DAC0_DAT0L) = thisX;
      *(volatile uint16_t *)&(DAC1_DAT0L) = thisY;
    }
   _WAIT_us(glowDelayOff);        // wait for glow to start
    BLANK_OFF;        // done, hide dot now
  }
}

// Standalone shape drawing for non-drawlist features

void drawALine(int xstart, int ystart, int xend, int yend) {
  Scale = 1;
  ChrXPos = 0;
  ChrYPos = 0;
  XStart = xstart;
  YStart = ystart;
  XEnd = xend;
  YEnd = yend;
  Shape = lin;
  DoSeg();
}
void drawACircle(int xcenter, int ycenter, int diameter) {
  Scale = 1;
  ChrXPos = 0;
  ChrYPos = 0;
  XCenter = xcenter;
  YCenter = ycenter;
  XSize = diameter;
  YSize = diameter;
  Shape = cir;
  FirstO = 6;
  LastO = 13;
  DoSeg();
}

// -------------------- Draw list centering code ---------------------

// GetWid calculates the pixel width of the text string pointed to by StrPtr
// for centering purposes. It scans the character segments for
// the last int which contains the width, adds them and their kerns.
// The result is returned in StrWid.
//
// If the line is terminated with an NL, it sets NewL in MenuFlg and
// subtracts one kern from the total width.
void GetWid() 
{
  ChrCnt = 0;
  StrWid = 0;
  // look up width of each printable char, sum into StrWid, leave StrPtr at \n or null
  while ((TheChr = *StrPtr++) >= 0x20)
  {
    TheSeg = Font[(TheChr % 0x7f)- 32]; // read font table - tricky
    while (*TheSeg < 0x80) // skip over all real segments, stopping at end flag
      TheSeg += 7;
    StrWid += *TheSeg % 128;   // mask off end flag to reveal width
    ChrCnt++;
  }
  // calculate string width depending on newline
  StrWid *= Scale;   // big chars use more space
  if (TheChr == '\n')
  {
    NewL = 1;
    StrWid += Kern * (ChrCnt - 1);  // newline - use n-1 kern spaces
  }
  else
  {
    NewL = 0;
    StrWid += Kern * ChrCnt;        // no newline, retain kern after last char
  }
}
 
// Center centers a draw list, calculating X and Y string positions and storing them 
// in the copy of the drawlist.

// This is tricky, as a line of text is composed of multiple strings. Only the 
// last string in each line has a newline char. 

// We need the total line width for all strings in a line so that we may
// calculate the start X position for each string. So we need this info
// on the first string, but it's only available after the last string.
// So we need to store it in LinWid and do another scan on each line in X pass.

//  The first X pass does the following:

// For each string, call GetWid and store StrWid in XPos.
// Store term char (0 or NL) in YPos. Store line width in LinWids array.

// The Y scan of the strings calculates XPos from LinWid and StrWid,
// for each string in the line. The YPos flag helps know when to NL.

// CentXLp sets the X positions and calculates the total height.
// CentYLp sets the Y positions using the height.

// It returns without doing anything if there is a string that has a non-zero position. 
// This is true in a drawlist such as pong or face clock. 

// center a draw list, filling in xpos and ypos fields for each element
void Center(struct item *list) 
{
  int LinWid = 0;     // scaled width of line in pixels
  int LinWids[10];    // array of line widths per line
  int *wp;       // points to array above
  struct item *p = list;

  NewL = 1;    // flag if a newline seen yet on this line

  DispHt = 0;
  wp = LinWids;   // array of widths of every line
  // CXLoop scans all lines of text, calcs height and widths
  while (p->type != listend) 
  {
    if (p->xpos != 0)
    {
      return;  // there is a string that has a position, so don't do any centering of this list
    }
    Scale = p->scale;
    StrPtr = p->string;   // read string pointer
    SetScale();           // set Kern, Scale
    GetWid();
    p->xpos = StrWid;  // save string width in x pos space
    p->ypos = TheChr;  // save termination char in Y pos space
    LinWid += StrWid;
    if (NewL) {    // process another row because newline spotted
      *wp++ = LinWid;
      DispHt += (RowGap + ChrHt);   // accumulate height of row for Y centering
      LinWid = 0;
    }
    p++;    // look at next list element
  }
  // calculate the Y start point from total size and row heights
  ChrYPos = ((DispHt - RowGap)/2);
  
  // this last loop fills in xpos and ypos in each string, based on entire drawlist
  NewL = 1;
  wp = LinWids;
  p = list;
  while (p->type != listend) 
  {
    Scale = p->scale;
    StrPtr = p->string;   // read string pointer
    StrWid = p->xpos;  // fetch string width we stashed in x pos space
    TheChr = p->ypos;  // fetch termination char we stashed in Y pos space
    if (NewL) {    // process another row because newline spotted
      ChrXPos = 0-*wp++/2;   // left end = middle - length/2
      SetScale();
      ChrYPos -= ChrHt;
    }
    NewL = 0;
    if (TheChr == '\n') 
      NewL = 1;
    p->xpos = ChrXPos;   // write the real position into list
    ChrXPos += StrWid;
    p->ypos = ChrYPos;
    if (NewL)
      ChrYPos -= RowGap;   // remove one inter-row gap since n-1 gaps in n lines
    p++;    // look at next list element
  }
}


// copy a draw list to TheList, since TheList is modified by Center(). 
void copyList(struct item *list) 
{
  struct item *p = list;
  struct item *q = TheList; 
  p = list;
  while (p->type != listend) 
  {
    q->type   = p->type;
    q->scale  = p->scale;
    q->func   = p->func;
    q->string = p->string; 
    q->xpos   = p->xpos;  
    q->ypos   = p->ypos; 
    p++;    // look at next list element
    q++; 
  }
  q->type   = p->type;  // copy the last listend to terminate the list
}

// Get the segment into RAM from TheSeg pointer
int GetSeg() {
  int theChar;
  // read a segment's descriptor from char font array
  theChar   = *TheSeg++;
  if (theChar < 128) {                // if < 128 it's shape, else it's char width
    Shape   = theChar;
    XCenter = XStart = *TheSeg++;
    YCenter = YStart = *TheSeg++;
    XSize   = XEnd = *TheSeg++;
    YSize   = YEnd = *TheSeg++;
    FirstO  = *TheSeg++;              // these are not used for lines, only for arcs
    LastO   = *TheSeg++;
    return 1;  // not end of character
  }
  else {
    ChrWid = (theChar % 128) * Scale;  // bit 7 is end flag - mask it off
    return 0;  // end of character
  }
}

// DispStr displays the zero-terminated text string pointed to by StrPtr
void DispStr() 
{
  int notLast;

  TheChr = (*StrPtr++);
  while ((TheChr >= 32)) // printable
  {
    // look up character in ROM font table. This is tricky. 
    TheSeg = Font[(TheChr & 0x7f) - 32];
    notLast = GetSeg();
    while (notLast) 
    {
      DoSeg();           // display segments until last one found
      notLast = GetSeg();
    }
    ChrXPos = ChrXPos + ChrWid + Kern;   // move over by width of char
    TheChr = (*StrPtr++);
  }
//    DoEnc();          // polled mode I/O, so we need to do it many times
}


// Display a draw list once
void DoAList(struct item *list) 
{
  struct item *p = list;
  while (p->type != listend) 
  {
    Scale = p->scale;
    StrPtr = p->string;   // read string pointer
    ChrXPos = p->xpos;  // read position of string (not used by most!)
    ChrYPos = p->ypos;
    // seg tells it to draw a circle or a line. This code does a circle. Need to do a line also. 
    if (p->type == seg) {
      XCenter = YCenter = 0;  // position done by ChrX,YPos
      XSize = StrPtr[0];   // read out segment parameters from 'string' array
      YSize = StrPtr[1];
      XStart = -XSize/2;
      YStart = -YSize/2;
      XEnd = XSize/2;
      YEnd = YSize/2;
      Shape = StrPtr[2];
      FirstO = StrPtr[3];
      LastO = StrPtr[4];
      DoSeg();
    }
    else
    {
      SetScale();
      DispStr();
    }
    p++;
  }
}

// Once an hour, advance the screensaver to the next position
// in the list. This subtly moves the display but spreads out
// any phosphor burn over a 4 by 8 pixel area.
//

// Screensaver moves the display impercetibly in a triangle raster scan 
// every hour, to spread out the screen burn.


void updateScreenSaver() {
  if (Hrs != lastHour) {  // on the hour
    ScrX++;
    if (ScrX >= 31) ScrX = 0;
    XSaver = 4*abs(ScrX - 31/2);
    YSaver = 15*abs(ScrX%8 - (31%8)/2);
  }
  lastHour = Hrs;
}



struct item splashList[] = {
  //{text,40,0,BlankLn, 0,0},
  {text,15,0,scopeClock,0,0},
  {text,20,0,nm,   0,0},
  {text,10,0,BlankLn, 0,0},
  {text,15,0,versionNo, 0,0},
 // {text,40,0,BlankLn, 0,0},
  //{text,6,0,DotLn, 0,0},
  {listend,0,0,BlankLn,0,0}
};

struct item splashList2[] = {
  //{text,40,0,BlankLn, 0,0},
  {text,15,0,manuf,  0,0},
  {text,15,0,manufCountry, 0,0},
  {text,15,0,BlankLn, 0,0},
  {text,10,0,copyR, 0,0},
 // {text,15,0,BlankLn, 0,0},
 // {text,30,0,BlankLn, 0,0},
  //{text,6,0,DotLn, 0,0},
  {listend,0,0,BlankLn,0,0}
};


// ----------------------------- Analog clock drawing -------------------

// The Face draw list has circles for minor marks and Roman
// numerals for the compass points. A circle in the center too.
// The hands are drawn by DrawClk
// The positions of the numerals are trial-and-error.



struct item faceListRom[] = {
  {text,10,0,romanI,      490,  760},    //  1 o'clock
  {text,10,0,romanII,     820,  400},    //  2 o'clock
  {text,10,0,romanIII,    900, -100},    //  3 o'clock
  {text,10,0,romanIV,   740, -590},    //  4 o'clock
  {text,10,0,romanV,      400, -960},    //  5 o'clock
  {text,10,0,romanVI,    -100,-1080},    //  6 o'clock
  {text,10,0,romanVII,   -600, -960},    //  7 o'clock
  {text,10,0,romanVIII, -1000, -600},    //  7 o'clock
  {text,10,0,romanIX,   -1040, -100},    //  9 o'clock
  {text,10,0,romanX,     -940,  400},    // 10 o'clock
  {text,10,0,romanXI,    -600,  760},    // 11 o'clock
  {text,10,0,romanXII,   -160,  880},    // 12 o'clock
  {text,6,0,Dot, -160,  2000},
  {listend,0,0,BlankLn,0,0}
};

struct item faceListNum[] = {
  {text,10,0,o1,    490,  760},    //  1 o'clock
  {text,10,0,o2,    820,  400},    //  2 o'clock
  {text,10,0,o3,    900, -100},    //  3 o'clock
  {text,10,0,o4,    740, -590},    //  4 o'clock
  {text,10,0,o5,    400, -960},    //  5 o'clock
  {text,10,0,o6,   -100,-1080},    //  6 o'clock
  {text,10,0,o7,   -600, -960},    //  7 o'clock
  {text,10,0,o8,  -1000, -600},    //  7 o'clock
  {text,10,0,o9,  -1040, -100},    //  9 o'clock
  {text,10,0,o10,  -940,  400},    // 10 o'clock
  {text,10,0,o11,  -600,  760},    // 11 o'clock
  {text,10,0,o12,  -160,  880},    // 12 o'clock
  {text,6,0,Dot, -160,  2000},
  {listend,0,0,BlankLn,0,0}
};

// Draw clock hand on screen

// angle is the angle (0=N, 60=E, 120=S, 180=W)
// len is length of hand

void DoHand(int len, int angle) { 
  XSize = tabXSize[angle];
  YSize = tabYSize[angle];      // swap X and Y, because 0 deg is at north CW like a clock, not east CCW like math 
  XStart = tabXStart[angle]; // start at center circle radius
  YStart = tabYSize[angle];
  XEnd = (len * XSize) >>8;   // scale to hand length
  YEnd = (len * YSize) >>8;
  Scale = 1;
  ChrXPos = ChrYPos = 0;
  Shape = lin;
  DoSeg();
}
// DrawClk draws the three clock hands. They are drawn in fine
// increments of 1.5 degrees per step. The steps are calculated
// incorporating the smaller time units for smoother movement.
// The angle is (0=N, 60=E, 120=S, 180=W)
void DrawClk() {
  drawACircle(0, 0, 180);   //

  DoHand(2200, ((Secs << 2)*nsteps/240) % nsteps);
  DoHand(1600, ((Secs + Mins*60)*nsteps/3600) % nsteps);
  DoHand(1200, ((Secs + Mins*60+(Hrs % 12)*3600)*nsteps/43200) % nsteps);
}

// ------------------------ digital clocks -----------------------------------

// total time/date/day digital clock draw list
struct item timefList[]  = {
  //{text,30,0,BlankLn, 0,0},
  {text,20,0,WDayStr, 0,0},   // day of week
  {text,20,0,HrsStr,  0,0},   // hours
  {text,20,0,ColStr,  0,0},   // colon
  {text,20,0,MinStr,  0,0},   // mins
  {text,20,0,ColStr,  0,0},   // colon
  {text,20,0,SecStr,  0,0},   // secs
  {text,20,0,MontStr,0,0},   // month
  {text,20,0,SpaStr,  0,0},
  {text,20,0,DayStr,  0,0},   // day
  {text,20,0,CenStr,  0,0},   // the full year
  {text,20,0,YrsStr,  0,0},
  {text,20,0,BlankLn, 0,0},//att1
  //{text,20,0,BlankLn, 0,0},
 // {text,6,0,DotLn, 0,0},
  {listend,0,0,BlankLn, 0,0}
};

// 6 digit digital clock with date draw list
struct item time6dList[] = {
  //{text,40,0,BlankLn, 0,0},
  {text,15,0,MonthStr,0,0},  // months
  {text,15,0,SpaStr,  0,0},  // space
  {text,15,0,DayStr,  0,0},  // days
  {text,40,0,HrsStr,  0,0},  // hours
  {text,40,0,ColStr,  0,0},  // colon
  {text,40,0,MinStr,  0,0},  // mins
  {text,40,0,BlankLn, 0,0},  // next line
  {text,30,0,SecStr,  0,0},  // secs
  //{text,15,0,BlankLn, 0,0},//att2
  //{text,6,0,DotLn, 0,0}, //att2
  {listend,0,0,BlankLn, 0,0}
};


// 4 digit 12h digital clock with date draw list
struct item time4dList[] = {
  //{text,40,0,BlankLn, 0,0},//att1
  {text,15,0,WeekDayStr, 0,0},  // weekday
  {text,40,0,Hrs12Str,  0,0},  // hours
  {text,40,0,ColStr,  0,0},  // colon
  {text,40,0,MinStr,  0,0},  // mins
  {text,20,0,AMPMStr,  0,0},  // hours
  {text,40,0,BlankLn, 0,0},  // next line
  {text,15,0,MonthStr,0,0},  // months
  {text,15,0,SpaStr,  0,0},  // space
  {text,15,0,DayStr,  0,0},  // days
  {listend,0,0,BlankLn,0,0}
};


// 6 digit digital clock draw list
struct item time6nList[] = {
 // {text,40,0,BlankLn, 0,0},
  //{text,10,0,BlankLn, 0,0},  // make hh:mm line lower for better centered appearance
  {text,40,0,HrsStr,  0,0},  // hours
  {text,40,0,ColStr,  0,0},  // colon
  {text,40,0,MinStr,  0,0},  // mins
  {text,40,0,BlankLn, 0,0},  // next line
  {text,30,0,SecStr,  0,0},  // secs
 // {text,20,0,BlankLn, 0,0}, //att0
  //{text,6,0,DotLn, 0,0}, //att0
  {listend,0,0,BlankLn,0,0}
};


// 4 digit digital clock draw list
struct item time4nList[] = {
  //{text,30,0,BlankLn, 0,0},//att3
  //{text,20,0,BlankLn, 0,0},//att3
  {text,40,0,HrsStr,  0,0},  // hours
  {text,40,0,ColStr,  0,0},  // colon
  {text,40,0,MinStr,  0,0},  // mins
  //{text,40,0,BlankLn, 0,0}, // next line
  {text,30,0,BlankLn, 0,0},
  {text,20,0,BlankLn, 0,0},
  //{text,6,0,DotLn, 0,0},
  {listend,0,0,BlankLn,0,0}
};

// 4 digit digital clock draw list

struct item time6nList2[] = {
 // {text,30,0,BlankLn, 0,0},//att0
 // {text,20,0,BlankLn, 0,0},
  {text,40,0,HrsStr,  0,0},  // hours
  {text,40,0,ColStr,  0,0},  // colon
  {text,40,0,MinStr,  0,0},  // mins
  {text,20,0,SecStr,  0,0},  // secs
//  {text,30,0,BlankLn, 0,0},
//  {text,20,0,BlankLn, 0,0},
//  {text,6,0,DotLn, 0,0},
  {listend,0,0,BlankLn,0,0}
};

// ----------------------- Time string generator -----------------------

// makeTimeStrings fills in the time and date strings in RAM with the
// current time in ASCII. The month and weekday strings are set also.
void makeTimeStrings() {
  //int zon;  // temp storage for manipulation
  
 
  CenStr[0] = (Century / 10) | '0';
  CenStr[1] = (Century % 10) | '0';
  YrsStr[0] = (Years / 10) | '0';
  YrsStr[1] = (Years % 10) | '0';
  MonStr[0] = (Mons / 10) | '0';
  MonStr[1] = (Mons % 10) | '0';

  DayStr[0] = (Days / 10) | '0';
  DayStr[1] = (Days % 10) | '0';
  DayStr[2] = '\n';

  // make the day of month be one digit if leading zero
  if (DayStr[0] == '0')
  {
    DayStr[0] = DayStr[1];
    DayStr[1] = '\n';
    DayStr[2] = 0;
  }

  // read the weekday string from ROM
  strcpy(WDayStr, SunStr[WDay]);
  strcpy(WeekDayStr, SundayStr[WDay]);
  // read the month string from ROM
  strcpy(MontStr, JanStr[Mons - 1]);
  strcpy(MonthStr, JanuaryStr[Mons - 1]);

  if (Hrs == 0) 
     {
      Hrs12Str[0] = '1';
      Hrs12Str[1] = '2';
      AMPMStr[0] = 'A';
     }
  else if (Hrs < 10)
          { 
           Hrs12Str[0] = (Hrs) | '0';
           Hrs12Str[1] = 0;
           AMPMStr[0] = 'A';
          }
  else if (Hrs < 12)
          { 
           Hrs12Str[0] = (Hrs / 10) | '0';
           Hrs12Str[1] = (Hrs % 10) | '0';
           AMPMStr[0] = 'A';
          }
  else if (Hrs == 12)
          { 
           Hrs12Str[0] = '1';
           Hrs12Str[1] = '2';
           AMPMStr[0] = 'P';
          }
  else if (Hrs < 21)
          { 
           Hrs12Str[0] = (Hrs-12) | '0';
           Hrs12Str[1] = 0;
           AMPMStr[0] = 'P';
          }
  else {
           Hrs12Str[0] = ((Hrs-12) / 10) | '0';
           Hrs12Str[1] = ((Hrs-12) % 10) | '0';
           AMPMStr[0] = 'P';
       }

  HrsStr[0] = (Hrs / 10) | '0';
  HrsStr[1] = (Hrs % 10) | '0';
  MinStr[0] = (Mins / 10) | '0';
  MinStr[1] = (Mins % 10) | '0';
  SecStr[0] = (Secs / 10) | '0';
  SecStr[1] = (Secs % 10) | '0';

}


   // number of clock faces to choose from list of clock face draw lists
struct item * ClkList[] =  
   {faceListNum,    // analog clock face, needs hands drawn
    faceListRom,
    timefList,   // 6 digit digital clock with full date, day
    time4nList,  // 4 digit digital clock
    time6nList,  // 6 digit digital clock
    time6nList2,  // 6 digit digital clock
    time4dList,  // 4 digit digital clock with date
    time6dList,  // 6 digit digital clock with date
    splashList,
    splashList2};






int main (void) {

 for (i=0;i<nsteps;i++) {
    sintab[i]    = (int)(65536*sin(TWO_PI*i/nsteps));
    costab[i]    = (int)(65536*cos(TWO_PI*i/nsteps));
 }
  
  for (i=0;i<nsteps;i++) {
    tabXSize[i]  = (int)sintab[i]/500;
    tabYSize[i]  = (int)costab[i]/500; 
    tabXStart[i] = (int)sintab[i]/650;
    tabYStart[i] = (int)costab[i]/650; 
 }
  
  SIM_SCGC5 |=   SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;
  SIM_SCGC2 |= (SIM_SCGC2_DAC0_MASK|SIM_SCGC2_DAC1_MASK);
  //SIM_SCGC5 |=   (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK);
  PORTA->PCR[13] = PORT_PCR_MUX(1U);
  PTA->PDDR = (1U << 13U);
  PORTC->PCR[5] = PORT_PCR_MUX(1U);
  PTC->PDDR = (1U << 5U);
  PORTD->PCR[0] = PORT_PCR_MUX(1U);
  PTD->PDDR = (1U);
  
  LED_ON;
  _WAIT(100);
  VREF_TRM = 0x60;
  VREF_SC = 0xE1;

  DAC0_C0 = (DAC_C0_DACEN_MASK);
  DAC1_C0 = (DAC_C0_DACEN_MASK);
  _WAIT(7000);
  REL_ON;
  LED_OFF;




  
  while (1) {
        
  Center(TheList);                // fill in the positions of each string in our copy
  DoAList(TheList);  
  
  if ((theClock == 0) || (theClock == 1)) DrawClk();
  
 
        
       
  
  readRTCtime();   // read whichever clock is correctest, make it be local time
  makeTimeStrings();              // fill in the time variables into number strings
  updateScreenSaver();
  //theClock = 4;
  whichList = ClkList[theClock];       // point to the clock drawlist we are displaying now       // clock 0 has hands to draw
  copyList(whichList); 
  Center(TheList);
  _WAIT_us(5600);
  }   
    

       }
















