#include "Beeper.h"

Beeper::Beeper(uint8_t *ioPorts, int sampleRate, int bufferSize, BeeperCallback callback) :
	m_ioPorts(ioPorts),
	m_sampleRate(sampleRate),
	m_bufferSize(bufferSize),
	m_callback(callback)
{
	m_buffer = new int16_t[m_bufferSize];
}

void Beeper::loop() {
	m_isRunning = true;
	uint64_t expectedTime = 1000000000 / m_sampleRate;

	while (m_isRunning) {
		for (int i = 0; i < m_bufferSize; i++) {
			clock_gettime(CLOCK_MONOTONIC, &m_startTimestamp);
			
			m_buffer[i] = (m_ioPorts[PORT_ADDRESS] & EAR_BIT_MASK) == 0 ? 0xc000 : 0x3fff;
			m_buffer[i] += (m_ioPorts[PORT_ADDRESS] & MIC_BIT_MASK) == 0 ? 0xc000 : 0x3fff;

			uint64_t actualTime;
			do {
				clock_gettime(CLOCK_MONOTONIC, &m_currentTimestamp);
				actualTime = uint64_t(m_currentTimestamp.tv_nsec - m_startTimestamp.tv_nsec);
			} while (expectedTime > actualTime);
		}
		m_callback(m_buffer, m_bufferSize);
	}
}

void Beeper::stop() {
	m_isRunning = false;
}

Beeper::~Beeper() {
	delete m_buffer;
}
