//
// Created by Игорь on 17.06.2018.
//

#include <thread>
#include <string>
#include <ctime>
#include "ZxSpectrum.h"

static uint8_t readFromMemory(void* userData, uint16_t address) {
	uint8_t* memory = static_cast<uint8_t *>(userData);
	return memory[address];
}

static void writeToMemory(void* userData, uint16_t address, uint8_t data) {
	if (address < 0x4000) {
		return;
	}
	uint8_t* memory = static_cast<uint8_t *>(userData);
	memory[address] = data;
}

static uint8_t readFromPort(void* userData, uint16_t port) {
	uint8_t* io = static_cast<uint8_t *>(userData);
	return io[port];
}

static void writeToPort(void* userData, uint16_t port, uint8_t data) {
	uint8_t* io = static_cast<uint8_t *>(userData);
	io[port] = data;
}

ZxSpectrum::ZxSpectrum(std::string logFilePath) :
		m_isRunning(true),
        m_shouldInterrupt(0),
//		m_prevTstates(0),
		m_totalInstructionsCounter(0),
		m_exceededInstructionsCounter(0),
		m_interruptsCount(0)
{
	m_keyboard = new Keyboard(m_portsArray);
	m_cpu = new Z80(readFromMemory, writeToMemory, readFromPort, writeToPort);
	m_cpu->setMemoryUserData(m_memoryArray);
	m_cpu->setIoUserData(m_portsArray);

	reset();
}

ZxSpectrum::~ZxSpectrum() {
	delete m_cpu;
	delete m_keyboard;
}

void ZxSpectrum::loop() {
	while (m_isRunning) {
		const uint64_t startTstates = m_cpu->tStates();
		clock_gettime(CLOCK_MONOTONIC, &m_startTimestamp);

		bool shouldInterrupt = m_shouldInterrupt > 0;
		if (shouldInterrupt) {
			m_interruptsCount++;
			m_shouldInterrupt--;

			m_cpu->interrupt(0);
		}

		m_cpu->execute();

		uint64_t currentTstates = m_cpu->tStates();
		clock_gettime(CLOCK_MONOTONIC, &m_currentTimestamp);

		uint64_t expectedTime = (currentTstates - startTstates) * CPU_CLOCK_PERIOD;
		uint64_t actualTime = uint64_t(m_currentTimestamp.tv_nsec - m_startTimestamp.tv_nsec);

		if (expectedTime > actualTime) {
			while (expectedTime > actualTime) {
				clock_gettime(CLOCK_MONOTONIC, &m_currentTimestamp);
				actualTime = uint64_t(m_currentTimestamp.tv_nsec - m_startTimestamp.tv_nsec);
			}
		} else {
			m_exceededInstructionsCounter++;
		}

		m_totalInstructionsCounter++;

		m_instructionsCount++;
	}
}

void ZxSpectrum::quit() {
	m_isRunning = false;
}

void ZxSpectrum::reset() {
	m_cpu->reset();
}

void ZxSpectrum::verticalRefresh() {
	m_shouldInterrupt++;
}

uint32_t ZxSpectrum::interruptsCount() {
	uint32_t count = m_interruptsCount;
	m_interruptsCount = 0;
	return count;
}

uint32_t ZxSpectrum::instructionsCount() {
    uint32_t count = m_instructionsCount;
    m_instructionsCount = 0;
    return count;
}

void ZxSpectrum::onKeyPressed(const int keyCode) {
	m_keyboard->onKeyPressed(keyCode);
}

void ZxSpectrum::onKeyReleased(const int keyCode) {
	m_keyboard->onKeyReleased(keyCode);
}
