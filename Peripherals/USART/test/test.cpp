extern "C"
{
	#include "virtual_com.h"
}
#include "hardware_mock.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include "gtest/gtest.h"

using namespace std;

// Since these register pointers were only declared,
// they must be defined here.
RCC_Mock*  RCC = new RCC_Mock;
GPIO_Mock* GPIOA = new GPIO_Mock;
USART_Mock* USART2 = new USART_Mock;

HardwareMock* mock = new HardwareMock();

/**
 * Test SendChar() with capital letters
 */
 TEST(VirtualCom, SendChar_with_capital)
{
	// Since SendChar() polls for the ACK bit once, 
	// the bit can be set in the beginning, and the 
	// data register can be used in the assertions 
	// (no need for hardware mock)
	USART2->SR = 0x1 << 7;
	
	SendChar('A');
	EXPECT_EQ(0x41, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('B');
	EXPECT_EQ(0x42, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('C');
	EXPECT_EQ(0x43, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('X');
	EXPECT_EQ(0x58, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('Y');
	EXPECT_EQ(0x59, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('Z');
	EXPECT_EQ(0x5A, USART2->DR);
}

/**
 * Test SendChar() with lowercase letters
 */
 TEST(VirtualCom, SendChar_with_lowercase)
{
	USART2->SR = 0x1 << 7;
	
	SendChar('a');
	EXPECT_EQ(0x61, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('b');
	EXPECT_EQ(0x62, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('c');
	EXPECT_EQ(0x63, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('x');
	EXPECT_EQ(0x78, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('y');
	EXPECT_EQ(0x79, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('z');
	EXPECT_EQ(0x7A, USART2->DR);
}

/**
 * Test SendChar() with digits
 */
 TEST(VirtualCom, SendChar_with_digits)
{
	USART2->SR = 0x1 << 7;
	
	SendChar('0');
	EXPECT_EQ(0x30, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('1');
	EXPECT_EQ(0x31, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('2');
	EXPECT_EQ(0x32, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('3');
	EXPECT_EQ(0x33, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('4');
	EXPECT_EQ(0x34, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('5');
	EXPECT_EQ(0x35, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('6');
	EXPECT_EQ(0x36, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('7');
	EXPECT_EQ(0x37, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('8');
	EXPECT_EQ(0x38, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('9');
	EXPECT_EQ(0x39, USART2->DR);
}

/**
 * Test SendChar() with special symbols
 */
 TEST(VirtualCom, SendChar_with_symbols)
{
	USART2->SR = 0x1 << 7;
	
	SendChar(' ');
	EXPECT_EQ(0x20, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('!');
	EXPECT_EQ(0x21, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('/');
	EXPECT_EQ(0x2F, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar('(');
	EXPECT_EQ(0x28, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar(')');
	EXPECT_EQ(0x29, USART2->DR);

	USART2->SR = 0x1 << 7;
	
	SendChar('[');
	EXPECT_EQ(0x5B, USART2->DR);
	
	USART2->SR = 0x1 << 7;
	
	SendChar(']');
	EXPECT_EQ(0x5D, USART2->DR);
}

/**
 * Test SendString() with empty string
 */
TEST(VirtualCom, SendString_with_empty)
{
	// SendString() needs to be launched in a separate thread so 
	// the mock function can be run on the main thread
	std::thread t([] {
		SendString("");
	});
	t.detach();
	
	mock->Mock(0);
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	// Clear buffer to reset the mock (test cleanup)
	mock->ClearBuffer();
}

/**
 * Test SendString() with only one character
 */
 TEST(VirtualCom, SendString_with_one_char)
{
	std::thread t([] {
		SendString("A");
	});
	t.detach();
	
	mock->Mock(1);
	EXPECT_EQ(1, mock->ReturnBufferLength());
	EXPECT_EQ(0x41, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	// Clear buffer to reset the mock (test cleanup)
	mock->ClearBuffer();
}

/**
 * Test SendString() with only single words
 */
 TEST(VirtualCom, SendString_with_one_word)
{
	std::thread t0([] {
		SendString("cat");
	});
	t0.detach();
	
	mock->Mock(3);
	EXPECT_EQ(3, mock->ReturnBufferLength());
	EXPECT_EQ(0x63, mock->ReturnBufferChar());
	EXPECT_EQ(0x61, mock->ReturnBufferChar());
	EXPECT_EQ(0x74, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	// Clear buffer to reset the mock (test cleanup)
	mock->ClearBuffer();
	
	std::thread t1([] {
		SendString("Solar");
	});
	t1.detach();
	
	mock->Mock(5);
	EXPECT_EQ(5, mock->ReturnBufferLength());
	EXPECT_EQ(0x53, mock->ReturnBufferChar());
	EXPECT_EQ(0x6F, mock->ReturnBufferChar());
	EXPECT_EQ(0x6C, mock->ReturnBufferChar());
	EXPECT_EQ(0x61, mock->ReturnBufferChar());
	EXPECT_EQ(0x72, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	// Clear buffer to reset the mock (test cleanup)
	mock->ClearBuffer();
	
	std::thread t2([] {
		SendString("ViRtUaLcOm");
	});
	t2.detach();
	
	mock->Mock(10);
	EXPECT_EQ(10, mock->ReturnBufferLength());
	EXPECT_EQ(0x56, mock->ReturnBufferChar());
	EXPECT_EQ(0x69, mock->ReturnBufferChar());
	EXPECT_EQ(0x52, mock->ReturnBufferChar());
	EXPECT_EQ(0x74, mock->ReturnBufferChar());
	EXPECT_EQ(0x55, mock->ReturnBufferChar());
	EXPECT_EQ(0x61, mock->ReturnBufferChar());
	EXPECT_EQ(0x4C, mock->ReturnBufferChar());
	EXPECT_EQ(0x63, mock->ReturnBufferChar());
	EXPECT_EQ(0x4F, mock->ReturnBufferChar());
	EXPECT_EQ(0x6D, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	// Clear buffer to reset the mock (test cleanup)
	mock->ClearBuffer();
	
	// Test duplicate characters
	std::thread t3([] {
		SendString("bbbbb");
	});
	t3.detach();
	
	mock->Mock(5);
	EXPECT_EQ(5, mock->ReturnBufferLength());
	EXPECT_EQ(0x62, mock->ReturnBufferChar());
	EXPECT_EQ(0x62, mock->ReturnBufferChar());
	EXPECT_EQ(0x62, mock->ReturnBufferChar());
	EXPECT_EQ(0x62, mock->ReturnBufferChar());
	EXPECT_EQ(0x62, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	// Clear buffer to reset the mock (test cleanup)
	mock->ClearBuffer();
}

/**
 * Test SendString() with sentences/symbols
 */
 TEST(VirtualCom, SendString_with_sentence)
{
	std::thread t0([] {
		SendString("hello world");
	});
	t0.detach();
	
	mock->Mock(11);
	EXPECT_EQ(11, mock->ReturnBufferLength());
	EXPECT_EQ(0x68, mock->ReturnBufferChar());
	EXPECT_EQ(0x65, mock->ReturnBufferChar());
	EXPECT_EQ(0x6C, mock->ReturnBufferChar());
	EXPECT_EQ(0x6C, mock->ReturnBufferChar());
	EXPECT_EQ(0x6F, mock->ReturnBufferChar());
	EXPECT_EQ(0x20, mock->ReturnBufferChar());
	EXPECT_EQ(0x77, mock->ReturnBufferChar());
	EXPECT_EQ(0x6F, mock->ReturnBufferChar());
	EXPECT_EQ(0x72, mock->ReturnBufferChar());
	EXPECT_EQ(0x6C, mock->ReturnBufferChar());
	EXPECT_EQ(0x64, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	// Clear buffer to reset the mock (test cleanup)
	mock->ClearBuffer();
	
	std::thread t1([] {
		SendString("data: ");
	});
	t1.detach();
	
	mock->Mock(6);
	EXPECT_EQ(6, mock->ReturnBufferLength());
	EXPECT_EQ(0x64, mock->ReturnBufferChar());
	EXPECT_EQ(0x61, mock->ReturnBufferChar());
	EXPECT_EQ(0x74, mock->ReturnBufferChar());
	EXPECT_EQ(0x61, mock->ReturnBufferChar());
	EXPECT_EQ(0x3A, mock->ReturnBufferChar());
	EXPECT_EQ(0x20, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	// Clear buffer to reset the mock (test cleanup)
	mock->ClearBuffer();
	
	std::thread t2([] {
		SendString("0K !!");
	});
	t2.detach();
	
	mock->Mock(5);
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x4B, mock->ReturnBufferChar());
	EXPECT_EQ(0x20, mock->ReturnBufferChar());
	EXPECT_EQ(0x21, mock->ReturnBufferChar());
	EXPECT_EQ(0x21, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	// Clear buffer to reset the mock (test cleanup)
	mock->ClearBuffer();
}

/**
 * Test SendInt() with generic and boundary cases
 */
TEST(VirtualCom, SendInt)
{
	// Test 0. Expected String: "0X00000000"
	std::thread t0([] {
		SendInt(0x0);
	});
	t0.detach();
	
	mock->Mock(10);
	EXPECT_EQ(10, mock->ReturnBufferLength());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x58, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	mock->ClearBuffer();
	
	// Test 1. Expected String: "0X00000001"
	std::thread t1([] {
		SendInt(0x1);
	});
	t1.detach();
	
	mock->Mock(10);
	EXPECT_EQ(10, mock->ReturnBufferLength());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x58, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x31, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	mock->ClearBuffer();
	
	// Test even number (28) Expected String: "0X0000001C"
	std::thread t2([] {
		SendInt(28);
	});
	t2.detach();
	
	mock->Mock(10);
	EXPECT_EQ(10, mock->ReturnBufferLength());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x58, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x31, mock->ReturnBufferChar());
	EXPECT_EQ(0x43, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	mock->ClearBuffer();
	
	// Test odd number (311) Expected String: "0X00000137"
	std::thread t3([] {
		SendInt(311);
	});
	t3.detach();
	
	mock->Mock(10);
	EXPECT_EQ(10, mock->ReturnBufferLength());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x58, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x31, mock->ReturnBufferChar());
	EXPECT_EQ(0x33, mock->ReturnBufferChar());
	EXPECT_EQ(0x37, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	mock->ClearBuffer();
	
	// Test MAX_INT Expected String: "0XFFFFFFFF"
	std::thread t4([] {
		SendInt(0xFFFFFFFF);
	});
	t4.detach();
	
	mock->Mock(10);
	EXPECT_EQ(10, mock->ReturnBufferLength());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x58, mock->ReturnBufferChar());
	EXPECT_EQ(0x46, mock->ReturnBufferChar());
	EXPECT_EQ(0x46, mock->ReturnBufferChar());
	EXPECT_EQ(0x46, mock->ReturnBufferChar());
	EXPECT_EQ(0x46, mock->ReturnBufferChar());
	EXPECT_EQ(0x46, mock->ReturnBufferChar());
	EXPECT_EQ(0x46, mock->ReturnBufferChar());
	EXPECT_EQ(0x46, mock->ReturnBufferChar());
	EXPECT_EQ(0x46, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	mock->ClearBuffer();
} 

/**
 * Test SendLine()
 */
TEST(VirtualCom, SendLine)
{
	std::thread t([] {
		SendLine();
	});
	t.detach();
	
	mock->Mock(2);
	EXPECT_EQ(2, mock->ReturnBufferLength());
	EXPECT_EQ(0xA, mock->ReturnBufferChar());
	EXPECT_EQ(0xD, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	mock->ClearBuffer();
}

/**
 * Functional Test: test generic logging messages
 */
TEST(VirtualCom, Send_generic_log_message)
{
	// Test generic log message "Temp: 0x00000010C\n"
	std::thread t([] {
		SendString("Temp: ");
		SendInt(0x10);
		SendChar('C');
		SendLine();
	});
	t.detach();
	
	mock->Mock(19);
	EXPECT_EQ(19, mock->ReturnBufferLength());
	EXPECT_EQ(0x54, mock->ReturnBufferChar());
	EXPECT_EQ(0x65, mock->ReturnBufferChar());
	EXPECT_EQ(0x6D, mock->ReturnBufferChar());
	EXPECT_EQ(0x70, mock->ReturnBufferChar());
	EXPECT_EQ(0x3A, mock->ReturnBufferChar());
	EXPECT_EQ(0x20, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x58, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x31, mock->ReturnBufferChar());
	EXPECT_EQ(0x30, mock->ReturnBufferChar());
	EXPECT_EQ(0x43, mock->ReturnBufferChar());
	EXPECT_EQ(0x0A, mock->ReturnBufferChar());
	EXPECT_EQ(0x0D, mock->ReturnBufferChar());
	EXPECT_EQ(0, mock->ReturnBufferLength());
	
	mock->ClearBuffer();
}
 
 