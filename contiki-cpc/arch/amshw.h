/* Amstrad hardware I/O ports 
 
Although these I/O port addresses are actually partially decoded,
we use the recommended address here.
*/

/* Gate Array */
sfr banked at 0x07F00 IO_GateArray;

/* CRTC */
sfr banked at 0x0BC00 IO_CRTC_Register;
sfr banked at 0x0BD00 IO_CRTC_Data;

/* FDD */
sfr banked at 0x0FA7E IO_FDD_Motor;		/* motor state for all floppy disc drives */

/* FDC 

NOTE: The Amstrad Floppy Disc Controller is polled for data and has strict timing constraints.
Therefore it is best to perform all floppy access using assembler code, or using the firmware or
CP/M functions 
*/
sfr banked at 0x0FB7E IO_FDC_Status;	/* FDC Main Status register */
sfr banked at 0x0FB7F IO_FDC_Data;		/* FDC Data register */

/* PPI */
sfr banked at 0x0F400 IO_PPI_PortA;
sfr banked at 0x0F500 IO_PPI_PortB;
sfr banked at 0x0F600 IO_PPI_PortC;
sfr banked at 0x0F700 IO_PPI_Control;

