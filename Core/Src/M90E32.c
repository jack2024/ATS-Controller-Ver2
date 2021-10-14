/* ATM90E36 Energy Monitor Functions
*/
#include "M90E32.h"
#include "spi.h"

void Delay_usec(uint32_t count);
////////////////M90E26///////////////////////////
uint16_t CommEnergyIC(uint8_t selectsource,unsigned char RW, uint16_t address, uint16_t val) {
	uint8_t SPIbuf[5];
	//unsigned char* data=(unsigned char*)&val;
	//unsigned short output;
	uint16_t Result;
  //SPI interface rate is 200 to 160k bps. It Will need to be slowed down for EnergyIC
  //switch MSB and LSB of value
	
	
  //Set read write flag
	if(RW)
	{
		address |= 0x8000;
	}
	else
	{
		address &= ~0x8000;
	}
	//  CS Select
	if(selectsource == SOURCE1)
	{
		HAL_GPIO_WritePin(SPI1_CS1_GPIO_Port,SPI1_CS1_Pin,GPIO_PIN_RESET);
	}
	else // Read V2
	{
		//address = ~address;
		//val = ~val;
		HAL_GPIO_WritePin(SPI2_CS_GPIO_Port,SPI2_CS_Pin,GPIO_PIN_RESET);
	}
	
  //Write address byte by byte
	Delay_usec(4);
  SPIbuf[1] = (uint8_t)address;	//lsb
	address = address>>8;
	SPIbuf[0] = (uint8_t)address;	//msb
	
	if(selectsource == SOURCE1)
	{
		HAL_SPI_Transmit(&hspi1, (uint8_t*)SPIbuf, 0x02, 0x05); 
	}
	else // Read V2
	{
		HAL_SPI_Transmit(&hspi2, (uint8_t*)SPIbuf, 0x02, 0x05); 
	}
	
	
  /* Must wait 4 us for data to become valid */
  Delay_usec(4);

  if(RW)
  {
		if(selectsource == SOURCE1){
			HAL_SPI_Receive(&hspi1, (uint8_t*)SPIbuf, 0x02, 0x05); 
		}
		else{
			HAL_SPI_Receive(&hspi2, (uint8_t*)SPIbuf, 0x02, 0x05); 
		}
		Result = SPIbuf[0];	//msb
		Result = Result<<8;
		Result |= SPIbuf[1];	//lsb
  }
  else
  {
		SPIbuf[1] = (uint8_t)val;	//lsb
		val = val>>8;
		SPIbuf[0] = (uint8_t)val;	//msb
		if(selectsource == SOURCE1)
		{
			HAL_SPI_Transmit(&hspi1, (uint8_t*)SPIbuf, 0x02, 0x05);
		}
		else
		{
			HAL_SPI_Transmit(&hspi2, (uint8_t*)SPIbuf, 0x02, 0x05);
		}
  }
	
	if(selectsource == SOURCE1)
	{
		HAL_GPIO_WritePin(SPI1_CS1_GPIO_Port,SPI1_CS1_Pin,GPIO_PIN_SET);
	}
	else // Read V2
	{
		HAL_GPIO_WritePin(SPI2_CS_GPIO_Port,SPI2_CS_Pin,GPIO_PIN_SET);
	}
	
	if(selectsource == SOURCE1)
	{
		return Result;
	}
	else // Read V2
	{
		//Result = ~Result;
		return Result;
	}
  //return Result;
}
float  GetLineVoltageA(unsigned char selectsource) {
  unsigned short voltage = CommEnergyIC(selectsource, 1, UrmsA, 0xFFFF);
	return (float)voltage / 238.5;
	
}

float  GetLineVoltageB(unsigned char selectsource) {
  unsigned short voltage = CommEnergyIC(selectsource, 1, UrmsB, 0xFFFF);
	return (float)voltage / 238.5;
	
}

float  GetLineVoltageC(unsigned char selectsource) {
  unsigned short voltage = CommEnergyIC(selectsource, 1, UrmsC, 0xFFFF);
	return (float)voltage / 238.5;
	
}

unsigned short  GetMeterStatus0(unsigned char selectsource) {
  return CommEnergyIC(selectsource, 1, EnStatus0, 0xFFFF);
}

unsigned short  GetMeterStatus1(unsigned char selectsource) {
  return CommEnergyIC(selectsource, 1, EnStatus1, 0xFFFF);
}
float GetLineCurrentA(unsigned char selectsource) {
  unsigned short current = CommEnergyIC(selectsource, 1, IrmsA, 0xFFFF);
  return (float)current * 7.13 / 1000;
}

float GetLineCurrentB(unsigned char selectsource) {
  unsigned short current = CommEnergyIC(selectsource, 1, IrmsB, 0xFFFF);
  return (float)current * 7.13 / 1000;
}

float GetLineCurrentC(unsigned char selectsource) {
  unsigned short current = CommEnergyIC(selectsource, 1, IrmsC, 0xFFFF);
  return (float)current * 7.13 / 1000;
}

float GetActivePowerA(unsigned char selectsource) {
  short int apower = (short int)CommEnergyIC(selectsource, 1, PmeanA, 0xFFFF); //Complement, MSB is signed bit
  return (float)apower * 2.94;
}

float GetFrequency(unsigned char selectsource) {
  unsigned short freq = CommEnergyIC(selectsource, 1, Freq, 0xFFFF);
  return (float)freq / 100;
}

float GetPowerFactor(unsigned char selectsource) {
  short int pf = (short int)CommEnergyIC(selectsource, 1, PFmeanT, 0xFFFF); //MSB is signed bit
  //if negative
  if (pf & 0x8000) {
    pf = (pf & 0x7FFF) * -1;
  }
  return (float)pf / 1000;
}

float GetImportEnergy(unsigned char selectsource) {
  //Register is cleared after reading
  unsigned short ienergy = CommEnergyIC(selectsource, 1, APenergyA, 0xFFFF);
  return (float)ienergy / 10 / 1000; //returns kWh if PL constant set to 1000imp/kWh
}

float GetExportEnergy(unsigned char selectsource) {
  //Register is cleared after reading
  unsigned short eenergy = CommEnergyIC(selectsource, 1, ANenergyT, 0xFFFF);
  return (float)eenergy / 10 / 1000; //returns kWh if PL constant set to 1000imp/kWh
}

unsigned short GetSysStatus0(unsigned char selectsource) {
  return CommEnergyIC(selectsource, 1, EMMState0, 0xFFFF);
}

unsigned short GetSysStatus1(unsigned char selectsource) {
  return CommEnergyIC(selectsource, 1, EMMState1, 0xFFFF);
}

void InitEnergyIC(unsigned char selectsource) {
//  unsigned short systemstatus0;
	
	//CommEnergyIC(selectsource, 0, MeterEn, 0x0001); //Perform EnMeter ---jj
	//CommEnergyIC(selectsource, 0, SoftReset, 0x789A); //Perform soft reset
	
	CommEnergyIC(selectsource, 0, SoftReset, 0x789A);     // 70 Perform soft reset
	CommEnergyIC(selectsource, 0, CfgRegAccEn, 0x55AA);   // 7F enable register config access
	CommEnergyIC(selectsource, 0, MeterEn, 0x0001);       // 00 Enable Metering
	
	CommEnergyIC(selectsource, 0, EMMIntEn0, 0xB76F);     // 75 Enable interrupts
  CommEnergyIC(selectsource, 0, EMMIntEn1, 0xDDFD);     // 76 Enable interrupts
  CommEnergyIC(selectsource, 0, EMMIntState0, 0x0001);  // 73 Clear interrupt flags
  CommEnergyIC(selectsource, 0, EMMIntState1, 0x0001);  // 74 Clear interrupt flags
	
  //CommEnergyIC(0, FuncEn0, 0x0030); //Voltage sag irq=1, report on warnout pin=1, energy dir change irq=0
  //CommEnergyIC(0, FuncEn1, 0x0030); //Voltage sag irq=1, report on warnout pin=1, energy dir change irq=0
  //CommEnergyIC(selectsource, 0, SagTh, 0x1F2F); //Voltage sag threshhold
  
  //Set metering config values
  CommEnergyIC(selectsource, 0, ConfigStart, 0x5678); //Metering calibration startup command. Register 31 to 3B need to be set
  CommEnergyIC(selectsource, 0, PLconstH, 0x00B9); //PL Constant MSB
  CommEnergyIC(selectsource, 0, PLconstL, 0xC1F3); //PL Constant LSB
  CommEnergyIC(selectsource, 0, MMode0, 0x0087); //Metering Mode Configuration. All defaults. See pg 58 of datasheet.
  CommEnergyIC(selectsource, 0, MMode1, 0x5555); //PGA Gain Configuration. x2 for DPGA and PGA. See pg 59 of datasheet
  CommEnergyIC(selectsource, 0, PStartTh, 0x08BD); //Active Startup Power Threshold
  CommEnergyIC(selectsource, 0, QStartTh, 0x0AEC); //Reactive Startup Power Threshold
  CommEnergyIC(selectsource, 0, CSZero, 0x5F59); //Write CSOne, as self calculated
  
  //Serial.print("Checksum 0:");
  //Serial.println(CommEnergyIC(1, CSZero, 0x0000), HEX); //Checksum 0. Needs to be calculated based off the above values.
  CommEnergyIC(selectsource, 1, CSZero, 0x0000);
	
  //Set metering calibration values
  CommEnergyIC(selectsource, 0, CalStart, 0x5678); //Metering calibration startup command. Register 41 to 4D need to be set
  CommEnergyIC(selectsource, 0, GainA, 0x1D39); //Line calibration gain
  CommEnergyIC(selectsource, 0, PhiA, 0x0000); //Line calibration angle
  CommEnergyIC(selectsource, 0, GainB, 0x1D39); //Line calibration gain
  CommEnergyIC(selectsource, 0, PhiB, 0x0000); //Line calibration angle
  CommEnergyIC(selectsource, 0, GainC, 0x1D39); //Line calibration gain
  CommEnergyIC(selectsource, 0, PhiC, 0x0000); //Line calibration angle
  CommEnergyIC(selectsource, 0, PoffsetA, 0x0000); //A line active power offset
  CommEnergyIC(selectsource, 0, QoffsetA, 0x0000); //A line reactive power offset
  CommEnergyIC(selectsource, 0, PoffsetB, 0x0000); //B line active power offset
  CommEnergyIC(selectsource, 0, QoffsetB, 0x0000); //B line reactive power offset
  CommEnergyIC(selectsource, 0, PoffsetC, 0x0000); //C line active power offset
  CommEnergyIC(selectsource, 0, QoffsetC, 0x0000); //C line reactive power offset
  CommEnergyIC(selectsource, 0, CSOne, 0x2402); //Write CSOne, as self calculated
  
  //Serial.print("Checksum 1:");
  //Serial.println(CommEnergyIC(1, CSOne, 0x0000), HEX); //Checksum 1. Needs to be calculated based off the above values.
	CommEnergyIC(selectsource, 1, CSOne, 0x0000);

  //Set measurement calibration values
	
//  CommEnergyIC(selectsource, 0, AdjStart, 0x5678); //Measurement calibration startup command, registers 61-6F
//  CommEnergyIC(selectsource, 0, UgainA, 0xD8E9);  //A SVoltage rms gain
//  CommEnergyIC(selectsource, 0, IgainA, 0x1BC9); //A line current gain
//  CommEnergyIC(selectsource, 0, UoffsetA, 0x0000); //A Voltage offset
//  CommEnergyIC(selectsource, 0, IoffsetA, 0x0000); //A line current offset
//  CommEnergyIC(selectsource, 0, UgainB, 0xD8E9);  //B Voltage rms gain
//  CommEnergyIC(selectsource, 0, IgainB, 0x1BC9); //B line current gain
//  CommEnergyIC(selectsource, 0, UoffsetB, 0x0000); //B Voltage offset
//  CommEnergyIC(selectsource, 0, IoffsetB, 0x0000); //B line current offset
//  CommEnergyIC(selectsource, 0, UgainC, 0xD8E9);  //C Voltage rms gain
//  CommEnergyIC(selectsource, 0, IgainC, 0x1BC9); //C line current gain
//  CommEnergyIC(selectsource, 0, UoffsetC, 0x0000); //C Voltage offset
//  CommEnergyIC(selectsource, 0, IoffsetC, 0x0000); //C line current offset
//  CommEnergyIC(selectsource, 0, CSThree, 0xA694); //Write CSThree, as self calculated

  //Serial.print("Checksum 3:");
  //Serial.println(CommEnergyIC(1, CSThree, 0x0000), HEX); //Checksum 3. Needs to be calculated based off the above values.
	CommEnergyIC(selectsource, 1, CSThree, 0x0000);
	
  CommEnergyIC(selectsource, 0, ConfigStart, 0x8765); //Checks correctness of 31-3B registers and starts normal metering if ok
  CommEnergyIC(selectsource, 0, CalStart, 0x8765); //Checks correctness of 41-4D registers and starts normal metering if ok
  CommEnergyIC(selectsource, 0, AdjStart, 0x8765); //Checks correct ness of 61-6F registers and starts normal measurement  if ok

	if(selectsource == SOURCE1)
	{
		CommEnergyIC(selectsource, 0, ZXConfig, 0); //0x9387 = ~0x6C78
	}
	else{ // SOURCE2
		CommEnergyIC(selectsource, 0, ZXConfig, 0x0);
	}

  //CommEnergyIC(selectsource, 0, ZXConfig, 0x6C78); //  jj
	//CommEnergyIC(selectsource, 0, ZXConfig, 0x9387);
	
	CommEnergyIC(selectsource, 0, CfgRegAccEn, 0x0000); // 7F end configuration
	
	//CommEnergyIC(selectsource, 0, SagTh, 0xBA54); //  jj 200v Threshold
	//CommEnergyIC(selectsource, 0, PhaseLossTh, 0xBA54); //  jj 200v Threshold
	//CommEnergyIC(selectsource, 0, EMMIntEn1, 0x7F00);

	//systemstatus0 = GetSysStatus0(selectsource);
	
}

void Delay_usec(uint32_t count)
{
	for(uint8_t i=0; i<count; i++)
  {
		for(uint8_t j=0; j<20; j++)
		{
			__ASM volatile ("NOP");
			__ASM volatile ("NOP");
			__ASM volatile ("NOP");
			__ASM volatile ("NOP");
			__ASM volatile ("NOP");
			__ASM volatile ("NOP");
			__ASM volatile ("NOP");
			__ASM volatile ("NOP");
			__ASM volatile ("NOP");
		}
	}
}


