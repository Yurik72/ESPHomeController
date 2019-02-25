#pragma once

#ifndef ESP32
# error Driver is supports only ESP32
#endif

#include "Adafruit_NeoPixel.h"
#ifdef  ESP32

#ifdef __cplusplus
extern "C" {
#endif

#include "esp32-hal.h"
#include "esp_intr.h"
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "freertos/semphr.h"
#include "soc/rmt_struct.h"

#include "esp_log.h"

#ifdef __cplusplus
}
#endif

__attribute__((always_inline)) inline static uint32_t __clock_cycles() {
	uint32_t cyc;
	__asm__ __volatile__("rsr %0,ccount":"=a" (cyc));
	return cyc;
}

#define FASTLED_HAS_CLOCKLESS 1

// -- Configuration constants
#define DIVIDER             2 /* 4, 8 still seem to work, but timings become marginal */
#define MAX_PULSES         32 /* A channel has a 64 "pulse" buffer - we use half per pass */
#define F_CPU_MHZ 80
// -- Convert ESP32 cycles back into nanoseconds
#define ESPCLKS_TO_NS(_CLKS) (((long)(_CLKS) * 1000L) / F_CPU_MHZ)

// -- Convert nanoseconds into RMT cycles
#define F_CPU_RMT       (  80000000L)
#define NS_PER_SEC      (1000000000L)
#define CYCLES_PER_SEC  (F_CPU_RMT/DIVIDER)
#define NS_PER_CYCLE    ( NS_PER_SEC / CYCLES_PER_SEC )
#define NS_TO_CYCLES(n) ( (n) / NS_PER_CYCLE )

// -- Convert ESP32 cycles to RMT cycles
#define TO_RMT_CYCLES(_CLKS) NS_TO_CYCLES(ESPCLKS_TO_NS(_CLKS))    

// -- Number of cycles to signal the strip to latch
#define RMT_RESET_DURATION NS_TO_CYCLES(50000)

#define NS(_NS) (((_NS * F_CPU_MHZ) + 999) / 1000)
// -- Core or custom driver
#ifndef FASTLED_RMT_BUILTIN_DRIVER
#define FASTLED_RMT_BUILTIN_DRIVER false
#endif

// -- Max number of controllers we can support
#ifndef DRIVER_RMT_MAX_CONTROLLERS
#define DRIVER_RMT_MAX_CONTROLLERS 32
#endif
#define DRIVER_RMT_MAX_CHANNELS 1
// -- Number of RMT channels to use (up to 8)
//    Redefine this value to 1 to force serial output
#ifndef DRIVER_RMT_MAX_CHANNELS
#define FASTLED_RMT_MAX_CHANNELS 8
#endif

// -- Array of all controllers
//static CLEDController * gControllers[FASTLED_RMT_MAX_CONTROLLERS];

// -- Current set of active controllers, indexed by the RMT
//    channel assigned to them.
//static CLEDController * gOnChannel[FASTLED_RMT_MAX_CHANNELS];

static int glob_drivers_count = 0;
static int glob_driver_started = 0;
static int glob_driver_done = 0;
static int glob_driver_next = 0;

static intr_handle_t gRMT_intr_handle = NULL;

// -- Global semaphore for the whole show process
//    Semaphore is not given until all data has been sent
static xSemaphoreHandle gTX_sem = NULL;

static bool glob_IsInitialized = false;
class LedInterruptDriverBase {

};

static LedInterruptDriverBase * glob_channels[DRIVER_RMT_MAX_CHANNELS];
static LedInterruptDriverBase * glob_drivers[DRIVER_RMT_MAX_CONTROLLERS];


template <int T1, int T2, int T3>
class LedInterruptDriver :public LedInterruptDriverBase//: public CPixelLEDController<RGB_ORDER>
{
	// -- RMT has 8 channels, numbered 0 to 7
	rmt_channel_t  mRMT_channel;

	// -- Store the GPIO pin
	gpio_num_t     mPin;

	// -- This instantiation forces a check on the pin choice
	//FastPin<DATA_PIN> mFastPin;

	// -- Timing values for zero and one bits, derived from T1, T2, and T3
	rmt_item32_t   mZero;
	rmt_item32_t   mOne;

	// -- State information for keeping track of where we are in the pixel data
	uint8_t *      mPixelData = NULL;
	int            mSize = 0;
	int            mCurByte;
	uint16_t       mCurPulse;

	// -- Buffer to hold all of the pulses. For the version that uses
	//    the RMT driver built into the ESP core.
	rmt_item32_t * mBuffer;
	uint16_t       mBufferSize;
	WS2812FX *pNeopixel = NULL;

public:
	void init(WS2812FX* neopixel) {
		this->pNeopixel = neopixel;
		this->init(neopixel->getPin(), neopixel->numPixels());

	}

	void init(int datapin, int numleds)
	{
		// -- Precompute rmt items corresponding to a zero bit and a one bit
		//    according to the timing values given in the template instantiation
		// T1H

		mOne.level0 = 1;
		mOne.duration0 = TO_RMT_CYCLES(T1 + T2);
		// T1L
		mOne.level1 = 0;
		mOne.duration1 = TO_RMT_CYCLES(T3);

		// T0H
		mZero.level0 = 1;
		mZero.duration0 = TO_RMT_CYCLES(T1);
		// T0L
		mZero.level1 = 0;
		mZero.duration1 = TO_RMT_CYCLES(T2 + T3);

		glob_drivers[glob_drivers_count] = this;
		glob_drivers_count++;

		mPin = gpio_num_t(datapin);
	}

	virtual uint16_t getMaxRefreshRate() const { return 400; }

protected:

	void initRMT()
	{
		// -- Only need to do this once
		if (glob_IsInitialized) return;

		for (int i = 0; i < DRIVER_RMT_MAX_CHANNELS; i++) {
			glob_channels[i] = NULL;

			// -- RMT configuration for transmission
			rmt_config_t rmt_tx;
			rmt_tx.channel = rmt_channel_t(i);
			rmt_tx.rmt_mode = RMT_MODE_TX;
			rmt_tx.gpio_num = mPin;  // The particular pin will be assigned later
			rmt_tx.mem_block_num = 1;
			rmt_tx.clk_div = DIVIDER;
			rmt_tx.tx_config.loop_en = false;
			rmt_tx.tx_config.carrier_level = RMT_CARRIER_LEVEL_LOW;
			rmt_tx.tx_config.carrier_en = false;
			rmt_tx.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
			rmt_tx.tx_config.idle_output_en = true;

			// -- Apply the configuration
			rmt_config(&rmt_tx);

			if (FASTLED_RMT_BUILTIN_DRIVER) {
				rmt_driver_install(rmt_channel_t(i), 0, 0);
			}
			else {
				// -- Set up the RMT to send 1/2 of the pulse buffer and then
				//    generate an interrupt. When we get this interrupt we
				//    fill the other half in preparation (kind of like double-buffering)
				rmt_set_tx_thr_intr_en(rmt_channel_t(i), true, MAX_PULSES);
			}
		}

		// -- Create a semaphore to block execution until all the controllers are done
		if (gTX_sem == NULL) {
			gTX_sem = xSemaphoreCreateBinary();
			xSemaphoreGive(gTX_sem);
		}

		if (!FASTLED_RMT_BUILTIN_DRIVER) {
			// -- Allocate the interrupt if we have not done so yet. This
			//    interrupt handler must work for all different kinds of
			//    strips, so it delegates to the refill function for each
			//    specific instantiation of ClocklessController.
			if (gRMT_intr_handle == NULL)
				esp_intr_alloc(ETS_RMT_INTR_SOURCE, 0, interruptHandler, 0, &gRMT_intr_handle);
		}

		glob_IsInitialized = true;
	}

	// -- Show pixels
	//    This is the main entry point for the controller.
public:
	void customShow() {
		if (this->pNeopixel)
			this->showPixels(*pNeopixel);
	}
	void showPixels(Adafruit_NeoPixel& neopixel)
	{
		if (glob_driver_started == 0) {
			// -- First controller: make sure everything is set up
			initRMT();
			xSemaphoreTake(gTX_sem, portMAX_DELAY);
		}

		// -- Initialize the local state, save a pointer to the pixel
		//    data. We need to make a copy because pixels is a local
		//    variable in the calling function, and this data structure
		//    needs to outlive this call to showPixels.

		//if (mPixels != NULL) delete mPixels;
		//mPixels = new PixelController<RGB_ORDER>(pixels);
		//if (FASTLED_RMT_BUILTIN_DRIVER)
		//	convertAllPixelData(pixels);
		//else
		copyPixelData(neopixel);

		// -- Keep track of the number of strips we've seen
		glob_driver_started++;

		// -- The last call to showPixels is the one responsible for doing
		//    all of the actual worl
		if (glob_driver_started == glob_drivers_count) {
			glob_driver_next = 0;

			// -- First, fill all the available channels
			int channel = 0;
			while (channel < DRIVER_RMT_MAX_CHANNELS && glob_driver_next < glob_drivers_count) {
				startNext(channel);
				channel++;
			}

			// -- Wait here while the rest of the data is sent. The interrupt handler
			//    will keep refilling the RMT buffers until it is all sent; then it
			//    gives the semaphore back.
			xSemaphoreTake(gTX_sem, portMAX_DELAY);
			xSemaphoreGive(gTX_sem);

			// -- Reset the counters
			glob_driver_started = 0;
			glob_driver_done = 0;
			glob_driver_next = 0;
		}
	}

	// -- Copy pixel data
	//    Make a safe copy of the pixel data, so that the FastLED show
	//    function can continue to the next controller while the RMT
	//    device starts sending this data asynchronously.
	void copyPixelData(Adafruit_NeoPixel& neopixel)
	{
		// -- Make sure we have a buffer of the right size
		//    (3 bytes per pixel)

		//int size_needed = pixels.size() * 3;
		int size_needed = neopixel.numPixels() * 3;
		if (size_needed > mSize) {
			if (mPixelData != NULL) free(mPixelData);
			mSize = size_needed;
			mPixelData = (uint8_t *)malloc(mSize);
		}

		// -- Cycle through the R,G, and B values in the right order,
		//    storing the resulting raw pixel data in the buffer.
		//internalcopyPixelData(neopixel);
		internalFastcopyPixelData_3b_GRB(neopixel);
	}
	void internalcopyPixelData(Adafruit_NeoPixel& neopixel) {
		int cur = 0;
		uint16_t count = neopixel.numPixels();
		for (uint16_t i = 0;i < count; i++) {
			uint32_t pixelcolor = neopixel.getPixelColor(i);

			mPixelData[cur++] = (pixelcolor >> 8) & 0xFF;
			mPixelData[cur++] = (pixelcolor >> 16) & 0xFF;
			mPixelData[cur++] = pixelcolor & 0xFF;

		}
	}
	void internalFastcopyPixelData_3b_GRB(Adafruit_NeoPixel& neopixel) {
		//int cur = 0;
		uint16_t count = neopixel.numPixels();
		uint8_t *neodata = neopixel.getPixels();
		//int posneodata = 0;
		/// direct copy can be used, due to the right order sequence
		memcpy(mPixelData, neodata, count * 3);


		/*
		for (uint16_t i = 0;i < count; i++) {
			//uint32_t pixelcolor = neopixel.getPixelColor(i);
			uint8_t * curled = &neodata[i*3];

			mPixelData[cur++] = curled[0];
			mPixelData[cur++] = curled[1];
			mPixelData[cur++] = curled[2];

			//mPixelData[cur++] = (pixelcolor >> 8) & 0xFF; //green
			//mPixelData[cur++] = (pixelcolor >> 16) & 0xFF; //red
			//mPixelData[cur++] = pixelcolor & 0xFF;          //blue

		}
		*/
	}
	/*
	void convertByte(uint32_t byteval)
	{
		// -- Write one byte's worth of RMT pulses to the big buffer
		byteval <<= 24;
		for (register uint32_t j = 0; j < 8; j++) {
			mBuffer[mCurPulse] = (byteval & 0x80000000L) ? mOne : mZero;
			byteval <<= 1;
			mCurPulse++;
		}
	}
	*/

	static void IRAM_ATTR startNext(int channel)
	{

		if (glob_driver_next < glob_drivers_count) {
			LedInterruptDriver * pController = static_cast<LedInterruptDriver*>(glob_drivers[glob_driver_next]);
			pController->startOnChannel(channel);
			glob_driver_next++;
		}
	}

	// -- Start this controller on the given channel
	//    This function just initiates the RMT write; it does not wait
	//    for it to finish.
	void  IRAM_ATTR startOnChannel(int channel)
	{

		// -- Assign this channel and configure the RMT
		mRMT_channel = rmt_channel_t(channel);

		// -- Store a reference to this controller, so we can get it
		//    inside the interrupt handler
		glob_channels[channel] = this;

		// -- Assign the pin to this channel
		rmt_set_pin(mRMT_channel, RMT_MODE_TX, mPin);

		if (FASTLED_RMT_BUILTIN_DRIVER) {
			// -- Use the built-in RMT driver to send all the data in one shot
			rmt_register_tx_end_callback(doneOnChannel, 0);
			rmt_write_items(mRMT_channel, mBuffer, mBufferSize, false);
		}
		else {
			// -- Use our custom driver to send the data incrementally

			// -- Turn on the interrupts
			rmt_set_tx_intr_en(mRMT_channel, true);

			// -- Initialize the counters that keep track of where we are in
			//    the pixel data.
			mCurPulse = 0;
			mCurByte = 0;

			// -- Fill both halves of the buffer
			fillHalfRMTBuffer();
			fillHalfRMTBuffer();

			// -- Turn on the interrupts
			rmt_set_tx_intr_en(mRMT_channel, true);

			// -- Start the RMT TX operation
			rmt_tx_start(mRMT_channel, true);
		}
	}

	// -- A controller is done 
	//    This function is called when a controller finishes writing
	//    its data. It is called either by the custom interrupt
	//    handler (below), or as a callback from the built-in
	//    interrupt handler. It is static because we don't know which
	//    controller is done until we look it up.
	static void IRAM_ATTR doneOnChannel(rmt_channel_t channel, void * arg)
	{
		if (channel >= DRIVER_RMT_MAX_CHANNELS) return;

		LedInterruptDriver * controller = static_cast<LedInterruptDriver*>(glob_channels[channel]);
		portBASE_TYPE HPTaskAwoken = 0;

		// -- Turn off output on the pin
		gpio_matrix_out(controller->mPin, 0x100, 0, 0);

		glob_channels[channel] = NULL;
		glob_driver_done++;

		if (glob_driver_done == glob_drivers_count) {
			// -- If this is the last controller, signal that we are all done
			xSemaphoreGiveFromISR(gTX_sem, &HPTaskAwoken);
			if (HPTaskAwoken == pdTRUE) portYIELD_FROM_ISR();
		}
		else {
			// -- Otherwise, if there are still controllers waiting, then
			//    start the next one on this channel
			if (glob_driver_next < glob_drivers_count)
				startNext(channel);
		}
	}

	// -- Custom interrupt handler
	//    This interrupt handler handles two cases: a controller is
	//    done writing its data, or a controller needs to fill the
	//    next half of the RMT buffer with data.
	static IRAM_ATTR void interruptHandler(void *arg)
	{

		// -- The basic structure of this code is borrowed from the
		//    interrupt handler in esp-idf/components/driver/rmt.c
		uint32_t intr_st = RMT.int_st.val;

		uint8_t channel;

		for (channel = 0; channel < DRIVER_RMT_MAX_CHANNELS; channel++) {
			int tx_done_bit = channel * 3;
			int tx_next_bit = channel + 24;

			if (glob_channels[channel] != NULL) {

				// -- More to send on this channel
				if (intr_st & BIT(tx_next_bit)) {
					RMT.int_clr.val |= BIT(tx_next_bit);

					// -- Refill the half of the buffer that we just finished,
					//    allowing the other half to proceed.
					LedInterruptDriver * controller = static_cast<LedInterruptDriver*>(glob_channels[channel]);
					controller->fillHalfRMTBuffer();
				}
				else {
					// -- Transmission is complete on this channel
					if (intr_st & BIT(tx_done_bit)) {
						RMT.int_clr.val |= BIT(tx_done_bit);
						doneOnChannel(rmt_channel_t(channel), 0);
					}
				}
			}
		}
	}

	// -- Fill the RMT buffer
	//    This function fills the next 32 slots in the RMT write
	//    buffer with pixel data. It also handles the case where the
	//    pixel data is exhausted, so we need to fill the RMT buffer
	//    with zeros to signal that it's done.
	void  IRAM_ATTR fillHalfRMTBuffer()
	{

		uint32_t one_val = mOne.val;
		uint32_t zero_val = mZero.val;

		// -- Convert (up to) 32 bits of the raw pixel data into
		//    into RMT pulses that encode the zeros and ones.
		int pulses = 0;
		uint32_t byteval;

		while (pulses < 32 && mCurByte < mSize) {
			// -- Get one byte
			byteval = mPixelData[mCurByte++];
			byteval <<= 24;
			// Shift bits out, MSB first, setting RMTMEM.chan[n].data32[x] to the 
			// rmt_item32_t value corresponding to the buffered bit value
			for (register uint32_t j = 0; j < 8; j++) {
				uint32_t val = (byteval & 0x80000000L) ? one_val : zero_val;
				RMTMEM.chan[mRMT_channel].data32[mCurPulse].val = val;
				byteval <<= 1;
				mCurPulse++;
			}
			pulses += 8;
		}

		// -- When we reach the end of the pixel data, fill the rest of the
		//    RMT buffer with 0's, which signals to the device that we're done.
		if (mCurByte == mSize) {
			while (pulses < 32) {
				RMTMEM.chan[mRMT_channel].data32[mCurPulse].val = 0;
				mCurPulse++;
				pulses++;
			}
		}

		// -- When we have filled the back half the buffer, reset the position to the first half
		if (mCurPulse >= MAX_PULSES * 2)
			mCurPulse = 0;

	}

};
#endif


