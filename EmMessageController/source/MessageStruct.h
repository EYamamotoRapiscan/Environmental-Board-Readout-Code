#ifndef MESSAGESTRUCT
#define MESSAGESTRUCT

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define C2KELVIN(x) (x+273.15)

typedef struct{
	/*
	 * temp1
	 * humidity
	 * press
	 * temp2
	 * battery v
	 * dc current
	 * ac current
	 * reserved
	 */
	float temp1;
	float humidity;
	int pressure;
	float temp2;
	float vBatt;
	float dcCurrent;
	float acCurrent;
	float ADC4;
	float pillarThermocouple;
	float pillarExtHumidity;
	float dewPoint;
	float RFU;
	int vm; //see if it's been filled
}SensorMessage;

float signedTemp(long t)
{
	float ret = t <= 2000 ? t*0.1 : (t - 65535)*0.1;
	ret += 273.15;
	return ret;
}

float signedThermoCoupleTemp(long t)
{
	if(t == 1381126725) return 0.; //return 0 kelvin if there's a sensor fault.
	if(t == 1163022930) return 0.;

	float calcT = t >= 16128 ? (float) (t - 16383.) : (float) t*1.;
	printf("calcT %f\n",calcT);
	return calcT/4. + 273.15;
}

float dewPointInCelsius(float tK, float rH)
{
	if (tK < 10) return 0;
	float tC = tK - 273.15;
	float lnRH = (float) log(rH/100.);
	float iVal = (17.625*tC)/(243.04+tC);
	float numerator = 243.04*(lnRH + iVal);
	float denominator = (17.625 - lnRH - iVal);

	return numerator/denominator;
}

void fillMessage(long *array, SensorMessage *m)
{
	long noiseThreshold = 20;
	m->temp1 = signedTemp(array[1]);
	m->humidity = array[2]*0.1;
	m->pressure = (int) array[3]*0.01;
	m->temp2 = signedTemp(array[4]);
	m->vBatt = array[5] < noiseThreshold ? 0 : 0.00669129*array[5] - 2.65918493;
	m->dcCurrent = array[6] < noiseThreshold ? 0 : -0.00686993*array[6]  + 14.48507166;
	//m->dcCurrent = ((2548 - (array[6]*5./3.3)))/(1000/.185);
	m->acCurrent = array[7]/100.;
	m->ADC4 = array[8];
	//printf("Array9: %i",array[9]);
	m->pillarThermocouple = signedThermoCoupleTemp(array[9]);
	m->pillarExtHumidity = array[10]*0.l;
	m->RFU = array[11];

	//  TD: =243.04*(LN(RH/100)+((17.625*T)/(243.04+T)))/(17.625-LN(RH/100)-((17.625*T)/(243.04+T)))
	m->dewPoint = dewPointInCelsius(m->temp1,m->humidity) + 273.15;
	m->vm = 1;
}

void printMessage(SensorMessage *m)
{
	printf("Temperature 1: %f K\n", m->temp1);
	printf("Humidity (RH): %f %\n", m->humidity);
	printf("Pressure: %i hPascals\n", m->pressure);
	printf("Temperature 2: %f K\n", m->temp2);
	printf("Battery Voltage: %f V\n", m->vBatt);
	printf("DC Current: %f A\n", m->dcCurrent);
	printf("AC Current: %f A\n", m->acCurrent);
	printf("ADC4: %f\n", m->ADC4);
	printf("Pillar Thermocouple T: %f K\n", m->pillarThermocouple);
	printf("Pillar Humidity: %f %\n",m->pillarExtHumidity);
	printf("Dew Point: %f C\n", m->dewPoint - 273.15);
	printf("Valid Reading? %i\n\n", m->vm);
}

void initializeSensorMessage(SensorMessage *m)
{
	m->temp1 = 0.;
	m->humidity = 0.;
	m->pressure = 0;
	m->temp2 = 0.;
	m->vBatt = 0.;
	m->dcCurrent = 0.;
	m->acCurrent = 0.;
	m->ADC4 = 0.;
	m->pillarExtHumidity = 0;
	m->pillarThermocouple = 0;
	m->dewPoint = 0.;
	m->vm = 0;
}

#endif
