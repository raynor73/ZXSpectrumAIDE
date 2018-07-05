//
// Created by Игорь on 17.06.2018.
//

#ifndef ZXSPECTRUM_H
#define ZXSPECTRUM_H


#include <atomic>
#include <ctime>
#include "z80/Z80.h"
#include "Keyboard.h"

class ZxSpectrum {
public:
	ZxSpectrum(std::string logFilePath);
	~ZxSpectrum();

	void loop();
	void verticalRefresh();
	void quit();
	void reset();
	float exceededInstructionsPercent() const {
		if (m_totalInstructionsCounter > 0) {
			return float(m_exceededInstructionsCounter) / m_totalInstructionsCounter;
		} else {
			return 0;
		}
	}
	uint32_t interruptsCount();
	uint32_t instructionsCount();

	uint8_t* memoryArray() { return m_memoryArray; }
	uint8_t* portsArray() { return m_portsArray; }

	void onKeyPressed(const int keyCode);
	void onKeyReleased(const int keyCode);

private:
	static const int CPU_CLOCK_PERIOD = 286; //ns

	Z80* m_cpu;

	uint8_t m_memoryArray[0x10000];
	uint8_t m_portsArray[0x10000];
	Keyboard *m_keyboard;
	std::atomic<bool> m_isRunning;
	std::atomic<uint32_t> m_shouldInterrupt;
	uint64_t m_totalInstructionsCounter;
	uint64_t m_exceededInstructionsCounter;
	std::atomic<uint32_t> m_interruptsCount;
	std::atomic<uint32_t> m_instructionsCount;
    timespec m_startTimestamp;
    timespec m_currentTimestamp;
};


#endif //ZXSPECTRUM_H
