#include "stm32f0xx_hal.h"

#define SoftReset 0x70 //Software Reset
#define MeterEn 0x00 //Software Reset

#define ChannelMapI 0x01 //System Status0
#define ChannelMapU 0x02 //System Status1

//#define FuncEn0 0x03 //Function Enable0
//#define FuncEn1 0x04 //Function Enable1

#define SagPeakDetCfg 0x05 // Sag and Peak Detector Period Configuration
#define OVth 0x06        //Over Voltage Threshold
#define ZXConfig 0x07    //Zero-Crossing Configuration
#define SagTh 0x08       //Voltage Sag Threshold
#define PhaseLossTh 0x09 //Voltage Phase Losing Threshold
#define InWarnTh 0x0A    //Neutral Current (Calculated) Warning Threshold
#define OIth 0x0B        // Over Current Threshold
#define FreqLoTh 0x0C    // Low Threshold for Frequency Detection
#define FreqHiTh 0x0D    // High Threshold for Frequency Detection
#define PMPwrCtrl 0x0E   //Partial Measurement Mode Power Control
#define IRQ0MergeCfg 0x0F//IRQ0 Merge Configuration

#define ConfigStart 0x30 //Configuration start command
#define PLconstH 0x31 //High Word of PL_Constant
#define PLconstL 0x32 //Low Word of PL_Constant
#define MMode0 0x33 //Metering Mode Configuration
#define MMode1 0x34 //Metering Mode Configuration
#define PStartTh 0x35 //Active Startup Power Threshold
#define QStartTh 0x36 //Reactive Startup Power Threshold
#define CSZero 0x3B	//Checksum 0

#define CalStart 0x40 //Calibration Start Command
#define PoffsetA 0x41 //L Line Active Power Offset
#define QoffsetA 0x42 //L Line Reactive Power Offset
#define PoffsetB 0x43 //L Line Active Power Offset
#define QoffsetB 0x44 //L Line Reactive Power Offset
#define PoffsetC 0x45 //L Line Active Power Offset
#define QoffsetC 0x46 //L Line Reactive Power Offset
#define GainA 0x47 //A Line Calibration Gain
#define PhiA 0x48  //A Line Calibration Angle
#define GainB 0x49 //B Line Calibration Gain
#define PhiB 0x4A  //B Line Calibration Angle
#define GainC 0x4B //C Line Calibration Gain
#define PhiC 0x4C  //C Line Calibration Angle
#define CSOne 0x4D //Checksum 1

#define AdjStart 0x60 //Measurement Calibration Start Command
#define UgainA 0x61 //A Voltage rms Gain
#define IgainA 0x62 //A Line Current rms Gain
#define UgainB 0x65 //B Voltage rms Gain
#define UgainC 0x69 //C Voltage rms Gain
#define IgainB 0x66 //B Line Current rms Gain
#define IgainC 0x6A //C Line Current rms Gain
#define UoffsetA 0x63 //Voltage Offset
#define UoffsetB 0x67 //Voltage Offset
#define UoffsetC 0x6B //Voltage Offset
#define IoffsetA 0x64 //L Line Current Offset
#define IoffsetB 0x68 //N Line Current Offse
#define IoffsetC 0x6C //N Line Current Offse
#define CSThree 0x6F //Checksum 3

#define EMMState0 0x71      //EMM State 0
#define EMMState1 0x72      //EMM State 1
#define EMMIntState0 0x73   //EMM Interrupt Status 0
#define EMMIntState1 0x74   //EMM Interrupt Status 1
#define EMMIntEn0 0x75      //EMM Interrupt Enable 0
#define EMMIntEn1 0x76      //EMM Interrupt Enable 1
#define LastSPIData 0x78    //Last Read/Write SPI Value
#define CRCErrStatus 0x79   //CRC Error Status
#define CRCDigest 0x7A      //CRC Digest
#define CfgRegAccEn 0x7F    //Configure Register Access Enable

#define APenergyA 0x81 //Forward Active Energy
#define APenergyB 0x82 //Forward Active Energy
#define APenergyC 0x83 //Forward Active Energy
#define ANenergyA 0x85 //Reverse Active Energy
#define ANenergyB 0x86 //Reverse Active Energy
#define ANenergyC 0x87 //Reverse Active Energy
#define ANenergyT 0x84 //Absolute Active Energy
#define RPenergyA 0x89 //Forward (Inductive) Reactive Energy
#define RPenergyB 0x8A //Forward (Inductive) Reactive Energy
#define RPenergyC 0x8B //Forward (Inductive) Reactive Energy
#define RnenergyA 0x8D //Reverse (Capacitive) Reactive Energy
#define RnenergyB 0x8E //Reverse (Capacitive) Reactive Energy
#define RnenergyC 0x8F //Reverse (Capacitive) Reactive Energy
#define RPenergyT 0x88 //Absolute Reactive Energy
#define EnStatus0 0x95 //Metering Status
#define EnStatus1 0x96 //Metering Status
#define IrmsA 0xDD //L Line Current rms A
#define IrmsB 0xDE //L Line Current rms B
#define IrmsC 0xDF //L Line Current rms C
#define UrmsA 0xD9 //Voltage rms A
#define UrmsB 0xDA //Voltage rms B
#define UrmsC 0xDB //Voltage rms C

#define PmeanT 0xB0 //Total Mean Active Power
#define PmeanA 0xB1 //A Line Mean Active Power
#define PmeanB 0xB2 //B Line Mean Active Power
#define PmeanC 0xB3 //C Line Mean Active Power
#define QmeanA 0xB5 //A Line Mean Reactive Power
#define QmeanB 0xB6 //B Line Mean Reactive Power
#define QmeanC 0xB7 //C Line Mean Reactive Power
#define SmeanA 0xB9 //A Line Mean Apparent Power
#define SmeanB 0xBA //B Line Mean Apparent Power
#define SmeanC 0xBB //C Line Mean Apparent Power
#define PFmeanT 0xBC //Mean Power Factor

#define F0 0x22//Voltage Frequency jj

#define Freq 0xF8//Voltage Frequency
#define PangleA 0xF9 //Phase Angle between Voltage and A Line Current
#define PangleB 0xFA //Phase Angle between Voltage and B Line Current
#define PangleC 0xFB //Phase Angle between Voltage and C Line Current

#define SOURCE1 0
#define SOURCE2 1

void Delay_usec(uint32_t count);

unsigned short CommEnergyIC(unsigned char  selectsource,unsigned char RW, unsigned short address, unsigned short val);
float  GetLineVoltageA(unsigned char selectsource);
float  GetLineVoltageB(unsigned char selectsource);
float  GetLineVoltageC(unsigned char selectsource);
float GetLineCurrentA(unsigned char selectsource);
float GetLineCurrentB(unsigned char selectsource);
float GetLineCurrentC(unsigned char selectsource);
float GetActivePower(unsigned char selectsource);
float GetFrequency(unsigned char selectsource);
float GetPowerFactor(unsigned char selectsource);
float GetImportEnergy(unsigned char selectsource);
float GetExportEnergy(unsigned char selectsource);
void InitEnergyIC(unsigned char selectsource);
unsigned short GetSysStatus0(unsigned char selectsource);
unsigned short GetSysStatus1(unsigned char selectsource);
unsigned short  GetMeterStatus0(unsigned char selectsource);
unsigned short  GetMeterStatus1(unsigned char selectsource);

uint8_t TestFunction(uint8_t address, uint8_t reg);
