#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <wchar.h>
#include <vector>


// definitions copied from PQ PTUDemo files

// some important Tag Idents (TTagHead.Ident) that we will need to read the most common content of a PTU file
// check the output of this program and consult the tag dictionary if you need more
#define TTTRTagTTTRRecType "TTResultFormat_TTTRRecType"
#define TTTRTagNumRecords  "TTResult_NumberOfRecords"  // Number of TTTR Records in the File;
#define TTTRTagRes         "MeasDesc_Resolution"       // Resolution for the Dtime (T3 Only)
#define TTTRTagGlobRes     "MeasDesc_GlobalResolution" // Global Resolution of TimeTag(T2) /NSync (T3)
#define FileTagEnd         "Header_End"                // Always appended as last tag (BLOCKEND)

// TagTypes  (TTagHead.Typ)
#define tyEmpty8      0xFFFF0008
#define tyBool8       0x00000008
#define tyInt8        0x10000008
#define tyBitSet64    0x11000008
#define tyColor8      0x12000008
#define tyFloat8      0x20000008
#define tyTDateTime   0x21000008
#define tyFloat8Array 0x2001FFFF
#define tyAnsiString  0x4001FFFF
#define tyWideString  0x4002FFFF
#define tyBinaryBlob  0xFFFFFFFF

// RecordTypes
#define rtPicoHarpT3     0x00010303    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $03 (T3), HW: $03 (PicoHarp)
#define rtPicoHarpT2     0x00010203    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T2), HW: $03 (PicoHarp)
#define rtHydraHarpT3    0x00010304    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $03 (T3), HW: $04 (HydraHarp)
#define rtHydraHarpT2    0x00010204    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T2), HW: $04 (HydraHarp)
#define rtHydraHarp2T3   0x01010304    // (SubID = $01 ,RecFmt: $01) (V2), T-Mode: $03 (T3), HW: $04 (HydraHarp)
#define rtHydraHarp2T2   0x01010204    // (SubID = $01 ,RecFmt: $01) (V2), T-Mode: $02 (T2), HW: $04 (HydraHarp)
#define rtTimeHarp260NT3 0x00010305    // (SubID = $00 ,RecFmt: $01) (V2), T-Mode: $03 (T3), HW: $05 (TimeHarp260N)
#define rtTimeHarp260NT2 0x00010205    // (SubID = $00 ,RecFmt: $01) (V2), T-Mode: $02 (T2), HW: $05 (TimeHarp260N)
#define rtTimeHarp260PT3 0x00010306    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T3), HW: $06 (TimeHarp260P)
#define rtTimeHarp260PT2 0x00010206    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T2), HW: $06 (TimeHarp260P)
#define rtMultiHarpNT3   0x00010307    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T3), HW: $07 (MultiHarp150N)
#define rtMultiHarpNT2   0x00010207    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T2), HW: $07 (MultiHarp150N)

#pragma pack(8) //structure alignment to 8 byte boundaries

// A Tag entry
struct TgHd {
    char Ident[32];     // Identifier of the tag
    int Idx;            // Index for multiple tags or -1
    unsigned int Typ;  // Type of tag ty..... see const section
    long long TagValue; // Value of tag.
} TagHead;

// TODO see below function list
// - read header into struct - this will hold most of the required metadata about the scan and the tttr records
// - confirm overflow mechanism
// - 


struct sTTTR
{
    /*
    unsigned abs_time_pulse : 36;
    unsigned line_number : 8;
    unsigned dtime : 10;
    unsigned cycle_number : 24;
    unsigned pulse_in_cycle : 20;
    */
    unsigned long long abs_time_pulse;
    unsigned char line_number : 8;
    unsigned short dtime : 10;
    unsigned long cycle_number : 24;
    unsigned long pulse_in_cycle : 20;

};

class ptu_line
{
public:
    FILE* orig_ptu, * line_ptu;
    std::vector<TgHd> headers;
    std::vector<sTTTR> events;

};



// TDateTime (in file) to time_t (standard C) conversion
const int EpochDiff = 25569; // days between 30/12/1899 and 01/01/1970
const int SecsInDay = 86400; // number of seconds in a day

time_t TDateTime_TimeT(double Convertee)
{
    time_t Result((long)(((Convertee)-EpochDiff) * SecsInDay));
    return Result;
}

FILE* fpin, * fpout;
bool IsT2;
long long RecNum;
long long oflcorrection;
long long truensync, truetime;
int m, c;
double GlobRes = 0.0;
double Resolution = 0.0;
unsigned int dlen = 0;
unsigned int cnt_0 = 0, cnt_1 = 0;

// TimeHarp260 v2 T3 input
void ProcessTH260v2(unsigned int TTTRRecord)
{
    const int T3WRAPAROUND = 1024;
    union {
        unsigned int allbits;
        struct {
            unsigned nsync : 10;  // numer of sync period
            unsigned dtime : 15;    // delay from last sync in units of chosen resolution
            unsigned channel : 6;
            unsigned special : 1;
        } bits;
    } T3Rec;
    T3Rec.allbits = TTTRRecord;
    if (T3Rec.bits.special == 1)
    {
        if (T3Rec.bits.channel == 0x3F) //overflow
        {
            //number of overflows is stored in 
            oflcorrection += (int64_t)T3WRAPAROUND * T3Rec.bits.nsync;
            //GotOverflow(T3Rec.bits.nsync);
        }
        if ((T3Rec.bits.channel >= 1) && (T3Rec.bits.channel <= 15)) //markers
        {
            truensync = oflcorrection + T3Rec.bits.nsync;
            //the time unit depends on sync period which can be obtained from the file header
            c = T3Rec.bits.channel;

            //GotMarker(truensync, c);
        }
    }
    else //regular input channel
    {
        truensync = oflcorrection + T3Rec.bits.nsync;
        //the nsync time unit depends on sync period which can be obtained from the file header
        //the dtime unit depends on the resolution and can also be obtained from the file header
        c = T3Rec.bits.channel;
        m = T3Rec.bits.dtime;
        //GotPhoton(truensync, c, m);
    }
}

//void ProcessHeader()

int main(int argc, char* argv[])
{
    char Magic[8];
    char Version[8];
    char Buffer[40];
    char* AnsiBuffer;
    wchar_t* WideBuffer;
    int Result;

    long long NumRecords = -1;
    long long RecordType = 0;
    int RecordLength; // Length of an Record, by default 4 bytes, But LIN Camera has 8 bytes

    printf("\nPicoQuant TimeHarp260 v2 T3 File Processing");
    printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    if ((argc < 2) || (argc > 2))
    {
        printf("usage: tttr_funcs infile\n");
        printf("infile is a PQ .ptu T3 file from TimeHarp260 or HydraHarp, hardware version v2\n");
        //printf("outfile is ASCII\n");
        getchar();
        exit(-1);
    }
    if ((fpin = fopen(argv[1], "rb")) == NULL)
    {
        printf("\n ERROR! Input file cannot be opened, aborting.\n"); goto ex;
    }

    //if ((fpout = fopen(argv[2], "w")) == NULL)
    //{
    //    printf("\n ERROR! Output file cannot be opened, aborting.\n"); goto ex;
    //}

    printf("\n Loading data from %s \n", argv[1]);
    //printf("\n Writing output to %s \n", argv[2]);

    //  from
    Result = fread(&Magic, 1, sizeof(Magic), fpin);
    if (Result != sizeof(Magic))
    {
        printf("\nerror reading header, aborted.");
        goto close;
    }
    Result = fread(&Version, 1, sizeof(Version), fpin);
    if (Result != sizeof(Version))
    {
        printf("\nerror reading header, aborted.");
        goto close;
    }
    if (strncmp(Magic, "PQTTTR", 6))
    {
        printf("\nWrong Magic, this is not a PTU file.");
        goto close;
    }
    fprintf(fpout, "Tag Version: %s \n", Version);

    // read tagged header
    do
    {
        // This loop is very generic. It reads all header items and displays the identifier and the
        // associated value, quite independent of what they mean in detail.
        // Only some selected items are explicitly retrieved and kept in memory because they are
        // needed to subsequently interpret the TTTR record data.

        Result = fread(&TagHead, 1, sizeof(TagHead), fpin);
        if (Result != sizeof(TagHead))
        {
            printf("\nIncomplete File.");
            goto close;
        }

        strcpy(Buffer, TagHead.Ident);
        if (TagHead.Idx > -1)
        {
            sprintf(Buffer, "%s(%d)", TagHead.Ident, TagHead.Idx);
        }
        fprintf(fpout, "\n%-40s", Buffer);
        switch (TagHead.Typ)
        {
        case tyEmpty8:
            fprintf(fpout, "<empty Tag>");
            break;
        case tyBool8:
            fprintf(fpout, "%s", bool(TagHead.TagValue) ? "True" : "False");
            break;
        case tyInt8:
            fprintf(fpout, "%lld", TagHead.TagValue);
            // get some Values we need to analyse records
            if (strcmp(TagHead.Ident, TTTRTagNumRecords) == 0) // Number of records
                NumRecords = TagHead.TagValue;
            if (strcmp(TagHead.Ident, TTTRTagTTTRRecType) == 0) // TTTR RecordType
                RecordType = TagHead.TagValue;
            break;
        case tyBitSet64:
            fprintf(fpout, "0x%16.16X", TagHead.TagValue);
            break;
        case tyColor8:
            fprintf(fpout, "0x%16.16X", TagHead.TagValue);
            break;
        case tyFloat8:
            fprintf(fpout, "%E", *(double*)&(TagHead.TagValue));
            if (strcmp(TagHead.Ident, TTTRTagRes) == 0) // Resolution for TCSPC-Decay
                Resolution = *(double*)&(TagHead.TagValue);
            if (strcmp(TagHead.Ident, TTTRTagGlobRes) == 0) // Global resolution for timetag
                GlobRes = *(double*)&(TagHead.TagValue); // in ns
            break;
        case tyFloat8Array:
            fprintf(fpout, "<Float Array with %d Entries>", TagHead.TagValue / sizeof(double));
            // only seek the Data, if one needs the data, it can be loaded here
            fseek(fpin, (long)TagHead.TagValue, SEEK_CUR);
            break;
        case tyTDateTime:
            time_t CreateTime;
            CreateTime = TDateTime_TimeT(*((double*)&(TagHead.TagValue)));
            fprintf(fpout, "%s", asctime(gmtime(&CreateTime)), "\0");
            break;
        case tyAnsiString:
            AnsiBuffer = (char*)calloc((size_t)TagHead.TagValue, 1);
            Result = fread(AnsiBuffer, 1, (size_t)TagHead.TagValue, fpin);
            if (Result != TagHead.TagValue)
            {
                printf("\nIncomplete File.");
                free(AnsiBuffer);
                goto close;
            }
            fprintf(fpout, "%s", AnsiBuffer);
            free(AnsiBuffer);
            break;
        case tyWideString:
            WideBuffer = (wchar_t*)calloc((size_t)TagHead.TagValue, 1);
            Result = fread(WideBuffer, 1, (size_t)TagHead.TagValue, fpin);
            if (Result != TagHead.TagValue)
            {
                printf("\nIncomplete File.");
                free(WideBuffer);
                goto close;
            }
            fwprintf(fpout, L"%s", WideBuffer);
            free(WideBuffer);
            break;
        case tyBinaryBlob:
            fprintf(fpout, "<Binary Blob contains %d Bytes>", TagHead.TagValue);
            // only seek the Data, if one needs the data, it can be loaded here
            fseek(fpin, (long)TagHead.TagValue, SEEK_CUR);
            break;
        default:
            printf("Illegal Type identifier found! Broken file?");
            goto close;
        }
    } while ((strncmp(TagHead.Ident, FileTagEnd, sizeof(FileTagEnd))));
    fprintf(fpout, "\n-----------------------\n");
    // End Header loading
      // TTTR Record type
    RecordLength = 4; // all Record Length 4 until now
    switch (RecordType)
    {
    case rtPicoHarpT2:
        fprintf(fpout, "PicoHarp T2 data\n");
        fprintf(fpout, "\nrecord# chan timetag truetime/ps\n");
        break;
    case rtPicoHarpT3:
        fprintf(fpout, "PicoHarp T3 data\n");
        fprintf(fpout, "\nrecord# chan   nsync truetime/ns dtime\n");
        break;
    case rtHydraHarpT2:
        fprintf(fpout, "HydraHarp V1 T2 data\n");
        fprintf(fpout, "\nrecord# chan timetag truetime/ps\n");
        break;
    case rtHydraHarpT3:
        fprintf(fpout, "HydraHarp V1 T3 data\n");
        fprintf(fpout, "\nrecord# chan   nsync truetime/ns dtime\n");
        break;
    case rtHydraHarp2T2:
        fprintf(fpout, "HydraHarp V2 T2 data\n");
        fprintf(fpout, "\nrecord# chan timetag truetime/ps\n");
        break;
    case rtHydraHarp2T3:
        fprintf(fpout, "HydraHarp V2 T3 data\n");
        fprintf(fpout, "\nrecord# chan   nsync truetime/ns dtime\n");
        break;
    case rtTimeHarp260NT3:
        fprintf(fpout, "TimeHarp260N T3 data\n");
        fprintf(fpout, "\nrecord# chan   nsync truetime/ns dtime\n");
        break;
    case rtTimeHarp260NT2:
        fprintf(fpout, "TimeHarp260N T2 data\n");
        fprintf(fpout, "\nrecord# chan timetag truetime/ps\n");
        break;
    case rtTimeHarp260PT3:
        fprintf(fpout, "TimeHarp260P T3 data\n");
        fprintf(fpout, "\nrecord# chan   nsync truetime/ns dtime\n");
        break;
    case rtTimeHarp260PT2:
        fprintf(fpout, "TimeHarp260P T2 data\n");
        fprintf(fpout, "\nrecord# chan timetag truetime/ps\n");
        break;
    case rtMultiHarpNT3:
        fprintf(fpout, "MultiHarp150N T3 data\n");
        fprintf(fpout, "\nrecord# chan   nsync truetime/ns dtime\n");
        break;
    case rtMultiHarpNT2:
        fprintf(fpout, "MultiHarp150N T2 data\n");
        fprintf(fpout, "\nrecord# chan timetag truetime/ps\n");
        break;
    default:
        fprintf(fpout, "Unknown record type: 0x%X\n 0x%X\n ", RecordType);
        goto close;
    }

    unsigned int TTTRRecord;
    uint64_t TTTRRecord64;
    for (RecNum = 0; RecNum < NumRecords; RecNum++)
    {
        if (RecordLength == 4)
        {
            Result = fread(&TTTRRecord, 1, sizeof(TTTRRecord), fpin);
            if (Result != sizeof(TTTRRecord))
            {
                printf("\nUnexpected end of input file!");
                break;
            }
        }
        else
        {
            Result = fread(&TTTRRecord64, 1, sizeof(TTTRRecord64), fpin);
            if (Result != sizeof(TTTRRecord64))
            {
                printf("\nUnexpected end of input file!");
                break;
            }
        }

        IsT2 = false;
        ProcessTH260v2(TTTRRecord);
    }

close:
    fclose(fpin);
    fclose(fpout);

ex:
    printf("\n any key...");
    getchar();
    exit(0);
    return(0);
}