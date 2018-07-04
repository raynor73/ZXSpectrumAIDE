//
// Created by Igor Lapin on 14.06.18.
//

#ifndef Z80EXERCISER_H
#define Z80EXERCISER_H


#include <chrono>
#include "z80/Z80.h"

class Z80Exerciser {
public:
	Z80Exerciser(std::string &logFilePath);
	~Z80Exerciser();

	void runTest();

	uint8_t *memoryArray() { return m_memoryArray; }
	float mips() const { return m_mips; }

private:
	uint8_t m_memoryArray[0x10000];
	uint8_t m_ioArray[0x10000];

	Z80 *m_cpu;

	bool m_isPrevTimestampAvailable;
	std::chrono::time_point<std::chrono::steady_clock> m_prevTimestamp;
	float m_mips;
};


#endif //Z80EXERCISER_H
