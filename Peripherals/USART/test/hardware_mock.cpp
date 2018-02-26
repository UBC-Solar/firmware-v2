/**
 * Implementation of the hardware mock class.
 */
#include "hardware_mock.h"
#include <iostream>

/**
 * Constructor of the class.
 */
HardwareMock::HardwareMock()
{
	// Create the internal buffer
	buffer = new std::queue<uint8_t>;
}

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
void HardwareMock::Mock(uint8_t expected_char_num)
{
	// Get a starting time
	clock_t starting_time = clock();
	
	// If the buffer size is not equal to the expected number of characters received,
	// or it is not time to time out yet, keep polling for characters from the test 
	// function
	while (buffer->size() < expected_char_num && (float) (clock() - starting_time) < 30)
	{
		// Check the data register to see if there is any new data written 
		// The data register is a global variable declared in the header file
		// and defined in the test suite main
		if (USART2->DR)
		{
			// If new data exists, push to buffer
			// At this point, the test function should be polling for the ACK bit 
			buffer->push(USART2->DR & 0xFF); 
			
			// Clear the data register
			// This is not done by hardware, but it is done in the mock to 
			// look for new data
			USART2->DR = 0;
			
			// Set the ACK bit
			USART2->SR = 0x1 << 7;
		}
	}
}

/**
 * Function returns character from USART FIFO buffer.
 *
 * @return Next character in buffer.
 */
uint8_t HardwareMock::ReturnBufferChar()
{
	// Sanity check
	// if there are no characters in the buffer, return 0 (NULL in ASCII)
	if (buffer->empty())
	{
		return 0;
	}
	uint8_t front = buffer->front();
	buffer->pop();
	return front;
}

/**
 * Function returns current length of the USART FIFO buffer.
 *
 * @return Current length of buffer.
 */
uint32_t HardwareMock::ReturnBufferLength()
{ 
	return buffer->size();
}

/**
 * Function clears buffer.
 */
void HardwareMock::ClearBuffer()
{
	// While buffer is not empty, keep popping every character out
	while (!buffer->empty())
	{
		buffer->pop();
	}
}

/**
 * Destructor of the class
 */
HardwareMock::~HardwareMock()
{
	// Delete the internal buffer
	delete buffer;
}