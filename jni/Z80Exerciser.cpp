//
// Created by Igor Lapin on 14.06.18.
//

#include <stdexcept>
#include <android/log.h>
#include <sstream>
#include <cstring>
#include "Z80Exerciser.h"

static bool g_isFinished;
static bool g_isFirstPass = true;

static uint8_t* g_memoryArray;

static uint8_t readFromMemory(void *userData, uint16_t address) {
	Z80* cpu = static_cast<Z80 *>(userData);
	if (address == 0) {
		if (g_isFirstPass) {
			g_isFirstPass = false;
		} else {
			__android_log_print(ANDROID_LOG_DEBUG, "!@#", "Finishing at address 0x0000");
			g_isFinished = true;
		}
	} else if (address == 5) {
		// Emulate CP/M Syscall at address 5
		switch (cpu->regC()) {
			case 0: // BDOS 0 System Reset
				__android_log_print(ANDROID_LOG_DEBUG, "!@#", "Z80 reset after %lu t-states",
									cpu->tStates());
				g_isFinished = true;
				break;
			case 2: // BDOS 2 console char output
				__android_log_print(ANDROID_LOG_DEBUG, "!@#", "%c", cpu->regE());
				break;
			case 9: {// BDOS 9 console string output (string terminated by "$")
				uint16_t strAddr = cpu->regDE();
				std::ostringstream ss;
				while (g_memoryArray[strAddr] != '$') {
					ss << g_memoryArray[strAddr++];
				}
				__android_log_print(ANDROID_LOG_DEBUG, "!@#", "%s", ss.str().c_str());
				break;
			}
			default:
				__android_log_print(ANDROID_LOG_DEBUG, "!@#", "BDOS Call %u", cpu->regC());
				g_isFinished = true;
		}
	}

    return g_memoryArray[address];
}

static void writeToMemory(void *userData, uint16_t address, uint8_t data){
	g_memoryArray[address] = data;
}

static uint8_t readFromPort(void *userData, uint16_t port) {
    uint8_t* io = static_cast<uint8_t *>(userData);
    return io[port];
}

static void writeToPort(void *userData, uint16_t port, uint8_t data){
    uint8_t* io = static_cast<uint8_t *>(userData);
    io[port] = data;
}

Z80Exerciser::Z80Exerciser(std::string &logFilePath) :
		m_isPrevTimestampAvailable(false),
		m_mips(0)
{
	g_memoryArray = m_memoryArray;

	std::memset(m_memoryArray, 0, 0x10000);
    std::memset(m_ioArray, 0, 0x10000);

    m_cpu = new Z80(readFromMemory, writeToMemory, readFromPort, writeToPort);
    m_cpu->setMemoryUserData(m_cpu);
    m_cpu->setIoUserData(m_ioArray);
}

Z80Exerciser::~Z80Exerciser() {
    delete m_cpu;
}

void Z80Exerciser::runTest() {
    g_isFinished = false;

    m_memoryArray[0] = 0xC3;
    m_memoryArray[1] = 0x00;
    m_memoryArray[2] = 0x01; // JP 0x100 CP/M TPA
    m_memoryArray[5] = 0xC9; // Return from BDOS call

    __android_log_print(ANDROID_LOG_DEBUG, "!@#", "Starting test");
    while (!g_isFinished) {
		std::chrono::time_point<std::chrono::steady_clock> currentTimestamp = std::chrono::steady_clock::now();
		if (m_isPrevTimestampAvailable) {
			m_mips = 1e3f / std::chrono::duration_cast<std::chrono::nanoseconds>(currentTimestamp - m_prevTimestamp).count();
		}
		m_isPrevTimestampAvailable = true;
		m_prevTimestamp = currentTimestamp;

        m_cpu->execute();
    }
    __android_log_print(ANDROID_LOG_DEBUG, "!@#", "Test  ended");
}

