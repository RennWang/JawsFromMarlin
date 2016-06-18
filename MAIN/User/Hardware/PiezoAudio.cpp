#include "PiezoAudio.h"
#include "Fan.h"

static void TimInit();
static void TimSetFreq(uint32_t freq);
static void TimStop();

#define FREQUENCY 0
#define DURATION  1

struct tone {
	union {
		uint32_t tone_entry;	//Place holder for referencing
		uint16_t tone_data[2];	//index 0 is frequency, index 1 is duration
	};
};

const uint16_t tune_error[] = {
  NOTE_E5,    105,
  NOTE_0,      105,
  NOTE_B4,    105,
  NOTE_0,      105,
  NOTE_D4,    440,
  NOTE_0,	0,	//Zero duration is a terminator
};

const uint16_t tune_startup[] = {
	NOTE_A7,	288,
	NOTE_0,		188,
	NOTE_CS8,	188,
	NOTE_0,		188,
	NOTE_D8,	433,
	NOTE_0,		0,	//Zero duration is a terminator
};

const uint16_t tune_print_done[] = {
	NOTE_A5,	188,
	NOTE_0,		188,
	NOTE_A6,	188,
	NOTE_0,		188,
	NOTE_B6,	188,
	NOTE_0,		188,
	NOTE_C7,	188,
	NOTE_CS7,	188,
	NOTE_0,		188,
	NOTE_A7,	333,
	NOTE_0,		0,	//Zero duration is a terminator
};

const uint16_t tune_makerbot_tv[] = {
	NOTE_A7,	200,
	NOTE_0,		20,
	NOTE_A7,	200,
	NOTE_0,		20,
	NOTE_AS7,	400,
	NOTE_0,		20,
	NOTE_C8,	400,
	NOTE_0,		20,
	NOTE_A7,	800,
	NOTE_0,		0,	//Zero duration is a terminator
};

const uint16_t tune_beethoven_5th[] = {
	NOTE_FS6,	200,
	NOTE_0,		20,
	NOTE_FS6,	200,
	NOTE_0,		20,
	NOTE_FS6,	200,
	NOTE_0,		20,
	NOTE_DS6,	1200,
	NOTE_0,		20,
	NOTE_F6,	200,
	NOTE_0,		20,
	NOTE_F6,	200,
	NOTE_0,		20,
	NOTE_F6,	200,
	NOTE_0,		20,
	NOTE_D6,	1200,
	NOTE_0,		0,	//Zero duration is a terminator
};

const uint16_t tune_filament_start[] = {
	NOTE_A5,	400,
	NOTE_0,		20,
	NOTE_CS6,	200,
	NOTE_0,		20,
	NOTE_E6,	200,
	NOTE_0,		20,
	NOTE_A5,	400,
	NOTE_0,		0,	//Zero duration is a terminator
};

const uint16_t tune_pause[] = {
	NOTE_A7,	200,
	NOTE_0,		20,
	NOTE_A7,	200,
	NOTE_0,		20,
	NOTE_C8,	200,
	NOTE_0,		20,
	NOTE_A7,	400,
	NOTE_0,		0,	//Zero duration is a terminator
};

const uint16_t tune_sailfish_startup[] = {
	NOTE_C6,	200,
	NOTE_0,		220,
	NOTE_E6,	100,
	NOTE_0,		120,
	NOTE_F6,	200,
	NOTE_0,		220,
	NOTE_G6,	100,
	NOTE_0,		120,
	NOTE_A6,	200,
	NOTE_0,		220,
	NOTE_G6,	200,
	NOTE_0,		220,
	NOTE_G6,	200,
	NOTE_0,		220,
	NOTE_C7,	200,
	NOTE_0,		0,	//Zero duration is a terminator
};

PiezoAudio::PiezoAudio():
    soundEnabled(false),
    playing(false)
{

}

void PiezoAudio::init()
{
    reset();
}

void PiezoAudio::reset()
{
    // Reads the sound setting in from eeprom
	soundEnabled = true;

	//Empty the queue
	tones.reset();

	// Clear the timer, and we're not playing anymore
	piezoTimeout.clear();
	playing = false;

    TimInit();
    //freqInit();
	// And clear the timers
	shutdown_timer();
}

// Shuts the timer off
void PiezoAudio::shutdown_timer(void)
{
    TimStop();
    //stopFreqOut();
}

void PiezoAudio::processNextTone(void)
{
	if ( tones.isEmpty() ) {
		reset();
	} else {
 		playing = true;

		//Get the next tone from the buffer
		struct tone tone;

		tone.tone_entry		= tones.pop();

		piezoTimeout.start(tone.tone_data[DURATION]);

		if ( tone.tone_data[FREQUENCY] == NOTE_0 ) {
			//Note 0 acts as a rest (i.e. doesn't play a note), so we shut the timer down
			shutdown_timer();
		} else {
			uint32_t freq = (uint32_t)tone.tone_data[FREQUENCY];
            TimSetFreq(freq);
            //setFreqOut(freq);
		}
	}
}
// Call every so often to handle the queue and start/end tones
void PiezoAudio::runPiezoSlice(void)
{
    // If we're playing a sound and the last sound has finished, queue
	// the next one if we have one
	if ( (soundEnabled ) && ( playing ) && ( piezoTimeout.hasElapsed() )) {
		processNextTone();
	}
}

 	// is the buzzer playing a song?
bool PiezoAudio::isPlaying()
{
    return playing;
}

// Schedule a tone to be played // inputs: frequency (Hz) duration(ms)
void PiezoAudio::setTone(uint16_t frequency, uint16_t duration)
{
    	//If sound is switched off, we do nothin
	if ( ! soundEnabled )	return;

	//Add the tone to the buffer
	if ( tones.getRemainingCapacity() ) {
		struct tone tone;

		tone.tone_data[FREQUENCY] = frequency;
		tone.tone_data[DURATION]  = duration;

		tones.push(tone.tone_entry);
	}

	//If we're not playing tones, then we schedule the tone we just put in the queue
	if ( ! playing )	processNextTone();
}

//Plays a tune when provided with a tuneid (ideally enum TuneId)
void PiezoAudio::playTune(uint8_t tuneid)
{
	const uint16_t *tunePtr = NULL;

	switch ( tuneid ) {
		case TUNE_ERROR:
			tunePtr = tune_error;
			break;
		case TUNE_PRINT_DONE:
			tunePtr = tune_print_done;
			break;
		case TUNE_STARTUP:
			tunePtr = tune_startup;
			break;
		case TUNE_MAKERBOT_TV:
			tunePtr = tune_makerbot_tv;
			break;
		case TUNE_BEETHOVEN_5TH:
			tunePtr = tune_beethoven_5th;
			break;
		case TUNE_FILAMENT_START:
			tunePtr = tune_filament_start;
			break;
		case TUNE_PAUSE:
			tunePtr = tune_pause;
			break;
		case TUNE_SAILFISH_STARTUP:
			tunePtr = tune_sailfish_startup;
			break;
		default:
			break;
	}

	if ( tunePtr ) {
		//Play the tune
		uint8_t count = 0;
		uint16_t note, duration;
		do {
			note		= *(tunePtr + count * 2);
			duration	= *(tunePtr + count * 2 + 1);

			if ( duration != 0 )
			    setTone(note, duration);

			count ++;
		} while (duration != 0 ); 	//duration == 0 marks the end of the tune
	}
	else setTone(NOTE_B2, 500);	//Play this is the tuneid doesn't exist
}

// call this sequence on error
void PiezoAudio::errorTone(uint8_t iterations)
{
	for(int i = 0; i < iterations; i++) {
		setTone(NOTE_B4, 300);
		setTone(NOTE_A4, 300);
	}
}

PiezoAudio piezoaudio;

static TIM_HandleTypeDef  TIM_PWM_HandleStruct;

static void TimInit()
{
    GPIO_InitTypeDef    GPIO_InitStruct;

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    TIM_PWM_HandleStruct.Instance = TIM5;
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);
}

static void TimSetFreq(uint32_t freq)
{
    TIM_OC_InitTypeDef  TIM_InitStruct;
    ///PWMÊ±ÖÓÅäÖÃ
    __HAL_RCC_TIM5_CLK_ENABLE();
    TIM_PWM_HandleStruct.Instance = TIM5;
    TIM_PWM_HandleStruct.Init.Prescaler = (uint32_t)((SystemCoreClock/2)/2/freq) - 1;
    TIM_PWM_HandleStruct.Init.Period = 2-1;
    TIM_PWM_HandleStruct.Init.ClockDivision = 0;
    TIM_PWM_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
    HAL_TIM_PWM_Init(&TIM_PWM_HandleStruct);

    TIM_InitStruct.OCMode = TIM_OCMODE_PWM1;
    TIM_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
    TIM_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
    TIM_InitStruct.Pulse = 1;

    HAL_TIM_PWM_ConfigChannel(&TIM_PWM_HandleStruct, &TIM_InitStruct, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&TIM_PWM_HandleStruct, TIM_CHANNEL_2);
}

static void TimStop()
{
    HAL_TIM_PWM_Stop(&TIM_PWM_HandleStruct, TIM_CHANNEL_2);
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);
}







