//
// Created by Igor Lapin on 06.07.18.
//

#ifndef ZX_SPECTRUM_BEEPER_H
#define ZX_SPECTRUM_BEEPER_H


#include <atomic>
#include <ctime>

typedef void (*BeeperCallback)(int16_t* data, int size);

class Beeper {
public:
	Beeper(uint8_t *ioPorts, int sampleRate, int bufferSize, BeeperCallback callback);
	~Beeper();
	
	void loop();
	void stop();

private:
	static const int PORT_ADDRESS = 0xfe;
	static const int BEEPER_BIT_MASK = 0x10;

	int m_sampleRate;
	int m_bufferSize;
	BeeperCallback m_callback;
	uint8_t* m_ioPorts;
	int16_t* m_buffer;
	std::atomic<bool> m_isRunning;
	timespec m_startTimestamp;
    timespec m_currentTimestamp;
};


#endif //ZX_SPECTRUM_BEEPER_H
