#include <stm32G474.h>

//This is just a Pseudo Code, peripheral programming TBA

//init ADC continous conversion
//init DMA in a source ADC buffer, destination Memory
//If ADC reads 0V, disregard:
//ADC reads VPWM_LOW, it is


/*idea, the stm32 accepts PWM signals, PWM signals can be interrpretted as analogue signals, hence from the precespective
 * of the mcu at the input pin, at every i.e 1.67, there will either be 0v (no signal), 0.825V (Low signal), or 2.45 V (High Signal)
 *
 * In order not to backlog the CPU, the DMA is configured to take a stream of ADC data, and store it in memory
 *
 * ADC samples and converts signals continously at a rate of time per bit (i.e 1.67us)
 * DMA configured to take data from the ADC buffer and store it onto memory (var called tmp), when DMA stores data
 * in memory (dest) it shall go to an ISR that performs the logic below
 * if tmp == 0, disregard it
 * if tmp == PWM Voltage High, Bit is HIGH
 * if tmp == PWM Voltage LOW , Bit is LOW
 */

//DSHOT
#define BAUDRATE		600000 // BR between FC and ESC
#define FRAME_RATE		1000 //1KHz
#define TIME_PER_FRAME	(BAUDRATE / 16)
#define TIME_PER_BIT	(TIME_PER_FRAME / 16)
#define V_PWM_HIGH		(3.3 * 10 * 0.75)//multiplied by 10 to get us an
#define V_PWM_LOW		(3.3 * 10 * 0.25)


//TELEMETRY
#define TEL_FRAME_RATE		2000
#define TEL_BAUDRATE		115200
#define TEL_TIME_PER_FRAME


//define the States
#define READING			0//reads sig from ADC
#define PROCESSING		1//extracts the crc, throttle, and tel of sig
#define ERROR_CHECK		2//performs crc check validation
#define PROCESS_CMD		3//processes the signal after error check
#define TELEMETRY		4//sends tel from ESC to FC

struct dshot_packet
{
	uint32_t throttle;
	uint8_t crc;
	uint8_t telemetry;
};

uint16_t dshot_signal = 0x0000;
uint16_t bit_high = 0x0001;
uint16_t bit_low = 0x0000;
uint8_t tmp = 0;
uint8_t pos = 0;
uint8_t state = READING;

//Pseudo DMA ISR
void dma_isr (void)//gets called when the dma stores data in a var called tmp
{
	if (tmp == 0){}

	else
	{
		if (tmp == V_PWM_HIGH)
		{
			dshot_signal |= dshot_signal | (bit_high<<pos);
		}
		else if (tmp == V_PWM_LOW)
		{
			dshot_signal |= dshot_signal | (bit_low<<pos);
		}
		pos++;
		if(pos == 16)
		{
			pos = 0;
			state = PROCESSING;
		}
	}

}

void processing_signal(uint16_t dshot_signal, struct dshot_pack* dshot_pack_ins);
void error_check_signal(uint8_t crc);
void send_telemetry();

void beep_tone(uint8_t);
void Rotate_Motor_Forward (void);
void Rotate_Motor_Reverse (void);


int main(void)
{
	uint8_t pos = 0;//index to iterate
	uint8_t state = READING;
	struct dshot_packet dshot_packet_ins = {0,0,0};


	uint32_t tmp = 0;
	init_ADC();//take continous samples at a rate of the Time per bit
	init_DMA();//initialializes DMA, source ADC buffer, dest Memory, initializes DMA ISR


	while(1)
	{
		//ESC code

		if (state == PROCESSING)
			process_signal(bits,&dshot_packet_ins);

		if (state == ERROR_CHECKING)//if there are no errors, process the commands and telemetry, else state = READING
			if (!error_check_signal(dshot_packet_ins.crc))//no errors
				state = PROCESS_CMDS;



		if (dshot_packet_ins.telemetry == 1)//state = telemetry
			send_telemetry();

		//ESC code
	}


	return 0;

}

void processing_signal(uint16_t dshot_signal, struct dshot_pack* dshot_pack_ins)
{
	//bits 0 1 2 3 are crc
	uint16_t crc = dshot_signal & 0x000F;
	uint16_t telemetry = dshot_signal & 0x0010;
	uint16_t throttle = dshot_signal & 0xFFC0;
	uint8_t command_id = 0;//depends on the throttle value


	//store the throttle value
	uint16_t throttle_value = 0;

	static Normal_Dir_Rotation = 0;//in case ndr command, we increment that var till 5 before rotating the motor
	static Reverse_Dir_Rotation = 0;//in case rdr command, we increment that var till 5 before rotating the motor in the reverse direction
	/*
	 * throttle logic
	 * 1 - 47 commands
	 * 48 - 20
	 * 0
	 *
	 * */

	if (throttle >= 1 && throttle < 48)
	{
		command_id = throttle;

		switch(command_id)
		{
		case 1: Beep_Tone(1);
		case 2: Beep_Tone(2);
		case 3: Beep_Tone(3);
		case 4: Beep_Tone(4);
		case 5: Beep_Tone(5);
		case 20:
			Normal_Dir_Rotation++;
			if(Normal_Dir_Rotation > 4)
			{
				Rotate_Motor_Forward();
				Normal_Dir_Rotation = 0;
			}
		case 21:
			Reverse_Dir_Rotation++;
			if(Reverse_Dir_Rotation > 4)
			{
				Rotate_Motor_Reverse();
				Reverse_Dir_Rotation = 0;
			}

		}
	}
	else
	{
		throttle_value = throttle - 47;
	}


	dshot_pack_ins->crc = (uint8_t) crc;
	dshot_pack_ins->telemetry = (uint8_t) telemetry;
	dshot_pack_ins->throttle = throttle_value;

}

void error_checking(uint8_t crc)
{
	error_check_signal
	uint8_t error = 0;//no error


	return error;//1 there is an error 0 there are no errors
}

//telemetry shall be added
