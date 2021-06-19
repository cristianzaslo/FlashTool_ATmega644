/*~-1*/
/*~XSF_LANGUAGE: C/C++*/
/*~A:1*/
/*~+:Header*/
/*~T*/
/* SOURCE FILE    <Main.c>

 **************************************************************************
 *  COPYRIGHT (C) Cristian Zaslo                                          *
 *                                                                        *
 *  The reproduction, transmission or use of this document or its         *
 *  contents is not permitted without express written authority.          *
 *  Offenders will be liable for damages. All rights, including rights    *
 *  created by patent grant or registration of a utility model or design, *
 *  are reserved.                                                         *
 **************************************************************************
 *  PROJECT       <AVR Flash Tool AT644>
 *  MODULE        <Main>
 *  RESPONSIBLE   <Cristian Zaslo>
 **************************************************************************
 *  PURPOSE:
 *  Program entry point
 *
 **************************************************************************
 *  REMARKS:
 *  none
 *
 **************************************************************************
 *  CHANGE HISTORY:
 *
 *  Rev 1.0   Apr 19 2021  CrZa
 *  Initial revision.
 *
 *
 *************************************************************************/



/*~E:A1*/
/*~A:2*/
/*~+:Includes*/
/*~T*/
#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include "Main.h"
  
/*~E:A2*/
/*~A:3*/
/*~+:Types*/
/*~T*/

/*~E:A3*/
/*~A:4*/
/*~+:Constants*/
/*~T*/
/* maximum rows in a 64kb hex file -> 4096 x 16 = 65536 */
#define MA_w_MAX_HEX_ROWS      (WORD)4096

/* maximum application size is 4032 x 16 = 63 kb */
#define MA_w_MAX_HEX_APP_ROWS  (WORD)4032

/* maximum columns in a hex file -> 43 + \n + \0 = 45 */
#define MA_w_MAX_HEX_COLUMNS   (WORD)45

/* port name size */
#define MA_w_PORT_NAME         (WORD)9

/* file name size */
#define MA_w_FILE_NAME         (WORD)30

/* page size in bytes */
#define MA_w_PAGE_SIZE         (WORD)256

/* number of FBL pages -> 256 - 4 (FBL size) */
#define MA_w_PAGE_NUMBER       (WORD)252

/* read buffer size 43 chars + \n + \0 */
#define MA_w_READ_BUFFER_SIZE  (WORD)45

/*~E:A4*/
/*~A:5*/
/*~+:Variables*/
/*~T*/
/* array to store a raw hexfile of 64kb */
CHAR MA_ac_HexFileRaw [MA_w_MAX_HEX_ROWS][MA_w_MAX_HEX_COLUMNS];

/* hex file processed */
CHAR MA_ac_HexFileProc [MA_w_PAGE_NUMBER][MA_w_PAGE_SIZE];

/* input buffer */
CHAR MA_ac_Read_Buffer [MA_w_READ_BUFFER_SIZE];

/*~E:A5*/
/*~A:6*/
/*~+:Macros*/
/*~T*/

/*~E:A6*/
/*~A:7*/
/*~+:Flags / Bit Masks*/
/*~T*/

/*~E:A7*/
/*~A:8*/
/*~+:Function prototypes*/
/*~T*/
/* function declaration */
static WORD MA_w_Letter2Number (const CHAR c_Char);
/*~E:A8*/
/*~A:9*/
/*~+:static WORD MA_w_Letter2Number (const CHAR c_Char)*/
/*~F:10*/
static WORD MA_w_Letter2Number (const CHAR c_Char)
/*~-1*/
{
   /*~I:11*/
   /* letter ? */
   if ((c_Char >= 'A') && (c_Char <= 'F'))
   /*~-1*/
   {
      /*~T*/
      /* return */
      return ((WORD)c_Char - (WORD)55);
   /*~-1*/
   }
   /*~O:I11*/
   /*~-2*/
   else
   {
      /*~I:12*/
      /* number ? */
      if ((c_Char >= '0') && (c_Char <= '9'))
      /*~-1*/
      {
         /*~T*/
         /* return */
         return ((WORD)c_Char - (WORD)48);
      /*~-1*/
      }
      /*~O:I12*/
      /*~-2*/
      else
      {
         /*~T*/
         /* error trap */
         assert (0);
      /*~-1*/
      }
      /*~E:I12*/
   /*~-1*/
   }
   /*~E:I11*/
/*~-1*/
}
/*~E:F10*/
/*~E:A9*/
/*~A:13*/
/*~+:int main (int argc, char *argv[])*/
/*~A:14*/
/*~+:Function header*/
/*~T*/
/*  FUNCTION    <main>

 **************************************************************************
 *  PURPOSE:    program entry point
 *   
 *
 *  PARAMETERS:
 *
 *
 *
 *  RETURN:     0 if successful, -1 if error
 *
 **************************************************************************
 *  REMARKS:
 *
 *
 *************************************************************************/
/*~E:A14*/
/*~F:15*/
int main(int argc, char *argv[])
/*~-1*/
{
   /*~A:16*/
   /*~+:variables*/
   /*~T*/
   /* comm handle */
   HANDLE h_Comm;

   /* serial params */
   DCB  dcb_SerialParams;

   /* timeouts structure */
   COMMTIMEOUTS com_timeouts;

   /* read in character */
   CHAR  c_ReadChar;

   /* event mask */
   DWORD dw_EventMask;

   /* counter */
   WORD w_Loop;

   /* number bytes read in */
   DWORD dw_NoBytesRead;

   /* number bytes written out */
   DWORD dw_NoBytesWritten;

   /* input hex file */
   FILE *ptl_HexFile;

   /* port name \\.\COMX */
   CHAR ac_PortName [MA_w_PORT_NAME] = "\\\\.\\COMX";

   /* file name */
   CHAR ac_FileName [MA_w_FILE_NAME];

   /* buffer to send and receive data */
   CHAR ac_SerialBuffer [MA_w_PAGE_SIZE];

   /* status */
   BOOL b_Status;

   /* indexes */
   WORD w_Index1, w_Index2, w_Index3;

   /* page number */
   WORD w_Page;

   /* index in page */
   WORD w_PageIndex;

   /* checksum total */
   WORD w_Checksum;

   /* checksum low byte and high byte */
   UCHAR uc_Checksum_Low;
   UCHAR uc_Checksum_High;

   /*~E:A16*/
   /*~A:17*/
   /*~+:initialization*/
   /*~L:18*/
   /* loop over rows */
   for (w_Index1 = 0; w_Index1 < MA_w_MAX_HEX_ROWS; w_Index1++)
   /*~-1*/
   {
      /*~L:19*/
      /* loop over columns */
      for (w_Index2 = 0; w_Index2 < MA_w_MAX_HEX_COLUMNS; w_Index2++)
      /*~-1*/
      {
         /*~T*/
         /* prefill with 'F' */
         MA_ac_HexFileRaw [w_Index1][w_Index2] = 'F';
      /*~-1*/
      }
      /*~E:L19*/
   /*~-1*/
   }
   /*~E:L18*/
   /*~E:A17*/
   /*~A:20*/
   /*~+:read in COM number*/
   /*~T*/
   /* welcome message */
   printf ("*** AVR ATMEGA644 Flash Tool 1.0 ***\n");

   /* select COM number */
   printf ("Enter port number 1 - 9: ");

   /* read COM number */
   scanf ("%c", ac_PortName + 7);

   /* print assigned port number */
   printf ("Assinged port number: %s\n", ac_PortName);

   /*~T*/
   /* open port COM */
   h_Comm = CreateFileA  (ac_PortName,                 //port name
                         GENERIC_READ | GENERIC_WRITE, //Read/Write
                         0,                            // No Sharing
                         NULL,                         // No Security
                         OPEN_EXISTING,// Open existing port only
                         0,            // Non Overlapped I/O
                         NULL);        // Null for Comm Devices


   /*~I:21*/
   /* invalid handle ? */
   if (h_Comm == INVALID_HANDLE_VALUE)
   /*~-1*/
   {
      /*~T*/
      /* print error */
      printf ("Error in opening serial port.\n");

      /* return error */
      goto exit_1;
   /*~-1*/
   }
   /*~O:I21*/
   /*~-2*/
   else
   {
      /*~T*/
      /* continue */
      printf ("Opening serial port successful.\n");

   /*~-1*/
   }
   /*~E:I21*/
   /*~E:A20*/
   /*~A:22*/
   /*~+:read in hex file*/
   /*~T*/
   /* select file name */
   printf ("Enter hexfile name: ");

   /* make sure file name is long enough */
   assert (MA_w_FILE_NAME > 25);

   /* read file name, clip to 25 chars */
   scanf ("%25s", ac_FileName);

   /* print filename */
   printf ("Assigned hexfile name: %s\n", ac_FileName);

   /*~I:23*/
   /* try to open input file */
   if ((ptl_HexFile = fopen (ac_FileName, "r")) == NULL)
   /*~-1*/
   {
      /*~T*/
      /* print error */
      printf ("Error cannot open input file.\n");

      /* return error */
      goto exit_1;
   /*~-1*/
   }
   /*~O:I23*/
   /*~-2*/
   else
   {
      /*~T*/
      /* continue */
   /*~-1*/
   }
   /*~E:I23*/
   /*~L:24*/
   /* read input line */
   while (fgets (MA_ac_Read_Buffer, MA_w_READ_BUFFER_SIZE, ptl_HexFile) != NULL)
   /*~-1*/
   {
      /*~I:25*/
      /* invalid line ? */
      if ((MA_ac_Read_Buffer [1] == '0') && (MA_ac_Read_Buffer [2] == '0'))
      /*~-1*/
      {
         /*~T*/
         /* skip */
         continue;
      /*~-1*/
      }
      /*~O:I25*/
      /*~-2*/
      else
      {
         /*~T*/
         /* continue */
      /*~-1*/
      }
      /*~E:I25*/
      /*~T*/
      /* compute 16-byte page number */
      w_Index1 = MA_w_Letter2Number (MA_ac_Read_Buffer [3]) * 256 + MA_w_Letter2Number (MA_ac_Read_Buffer [4]) * 16 + MA_w_Letter2Number (MA_ac_Read_Buffer [5]);

      /*~L:26*/
      /* override \n and \0 characters */
      for (w_Index2 = 0; w_Index2 < (MA_w_READ_BUFFER_SIZE - 2); w_Index2++)
      /*~-1*/
      {
         /*~I:27*/
         /* exit if \n encountered after checksum field */
         if ((MA_ac_Read_Buffer [w_Index2 + 2] == '\n'))
         /*~-1*/
         {
            /*~T*/
            /* break */
            /* write FF before (override checksum) */
            break;
         /*~-1*/
         }
         /*~O:I27*/
         /*~-2*/
         else
         {
            /*~T*/
            /* continue */
         /*~-1*/
         }
         /*~E:I27*/
         /*~T*/
         /* copy to new array */
         MA_ac_HexFileRaw [w_Index1][w_Index2] = MA_ac_Read_Buffer [w_Index2]; 
      /*~-1*/
      }
      /*~E:L26*/
   /*~-1*/
   }
   /*~E:L24*/
   /*~T*/
   /* close file */
   fclose (ptl_HexFile);
   /*~T*/
   /* print filename */
   printf ("Hexfile %s read in.\n", ac_FileName);

   /*~E:A22*/
   /*~A:28*/
   /*~+:process hex file*/
   /*~T*/
   /* init page */
   w_Page = 0;
   /*~L:29*/
   /* override \n and \0 characters */
   for (w_Index1 = 0; w_Index1 < MA_w_MAX_HEX_APP_ROWS; w_Index1 += 16)
   /*~-1*/
   {
      /*~T*/
      /* index in page */
      w_PageIndex = 0;

      /*~L:30*/
      /* take 16 rows x 16 bytes -> 256 bytes */
      for (w_Index2 = 0; w_Index2 < 16; w_Index2++)
      /*~-1*/
      {
         /*~L:31*/
         /* extract byte and store into page */
         for (w_Index3 = 9; w_Index3 < 40; w_Index3 += 2)
         /*~-1*/
         {
            /*~T*/
            /* store byte into page */
            MA_ac_HexFileProc [w_Page][w_PageIndex++] = MA_w_Letter2Number (MA_ac_HexFileRaw [w_Index1 + w_Index2][w_Index3]) * 16 + MA_w_Letter2Number (MA_ac_HexFileRaw [w_Index1 + w_Index2][w_Index3 + 1]);

         /*~-1*/
         }
         /*~E:L31*/
      /*~-1*/
      }
      /*~E:L30*/
      /*~T*/
      /* increase page number */
      w_Page++;
   /*~-1*/
   }
   /*~E:L29*/
   /*~E:A28*/
   /*~A:32*/
   /*~+:compute checksum*/
   /*~T*/
   /* init checksum */
   w_Checksum = 0;
   /*~L:33*/
   /* loop over 252 pages */
   for (w_Index1 = 0; w_Index1 < MA_w_PAGE_NUMBER; w_Index1++)
   /*~-1*/
   {
      /*~L:34*/
      /* loop over 256 bytes in a page */
      for (w_Index2 = 0; w_Index2 < MA_w_PAGE_SIZE; w_Index2++)
      /*~-1*/
      {
         /*~T*/
         /* update checksum */
         w_Checksum += (UCHAR)MA_ac_HexFileProc [w_Index1][w_Index2];
      /*~-1*/
      }
      /*~E:L34*/
   /*~-1*/
   }
   /*~E:L33*/
   /*~T*/
   /* extract low byte */
   uc_Checksum_Low = (UCHAR)w_Checksum % 256;

   /* prepare high byte extraction */
   w_Checksum >>= 8;

   /* extract high byte */
   uc_Checksum_High = (UCHAR)w_Checksum % 256;

   /*~T*/
   /* print checksum */
   printf ("Checksum low: %02X\n", uc_Checksum_Low);
   printf ("Checksum high: %02X\n",uc_Checksum_High);
    
   /*~E:A32*/
   /*~A:35*/
   /*~+:init communication parameters*/
   /*~T*/
   /* set length */
   dcb_SerialParams.DCBlength = sizeof(dcb_SerialParams);
   /*~I:36*/
   /* get comm state */
   if (GetCommState (h_Comm, &dcb_SerialParams))
   /*~-1*/
   {
      /*~T*/
      /* continue  */
   /*~-1*/
   }
   /*~O:I36*/
   /*~-2*/
   else
   {
      /*~T*/
      /* print error */
      printf ("Cannot get comm state.\n");

      /* return error */
      goto exit_2;

   /*~-1*/
   }
   /*~E:I36*/
   /*~T*/
   /* print old baudrate */
   printf ("Comm speed: %d\n", dcb_SerialParams.BaudRate);

   /* BaudRate = 9600 */
   dcb_SerialParams.BaudRate = CBR_9600;
   dcb_SerialParams.ByteSize = 8;             //ByteSize = 8
   dcb_SerialParams.StopBits = ONESTOPBIT;    //StopBits = 1
   dcb_SerialParams.Parity = NOPARITY;        //Parity = None

   /*~I:37*/
   /* set comm state */
   if (SetCommState (h_Comm, &dcb_SerialParams))
   /*~-1*/
   {
      /*~T*/
      /* continue */
   /*~-1*/
   }
   /*~O:I37*/
   /*~-2*/
   else
   {
      /*~T*/
      /* print error */
      printf ("Cannot set comm state.\n");

      /* return error */
      goto exit_2;

   /*~-1*/
   }
   /*~E:I37*/
   /*~T*/
   /* print new comm speed */
   printf ("New comm speed: %d\n", dcb_SerialParams.BaudRate);
     
   /*~T*/
   /* setting timeouts */
   com_timeouts.ReadIntervalTimeout = 50;
   com_timeouts.ReadTotalTimeoutConstant = 50;
   com_timeouts.ReadTotalTimeoutMultiplier = 50;
   com_timeouts.WriteTotalTimeoutConstant = 50;
   com_timeouts.WriteTotalTimeoutMultiplier = 50;
   /*~I:38*/
   /* get comm state */
   if (SetCommTimeouts (h_Comm, &com_timeouts))
   /*~-1*/
   {
      /*~T*/
      /* continue  */
   /*~-1*/
   }
   /*~O:I38*/
   /*~-2*/
   else
   {
      /*~T*/
      /* print error */
      printf ("Error in setting timeouts.\n");

      /* return error */
      goto exit_2;

   /*~-1*/
   }
   /*~E:I38*/
   /*~E:A35*/
   /*~A:39*/
   /*~+:read 0x55*/
   /*~T*/
   /* print new line */
   printf ("............................\n");

   /*~T*/
   /* setting receive mask */
   b_Status = SetCommMask (h_Comm, EV_RXCHAR);

   /*~I:40*/
   /* error ? */
   if (b_Status == FALSE)
   /*~-1*/
   {
      /*~T*/
      /* print error */
      printf ("Error in Setting CommMask.\n");

      /* return error */
      goto exit_2;

   /*~-1*/
   }
   /*~O:I40*/
   /*~-2*/
   else
   {
      /*~T*/
      /* continue */
   /*~-1*/
   }
   /*~E:I40*/
   /*~T*/
   /* print error */
   printf ("Waiting for 0x55 char...\n");

   /*~T*/
   /* setting WaitComm() Event */
   b_Status = WaitCommEvent (h_Comm, &dw_EventMask, NULL);
   /* Wait for the character to be received */

   /*~I:41*/
   /* error ? */
   if (b_Status == FALSE)
   /*~-1*/
   {
      /*~T*/
      /* print error */
      printf ("Error in WaitCommEvent.\n");

      /* return error */
      goto exit_2;

   /*~-1*/
   }
   /*~O:I41*/
   /*~-2*/
   else
   {
      /*~T*/
      /* continue */
   /*~-1*/
   }
   /*~E:I41*/
   /*~T*/
   /* read char */
   b_Status = ReadFile (h_Comm, &c_ReadChar, sizeof (c_ReadChar), &dw_NoBytesRead, NULL);

   /*~I:42*/
   /* Rx complete ? */
   if (b_Status && (dw_NoBytesRead == sizeof (c_ReadChar)) && (c_ReadChar == '\x55'))
   /*~-1*/
   {
      /*~T*/
      /* print error */
      printf ("Arrived 0x55 char.\n");

   /*~-1*/
   }
   /*~O:I42*/
   /*~-2*/
   else
   {
      /*~T*/
      /* print error */
      printf ("Incorrect 0x55 char.\n");

      /* return error */
      goto exit_2;

   /*~-1*/
   }
   /*~E:I42*/
   /*~E:A39*/
   /*~A:43*/
   /*~+:send 0x55 + Checksum LOW + Checksum HIGH*/
   /*~T*/
   /* print error */
   printf ("Sending 0x55 & Checksum.\n");
   ac_SerialBuffer [0] = 0x55;
   ac_SerialBuffer [1] = uc_Checksum_Low;
   ac_SerialBuffer [2] = uc_Checksum_High;

   /*~T*/
   /* writing data to Serial Port */
   b_Status = WriteFile (h_Comm,// Handle to the Serialport
                      ac_SerialBuffer,            // Data to be written to the port
                      3,   // No of bytes to write into the port
                      &dw_NoBytesWritten,  // No of bytes written to the port
                      NULL);

   /*~I:44*/
   /* write failed ? */
   if (b_Status == FALSE)
   /*~-1*/
   {
      /*~T*/
      /* print error */
      printf ("Failed to write header.\n");

      /* return error */
      goto exit_2;

   /*~-1*/
   }
   /*~O:I44*/
   /*~-2*/
   else
   {
      /*~T*/
      /* print numbers of byte written to the serial port */
      printf ("Header written %d bytes.\n", dw_NoBytesWritten);
   /*~-1*/
   }
   /*~E:I44*/
   /*~E:A43*/
   /*~A:45*/
   /*~+:read 0x55*/
   /*~T*/
   /* print error */
   printf ("Waiting for 0x55 char...\n");

   /*~T*/
   /* setting WaitComm() Event */
   b_Status = WaitCommEvent (h_Comm, &dw_EventMask, NULL);
   /* Wait for the character to be received */

   /*~I:46*/
   /* error ? */
   if (b_Status == FALSE)
   /*~-1*/
   {
      /*~T*/
      /* print error */
      printf ("Error in WaitCommEvent.\n");

      /* return error */
      goto exit_2;

   /*~-1*/
   }
   /*~O:I46*/
   /*~-2*/
   else
   {
      /*~T*/
      /* continue */
   /*~-1*/
   }
   /*~E:I46*/
   /*~T*/
   /* read char */
   b_Status = ReadFile (h_Comm, &c_ReadChar, sizeof (c_ReadChar), &dw_NoBytesRead, NULL);

   /*~I:47*/
   /* Rx complete ? */
   if (b_Status && (dw_NoBytesRead == sizeof (c_ReadChar)) && (c_ReadChar == '\x55'))
   /*~-1*/
   {
      /*~T*/
      /* print error */
      printf ("Arrived 0x55 char.\n");

   /*~-1*/
   }
   /*~O:I47*/
   /*~-2*/
   else
   {
      /*~T*/
      /* print error */
      printf ("Incorrect 0x55 char.\n");

      /* return error */
      goto exit_2;

   /*~-1*/
   }
   /*~E:I47*/
   /*~E:A45*/
   /*~A:48*/
   /*~+:Write hex file 252 pages*/
   /*~L:49*/
   /* loop over 252 pages -> 63kb */
   for (w_Page = 0; w_Page < MA_w_PAGE_NUMBER; w_Page++)
   /*~-1*/
   {
      /*~L:50*/
      /* loop over 256 bytes */
      for (w_Index1 = 0; w_Index1 < MA_w_PAGE_SIZE; w_Index1++)
      /*~-1*/
      {
         /*~T*/
         /* copy into Tx buffer */
         ac_SerialBuffer [w_Index1] = MA_ac_HexFileProc [w_Page][w_Index1];
      /*~-1*/
      }
      /*~E:L50*/
      /*~T*/
      /* Writing data to Serial Port */
      b_Status = WriteFile (h_Comm,// Handle to the Serialport
                         ac_SerialBuffer,            // Data to be written to the port
                         sizeof (ac_SerialBuffer),   // No of bytes to write into the port
                         &dw_NoBytesWritten,  // No of bytes written to the port
                         NULL);

      /*~I:51*/
      /* write failed ? */
      if (b_Status == FALSE)
      /*~-1*/
      {
         /*~T*/
         /* print error */
         printf ("Failed to write data page %d.\n",w_Page);

         /* return error */
         goto exit_2;

      /*~-1*/
      }
      /*~O:I51*/
      /*~-2*/
      else
      {
         /*~T*/
         /* print numbers of byte written to the serial port */
         printf ("Data page %d written %d bytes.\n",w_Page, dw_NoBytesWritten);
      /*~-1*/
      }
      /*~E:I51*/
      /*~K*/
      /*~+:.......... Wait for feedback ..........*/
      /*~T*/
      /* setting WaitComm() Event */
      b_Status = WaitCommEvent (h_Comm, &dw_EventMask, NULL);
      /* wait for the character to be received */

      /*~I:52*/
      /* error ? */
      if (b_Status == FALSE)
      /*~-1*/
      {
         /*~T*/
         /* print error */
         printf ("Error in WaitCommEvent.\n");

         /* return error */
         goto exit_2;

      /*~-1*/
      }
      /*~O:I52*/
      /*~-2*/
      else
      {
         /*~T*/
         /* continue */
      /*~-1*/
      }
      /*~E:I52*/
      /*~T*/
      /* read char */
      b_Status = ReadFile (h_Comm, &c_ReadChar, sizeof (c_ReadChar), &dw_NoBytesRead, NULL);

      /*~I:53*/
      /* Rx complete ? */
      if (b_Status && (dw_NoBytesRead == sizeof (c_ReadChar)) && ((UCHAR)c_ReadChar == w_Page))
      /*~-1*/
      {
         /*~T*/
         /* print page */
         printf ("Acknowledged page %d.\n", w_Page);

      /*~-1*/
      }
      /*~O:I53*/
      /*~-2*/
      else
      {
         /*~T*/
         /* print error */
         printf ("Error: not acknowledged page %d.\n", w_Page);

         /* return error */
         goto exit_2;

      /*~-1*/
      }
      /*~E:I53*/
   /*~-1*/
   }
   /*~E:L49*/
   /*~E:A48*/
   /*~A:54*/
   /*~+:read programming status */
   /*~T*/
   /* setting WaitComm() Event */
   b_Status = WaitCommEvent (h_Comm, &dw_EventMask, NULL);
   /* wait for the character to be received */

   /*~I:55*/
   /* error ? */
   if (b_Status == FALSE)
   /*~-1*/
   {
      /*~T*/
      /* print error */
      printf ("Error in WaitCommEvent.\n");

      /* return error */
      goto exit_2;

   /*~-1*/
   }
   /*~O:I55*/
   /*~-2*/
   else
   {
      /*~T*/
      /* continue */
   /*~-1*/
   }
   /*~E:I55*/
   /*~T*/
   /* read char */
   b_Status = ReadFile (h_Comm, &c_ReadChar, sizeof (c_ReadChar), &dw_NoBytesRead, NULL);

   /*~I:56*/
   /* Rx complete ? */
   if (b_Status && (dw_NoBytesRead == sizeof (c_ReadChar)) && (c_ReadChar == '\x55'))
   /*~-1*/
   {
      /*~T*/
      /* print confirmation */
      printf ("Programming successful.\n");

   /*~-1*/
   }
   /*~O:I56*/
   /*~-2*/
   else
   {
      /*~T*/
      /* print error */
      printf ("Error: checksum failure.\n");

      /* return error */
      goto exit_2;

   /*~-1*/
   }
   /*~E:I56*/
   /*~T*/
   /* print new line */
   printf ("............................\n");

   /*~E:A54*/
   /*~A:57*/
   /*~+:exit*/
   /*~T*/
   /* terminate successfully */
   return 0;
   /*~T*/
   exit_1:

   /* return value */
   return 1;

   /*~T*/
   exit_2:

   /* closing the Serial Port */
   CloseHandle (h_Comm);

   /* return value */
   return 2;

   /*~E:A57*/
/*~-1*/
}
/*~E:F15*/
/*~E:A13*/
/*~T*/

