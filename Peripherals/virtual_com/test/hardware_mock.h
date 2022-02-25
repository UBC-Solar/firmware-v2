/**
 * Header file for the mock class.
 */
#include "stm32f103xb.h"
#include <queue>
#include <ctime>

/**
 * Class attempts to mock and simulate any responses from 
 * the hardware.
 *  
 * Generally, when the micro-controller wants to send a character 
 * or string, the following occurs:
 *
 * 1) Write character to data register.
 * 2) Hardware detects the write, data is transported to shift 
 *    register that will be sent. In response, the hardware sets
 *    a bit to acknowledge the character.
 * 3) The firmware polls until the acknowledge bit is set by 
 *    the hardware before sending any other character.
 *
 * Since the data register keeps getting overwritten every time
 * a new character is sent, there is no way to verify that the 
 * correct character is written to the data register, unless the assert
 * statements are inside the function. In addition, without hardware,
 * the function will hang while it polls for the ACK (acknowledgment) bit.
 *
 * To remedy both these problems, this class was created, along with 
 * additions to the tested function (the additions are only valid during
 * tests.) Ideally, the tested function will be run on a separate thread,
 * while the main thread runs a class function that:
 * a) keeps track of the written characters and stores them in a buffer
      while the tested function polls for the ACK bit
 * b) sets the ACK bit 
 * 
 * However, if the ACK bit was set once, the tested function will stop 
 * polling for it, since the bit will stay high. If the class function 
 * tries to set it back to low, the non-deterministic nature of the 
 * scheduler may lead to wrong hardware behaviors (for example, the class 
 * function sets the bit to high, then low before the tested function even 
 * has a chance to poll for the bit.) To counter this, the tested function 
 * itself will set this bit back to low to mimic a pulse. This guarantees the 
 * following interaction every time:
 * 1) character is written to data register, test function starts polling
 * 2) mock function sees new data being written to register, pushes new character to buffer
 * 3) mock function sets ACK bit 
 * 4) test function sees ACK bit being set, sets the ACK bit back to low
 * 5) new character is written, test function polls for ACK bit again
 *
 * Ideally, asserts can be made about the contents of the buffer after the test 
 * function is executed.
 */
class HardwareMock 
{
	public:
		/**
		 * Constructor of the class.
		 */
		HardwareMock();
		
		/**
		 * Function mimics the hardware and keeps track of what was written 
		 * to the USART.
		 *
		 * @param expected_char_num: Expected number of characters sent. This
		 *							 lets the function know when to stop mocking 
		 * 							 the hardware. In case the test function has 
		 *							 bugs and not all the expected characters are 
		 *							 received, a timeout mechanism is implemented.
		 */
		void Mock(uint8_t expected_char_num);
		
		/**
		 * Function returns character from USART FIFO buffer.
		 *
		 * @return Next character in buffer.
		 */
		uint8_t ReturnBufferChar();
		
		/**
		 * Function returns current length of the USART FIFO buffer.
		 *
		 * @return Current length of buffer.
		 */
		uint32_t ReturnBufferLength();
		
		/**
		 * Function clears buffer.
		 */
		void ClearBuffer();
		
		/**
		 * Destructor of the class
		 */
		~HardwareMock();
	private:
		// Internal buffer that holds every character written to the data register
		std::queue<uint8_t>* buffer;
};