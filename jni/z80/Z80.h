//
// Created by Игорь on 20.06.2018.
//

#ifndef Z80_H
#define Z80_H


#include <cstdint>
#include <fstream>

typedef uint8_t (*Z80ReadData)(void *userData, uint16_t address);
typedef void (*Z80WriteData)(void *userData, uint16_t address, uint8_t data);

class Z80 {
public:
	Z80(Z80ReadData readFromMemory, Z80WriteData writeToMemory, Z80ReadData readFromPort, 
		Z80WriteData writeToPort);

	void setMemoryUserData(void *data) { m_memoryUserData = data; }
	void setIoUserData(void *data) { m_ioUserData = data; }

	void reset();
	void execute();
	void interrupt(uint8_t data);
	void nmi();

	uint8_t regA() const { return m_AF >> 8; }
	uint8_t regF() const { return m_AF & 0xff; }

	uint8_t regB() const { return m_BC >> 8; }
	uint8_t regC() const { return m_BC & 0xff; }

	uint8_t regD() const { return m_DE >> 8; }
	uint8_t regE() const { return m_DE & 0xff; }

	uint8_t regH() const { return m_HL >> 8; }
	uint8_t regL() const { return m_HL & 0xff; }

	uint8_t regAalt() const { return m_AFalt >> 8; }
	uint8_t regFalt() const { return m_AFalt & 0xff; }

	uint8_t regBalt() const { return m_BCalt >> 8; }
	uint8_t regCalt() const { return m_BCalt & 0xff; }

	uint8_t regDalt() const { return m_DEalt >> 8; }
	uint8_t regEalt() const { return m_DEalt & 0xff; }

	uint8_t regHalt() const { return m_HLalt >> 8; }
	uint8_t regLalt() const { return m_HLalt & 0xff; }

	uint16_t regBC() const { return m_BC; }
	uint16_t regDE() const { return m_DE; }
	uint16_t regHL() const { return m_HL; }

	uint64_t tStates() const { return m_tStates; }

	void setLogStream(std::fstream* logStream) { m_logStream = logStream; }

private:
	static const int FLAG_C_MASK =  1;
	static const int FLAG_N_MASK =  2;
	static const int FLAG_PV_MASK = 4;
	static const int FLAG_3_MASK =  8;
	static const int FLAG_H_MASK =  16;
	static const int FLAG_5_MASK =  32;
	static const int FLAG_Z_MASK =  64;
	static const int FLAG_S_MASK =  128;

	static const int parityBit[256];

	uint8_t m_sz53n_addTable[256];
	std::fstream* m_logStream;

	uint16_t m_PC;

	uint16_t m_AF;
	uint16_t m_BC;
	uint16_t m_DE;
	uint16_t m_HL;
	uint16_t m_IX;
	uint16_t m_IY;
	uint16_t m_SP;

	uint16_t m_AFalt;
	uint16_t m_BCalt;
	uint16_t m_DEalt;
	uint16_t m_HLalt;
	uint16_t m_IXalt;
	uint16_t m_IYalt;
	uint16_t m_SPalt;

	uint8_t m_I;
	uint8_t m_R;

	uint8_t (*m_readFromMemory)(void *userData, uint16_t address);
	void (*m_writeToMemory)(void *userData, uint16_t address, uint8_t data);
	void* m_memoryUserData;

	uint8_t (*m_readFromPort)(void *userData, uint16_t port);
	void (*m_writeToPort)(void *userData, uint16_t port, uint8_t data);
	void* m_ioUserData;

	uint64_t m_tStates;

	bool m_isHalted;
	bool m_isUndefinedState;

	bool m_IFF1;
	bool m_IFF2;
	uint8_t m_IM;
	bool m_isNmiRequested;
	bool m_isIntRequested;
	bool m_shouldDeferInt;
	uint8_t m_intVector;
	bool m_shouldExecuteIntVector;

	void setRegA(uint8_t value) { m_AF &= 0x00ff; m_AF |= value << 8; }
	void setRegF(uint8_t value) { m_AF &= 0xff00; m_AF |= value; }

	void setRegB(uint8_t value) { m_BC &= 0x00ff; m_BC |= value << 8; }
	void setRegC(uint8_t value) { m_BC &= 0xff00; m_BC |= value; }

	void setRegD(uint8_t value) { m_DE &= 0x00ff; m_DE |= value << 8; }
	void setRegE(uint8_t value) { m_DE &= 0xff00; m_DE |= value; }

	void setRegH(uint8_t value) { m_HL &= 0x00ff; m_HL |= value << 8; }
	void setRegL(uint8_t value) { m_HL &= 0xff00; m_HL |= value; }

	void setRegAalt(uint8_t value) { m_AFalt &= 0x00ff; m_AFalt |= value << 8; }
	void setRegFalt(uint8_t value) { m_AFalt &= 0xff00; m_AFalt |= value; }

	void setRegBalt(uint8_t value) { m_BCalt &= 0x00ff; m_BCalt |= value << 8; }
	void setRegCalt(uint8_t value) { m_BCalt &= 0xff00; m_BCalt |= value; }

	void setRegDalt(uint8_t value) { m_DEalt &= 0x00ff; m_DEalt |= value << 8; }
	void setRegEalt(uint8_t value) { m_DEalt &= 0xff00; m_DEalt |= value; }

	void setRegHalt(uint8_t value) { m_HLalt &= 0x00ff; m_HLalt |= value << 8; }
	void setRegLalt(uint8_t value) { m_HLalt &= 0xff00; m_HLalt |= value; }

	uint8_t read8(uint16_t address);
	void write8(uint16_t address, uint8_t value);

	uint16_t read16(uint16_t address);
	void write16(uint16_t address, uint16_t value);

	uint8_t ioRead(uint16_t port);
	void ioWrite(uint16_t port, uint8_t value);

	void incR() { m_R = (m_R & 0x80) | ((m_R + 1) & 0x7f); }
	void decR() { m_R = (m_R & 0x80) | ((m_R - 1) & 0x7f); }

	void doNmi();
	void doInt();
	void doExecute();
	void doExecuteED();
	void doExecuteCB();
	uint16_t doExecutionDDFD(uint16_t regValue, bool isIX);
	void doExecutionDDFDCB(uint16_t address, bool isIX);
	void unhalt();

	void doPush(uint16_t value);
	uint16_t doPop();

	void doOR(uint8_t value);
	void doXOR(uint8_t value);
	void doAND(uint8_t value);

	uint8_t doArithmetic(uint8_t value, bool withCarry, bool isSub);
	uint16_t doAddWord(uint16_t a1, uint16_t a2, bool withCarry, bool isSub);
	uint8_t doIncDec(uint8_t value, bool isDec);
	uint8_t doRLC(bool adjFlags, uint8_t value);
	uint8_t doRL(bool adjFlags, uint8_t value);
	uint8_t doRRC(bool adjFlags, uint8_t value);
	uint8_t doRR(bool adjFlags, uint8_t value);
	uint8_t doSL(uint8_t value, int isArith);
	uint8_t doSR(uint8_t value, int isArith);
	void doBIT(int b, uint8_t value);
	void doBIT_r(int b, uint8_t value);
	void doBIT_indexed(int b, uint16_t address);
	uint8_t doCP_HL();
	void doCPD();
	void doCPI();
	void doDAA();
	void doLDD();
	void doLDI();
	uint8_t doSetRes(bool isSet, int pos, uint8_t value);

	inline void setFlag(int mask, bool flag);
	inline bool isFlagSet(int mask);
	void adjustLogicFlag(bool flagH);
	void adjustFlags (uint8_t value);
	void adjustFlagSZP (uint8_t value);
	void adjust53Flags(uint16_t address);
};


#endif //Z80_H
