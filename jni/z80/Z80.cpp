//
// Created by Игорь on 20.06.2018.
//

#include <stdexcept>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <cstdio>
#include <cstring>
#include "Z80.h"

/*#define LOG_OPCODE(name) do { \
if (g_logCounter > 0) { g_logCounter--; __android_log_print(ANDROID_LOG_DEBUG, "!@#", "%X: %s; A: %X; B: %X; F: %X", m_PC, name, regA(), regB(), regF()); }\
 } while (false)*/
#define LOG_OPCODE(name)

const int Z80::parityBit[256] = {
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1 };

Z80::Z80(Z80ReadData readFromMemory, Z80WriteData writeToMemory, Z80ReadData readFromPort,
		 Z80WriteData writeToPort) :
		m_readFromMemory(readFromMemory),
		m_writeToMemory(writeToMemory),
		m_memoryUserData(nullptr),
		m_readFromPort(readFromPort),
		m_writeToPort(writeToPort),
		m_ioUserData(nullptr)
{
	std::memset(m_sz53n_addTable, 0, 256);
	for (int i = 0; i < 256; i++) {
		if (i > 0x7f) {
			m_sz53n_addTable[i] |= FLAG_S_MASK;
		}
		m_sz53n_addTable[i] |= i & (FLAG_5_MASK | FLAG_3_MASK);
	}
	m_sz53n_addTable[0] |= FLAG_Z_MASK;
	reset();
}

void Z80::reset() {
	m_PC = 0x0000;
	m_AF = 0xffff;
	m_SP = 0xffff;
	m_IM = 0;
	m_IFF1 = m_IFF2 = false;
	m_R = 0;
	m_I = 0;
	m_isHalted = false;
	m_isUndefinedState = false;
	m_tStates = 0;
	m_isNmiRequested = false;
	m_isIntRequested = false;
	m_shouldExecuteIntVector = false;
}

void Z80::interrupt(uint8_t data) {
	m_isIntRequested = true;
	m_intVector = data;
}

void Z80::nmi() {
	m_isNmiRequested = true;
}

//static int g_debugCounter = 0;
//static int g_logCounter = 0;
//static std::map<int, int> g_addressCounters;

void Z80::execute() {
	/*std::map<int, int>::const_iterator search = g_addressCounters.find(m_PC);
	if (search == g_addressCounters.end()) {
		g_addressCounters[m_PC] = 1;
	} else {
		g_addressCounters[m_PC] = search->second + 1;
	}*/
	/*if (m_PC == 0x1B3E) {
		g_debugCounter++;
	}*/
	/*if (m_PC == 0x1B3E) {
		g_debugCounter++;
	}
	g_debugCounter++;
	if (g_debugCounter % 1000000 == 0) {
		__android_log_print(ANDROID_LOG_DEBUG, "!@#", "g_debugCounter: %d", g_debugCounter);
	}*/
	/*g_debugCounter++;
	if (g_debugCounter > 1000000) {
		//for (const auto &pair : g_addressCounters) {
		//	__android_log_print(ANDROID_LOG_DEBUG, "!@#", "Address: %X: %d", pair.first, pair.second);
		//}

		m_isUndefinedState = true;
		throw std::runtime_error("Instructions limit exceeded");
	}*/

	if (m_isUndefinedState) {
		return;
	}

	if (m_isNmiRequested) {
		doNmi();
	} else if (m_isIntRequested && !m_shouldDeferInt && m_IFF1) {
		doInt();
	} else {
		m_shouldDeferInt = false;
		doExecute();
	}
}

uint8_t Z80::read8(uint16_t address) {
	m_tStates += 3;
	return m_readFromMemory(m_memoryUserData, address);
}

void Z80::write8(uint16_t address, uint8_t value) {
	m_tStates += 3;
	m_writeToMemory(m_memoryUserData, address, value);
}

uint16_t Z80::read16(uint16_t address) {
	uint8_t lsb = read8(address);
	uint8_t msb = read8(address + 1);
	return (msb << 8) | lsb;
}

void Z80::write16(uint16_t address, uint16_t value) {
	write8(address, value);
	write8(address + 1, value >> 8);
}

uint8_t Z80::ioRead(uint16_t port) {
	m_tStates += 4;
	return m_readFromPort(m_ioUserData, port);
}

void Z80::ioWrite(uint16_t port, uint8_t value) {
	m_tStates += 4;
	m_writeToPort(m_ioUserData, port, value);
}

void Z80::doNmi() {
	unhalt();
	m_IFF2 = m_IFF1;
	m_IFF1 = false;
	doPush(m_PC);
	m_isNmiRequested = false;
	m_tStates += 5;
}

void Z80::doInt() {
	unhalt();
	m_IFF1 = false;
	m_IFF2 = false;
	m_isIntRequested = false;
	switch (m_IM) {
		case 0:
			m_shouldExecuteIntVector = true;
			doExecute();
			m_shouldExecuteIntVector = false;
			break;

		case 1:
			doPush(m_PC);
			m_PC = 0x0038;
			m_tStates += 7;
			break;

		case 2: {
			doPush(m_PC);
			uint16_t vectorAddress = (m_I << 8) | m_intVector;
			m_PC = read16(vectorAddress);
			m_tStates += 7;
			break;
		}

		default: {
			m_isUndefinedState = true;
			std::ostringstream ss;
			ss << "Invalid interrupt mode: " << std::hex << uint16_t(m_IM);
			throw std::runtime_error(ss.str());
		}
	}
}

void Z80::doExecute() {
	incR();
	uint8_t opcode;
	if (m_isHalted) {
		opcode = 0x00;
	} else if (m_shouldExecuteIntVector) {
		opcode = m_intVector;
		m_tStates += 6;
	} else {
		opcode = read8(m_PC);
		m_PC++;
		m_tStates += 1;
	}

	switch (opcode) {
		case 0x00:
			LOG_OPCODE("NOP");
			break;

		case 0x01:
			LOG_OPCODE("LD BC, nn");
			m_BC = read16(m_PC);
			m_PC += 2;
			break;

		case 0x02:
			LOG_OPCODE("LD (BC), A");
			write8(m_BC, regA());
			break;

		case 0x03:
			LOG_OPCODE("INC BC");
			m_BC++;
			m_tStates += 2;
			break;

		case 0x04:
			LOG_OPCODE("INC B");
			setRegB(doIncDec(regB(), false));
			break;

		case 0x05:
			LOG_OPCODE("DEC B");
			setRegB(doIncDec(regB(), true));
			break;

		case 0x06:
			LOG_OPCODE("LD B, n");
			setRegB(read8(m_PC));
			m_PC += 1;
			break;

		case 0x07:
			LOG_OPCODE("RLCA");
			setRegA(doRLC(false, regA()));
			break;

		case 0x08: {
			LOG_OPCODE("EX AF, AF'");
			uint16_t tmp = m_AF;
			m_AF = m_AFalt;
			m_AFalt = tmp;
			break;
		}
			
		case 0x09:
			LOG_OPCODE("ADD HL, BC");
			m_HL = doAddWord(m_HL, m_BC, false, false);
			m_tStates += 7;
			break;

		case 0x0a:
			LOG_OPCODE("LD A, (BC)");
			setRegA(read8(m_BC));
			break;

		case 0x0b:
			LOG_OPCODE("DEC BC");
			m_BC--;
			m_tStates += 2;
			break;

		case 0x0c:
			LOG_OPCODE("INC C");
			setRegC(doIncDec(regC(), false));
			break;

		case 0x0d:
			LOG_OPCODE("DEC C");
			setRegC(doIncDec(regC(), true));
			break;

		case 0x0e:
			LOG_OPCODE("LD C, n");
			setRegC(read8(m_PC));
			m_PC += 1;
			break;

		case 0x0f:
			LOG_OPCODE("RRCA");
			setRegA(doRRC(false, regA()));
			break;

		case 0x10: {
			LOG_OPCODE("DJNZ");
			int8_t offset = read8(m_PC);
			m_PC++;
			setRegB(regB() - 1);
			if (regB() != 0) {
				m_tStates += 5;
				int32_t address = m_PC;
				address += offset;
				m_PC = address;
			}
			m_tStates += 1;
			break;
		}
			
		case 0x11:
			LOG_OPCODE("LD DE, nn");
			m_DE = read16(m_PC);
			m_PC += 2;
			break;

		case 0x12:
			LOG_OPCODE("LD (DE), A");
			write8(m_DE, regA());
			break;

		case 0x13:
			LOG_OPCODE("INC DE");
			m_DE++;
			m_tStates += 2;
			break;

		case 0x14:
			LOG_OPCODE("INC D");
			setRegD(doIncDec(regD(), false));
			break;

		case 0x15:
			LOG_OPCODE("DEC D");
			setRegD(doIncDec(regD(), true));
			break;

		case 0x16:
			LOG_OPCODE("LD D, n");
			setRegD(read8(m_PC));
			m_PC += 1;
			break;

		case 0x17:
			LOG_OPCODE("RLA");
			setRegA(doRL(false, regA()));
			break;

		case 0x18: {
			LOG_OPCODE("JR n");
			int8_t offset = read8(m_PC);
			m_PC += 1;
			int32_t address = m_PC;
			address += offset;
			m_PC = address;
			m_tStates += 5;
			break;
		}
			
		case 0x19:
			LOG_OPCODE("ADD HL, DE");
			m_HL = doAddWord(m_HL, m_DE, false, false);
			m_tStates += 7;
			break;

		case 0x1a:
			LOG_OPCODE("LD A, (DE)");
			setRegA(read8(m_DE));
			break;

		case 0x1b:
			LOG_OPCODE("DEC DE");
			m_DE--;
			m_tStates += 2;
			break;

		case 0x1c:
			LOG_OPCODE("INC E");
			setRegE(doIncDec(regE(), false));
			break;

		case 0x1d:
			LOG_OPCODE("DEC E");
			setRegE(doIncDec(regE(), true));
			break;

		case 0x1e:
			LOG_OPCODE("LD E, n");
			setRegE(read8(m_PC));
			m_PC += 1;
			break;

		case 0x1f:
			LOG_OPCODE("RRA");
			setRegA(doRR(false, regA()));
			break;

		case 0x20: {
			LOG_OPCODE("JR NZ, e");
			if (!isFlagSet(FLAG_Z_MASK)) {
				int8_t offset = read8(m_PC);
				m_PC += 1;
				int32_t pc32 = m_PC;
				pc32 += offset;
				m_PC = pc32;
				m_tStates += 5;
			} else {
				m_PC += 1;
			}
			break;
		}

		case 0x21:
			LOG_OPCODE("LD HL, nn");
			m_HL = read16(m_PC);
			m_PC += 2;
			break;

		case 0x22:
			LOG_OPCODE("LD (nn), HL");
			write16(read16(m_PC), m_HL);
			m_PC += 2;
			break;

		case 0x23:
			LOG_OPCODE("INC HL");
			m_HL++;
			m_tStates += 2;
			break;

		case 0x24:
			LOG_OPCODE("INC H");
			setRegH(doIncDec(regH(), false));
			break;

		case 0x25:
			LOG_OPCODE("DEC H");
			setRegH(doIncDec(regH(), true));
			break;

		case 0x26:
			LOG_OPCODE("LD H, n");
			setRegH(read8(m_PC));
			m_PC += 1;
			break;

		case 0x27:
			LOG_OPCODE("DAA");
			doDAA();
			break;

		case 0x28: {
			LOG_OPCODE("JR Z, e");
			if (isFlagSet(FLAG_Z_MASK)) {
				int8_t offset = read8(m_PC);
				m_PC += 1;
				int32_t pc32 = m_PC;
				pc32 += offset;
				m_PC = pc32;
				m_tStates += 5;
			} else {
				m_PC += 1;
			}
			break;
		}

		case 0x29:
			LOG_OPCODE("ADD HL, HL");
			m_HL = doAddWord(m_HL, m_HL, false, false);
			m_tStates += 7;
			break;

		case 0x2a:
			LOG_OPCODE("LD HL, (nn)");
			m_HL = read16(read16(m_PC));
			m_PC += 2;
			break;

		case 0x2b:
			LOG_OPCODE("DEC HL");
			m_HL--;
			m_tStates += 2;
			break;

		case 0x2c:
			LOG_OPCODE("INC L");
			setRegL(doIncDec(regL(), false));
			break;

		case 0x2d:
			LOG_OPCODE("DEC L");
			setRegL(doIncDec(regL(), true));
			break;

		case 0x2e:
			LOG_OPCODE("LD L, n");
			setRegL(read8(m_PC));
			m_PC += 1;
			break;

		case 0x2f:
			LOG_OPCODE("CPL");
			setRegA(~regA());
			setFlag(FLAG_H_MASK | FLAG_N_MASK, true);
			adjustFlags(regA());
			break;

		case 0x30:
			LOG_OPCODE("JR NC, nn");
			if (!isFlagSet(FLAG_C_MASK)) {
				int8_t offset = read8(m_PC);
				m_PC += 1;
				int32_t pc32 = m_PC;
				pc32 += offset;
				m_PC = pc32;
				m_tStates += 5;
			} else {
				m_PC += 1;
			}
			break;

		case 0x31:
			LOG_OPCODE("LD SP, nn");
			m_SP = read16(m_PC);
			m_PC += 2;
			break;

		case 0x32: {
			LOG_OPCODE("LD (nn), A");
			uint16_t address = read16(m_PC);
			write8(address, regA());
			m_PC += 2;
			break;
		}

		case 0x33:
			LOG_OPCODE("INC SP");
			m_SP++;
			m_tStates += 2;
			break;

		case 0x34: {
			LOG_OPCODE("INC (HL)");
			uint8_t value = read8(m_HL);
			write8(m_HL, doIncDec(value, false));
			m_tStates += 1;
			break;
		}

		case 0x35: {
			LOG_OPCODE("DEC (HL)");
			uint8_t value = read8(m_HL);
			write8(m_HL, doIncDec(value, true));
			m_tStates += 1;
			break;
		}

		case 0x36:
			LOG_OPCODE("LD (HL), n");
			write8(m_HL, read8(m_PC));
			m_PC += 1;
			break;

		case 0x37:
			LOG_OPCODE("SCF");
			setFlag(FLAG_C_MASK, true);
			setFlag(FLAG_N_MASK | FLAG_H_MASK, false);
			adjustFlags(regA());
			break;

		case 0x38: {
			LOG_OPCODE("JR C, e");
			if (isFlagSet(FLAG_C_MASK)) {
				int8_t offset = read8(m_PC);
				m_PC += 1;
				int32_t pc32 = m_PC;
				pc32 += offset;
				m_PC = pc32;
				m_tStates += 5;
			} else {
				m_PC += 1;
			}
			break;
		}
			
		case 0x39:
			LOG_OPCODE("ADD HL, SP");
			m_HL = doAddWord(m_HL, m_SP, false, false);
			m_tStates += 7;
			break;

		case 0x3a:
			LOG_OPCODE("LD A, (nn)");
			setRegA(read8(read16(m_PC)));
			m_PC += 2;
			break;

		case 0x3b:
			LOG_OPCODE("DEC SP");
			m_SP--;
			m_tStates += 2;
			break;

		case 0x3c:
			LOG_OPCODE("INC A");
			setRegA(doIncDec(regA(), false));
			break;

		case 0x3d:
			LOG_OPCODE("DEC A");
			setRegA(doIncDec(regA(), true));
			break;

		case 0x3e:
			LOG_OPCODE("LD A, n");
			setRegA(read8(m_PC));
			m_PC += 1;
			break;

		case 0x3f:
			LOG_OPCODE("CCF");
			setFlag(FLAG_H_MASK, isFlagSet(FLAG_C_MASK));
			setFlag(FLAG_C_MASK, !isFlagSet(FLAG_C_MASK));
			setFlag(FLAG_N_MASK, false);
			adjustFlags(regA());
			break;

		case 0x40:
			LOG_OPCODE("LD B, B");
			// do nothing
			break;

		case 0x41:
			LOG_OPCODE("LD B, C");
			setRegB(regC());
			break;

		case 0x42:
			LOG_OPCODE("LD B, D");
			setRegB(regD());
			break;

		case 0x43:
			LOG_OPCODE("LD B, E");
			setRegB(regE());
			break;

		case 0x44:
			LOG_OPCODE("LD B, H");
			setRegB(regH());
			break;

		case 0x45:
			LOG_OPCODE("LD B, L");
			setRegB(regL());
			break;

		case 0x46:
			LOG_OPCODE("LD B, (HL)");
			setRegB(read8(m_HL));
			break;

		case 0x47:
			LOG_OPCODE("LD B, A");
			setRegB(regA());
			break;

		case 0x48:
			LOG_OPCODE("LD C, B");
			setRegC(regB());
			break;

		case 0x49:
			LOG_OPCODE("LD C, C");
			// do nothing
			break;

		case 0x4a:
			LOG_OPCODE("LD C, D");
			setRegC(regD());
			break;

		case 0x4b:
			LOG_OPCODE("LD C, E");
			setRegC(regE());
			break;

		case 0x4c:
			LOG_OPCODE("LD C, H");
			setRegC(regH());
			break;

		case 0x4d:
			LOG_OPCODE("LD C, L");
			setRegC(regL());
			break;

		case 0x4e:
			LOG_OPCODE("LD C, (HL)");
			setRegC(read8(m_HL));
			break;

		case 0x4f:
			LOG_OPCODE("LD C, A");
			setRegC(regA());
			break;

		case 0x56:
			LOG_OPCODE("LD D, (HL)");
			setRegD(read8(m_HL));
			break;

		case 0x50:
			LOG_OPCODE("LD D, B");
			setRegD(regB());
			break;

		case 0x51:
			LOG_OPCODE("LD D, C");
			setRegD(regC());
			break;

		case 0x52:
			LOG_OPCODE("LD D, D");
			// do nothing
			break;

		case 0x53:
			LOG_OPCODE("LD D, E");
			setRegD(regE());
			break;

		case 0x54:
			LOG_OPCODE("LD D, H");
			setRegD(regH());
			break;

		case 0x55:
			LOG_OPCODE("LD D, L");
			setRegD(regL());
			break;

		case 0x57:
			LOG_OPCODE("LD D, A");
			setRegD(regA());
			break;

		case 0x5e:
			LOG_OPCODE("LD E, (HL)");
			setRegE(read8(m_HL));
			break;

		case 0x58:
			LOG_OPCODE("LD E, B");
			setRegE(regB());
			break;

		case 0x59:
			LOG_OPCODE("LD E, C");
			setRegE(regC());
			break;

		case 0x5a:
			LOG_OPCODE("LD E, D");
			setRegE(regD());
			break;

		case 0x5b:
			LOG_OPCODE("LD E, E");
			// do nothing
			break;

		case 0x5c:
			LOG_OPCODE("LD E, H");
			setRegE(regH());
			break;

		case 0x5d:
			LOG_OPCODE("LD E, L");
			setRegE(regL());
			break;

		case 0x5f:
			LOG_OPCODE("LD E, A");
			setRegE(regA());
			break;

		case 0x60:
			LOG_OPCODE("LD H, B");
			setRegH(regB());
			break;

		case 0x61:
			LOG_OPCODE("LD H, C");
			setRegH(regC());
			break;

		case 0x62:
			LOG_OPCODE("LD H, D");
			setRegH(regD());
			break;

		case 0x63:
			LOG_OPCODE("LD H, E");
			setRegH(regE());
			break;

		case 0x64:
			LOG_OPCODE("LD H, H");
			// do nothing
			break;

		case 0x65:
			LOG_OPCODE("LD H, L");
			setRegH(regL());
			break;

		case 0x66:
			LOG_OPCODE("LD H, (HL)");
			setRegH(read8(m_HL));
			break;

		case 0x67:
			LOG_OPCODE("LD H, A");
			setRegH(regA());
			break;

		case 0x68:
			LOG_OPCODE("LD L, B");
			setRegL(regB());
			break;

		case 0x69:
			LOG_OPCODE("LD L, C");
			setRegL(regC());
			break;

		case 0x6a:
			LOG_OPCODE("LD L, D");
			setRegL(regD());
			break;

		case 0x6b:
			LOG_OPCODE("LD L, E");
			setRegL(regE());
			break;

		case 0x6c:
			LOG_OPCODE("LD L, H");
			setRegL(regH());
			break;

		case 0x6d:
			LOG_OPCODE("LD L, L");
			// do nothing
			break;

		case 0x6e:
			LOG_OPCODE("LD L, (HL)");
			setRegL(read8(m_HL));
			break;

		case 0x6f:
			LOG_OPCODE("LD L, A");
			setRegL(regA());
			break;

		case 0x70:
			LOG_OPCODE("LD (HL), B");
			write8(m_HL, regB());
			break;

		case 0x71:
			LOG_OPCODE("LD (HL), C");
			write8(m_HL, regC());
			break;

		case 0x72:
			LOG_OPCODE("LD (HL), D");
			write8(m_HL, regD());
			break;

		case 0x73:
			LOG_OPCODE("LD (HL), E");
			write8(m_HL, regE());
			break;

		case 0x74:
			LOG_OPCODE("LD (HL), H");
			write8(m_HL, regH());
			break;

		case 0x75:
			LOG_OPCODE("LD (HL), L");
			write8(m_HL, regL());
			break;

		case 0x76:
			LOG_OPCODE("HALT");
			m_isHalted = true;
			break;

		case 0x77:
			LOG_OPCODE("LD (HL), A");
			write8(m_HL, regA());
			break;

		case 0x78:
			LOG_OPCODE("LD A, B");
			setRegA(regB());
			break;

		case 0x79:
			LOG_OPCODE("LD A, C");
			setRegA(regC());
			break;

		case 0x7a:
			LOG_OPCODE("LD A, D");
			setRegA(regD());
			break;

		case 0x7b:
			LOG_OPCODE("LD A, E");
			setRegA(regE());
			break;

		case 0x7c:
			LOG_OPCODE("LD A, H");
			setRegA(regH());
			break;

		case 0x7d:
			LOG_OPCODE("LD A, L");
			setRegA(regL());
			break;

		case 0x7e:
			LOG_OPCODE("LD A, (HL)");
			setRegA(read8(m_HL));
			break;

		case 0x7f:
			LOG_OPCODE("LD A, A");
			// do nothing
			break;

		case 0x80:
			LOG_OPCODE("ADD A, B");
			setRegA(doArithmetic(regB(), false, false));
			break;

		case 0x81:
			LOG_OPCODE("ADD A, C");
			setRegA(doArithmetic(regC(), false, false));
			break;

		case 0x82:
			LOG_OPCODE("ADD A, D");
			setRegA(doArithmetic(regD(), false, false));
			break;

		case 0x83:
			LOG_OPCODE("ADD A, E");
			setRegA(doArithmetic(regE(), false, false));
			break;

		case 0x84:
			LOG_OPCODE("ADD A, H");
			setRegA(doArithmetic(regH(), false, false));
			break;

		case 0x85:
			LOG_OPCODE("ADD A, L");
			setRegA(doArithmetic(regL(), false, false));
			break;

		case 0x86:
			LOG_OPCODE("ADD A, (HL)");
			setRegA(doArithmetic(read8(m_HL), false, false));
			break;

		case 0x87:
			LOG_OPCODE("ADD A, A");
			setRegA(doArithmetic(regA(), false, false));
			break;

		case 0x88:
			LOG_OPCODE("ADC A, B");
			setRegA(doArithmetic(regB(), true, false));
			break;

		case 0x89:
			LOG_OPCODE("ADC A, C");
			setRegA(doArithmetic(regC(), true, false));
			break;

		case 0x8a:
			LOG_OPCODE("ADC A, D");
			setRegA(doArithmetic(regD(), true, false));
			break;

		case 0x8b:
			LOG_OPCODE("ADC A, E");
			setRegA(doArithmetic(regE(), true, false));
			break;

		case 0x8c:
			LOG_OPCODE("ADC A, H");
			setRegA(doArithmetic(regH(), true, false));
			break;

		case 0x8d:
			LOG_OPCODE("ADC A, L");
			setRegA(doArithmetic(regL(), true, false));
			break;

		case 0x8e:
			LOG_OPCODE("ADC A, (HL)");
			setRegA(doArithmetic(read8(m_HL), true, false));
			break;

		case 0x8f:
			LOG_OPCODE("ADC A, A");
			setRegA(doArithmetic(regA(), true, false));
			break;

		case 0x90:
			LOG_OPCODE("SUB A, B");
			setRegA(doArithmetic(regB(), false, true));
			break;

		case 0x91:
			LOG_OPCODE("SUB A, C");
			setRegA(doArithmetic(regC(), false, true));
			break;

		case 0x92:
			LOG_OPCODE("SUB A, D");
			setRegA(doArithmetic(regD(), false, true));
			break;

		case 0x93:
			LOG_OPCODE("SUB A, E");
			setRegA(doArithmetic(regE(), false, true));
			break;

		case 0x94:
			LOG_OPCODE("SUB A, H");
			setRegA(doArithmetic(regH(), false, true));
			break;

		case 0x95:
			LOG_OPCODE("SUB A, L");
			setRegA(doArithmetic(regL(), false, true));
			break;

		case 0x96:
			LOG_OPCODE("SUB A, (HL)");
			setRegA(doArithmetic(read8(m_HL), false, true));
			break;

		case 0x97:
			LOG_OPCODE("SUB A, A");
			setRegA(doArithmetic(regA(), false, true));
			break;

		case 0x98:
			LOG_OPCODE("SBC A, B");
			setRegA(doArithmetic(regB(), true, true));
			break;

		case 0x99:
			LOG_OPCODE("SBC A, C");
			setRegA(doArithmetic(regC(), true, true));
			break;

		case 0x9a:
			LOG_OPCODE("SBC A, D");
			setRegA(doArithmetic(regD(), true, true));
			break;

		case 0x9b:
			LOG_OPCODE("SBC A, E");
			setRegA(doArithmetic(regE(), true, true));
			break;

		case 0x9c:
			LOG_OPCODE("SBC A, H");
			setRegA(doArithmetic(regH(), true, true));
			break;

		case 0x9d:
			LOG_OPCODE("SBC A, L");
			setRegA(doArithmetic(regL(), true, true));
			break;

		case 0x9e:
			LOG_OPCODE("SBC A, (HL)");
			setRegA(doArithmetic(read8(m_HL), true, true));
			break;

		case 0x9f:
			LOG_OPCODE("SBC A, A");
			setRegA(doArithmetic(regA(), true, true));
			break;

		case 0xa0:
			LOG_OPCODE("AND B");
			doAND(regB());
			break;

		case 0xa1:
			LOG_OPCODE("AND C");
			doAND(regC());
			break;

		case 0xa2:
			LOG_OPCODE("AND D");
			doAND(regD());
			break;

		case 0xa3:
			LOG_OPCODE("AND E");
			doAND(regE());
			break;

		case 0xa4:
			LOG_OPCODE("AND H");
			doAND(regH());
			break;

		case 0xa5:
			LOG_OPCODE("AND L");
			doAND(regL());
			break;

		case 0xa6:
			LOG_OPCODE("AND (HL)");
			doAND(read8(m_HL));
			break;

		case 0xa7:
			LOG_OPCODE("AND A");
			doAND(regA());
			break;

		case 0xa8:
			LOG_OPCODE("XOR B");
			doXOR(regB());
			break;

		case 0xa9:
			LOG_OPCODE("XOR C");
			doXOR(regC());
			break;

		case 0xaa:
			LOG_OPCODE("XOR D");
			doXOR(regD());
			break;

		case 0xab:
			LOG_OPCODE("XOR E");
			doXOR(regE());
			break;

		case 0xac:
			LOG_OPCODE("XOR H");
			doXOR(regH());
			break;

		case 0xad:
			LOG_OPCODE("XOR L");
			doXOR(regL());
			break;

		case 0xae:
			LOG_OPCODE("XOR (HL)");
			doXOR(read8(m_HL));
			break;

		case 0xaf:
			LOG_OPCODE("XOR A");
			doXOR(regA());
			break;

		case 0xb0:
			LOG_OPCODE("OR B");
			doOR(regB());
			break;

		case 0xb1:
			LOG_OPCODE("OR C");
			doOR(regC());
			break;

		case 0xb2:
			LOG_OPCODE("OR D");
			doOR(regD());
			break;

		case 0xb3:
			LOG_OPCODE("OR E");
			doOR(regE());
			break;

		case 0xb4:
			LOG_OPCODE("OR H");
			doOR(regH());
			break;

		case 0xb5:
			LOG_OPCODE("OR L");
			doOR(regL());
			break;

		case 0xb6:
			LOG_OPCODE("OR (HL)");
			doOR(read8(m_HL));
			break;

		case 0xb7:
			LOG_OPCODE("OR A");
			doOR(regA());
			break;

		case 0xb8:
			LOG_OPCODE("CP B");
			doArithmetic(regB(), false, true);
			adjustFlags(regB());
			break;

		case 0xb9:
			LOG_OPCODE("CP C");
			doArithmetic(regC(), false, true);
			adjustFlags(regC());
			break;

		case 0xba:
			LOG_OPCODE("CP D");
			doArithmetic(regD(), false, true);
			adjustFlags(regD());
			break;

		case 0xbb:
			LOG_OPCODE("CP E");
			doArithmetic(regE(), false, true);
			adjustFlags(regE());
			break;

		case 0xbc:
			LOG_OPCODE("CP H");
			doArithmetic(regH(), false, true);
			adjustFlags(regH());
			break;

		case 0xbd:
			LOG_OPCODE("CP L");
			doArithmetic(regL(), false, true);
			adjustFlags(regL());
			break;

		case 0xbe: {
			LOG_OPCODE("CP (HL)");
			uint8_t value = read8(m_HL);
			doArithmetic(value, false, true);
			adjustFlags(value);
			break;
		}

		case 0xbf:
			LOG_OPCODE("CP A");
			doArithmetic(regA(), false, true);
			adjustFlags(regA());
			break;

		case 0xc0:
			LOG_OPCODE("RET NZ");
			if (!isFlagSet(FLAG_Z_MASK)) {
				m_PC = doPop();
			}
			m_tStates += 1;
			break;

		case 0xc1:
			LOG_OPCODE("POP BC");
			m_BC = doPop();
			break;

		case 0xc3:
			LOG_OPCODE("JP nn");
			m_PC = read16(m_PC);
			break;

		case 0xc5:
			LOG_OPCODE("PUSH BC");
			doPush(m_BC);
			m_tStates += 1;
			break;

		case 0xc8:
			LOG_OPCODE("RET Z");
			if (isFlagSet(FLAG_Z_MASK)) {
				m_PC = doPop();
			}
			m_tStates += 1;
			break;

		case 0xc9:
			LOG_OPCODE("RET");
			m_PC = doPop();
			break;

		case 0xc2:
			LOG_OPCODE("JP NZ, nn");
			if (!isFlagSet(FLAG_Z_MASK)) {
				m_PC = read16(m_PC);
			} else {
				m_PC += 2;
			}
			break;

		case 0xc4: {
			LOG_OPCODE("CALL NZ, nn");
			if (!isFlagSet(FLAG_Z_MASK)) {
				uint16_t address = read16(m_PC);
				m_PC += 2;
				doPush(m_PC);
				m_PC = address;
				m_tStates += 1;
			} else {
				m_PC += 2;
			}
			break;
		}

		case 0xc6:
			LOG_OPCODE("ADD A, n");
			setRegA(doArithmetic(read8(m_PC), false, false));
			m_PC += 1;
			break;

		case 0xca:
			LOG_OPCODE("JP Z, nn");
			if (isFlagSet(FLAG_Z_MASK)) {
				m_PC = read16(m_PC);
			} else {
				m_PC += 2;
			}
			break;

		case 0xcb:
			doExecuteCB();
			break;

		case 0xcc: {
			LOG_OPCODE("CALL Z, nn");
			if (isFlagSet(FLAG_Z_MASK)) {
				uint16_t address = read16(m_PC);
				m_PC += 2;
				doPush(m_PC);
				m_PC = address;
				m_tStates += 1;
			} else {
				m_PC += 2;
			}
			break;
		}

		case 0xcd: {
			LOG_OPCODE("CALL nn");
			uint16_t address = read16(m_PC);
			m_PC += 2;
			doPush(m_PC);
			m_PC = address;
			m_tStates += 1;
			break;
		}

		case 0xce:
			LOG_OPCODE("ADC A, n");
			setRegA(doArithmetic(read8(m_PC), true, false));
			m_PC += 1;
			break;
			
		case 0xcf:
			LOG_OPCODE("RST 0x08");
			doPush(m_PC);
			m_PC = 0x08;
			m_tStates += 1;
			break;

		case 0xd0:
			LOG_OPCODE("RET NC");
			if (!isFlagSet(FLAG_C_MASK)) {
				m_PC = doPop();
			}
			m_tStates += 1;
			break;

		case 0xd1:
			LOG_OPCODE("POP DE");
			m_DE = doPop();
			break;

		case 0xd2:
			LOG_OPCODE("JP NC, nn");
			if (!isFlagSet(FLAG_C_MASK)) {
				m_PC = read16(m_PC);
			} else {
				m_PC += 2;
			}
			break;

		case 0xd3: {
			LOG_OPCODE("OUT (n), A");
			uint8_t a = regA();
			ioWrite((a << 8) | read8(m_PC), a);
			m_PC += 1;
			break;
		}

		case 0xd4: {
			LOG_OPCODE("CALL NC, nn");
			if (!isFlagSet(FLAG_C_MASK)) {
				uint16_t address = read16(m_PC);
				m_PC += 2;
				doPush(m_PC);
				m_PC = address;
				m_tStates += 1;
			} else {
				m_PC += 2;
			}
			break;
		}

		case 0xd5:
			LOG_OPCODE("PUSH DE");
			doPush(m_DE);
			m_tStates += 1;
			break;

		case 0xd6:
			LOG_OPCODE("SUB A, n");
			setRegA(doArithmetic(read8(m_PC), false, true));
			m_PC += 1;
			break;

		case 0xd7:
			LOG_OPCODE("RST 0x10");
			doPush(m_PC);
			m_PC = 0x10;
			m_tStates += 1;
			break;
			
		case 0xd8:
			LOG_OPCODE("RET C");
			if (isFlagSet(FLAG_C_MASK)) {
				m_PC = doPop();
			}
			m_tStates += 1;
			break;

		case 0xd9: {
			LOG_OPCODE("EXX");
			uint16_t tmp = m_BC;
			m_BC = m_BCalt;
			m_BCalt = tmp;

			tmp = m_DE;
			m_DE = m_DEalt;
			m_DEalt = tmp;

			tmp = m_HL;
			m_HL = m_HLalt;
			m_HLalt = tmp;
			break;
		}

		case 0xda:
			LOG_OPCODE("JP C, nn");
			if (isFlagSet(FLAG_C_MASK)) {
				m_PC = read16(m_PC);
			} else {
				m_PC += 2;
			}
			break;

		case 0xdb: {
			LOG_OPCODE("IN A, (n)");
			uint8_t port = read8(m_PC);
			setRegA(ioRead(regA() << 8 | port));
			m_PC += 1;
			break;
		}
			
		case 0xdc: {
			LOG_OPCODE("CALL C, nn");
			if (isFlagSet(FLAG_C_MASK)) {
				uint16_t address = read16(m_PC);
				m_PC += 2;
				doPush(m_PC);
				m_PC = address;
				m_tStates += 1;
			} else {
				m_PC += 2;
			}
			break;
		}

		case 0xde:
			LOG_OPCODE("SBC A, n");
			setRegA(doArithmetic(read8(m_PC), true, true));
			m_PC += 1;
			break;

		case 0xdf:
			LOG_OPCODE("RST 0x18");
			doPush(m_PC);
			m_PC = 0x018;
			m_tStates += 1;
			break;
			
		case 0xe0:
			LOG_OPCODE("RET NP");
			if (!isFlagSet(FLAG_PV_MASK)) {
				m_PC = doPop();
			}
			m_tStates += 1;
			break;

		case 0xe1:
			LOG_OPCODE("POP HL");
			m_HL = doPop();
			break;

		case 0xe2:
			LOG_OPCODE("JP NP, nn");
			if (!isFlagSet(FLAG_PV_MASK)) {
				m_PC = read16(m_PC);
			} else {
				m_PC += 2;
			}
			break;

		case 0xe3: {
			LOG_OPCODE("EX (SP), HL");
			uint16_t tmp = read16(m_SP);
			write16(m_SP, m_HL);
			m_HL = tmp;
			m_tStates += 3;
			break;
		}

		case 0xe4: {
			LOG_OPCODE("CALL NP, nn");
			if (!isFlagSet(FLAG_PV_MASK)) {
				uint16_t address = read16(m_PC);
				m_PC += 2;
				doPush(m_PC);
				m_PC = address;
				m_tStates += 1;
			} else {
				m_PC += 2;
			}
			break;
		}

		case 0xe5:
			LOG_OPCODE("PUSH HL");
			doPush(m_HL);
			m_tStates += 1;
			break;

		case 0xe6:
			LOG_OPCODE("AND n");
			doAND(read8(m_PC));
			m_PC += 1;
			break;

		case 0xe7:
			LOG_OPCODE("RST 0x20");
			doPush(m_PC);
			m_PC = 0x020;
			m_tStates += 1;
			break;
			
		case 0xe8:
			LOG_OPCODE("RET P");
			if (isFlagSet(FLAG_PV_MASK)) {
				m_PC = doPop();
			}
			m_tStates += 1;
			break;

		case 0xe9:
			LOG_OPCODE("JP (HL)");
			m_PC = m_HL;
			break;

		case 0xea:
			LOG_OPCODE("JP P, nn");
			if (isFlagSet(FLAG_PV_MASK)) {
				m_PC = read16(m_PC);
			} else {
				m_PC += 2;
			}
			break;

		case 0xeb: {
			LOG_OPCODE("EX DE, HL");
			uint16_t tmp = m_DE;
			m_DE = m_HL;
			m_HL = tmp;
			break;
		}

		case 0xec: {
			LOG_OPCODE("CALL P, nn");
			if (isFlagSet(FLAG_PV_MASK)) {
				uint16_t address = read16(m_PC);
				m_PC += 2;
				doPush(m_PC);
				m_PC = address;
				m_tStates += 1;
			} else {
				m_PC += 2;
			}
			break;
		}

		case 0xed:
			doExecuteED();
			break;

		case 0xee:
			LOG_OPCODE("XOR n");
			doXOR(read8(m_PC));
			m_PC += 1;
			break;

		case 0xef:
			LOG_OPCODE("RST 0x28");
			doPush(m_PC);
			m_PC = 0x028;
			m_tStates += 1;
			break;
			
		case 0xdd:
			m_IX = doExecutionDDFD(m_IX, true);
			break;

		case 0xf0:
			LOG_OPCODE("RET NS");
			if (!isFlagSet(FLAG_S_MASK)) {
				m_PC = doPop();
			}
			m_tStates += 1;
			break;

		case 0xf1:
			LOG_OPCODE("POP AF");
			m_AF = doPop();
			break;

		case 0xf2:
			LOG_OPCODE("JP NS, nn");
			if (!isFlagSet(FLAG_S_MASK)) {
				m_PC = read16(m_PC);
			} else {
				m_PC += 2;
			}
			break;

		case 0xf3:
			LOG_OPCODE("DI");
			m_IFF1 = m_IFF2 = false;
			m_shouldDeferInt = true;
			break;

		case 0xf4: {
			LOG_OPCODE("CALL NS, nn");
			if (!isFlagSet(FLAG_S_MASK)) {
				uint16_t address = read16(m_PC);
				m_PC += 2;
				doPush(m_PC);
				m_PC = address;
				m_tStates += 1;
			} else {
				m_PC += 2;
			}
			break;
		}

		case 0xf5:
			LOG_OPCODE("PUSH AF");
			doPush(m_AF);
			m_tStates += 1;
			break;

		case 0xf6:
			LOG_OPCODE("OR n");
			doOR(read8(m_PC));
			m_PC += 1;
			break;

		case 0xf7:
			LOG_OPCODE("RST 0x30");
			doPush(m_PC);
			m_PC = 0x30;
			m_tStates += 1;
			break;
			
		case 0xf8:
			LOG_OPCODE("RET S");
			if (isFlagSet(FLAG_S_MASK)) {
				m_PC = doPop();
			}
			m_tStates += 1;
			break;

		case 0xf9:
			LOG_OPCODE("LD SP, HL");
			m_SP = m_HL;
			break;

		case 0xfa:
			LOG_OPCODE("JP S, nn");
			if (isFlagSet(FLAG_S_MASK)) {
				m_PC = read16(m_PC);
			} else {
				m_PC += 2;
			}
			break;

		case 0xfb:
			LOG_OPCODE("EI");
			m_IFF1 = m_IFF2 = true;
			m_shouldDeferInt = true;
			break;

		case 0xfc: {
			LOG_OPCODE("CALL S, nn");
			if (isFlagSet(FLAG_S_MASK)) {
				uint16_t address = read16(m_PC);
				m_PC += 2;
				doPush(m_PC);
				m_PC = address;
				m_tStates += 1;
			} else {
				m_PC += 2;
			}
			break;
		}

		case 0xfd:
			m_IY = doExecutionDDFD(m_IY, false);
			break;

		case 0xfe: {
			LOG_OPCODE("CP n");
			uint8_t value = read8(m_PC);
			doArithmetic(value, false, true);
			adjustFlags(value);
			m_PC += 1;
			break;
		}

		default: {
			m_isUndefinedState = true;
			std::ostringstream ss;
			ss << "Invalid opcode: " << std::hex << uint16_t(opcode);
			throw std::runtime_error(ss.str());
		}
	}
}

void Z80::doExecuteCB() {
	uint8_t opcode = read8(m_PC);
	m_PC++;
	switch (opcode) {
		case 0x00:
			LOG_OPCODE("RLC B");
			setRegB(doRLC(true, regB()));
			break;

		case 0x01:
			LOG_OPCODE("RLC C");
			setRegC(doRLC(true, regC()));
			break;

		case 0x02:
			LOG_OPCODE("RLC D");
			setRegD(doRLC(true, regD()));
			break;

		case 0x03:
			LOG_OPCODE("RLC E");
			setRegE(doRLC(true, regE()));
			break;

		case 0x04:
			LOG_OPCODE("RLC H");
			setRegH(doRLC(true, regH()));
			break;

		case 0x05:
			LOG_OPCODE("RLC L");
			setRegL(doRLC(true, regL()));
			break;

		case 0x06:
			LOG_OPCODE("RLC (HL)");
			write8(m_HL, doRLC(true, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0x07:
			LOG_OPCODE("RLC A");
			setRegA(doRLC(true, regA()));
			break;

		case 0x08:
			LOG_OPCODE("RRC B");
			setRegB(doRRC(true, regB()));
			break;

		case 0x09:
			LOG_OPCODE("RRC C");
			setRegC(doRRC(true, regC()));
			break;

		case 0x0a:
			LOG_OPCODE("RRC D");
			setRegD(doRRC(true, regD()));
			break;

		case 0x0b:
			LOG_OPCODE("RRC E");
			setRegE(doRRC(true, regE()));
			break;

		case 0x0c:
			LOG_OPCODE("RRC H");
			setRegH(doRRC(true, regH()));
			break;

		case 0x0d:
			LOG_OPCODE("RRC L");
			setRegL(doRRC(true, regL()));
			break;

		case 0x0e:
			LOG_OPCODE("RRC (HL)");
			write8(m_HL, doRRC(true, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0x0f:
			LOG_OPCODE("RRC A");
			setRegA(doRRC(true, regA()));
			break;

		case 0x10:
			LOG_OPCODE("RL B");
			setRegB(doRL(true, regB()));
			break;

		case 0x11:
			LOG_OPCODE("RL C");
			setRegC(doRL(true, regC()));
			break;

		case 0x12:
			LOG_OPCODE("RL D");
			setRegD(doRL(true, regD()));
			break;

		case 0x13:
			LOG_OPCODE("RL E");
			setRegE(doRL(true, regE()));
			break;

		case 0x14:
			LOG_OPCODE("RL H");
			setRegH(doRL(true, regH()));
			break;

		case 0x15:
			LOG_OPCODE("RL L");
			setRegL(doRL(true, regL()));
			break;

		case 0x16:
			LOG_OPCODE("RL (HL)");
			write8(m_HL, doRL(true, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0x17:
			LOG_OPCODE("RL A");
			setRegA(doRL(true, regA()));
			break;

		case 0x18:
			LOG_OPCODE("RR B");
			setRegB(doRR(true, regB()));
			break;

		case 0x19:
			LOG_OPCODE("RR C");
			setRegC(doRR(true, regC()));
			break;

		case 0x1a:
			LOG_OPCODE("RR D");
			setRegD(doRR(true, regD()));
			break;

		case 0x1b:
			LOG_OPCODE("RR E");
			setRegE(doRR(true, regE()));
			break;

		case 0x1c:
			LOG_OPCODE("RR H");
			setRegH(doRR(true, regH()));
			break;

		case 0x1d:
			LOG_OPCODE("RR L");
			setRegL(doRR(true, regL()));
			break;

		case 0x1e:
			LOG_OPCODE("RR (HL)");
			write8(m_HL, doRR(true, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0x1f:
			LOG_OPCODE("RR A");
			setRegA(doRR(true, regA()));
			break;

		case 0x20:
			LOG_OPCODE("SLA B");
			setRegB(doSL(regB(), true));
			break;

		case 0x21:
			LOG_OPCODE("SLA C");
			setRegC(doSL(regC(), true));
			break;

		case 0x22:
			LOG_OPCODE("SLA D");
			setRegD(doSL(regD(), true));
			break;

		case 0x23:
			LOG_OPCODE("SLA E");
			setRegE(doSL(regE(), true));
			break;

		case 0x24:
			LOG_OPCODE("SLA H");
			setRegH(doSL(regH(), true));
			break;

		case 0x25:
			LOG_OPCODE("SLA L");
			setRegL(doSL(regL(), true));
			break;

		case 0x26:
			LOG_OPCODE("SLA (HL)");
			write8(m_HL, doSL(read8(m_HL), true));
			m_tStates += 1;
			break;

		case 0x27:
			LOG_OPCODE("SLA A");
			setRegA(doSL(regA(), true));
			break;

		case 0x28:
			LOG_OPCODE("SRA B");
			setRegB(doSR(regB(), true));
			break;

		case 0x29:
			LOG_OPCODE("SRA C");
			setRegC(doSR(regC(), true));
			break;

		case 0x2a:
			LOG_OPCODE("SRA D");
			setRegD(doSR(regD(), true));
			break;

		case 0x2b:
			LOG_OPCODE("SRA E");
			setRegE(doSR(regE(), true));
			break;

		case 0x2c:
			LOG_OPCODE("SRA H");
			setRegH(doSR(regH(), true));
			break;

		case 0x2d:
			LOG_OPCODE("SRA L");
			setRegL(doSR(regL(), true));
			break;

		case 0x2e:
			LOG_OPCODE("SRA (HL)");
			write8(m_HL, doSR(read8(m_HL), true));
			m_tStates += 1;
			break;

		case 0x2f:
			LOG_OPCODE("SRA A");
			setRegA(doSR(regA(), true));
			break;
			
		case 0x30:
			LOG_OPCODE("SLL B");
			setRegB(doSL(regB(), false));
			break;

		case 0x31:
			LOG_OPCODE("SLL C");
			setRegC(doSL(regC(), false));
			break;

		case 0x32:
			LOG_OPCODE("SLL D");
			setRegD(doSL(regD(), false));
			break;

		case 0x33:
			LOG_OPCODE("SLL E");
			setRegE(doSL(regE(), false));
			break;

		case 0x34:
			LOG_OPCODE("SLL H");
			setRegH(doSL(regH(), false));
			break;

		case 0x35:
			LOG_OPCODE("SLL L");
			setRegL(doSL(regL(), false));
			break;

		case 0x36:
			LOG_OPCODE("SLL (HL)");
			write8(m_HL, doSL(read8(m_HL), false));
			m_tStates += 1;
			break;

		case 0x37:
			LOG_OPCODE("SLL A");
			setRegA(doSL(regA(), false));
			break;

		case 0x38:
			LOG_OPCODE("SRL B");
			setRegB(doSR(regB(), false));
			break;

		case 0x39:
			LOG_OPCODE("SRL C");
			setRegC(doSR(regC(), false));
			break;

		case 0x3a:
			LOG_OPCODE("SRL D");
			setRegD(doSR(regD(), false));
			break;

		case 0x3b:
			LOG_OPCODE("SRL E");
			setRegE(doSR(regE(), false));
			break;

		case 0x3c:
			LOG_OPCODE("SRL H");
			setRegH(doSR(regH(), false));
			break;

		case 0x3d:
			LOG_OPCODE("SRL L");
			setRegL(doSR(regL(), false));
			break;

		case 0x3e:
			LOG_OPCODE("SRL (HL)");
			write8(m_HL, doSR(read8(m_HL), false));
			m_tStates += 1;
			break;

		case 0x3f:
			LOG_OPCODE("SRL A");
			setRegA(doSR(regA(), false));
			break;

		case 0x40:
			LOG_OPCODE("BIT 0, B");
			doBIT_r(0, regB());
			break;

		case 0x41:
			LOG_OPCODE("BIT 0, C");
			doBIT_r(0, regC());
			break;

		case 0x42:
			LOG_OPCODE("BIT 0, D");
			doBIT_r(0, regD());
			break;

		case 0x43:
			LOG_OPCODE("BIT 0, E");
			doBIT_r(0, regE());
			break;

		case 0x44:
			LOG_OPCODE("BIT 0, H");
			doBIT_r(0, regH());
			break;

		case 0x45:
			LOG_OPCODE("BIT 0, L");
			doBIT_r(0, regL());
			break;

		case 0x46:
			LOG_OPCODE("BIT 0, (HL)");
			doBIT_r(0, read8(m_HL));
			adjust53Flags(m_HL);
			m_tStates += 1;
			break;

		case 0x47:
			LOG_OPCODE("BIT 0, A");
			doBIT_r(0, regA());
			break;

		case 0x48:
			LOG_OPCODE("BIT 1, B");
			doBIT_r(1, regB());
			break;

		case 0x49:
			LOG_OPCODE("BIT 1, C");
			doBIT_r(1, regC());
			break;

		case 0x4a:
			LOG_OPCODE("BIT 1, D");
			doBIT_r(1, regD());
			break;

		case 0x4b:
			LOG_OPCODE("BIT 1, E");
			doBIT_r(1, regE());
			break;

		case 0x4c:
			LOG_OPCODE("BIT 1, H");
			doBIT_r(1, regH());
			break;

		case 0x4d:
			LOG_OPCODE("BIT 1, L");
			doBIT_r(1, regL());
			break;

		case 0x4e:
			LOG_OPCODE("BIT 1, (HL)");
			doBIT_r(1, read8(m_HL));
			adjust53Flags(m_HL);
			m_tStates += 1;
			break;

		case 0x4f:
			LOG_OPCODE("BIT 1, A");
			doBIT_r(1, regA());
			break;

		case 0x50:
			LOG_OPCODE("BIT 2, B");
			doBIT_r(2, regB());
			break;

		case 0x51:
			LOG_OPCODE("BIT 2, C");
			doBIT_r(2, regC());
			break;

		case 0x52:
			LOG_OPCODE("BIT 2, D");
			doBIT_r(2, regD());
			break;

		case 0x53:
			LOG_OPCODE("BIT 2, E");
			doBIT_r(2, regE());
			break;

		case 0x54:
			LOG_OPCODE("BIT 2, H");
			doBIT_r(2, regH());
			break;

		case 0x55:
			LOG_OPCODE("BIT 2, L");
			doBIT_r(2, regL());
			break;

		case 0x56:
			LOG_OPCODE("BIT 2, (HL)");
			doBIT_r(2, read8(m_HL));
			adjust53Flags(m_HL);
			m_tStates += 1;
			break;

		case 0x57:
			LOG_OPCODE("BIT 2, A");
			doBIT_r(2, regA());
			break;

		case 0x58:
			LOG_OPCODE("BIT 3, B");
			doBIT_r(3, regB());
			break;

		case 0x59:
			LOG_OPCODE("BIT 3, C");
			doBIT_r(3, regC());
			break;

		case 0x5a:
			LOG_OPCODE("BIT 3, D");
			doBIT_r(3, regD());
			break;

		case 0x5b:
			LOG_OPCODE("BIT 3, E");
			doBIT_r(3, regE());
			break;

		case 0x5c:
			LOG_OPCODE("BIT 3, H");
			doBIT_r(3, regH());
			break;

		case 0x5d:
			LOG_OPCODE("BIT 3, L");
			doBIT_r(3, regL());
			break;

		case 0x5e:
			LOG_OPCODE("BIT 3, (HL)");
			doBIT_r(3, read8(m_HL));
			adjust53Flags(m_HL);
			m_tStates += 1;
			break;

		case 0x5f:
			LOG_OPCODE("BIT 3, A");
			doBIT_r(3, regA());
			break;

		case 0x60:
			LOG_OPCODE("BIT 4, B");
			doBIT_r(4, regB());
			break;

		case 0x61:
			LOG_OPCODE("BIT 4, C");
			doBIT_r(4, regC());
			break;

		case 0x62:
			LOG_OPCODE("BIT 4, D");
			doBIT_r(4, regD());
			break;

		case 0x63:
			LOG_OPCODE("BIT 4, E");
			doBIT_r(4, regE());
			break;

		case 0x64:
			LOG_OPCODE("BIT 4, H");
			doBIT_r(4, regH());
			break;

		case 0x65:
			LOG_OPCODE("BIT 4, L");
			doBIT_r(4, regL());
			break;

		case 0x66:
			LOG_OPCODE("BIT 4, (HL)");
			doBIT_r(4, read8(m_HL));
			adjust53Flags(m_HL);
			m_tStates += 1;
			break;

		case 0x67:
			LOG_OPCODE("BIT 4, A");
			doBIT_r(4, regA());
			break;

		case 0x68:
			LOG_OPCODE("BIT 5, B");
			doBIT_r(5, regB());
			break;

		case 0x69:
			LOG_OPCODE("BIT 5, C");
			doBIT_r(5, regC());
			break;

		case 0x6a:
			LOG_OPCODE("BIT 5, D");
			doBIT_r(5, regD());
			break;

		case 0x6b:
			LOG_OPCODE("BIT 5, E");
			doBIT_r(5, regE());
			break;

		case 0x6c:
			LOG_OPCODE("BIT 5, H");
			doBIT_r(5, regH());
			break;

		case 0x6d:
			LOG_OPCODE("BIT 5, L");
			doBIT_r(5, regL());
			break;

		case 0x6e:
			LOG_OPCODE("BIT 5, (HL)");
			doBIT_r(5, read8(m_HL));
			adjust53Flags(m_HL);
			m_tStates += 1;
			break;

		case 0x6f:
			LOG_OPCODE("BIT 5, A");
			doBIT_r(5, regA());
			break;

		case 0x70:
			LOG_OPCODE("BIT 6, B");
			doBIT_r(6, regB());
			break;

		case 0x71:
			LOG_OPCODE("BIT 6, C");
			doBIT_r(6, regC());
			break;

		case 0x72:
			LOG_OPCODE("BIT 6, D");
			doBIT_r(6, regD());
			break;

		case 0x73:
			LOG_OPCODE("BIT 6, E");
			doBIT_r(6, regE());
			break;

		case 0x74:
			LOG_OPCODE("BIT 6, H");
			doBIT_r(6, regH());
			break;

		case 0x75:
			LOG_OPCODE("BIT 6, L");
			doBIT_r(6, regL());
			break;

		case 0x76:
			LOG_OPCODE("BIT 6, (HL)");
			doBIT_r(6, read8(m_HL));
			adjust53Flags(m_HL);
			m_tStates += 1;
			break;

		case 0x77:
			LOG_OPCODE("BIT 6, A");
			doBIT_r(6, regA());
			break;

		case 0x78:
			LOG_OPCODE("BIT 7, B");
			doBIT_r(7, regB());
			break;

		case 0x79:
			LOG_OPCODE("BIT 7, C");
			doBIT_r(7, regC());
			break;

		case 0x7a:
			LOG_OPCODE("BIT 7, D");
			doBIT_r(7, regD());
			break;

		case 0x7b:
			LOG_OPCODE("BIT 7, E");
			doBIT_r(7, regE());
			break;

		case 0x7c:
			LOG_OPCODE("BIT 7, H");
			doBIT_r(7, regH());
			break;

		case 0x7d:
			LOG_OPCODE("BIT 7, L");
			doBIT_r(7, regL());
			break;

		case 0x7e:
			LOG_OPCODE("BIT 7, (HL)");
			doBIT_r(7, read8(m_HL));
			adjust53Flags(m_HL);
			m_tStates += 1;
			break;

		case 0x7f:
			LOG_OPCODE("BIT 7, A");
			doBIT_r(7, regA());
			break;

		case 0x80:
			LOG_OPCODE("RES 0, B");
			setRegB(doSetRes(false, 0, regB()));
			break;

		case 0x81:
			LOG_OPCODE("RES 0, C");
			setRegC(doSetRes(false, 0, regC()));
			break;

		case 0x82:
			LOG_OPCODE("RES 0, D");
			setRegD(doSetRes(false, 0, regD()));
			break;

		case 0x83:
			LOG_OPCODE("RES 0, E");
			setRegE(doSetRes(false, 0, regE()));
			break;

		case 0x84:
			LOG_OPCODE("RES 0, H");
			setRegH(doSetRes(false, 0, regH()));
			break;

		case 0x85:
			LOG_OPCODE("RES 0, L");
			setRegL(doSetRes(false, 0, regL()));
			break;

		case 0x86:
			LOG_OPCODE("RES 0, (HL)");
			write8(m_HL, doSetRes(false, 0, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0x87:
			LOG_OPCODE("RES 0, A");
			setRegA(doSetRes(false, 0, regA()));
			break;

		case 0x88:
			LOG_OPCODE("RES 1, B");
			setRegB(doSetRes(false, 1, regB()));
			break;

		case 0x89:
			LOG_OPCODE("RES 1, C");
			setRegC(doSetRes(false, 1, regC()));
			break;

		case 0x8a:
			LOG_OPCODE("RES 1, D");
			setRegD(doSetRes(false, 1, regD()));
			break;

		case 0x8b:
			LOG_OPCODE("RES 1, E");
			setRegE(doSetRes(false, 1, regE()));
			break;

		case 0x8c:
			LOG_OPCODE("RES 1, H");
			setRegH(doSetRes(false, 1, regH()));
			break;

		case 0x8d:
			LOG_OPCODE("RES 1, L");
			setRegL(doSetRes(false, 1, regL()));
			break;

		case 0x8e:
			LOG_OPCODE("RES 1, (HL)");
			write8(m_HL, doSetRes(false, 1, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0x8f:
			LOG_OPCODE("RES 1, A");
			setRegA(doSetRes(false, 1, regA()));
			break;

		case 0x90:
			LOG_OPCODE("RES 2, B");
			setRegB(doSetRes(false, 2, regB()));
			break;

		case 0x91:
			LOG_OPCODE("RES 2, C");
			setRegC(doSetRes(false, 2, regC()));
			break;

		case 0x92:
			LOG_OPCODE("RES 2, D");
			setRegD(doSetRes(false, 2, regD()));
			break;

		case 0x93:
			LOG_OPCODE("RES 2, E");
			setRegE(doSetRes(false, 2, regE()));
			break;

		case 0x94:
			LOG_OPCODE("RES 2, H");
			setRegH(doSetRes(false, 2, regH()));
			break;

		case 0x95:
			LOG_OPCODE("RES 2, L");
			setRegL(doSetRes(false, 2, regL()));
			break;

		case 0x96:
			LOG_OPCODE("RES 2, (HL)");
			write8(m_HL, doSetRes(false, 2, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0x97:
			LOG_OPCODE("RES 2, A");
			setRegA(doSetRes(false, 2, regA()));
			break;

		case 0x98:
			LOG_OPCODE("RES 3, B");
			setRegB(doSetRes(false, 3, regB()));
			break;

		case 0x99:
			LOG_OPCODE("RES 3, C");
			setRegC(doSetRes(false, 3, regC()));
			break;

		case 0x9a:
			LOG_OPCODE("RES 3, D");
			setRegD(doSetRes(false, 3, regD()));
			break;

		case 0x9b:
			LOG_OPCODE("RES 3, E");
			setRegE(doSetRes(false, 3, regE()));
			break;

		case 0x9c:
			LOG_OPCODE("RES 3, H");
			setRegH(doSetRes(false, 3, regH()));
			break;

		case 0x9d:
			LOG_OPCODE("RES 3, L");
			setRegL(doSetRes(false, 3, regL()));
			break;

		case 0x9e:
			LOG_OPCODE("RES 3, (HL)");
			write8(m_HL, doSetRes(false, 3, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0x9f:
			LOG_OPCODE("RES 3, A");
			setRegA(doSetRes(false, 3, regA()));
			break;

		case 0xa0:
			LOG_OPCODE("RES 4, B");
			setRegB(doSetRes(false, 4, regB()));
			break;

		case 0xa1:
			LOG_OPCODE("RES 4, C");
			setRegC(doSetRes(false, 4, regC()));
			break;

		case 0xa2:
			LOG_OPCODE("RES 4, D");
			setRegD(doSetRes(false, 4, regD()));
			break;

		case 0xa3:
			LOG_OPCODE("RES 4, E");
			setRegE(doSetRes(false, 4, regE()));
			break;

		case 0xa4:
			LOG_OPCODE("RES 4, H");
			setRegH(doSetRes(false, 4, regH()));
			break;

		case 0xa5:
			LOG_OPCODE("RES 4, L");
			setRegL(doSetRes(false, 4, regL()));
			break;

		case 0xa6:
			LOG_OPCODE("RES 4, (HL)");
			write8(m_HL, doSetRes(false, 4, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0xa7:
			LOG_OPCODE("RES 4, A");
			setRegA(doSetRes(false, 4, regA()));
			break;

		case 0xa8:
			LOG_OPCODE("RES 5, B");
			setRegB(doSetRes(false, 5, regB()));
			break;

		case 0xa9:
			LOG_OPCODE("RES 5, C");
			setRegC(doSetRes(false, 5, regC()));
			break;

		case 0xaa:
			LOG_OPCODE("RES 5, D");
			setRegD(doSetRes(false, 5, regD()));
			break;

		case 0xab:
			LOG_OPCODE("RES 5, E");
			setRegE(doSetRes(false, 5, regE()));
			break;

		case 0xac:
			LOG_OPCODE("RES 5, H");
			setRegH(doSetRes(false, 5, regH()));
			break;

		case 0xad:
			LOG_OPCODE("RES 5, L");
			setRegL(doSetRes(false, 5, regL()));
			break;

		case 0xae:
			LOG_OPCODE("RES 5, (HL)");
			write8(m_HL, doSetRes(false, 5, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0xaf:
			LOG_OPCODE("RES 5, A");
			setRegA(doSetRes(false, 5, regA()));
			break;

		case 0xb0:
			LOG_OPCODE("RES 6, B");
			setRegB(doSetRes(false, 6, regB()));
			break;

		case 0xb1:
			LOG_OPCODE("RES 6, C");
			setRegC(doSetRes(false, 6, regC()));
			break;

		case 0xb2:
			LOG_OPCODE("RES 6, D");
			setRegD(doSetRes(false, 6, regD()));
			break;

		case 0xb3:
			LOG_OPCODE("RES 6, E");
			setRegE(doSetRes(false, 6, regE()));
			break;

		case 0xb4:
			LOG_OPCODE("RES 6, H");
			setRegH(doSetRes(false, 6, regH()));
			break;

		case 0xb5:
			LOG_OPCODE("RES 6, L");
			setRegL(doSetRes(false, 6, regL()));
			break;

		case 0xb6:
			LOG_OPCODE("RES 6, (HL)");
			write8(m_HL, doSetRes(false, 6, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0xb7:
			LOG_OPCODE("RES 6, A");
			setRegA(doSetRes(false, 6, regA()));
			break;

		case 0xb8:
			LOG_OPCODE("RES 7, B");
			setRegB(doSetRes(false, 7, regB()));
			break;

		case 0xb9:
			LOG_OPCODE("RES 7, C");
			setRegC(doSetRes(false, 7, regC()));
			break;

		case 0xba:
			LOG_OPCODE("RES 7, D");
			setRegD(doSetRes(false, 7, regD()));
			break;

		case 0xbb:
			LOG_OPCODE("RES 7, E");
			setRegE(doSetRes(false, 7, regE()));
			break;

		case 0xbc:
			LOG_OPCODE("RES 7, H");
			setRegH(doSetRes(false, 7, regH()));
			break;

		case 0xbd:
			LOG_OPCODE("RES 7, L");
			setRegL(doSetRes(false, 7, regL()));
			break;

		case 0xbe:
			LOG_OPCODE("RES 7, (HL)");
			write8(m_HL, doSetRes(false, 7, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0xbf:
			LOG_OPCODE("RES 7, A");
			setRegA(doSetRes(false, 7, regA()));
			break;

		case 0xc0:
			LOG_OPCODE("SET 0, B");
			setRegB(doSetRes(true, 0, regB()));
			break;

		case 0xc1:
			LOG_OPCODE("SET 0, C");
			setRegC(doSetRes(true, 0, regC()));
			break;

		case 0xc2:
			LOG_OPCODE("SET 0, D");
			setRegD(doSetRes(true, 0, regD()));
			break;

		case 0xc3:
			LOG_OPCODE("SET 0, E");
			setRegE(doSetRes(true, 0, regE()));
			break;

		case 0xc4:
			LOG_OPCODE("SET 0, H");
			setRegH(doSetRes(true, 0, regH()));
			break;

		case 0xc5:
			LOG_OPCODE("SET 0, L");
			setRegL(doSetRes(true, 0, regL()));
			break;

		case 0xc6:
			LOG_OPCODE("SET 0, (HL)");
			write8(m_HL, doSetRes(true, 0, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0xc7:
			LOG_OPCODE("SET 0, A");
			setRegA(doSetRes(true, 0, regA()));
			break;

		case 0xc8:
			LOG_OPCODE("SET 1, B");
			setRegB(doSetRes(true, 1, regB()));
			break;

		case 0xc9:
			LOG_OPCODE("SET 1, C");
			setRegC(doSetRes(true, 1, regC()));
			break;

		case 0xca:
			LOG_OPCODE("SET 1, D");
			setRegD(doSetRes(true, 1, regD()));
			break;

		case 0xcb:
			LOG_OPCODE("SET 1, E");
			setRegE(doSetRes(true, 1, regE()));
			break;

		case 0xcc:
			LOG_OPCODE("SET 1, H");
			setRegH(doSetRes(true, 1, regH()));
			break;

		case 0xcd:
			LOG_OPCODE("SET 1, L");
			setRegL(doSetRes(true, 1, regL()));
			break;

		case 0xce:
			LOG_OPCODE("SET 1, (HL)");
			write8(m_HL, doSetRes(true, 1, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0xcf:
			LOG_OPCODE("SET 1, A");
			setRegA(doSetRes(true, 1, regA()));
			break;

		case 0xd0:
			LOG_OPCODE("SET 2, B");
			setRegB(doSetRes(true, 2, regB()));
			break;

		case 0xd1:
			LOG_OPCODE("SET 2, C");
			setRegC(doSetRes(true, 2, regC()));
			break;

		case 0xd2:
			LOG_OPCODE("SET 2, D");
			setRegD(doSetRes(true, 2, regD()));
			break;

		case 0xd3:
			LOG_OPCODE("SET 2, E");
			setRegE(doSetRes(true, 2, regE()));
			break;

		case 0xd4:
			LOG_OPCODE("SET 2, H");
			setRegH(doSetRes(true, 2, regH()));
			break;

		case 0xd5:
			LOG_OPCODE("SET 2, L");
			setRegL(doSetRes(true, 2, regL()));
			break;

		case 0xd6:
			LOG_OPCODE("SET 2, (HL)");
			write8(m_HL, doSetRes(true, 2, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0xd7:
			LOG_OPCODE("SET 2, A");
			setRegA(doSetRes(true, 2, regA()));
			break;

		case 0xd8:
			LOG_OPCODE("SET 3, B");
			setRegB(doSetRes(true, 3, regB()));
			break;

		case 0xd9:
			LOG_OPCODE("SET 3, C");
			setRegC(doSetRes(true, 3, regC()));
			break;

		case 0xda:
			LOG_OPCODE("SET 3, D");
			setRegD(doSetRes(true, 3, regD()));
			break;

		case 0xdb:
			LOG_OPCODE("SET 3, E");
			setRegE(doSetRes(true, 3, regE()));
			break;

		case 0xdc:
			LOG_OPCODE("SET 3, H");
			setRegH(doSetRes(true, 3, regH()));
			break;

		case 0xdd:
			LOG_OPCODE("SET 3, L");
			setRegL(doSetRes(true, 3, regL()));
			break;

		case 0xde:
			LOG_OPCODE("SET 3, (HL)");
			write8(m_HL, doSetRes(true, 3, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0xdf:
			LOG_OPCODE("SET 3, A");
			setRegA(doSetRes(true, 3, regA()));
			break;

		case 0xe0:
			LOG_OPCODE("SET 4, B");
			setRegB(doSetRes(true, 4, regB()));
			break;

		case 0xe1:
			LOG_OPCODE("SET 4, C");
			setRegC(doSetRes(true, 4, regC()));
			break;

		case 0xe2:
			LOG_OPCODE("SET 4, D");
			setRegD(doSetRes(true, 4, regD()));
			break;

		case 0xe3:
			LOG_OPCODE("SET 4, E");
			setRegE(doSetRes(true, 4, regE()));
			break;

		case 0xe4:
			LOG_OPCODE("SET 4, H");
			setRegH(doSetRes(true, 4, regH()));
			break;

		case 0xe5:
			LOG_OPCODE("SET 4, L");
			setRegL(doSetRes(true, 4, regL()));
			break;

		case 0xe6:
			LOG_OPCODE("SET 4, (HL)");
			write8(m_HL, doSetRes(true, 4, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0xe7:
			LOG_OPCODE("SET 4, A");
			setRegA(doSetRes(true, 4, regA()));
			break;

		case 0xe8:
			LOG_OPCODE("SET 5, B");
			setRegB(doSetRes(true, 5, regB()));
			break;

		case 0xe9:
			LOG_OPCODE("SET 5, C");
			setRegC(doSetRes(true, 5, regC()));
			break;

		case 0xea:
			LOG_OPCODE("SET 5, D");
			setRegD(doSetRes(true, 5, regD()));
			break;

		case 0xeb:
			LOG_OPCODE("SET 5, E");
			setRegE(doSetRes(true, 5, regE()));
			break;

		case 0xec:
			LOG_OPCODE("SET 5, H");
			setRegH(doSetRes(true, 5, regH()));
			break;

		case 0xed:
			LOG_OPCODE("SET 5, L");
			setRegL(doSetRes(true, 5, regL()));
			break;

		case 0xee:
			LOG_OPCODE("SET 5, (HL)");
			write8(m_HL, doSetRes(true, 5, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0xef:
			LOG_OPCODE("SET 5, A");
			setRegA(doSetRes(true, 5, regA()));
			break;

		case 0xf0:
			LOG_OPCODE("SET 6, B");
			setRegB(doSetRes(true, 6, regB()));
			break;

		case 0xf1:
			LOG_OPCODE("SET 6, C");
			setRegC(doSetRes(true, 6, regC()));
			break;

		case 0xf2:
			LOG_OPCODE("SET 6, D");
			setRegD(doSetRes(true, 6, regD()));
			break;

		case 0xf3:
			LOG_OPCODE("SET 6, E");
			setRegE(doSetRes(true, 6, regE()));
			break;

		case 0xf4:
			LOG_OPCODE("SET 6, H");
			setRegH(doSetRes(true, 6, regH()));
			break;

		case 0xf5:
			LOG_OPCODE("SET 6, L");
			setRegL(doSetRes(true, 6, regL()));
			break;

		case 0xf6:
			LOG_OPCODE("SET 6, (HL)");
			write8(m_HL, doSetRes(true, 6, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0xf7:
			LOG_OPCODE("SET 6, A");
			setRegA(doSetRes(true, 6, regA()));
			break;

		case 0xf8:
			LOG_OPCODE("SET 7, B");
			setRegB(doSetRes(true, 7, regB()));
			break;

		case 0xf9:
			LOG_OPCODE("SET 7, C");
			setRegC(doSetRes(true, 7, regC()));
			break;

		case 0xfa:
			LOG_OPCODE("SET 7, D");
			setRegD(doSetRes(true, 7, regD()));
			break;

		case 0xfb:
			LOG_OPCODE("SET 7, E");
			setRegE(doSetRes(true, 7, regE()));
			break;

		case 0xfc:
			LOG_OPCODE("SET 7, H");
			setRegH(doSetRes(true, 7, regH()));
			break;

		case 0xfd:
			LOG_OPCODE("SET 7, L");
			setRegL(doSetRes(true, 7, regL()));
			break;

		case 0xfe:
			LOG_OPCODE("SET 7, (HL)");
			write8(m_HL, doSetRes(true, 7, read8(m_HL)));
			m_tStates += 1;
			break;

		case 0xff:
			LOG_OPCODE("SET 7, A");
			setRegA(doSetRes(true, 7, regA()));
			break;

		default: {
			m_isUndefinedState = true;
			std::ostringstream ss;
			ss << "Invalid opcode: CB " << std::hex << uint16_t(opcode);
			throw std::runtime_error(ss.str());
		}
	}
}

void Z80::doExecuteED() {
	uint8_t opcode = read8(m_PC);
	m_PC++;
	switch (opcode) {
		case 0x4a:
			LOG_OPCODE("ADC HL, BC");
			m_HL = doAddWord(m_HL, m_BC, true, false);
			m_tStates += 7;
			break;

		case 0x47:
			LOG_OPCODE("LD I, A");
			m_I = regA();
			m_tStates += 1;
			break;

		case 0x42:
			LOG_OPCODE("SBC HL, BC");
			m_HL = doAddWord(m_HL, m_BC, true, true);
			m_tStates += 7;
			break;

		case 0x43:
			LOG_OPCODE("LD (nn), BC");
			write16(read16(m_PC), m_BC);
			m_PC += 2;
			break;

		case 0x44: {
			LOG_OPCODE("NEG");
			uint8_t tmp = regA();
			setRegA(0);
			setRegA(doArithmetic(tmp, false, true));
			setFlag(FLAG_N_MASK, true);
			break;
		}

		case 0x4b:
			LOG_OPCODE("LD BC, (nn)");
			m_BC = read16(read16(m_PC));
			m_PC += 2;
			break;

		case 0x52:
			LOG_OPCODE("SBC HL, DE");
			m_HL = doAddWord(m_HL, m_DE, true, true);
			m_tStates += 7;
			break;

		case 0x53:
			LOG_OPCODE("LD (nn), DE");
			write16(read16(m_PC), m_DE);
			m_PC += 2;
			break;

		case 0x56:
			LOG_OPCODE("IM 1");
			m_IM = 1;
			break;

		case 0x5a:
			LOG_OPCODE("ADC HL, DE");
			m_HL = doAddWord(m_HL, m_DE, true, false);
			m_tStates += 7;
			break;

		case 0x5b:
			LOG_OPCODE("LD DE, (nn)");
			m_DE = read16(read16(m_PC));
			m_PC += 2;
			break;

		case 0x62:
			LOG_OPCODE("SBC HL, HL");
			m_HL = doAddWord(m_HL, m_HL, true, true);
			m_tStates += 7;
			break;

		case 0x63:
			LOG_OPCODE("LD (nn), HL");
			write16(read16(m_PC), m_HL);
			m_PC += 2;
			break;

		case 0x67: {
			LOG_OPCODE("RRD");
			uint8_t Ah = regA() & 0x0f;
			uint8_t hl = read8(m_HL);
			setRegA((regA() & 0xf0) | (hl & 0x0f));
			hl = (hl >> 4) | (Ah << 4);
			write8(m_HL, hl);
			setFlag(FLAG_H_MASK | FLAG_N_MASK, false);
			adjustFlagSZP(regA());
			adjustFlags(regA());
			m_tStates += 4;
			break;
		}

		case 0x6a:
			LOG_OPCODE("ADC HL, HL");
			m_HL = doAddWord(m_HL, m_HL, true, false);
			m_tStates += 7;
			break;

		case 0x6b:
			LOG_OPCODE("LD HL, (nn)");
			m_HL = read16(read16(m_PC));
			m_PC += 2;
			break;

		case 0x6f: {
			LOG_OPCODE("RLD");
			uint8_t Ah = regA() & 0x0f;
			uint8_t hl = read8(m_HL);
			setRegA((regA() & 0xf0) | ((hl & 0xf0) >> 4));
			hl = (hl << 4) | Ah;
			write8(m_HL, hl);
			setFlag(FLAG_H_MASK | FLAG_N_MASK, false);
			adjustFlagSZP(regA());
			adjustFlags(regA());
			m_tStates += 4;
			break;
		}

		case 0x72:
			LOG_OPCODE("SBC HL, SP");
			m_HL = doAddWord(m_HL, m_SP, true, true);
			m_tStates += 7;
			break;
			
		case 0x73:
			LOG_OPCODE("LD (nn), SP");
			write16(read16(m_PC), m_SP);
			m_PC += 2;
			break;

		case 0x78:
			LOG_OPCODE("IN A, (C)");
			setRegA(ioRead(m_BC));
			setFlag(FLAG_H_MASK | FLAG_N_MASK, false);
			adjustFlagSZP(regA());
			adjustFlags(regA());
			break;
			
		case 0x7a:
			LOG_OPCODE("ADC HL, SP");
			m_HL = doAddWord(m_HL, m_SP, true, false);
			m_tStates += 7;
			break;

		case 0x7b:
			LOG_OPCODE("LD SP, (nn)");
			m_SP = read16(read16(m_PC));
			m_PC += 2;
			break;

		case 0xa0:
			LOG_OPCODE("LDI");
			doLDI();
			break;

		case 0xa1:
			LOG_OPCODE("CPI");
			doCPI();
			break;

		case 0xa8:
			LOG_OPCODE("LDD");
			doLDD();
			break;

		case 0xa9:
			LOG_OPCODE("CPD");
			doCPD();
			break;

		case 0xb0: {
			LOG_OPCODE("LDIR");
			uint8_t value = read8(m_HL);
			write8(m_DE, value);
			m_DE++;
			m_HL++;
			m_BC--;

			uint8_t result = regA() + value;
			setFlag(FLAG_5_MASK, (result & 0x02) != 0);
			setFlag(FLAG_3_MASK, (result & FLAG_3_MASK) != 0);
			setFlag(FLAG_H_MASK | FLAG_N_MASK, false);
			setFlag(FLAG_PV_MASK, m_BC != 0);

			m_tStates += 4;
			if (m_BC != 0) {
				m_tStates += 5;
				m_PC -= 2;
			}
			break;
		}

		/*case 0xb0:
			LOG_OPCODE("LDIR");
			doLDI();
			if (m_BC != 0) {
				m_tStates += 5;
				m_PC -= 2;
			}
			break;*/

		case 0xb1:
			LOG_OPCODE("CPIR");
			doCPI();
			if (m_BC != 0 && !isFlagSet(FLAG_Z_MASK)) {
				m_tStates += 5;
				m_PC -= 2;
			}
			break;

		case 0xb8: {
			LOG_OPCODE("LDDR");
			uint8_t value = read8(m_HL);
			write8(m_DE, value);
			m_DE--;
			m_HL--;
			m_BC--;

			uint8_t result = regA() + value;
			setFlag(FLAG_5_MASK, (result & 0x02) != 0);
			setFlag(FLAG_3_MASK, (result & FLAG_3_MASK) != 0);
			setFlag(FLAG_H_MASK | FLAG_N_MASK, false);
			setFlag(FLAG_PV_MASK, m_BC != 0);

			m_tStates += 4;
			if (m_BC != 0) {
				m_tStates += 5;
				m_PC -= 2;
			}
			break;
		}

		case 0xb9: {
			LOG_OPCODE("CPDR");
			doCPD();
			if (m_BC != 0 && !isFlagSet(FLAG_Z_MASK)) {
				m_tStates += 5;
				m_PC -= 2;
			}
			break;
		}

		default: {
			m_isUndefinedState = true;
			std::ostringstream ss;
			ss << "Invalid opcode: ED " << std::hex << uint16_t(opcode);
			throw std::runtime_error(ss.str());
		}
	}
}

static std::set<uint8_t> g_debugOpcodes;

uint16_t Z80::doExecutionDDFD(uint16_t regValue, bool isIX) {
	uint8_t opcode = read8(m_PC);
	m_PC++;
	switch (opcode) {
		case 0x09:
			LOG_OPCODE(isIX ? "ADD IX, BC" : "ADD IY, BC");
			regValue = doAddWord(regValue, m_BC, false, false);
			m_tStates += 7;
			break;

		case 0x19:
			LOG_OPCODE(isIX ? "ADD IX, DE" : "ADD IY, DE");
			regValue = doAddWord(regValue, m_DE, false, false);
			m_tStates += 7;
			break;

		case 0x21:
			LOG_OPCODE(isIX ? "LD IX, nn" : "LD IY, nn")
			regValue = read16(m_PC);
			m_PC += 2;
			break;

		case 0x22:
			LOG_OPCODE(isIX ? "LD (nn), IX" : "LD (nn), IY");
			write16(read16(m_PC), regValue);
			m_PC += 2;
			break;

		case 0x23:
			LOG_OPCODE(isIX ? "INC IX" : "INC IY");
			regValue++;
			m_tStates += 2;
			break;

		case 0x24: {
			LOG_OPCODE(isIX ? "INC IXh" : "INC IYh");
			uint8_t value = regValue >> 8;
			value = doIncDec(value, false);
			regValue &= 0xff;
			regValue |= value << 8;
			break;
		}

		case 0x25: {
			LOG_OPCODE(isIX ? "DEC IXh" : "DEC IYh");
			uint8_t value = regValue >> 8;
			value = doIncDec(value, true);
			regValue &= 0xff;
			regValue |= value << 8;
			break;
		}

		case 0x26:
			LOG_OPCODE(isIX ? "LD IXh, n" : "LD IYh, n");
			regValue &= 0xff;
			regValue |= read8(m_PC) << 8;
			m_PC += 1;
			break;

		case 0x29:
			LOG_OPCODE(isIX ? "ADD IX, IX" : "ADD IY, IY");
			regValue = doAddWord(regValue, regValue, false, false);
			m_tStates += 7;
			break;

		case 0x2a:
			LOG_OPCODE(isIX ? "LD IX, (nn)" : "LD IY, (nn)");
			regValue = read16(read16(m_PC));
			m_PC += 2;
			break;

		case 0x2b:
			LOG_OPCODE(isIX ? "DEC IX" : "DEC IY");
			regValue--;
			m_tStates += 2;
			break;

		case 0x2c: {
			LOG_OPCODE(isIX ? "INC IXl" : "INC IYl");
			uint8_t value = regValue;
			value = doIncDec(value, false);
			regValue &= 0xff00;
			regValue |= value;
			break;
		}

		case 0x2d: {
			LOG_OPCODE(isIX ? "DEC IXl" : "DEC IYl");
			uint8_t value = regValue;
			value = doIncDec(value, true);
			regValue &= 0xff00;
			regValue |= value;
			break;
		}

		case 0x2e:
			LOG_OPCODE(isIX ? "LD IXl, n" : "LD IYl, n");
			regValue &= 0xff00;
			regValue |= read8(m_PC);
			m_PC += 1;
			break;

		case 0x34: {
			LOG_OPCODE(isIX ? "INC (IX + d)" : "INC (IY + d)");
			int base = regValue;
			int8_t offset = read8(m_PC);
			uint8_t value = read8(base + offset);
			write8(base + offset, doIncDec(value, false));
			m_PC += 1;
			m_tStates += 6;
			break;
		}

		case 0x35: {
			LOG_OPCODE(isIX ? "DEC (IX + d)" : "DEC (IY + d)");
			int base = regValue;
			int8_t offset = read8(m_PC);
			uint8_t value = read8(base + offset);
			write8(base + offset, doIncDec(value, true));
			m_PC += 1;
			m_tStates += 6;
			break;
		}

		case 0x36: {
			LOG_OPCODE(isIX ? "LD (IX + d), n" : "LD (IY + d), n");
			int base = regValue;
			int8_t offset = read8(m_PC);
			m_PC += 1;
			write8(base + offset, read8(m_PC));
			m_PC += 1;
			m_tStates += 2;
			break;
		}

		case 0x39:
			LOG_OPCODE(isIX ? "ADD IX, SP" : "ADD IY, SP");
			regValue = doAddWord(regValue, m_SP, false, false);
			m_tStates += 7;
			break;

		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
			m_PC -= 1;
			doExecute();
			break;

		case 0x44:
			LOG_OPCODE(isIX ? "LD B, IXh" : "LD B, IYh");
			setRegB(regValue >> 8);
			break;

		case 0x45:
			LOG_OPCODE(isIX ? "LD B, IXl" : "LD B, IYl");
			setRegB(regValue);
			break;

		case 0x46: {
			LOG_OPCODE(isIX ? "LD B, (IX + d)" : "LD B, (IY + d)");
			int base = regValue;
			setRegB(read8(base + int8_t(read8(m_PC))));
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x47:
		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
			m_PC -= 1;
			doExecute();
			break;

		case 0x4c:
			LOG_OPCODE(isIX ? "LD C, IXh" : "LD B, IYh");
			setRegC(regValue >> 8);
			break;

		case 0x4d:
			LOG_OPCODE(isIX ? "LD C, IXl" : "LD B, IYl");
			setRegC(regValue);
			break;

		case 0x4e: {
			LOG_OPCODE(isIX ? "LD C, (IX + d)" : "LD C, (IY + d)");
			int base = regValue;
			setRegC(read8(base + int8_t(read8(m_PC))));
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x4f:
			m_PC -= 1;
			doExecute();
			break;

		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
			m_PC -= 1;
			doExecute();
			break;

		case 0x54:
			LOG_OPCODE(isIX ? "LD D, IXh" : "LD D, IYh");
			setRegD(regValue >> 8);
			break;

		case 0x55:
			LOG_OPCODE(isIX ? "LD D, IXl" : "LD D, IYl");
			setRegD(regValue);
			break;

		case 0x56: {
			LOG_OPCODE(isIX ? "LD D, (IX + d)" : "LD D, (IY + d)");
			int base = regValue;
			setRegD(read8(base + int8_t(read8(m_PC))));
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x57:
		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
			m_PC -= 1;
			doExecute();
			break;

		case 0x5c:
			LOG_OPCODE(isIX ? "LD E, IXh" : "LD E, IYh");
			setRegE(regValue >> 8);
			break;

		case 0x5d:
			LOG_OPCODE(isIX ? "LD E, IXl" : "LD E, IYl");
			setRegE(regValue);
			break;

		case 0x5e: {
			LOG_OPCODE(isIX ? "LD E, (IX + d)" : "LD E, (IY + d)");
			int base = regValue;
			setRegE(read8(base + int8_t(read8(m_PC))));
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x5f:
			m_PC -= 1;
			doExecute();
			break;

		case 0x60:
			LOG_OPCODE(isIX ? "LD IXh, B" : "LD IYh, B");
			regValue &= 0xff;
			regValue |= regB() << 8;
			break;

		case 0x61:
			LOG_OPCODE(isIX ? "LD IXh, C" : "LD IYh, C");
			regValue &= 0xff;
			regValue |= regC() << 8;
			break;

		case 0x62:
			LOG_OPCODE(isIX ? "LD IXh, D" : "LD IYh, D");
			regValue &= 0xff;
			regValue |= regD() << 8;
			break;

		case 0x63:
			LOG_OPCODE(isIX ? "LD IXh, E" : "LD IYh, E");
			regValue &= 0xff;
			regValue |= regE() << 8;
			break;

		case 0x64:
			LOG_OPCODE(isIX ? "LD IXh, IXh" : "LD IYh, IYh");
			// do nothing
			break;

		case 0x65: {
			LOG_OPCODE(isIX ? "LD IXh, IXl" : "LD IYh, IYl");
			uint8_t value = regValue;
			regValue &= 0xff;
			regValue |= value << 8;
			break;
		}

		case 0x66: {
			LOG_OPCODE(isIX ? "LD H, (IX + d)" : "LD H, (IY + d)");
			int base = regValue;
			setRegH(read8(base + int8_t(read8(m_PC))));
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x67:
			LOG_OPCODE(isIX ? "LD IXh, A" : "LD IYh, A");
			regValue &= 0xff;
			regValue |= regA() << 8;
			break;

		case 0x68:
			LOG_OPCODE(isIX ? "LD IXl, B" : "LD IYl, B");
			regValue &= 0xff00;
			regValue |= regB();
			break;

		case 0x69:
			LOG_OPCODE(isIX ? "LD IXl, C" : "LD IYl, C");
			regValue &= 0xff00;
			regValue |= regC();
			break;

		case 0x6a:
			LOG_OPCODE(isIX ? "LD IXl, D" : "LD IYl, D");
			regValue &= 0xff00;
			regValue |= regD();
			break;

		case 0x6b:
			LOG_OPCODE(isIX ? "LD IXl, E" : "LD IYl, E");
			regValue &= 0xff00;
			regValue |= regE();
			break;

		case 0x6c: {
			LOG_OPCODE(isIX ? "LD IXl, IXh" : "LD IYl, IYh");
			uint8_t value = regValue >> 8;
			regValue &= 0xff00;
			regValue |= value;
			break;
		}

		case 0x6d: {
			LOG_OPCODE(isIX ? "LD IXl, IXl" : "LD IYl, IYl");
			// do nothing
			break;
		}

		case 0x6e: {
			LOG_OPCODE(isIX ? "LD L, (IX + d)" : "LD L, (IY + d)");
			int base = regValue;
			setRegL(read8(base + int8_t(read8(m_PC))));
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x6f:
			LOG_OPCODE(isIX ? "LD IXl, A" : "LD IYl, A");
			regValue &= 0xff00;
			regValue |= regA();
			break;

		case 0x70: {
			LOG_OPCODE(isIX ? "LD (IX + d), B" : "LD (IY + d), B");
			int base = regValue;
			write8(base + int8_t(read8(m_PC)), regB());
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x71: {
			LOG_OPCODE(isIX ? "LD (IX + d), C" : "LD (IY + d), C");
			int base = regValue;
			write8(base + int8_t(read8(m_PC)), regC());
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x72: {
			LOG_OPCODE(isIX ? "LD (IX + d), D" : "LD (IY + d), D");
			int base = regValue;
			write8(base + int8_t(read8(m_PC)), regD());
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x73: {
			LOG_OPCODE(isIX ? "LD (IX + d), E" : "LD (IY + d), E");
			int base = regValue;
			write8(base + int8_t(read8(m_PC)), regE());
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x74: {
			LOG_OPCODE(isIX ? "LD (IX + d), H" : "LD (IY + d), H");
			int base = regValue;
			write8(base + int8_t(read8(m_PC)), regH());
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x75: {
			LOG_OPCODE(isIX ? "LD (IX + d), L" : "LD (IY + d), L");
			int base = regValue;
			write8(base + int8_t(read8(m_PC)), regL());
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x77: {
			LOG_OPCODE(isIX ? "LD (IX + d), A" : "LD (IY + d), A");
			int base = regValue;
			write8(base + int8_t(read8(m_PC)), regA());
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
			m_PC -= 1;
			doExecute();
			break;

		case 0x7c:
			LOG_OPCODE(isIX ? "LD A, IXh" : "LD A, IYh");
			setRegA(regValue >> 8);
			break;

		case 0x7d:
			LOG_OPCODE(isIX ? "LD A, IXl" : "LD A, IYl");
			setRegA(regValue);
			break;

		case 0x7e: {
			LOG_OPCODE(isIX ? "LD A, (IX + d)" : "LD A, (IY + d)");
			int base = regValue;
			setRegA(read8(base + int8_t(read8(m_PC))));
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x7f:
			m_PC -= 1;
			doExecute();
			break;

		case 0x84:
			LOG_OPCODE(isIX ? "ADD A, IXh" : "ADD A, IYh");
			setRegA(doArithmetic(regValue >> 8, false, false));
			break;

		case 0x85:
			LOG_OPCODE(isIX ? "ADD A, IXl" : "ADD A, IYl");
			setRegA(doArithmetic(regValue, false, false));
			break;

		case 0x86: {
			LOG_OPCODE(isIX ? "ADD A, (IX + d)" : "ADD A, (IY + d)");
			int32_t base = regValue;
			setRegA(doArithmetic(read8(base + int8_t(read8(m_PC))), false, false));
			m_tStates += 5;
			m_PC += 1;
			break;
		}

		case 0x89: {
			LOG_OPCODE(isIX ? "SUB A, (IX + d)" : "SUB A, (IY + d)");
			int32_t base = regValue;
			setRegA(doArithmetic(read8(base + int8_t(read8(m_PC))), false, true));
			m_tStates += 5;
			m_PC += 1;
			break;
		}

		case 0x8c:
			LOG_OPCODE(isIX ? "ADC A, IXh" : "ADC A, IYh");
			setRegA(doArithmetic(regValue >> 8, true, false));
			break;

		case 0x8d:
			LOG_OPCODE(isIX ? "ADC A, IXl" : "ADC A, IYl");
			setRegA(doArithmetic(regValue, true, false));
			break;

		case 0x8e: {
			LOG_OPCODE(isIX ? "ADC A, (IX + d)" : "ADC A, (IY + d)");
			int32_t base = regValue;
			setRegA(doArithmetic(read8(base + int8_t(read8(m_PC))), true, false));
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0x94:
			LOG_OPCODE(isIX ? "SUB A, IXh" : "SUB A, IYh");
			setRegA(doArithmetic(regValue >> 8, false, true));
			break;

		case 0x95:
			LOG_OPCODE(isIX ? "SUB A, IXl" : "SUB A, IYl");
			setRegA(doArithmetic(regValue, false, true));
			break;

		case 0x96: {
			LOG_OPCODE(isIX ? "SUB A, (IX + d)" : "SUB A, (IY + d)");
			int32_t base = regValue;
			setRegA(doArithmetic(read8(base + int8_t(read8(m_PC))), false, true));
			m_tStates += 5;
			m_PC += 1;
			break;
		}

		case 0x9c:
			LOG_OPCODE(isIX ? "SBC A, IXh" : "SBC A, IYh");
			setRegA(doArithmetic(regValue >> 8, true, true));
			break;

		case 0x9d:
			LOG_OPCODE(isIX ? "SBC A, IXl" : "SBC A, IYl");
			setRegA(doArithmetic(regValue, true, true));
			break;

		case 0x9e: {
			LOG_OPCODE(isIX ? "SBC A, (IX + d)" : "SBC A, (IY + d)");
			int32_t base = regValue;
			setRegA(doArithmetic(read8(base + int8_t(read8(m_PC))), true, true));
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0xa4:
			LOG_OPCODE(isIX ? "AND IXh" : "AND IYh");
			doAND(regValue >> 8);
			break;

		case 0xa5:
			LOG_OPCODE(isIX ? "AND IXl" : "AND IYl");
			doAND(regValue);
			break;

		case 0xa6: {
			LOG_OPCODE(isIX ? "AND (IX + d)" : "AND (IY + d)");
			int32_t base = regValue;
			doAND(read8(base + int8_t(read8(m_PC))));
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0xac:
			LOG_OPCODE(isIX ? "XOR IXh" : "XOR IYh");
			doXOR(regValue >> 8);
			break;

		case 0xad:
			LOG_OPCODE(isIX ? "XOR IXl" : "XOR IYl");
			doXOR(regValue);
			break;

		case 0xae: {
			LOG_OPCODE(isIX ? "XOR (IX + d)" : "XOR (IY + d)");
			int32_t base = regValue;
			doXOR(read8(base + int8_t(read8(m_PC))));
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0xb4:
			LOG_OPCODE(isIX ? "OR IXh" : "OR IYh");
			doOR(regValue >> 8);
			break;

		case 0xb5:
			LOG_OPCODE(isIX ? "OR IXl" : "OR IYl");
			doOR(regValue);
			break;

		case 0xb6: {
			LOG_OPCODE(isIX ? "OR (IX + d)" : "OR (IY + d)");
			int32_t base = regValue;
			doOR(read8(base + int8_t(read8(m_PC))));
			m_PC += 1;
			m_tStates += 5;
			break;
		}

		case 0xbc: {
			LOG_OPCODE(isIX ? "CP IXh" : "CP IYh");
			uint8_t value = regValue >> 8;
			doArithmetic(value, false, true);
			adjustFlags(value);
			break;
		}

		case 0xbd: {
			LOG_OPCODE(isIX ? "CP IXl" : "CP IYl");
			uint8_t value = regValue;
			doArithmetic(value, false, true);
			adjustFlags(value);
			break;
		}

		case 0xbe: {
			LOG_OPCODE(isIX ? "CP A, (IX + d)" : "CP A, (IY + d)");
			int32_t base = regValue;
			uint8_t value = read8(base + int8_t(read8(m_PC)));
			doArithmetic(value, false, true);
			adjustFlags(value);
			m_tStates += 5;
			m_PC += 1;
			break;
		}

		case 0xcb: {
			int base = regValue;
			uint16_t address = uint16_t(base + int8_t(read8(m_PC)));
			m_PC++;
			doExecutionDDFDCB(address, isIX);
			break;
		}

		case 0xdd:
			LOG_OPCODE("DD NOP");
			break;

		case 0xe1:
			LOG_OPCODE(isIX ? "POP IX" : "POP IY");
			regValue = doPop();
			break;

		case 0xe5:
			LOG_OPCODE(isIX ? "PUSH IX" : "PUSH IY");
			doPush(regValue);
			m_tStates += 1;
			break;

		case 0xe9:
			LOG_OPCODE(isIX ? "JP (IX)" : "JP (IY)");
			m_PC = regValue;
			break;
			
		case 0xed:
			LOG_OPCODE("ED NOP");
			break;

		case 0xf9:
			LOG_OPCODE(isIX ? "LD SP, IX" : "LD SP, IY");
			m_SP = regValue;
			m_tStates += 2;
			break;

		case 0xfd:
			LOG_OPCODE("FD NOP");
			break;

		default: {
			/*auto search = g_debugOpcodes.find(opcode);
			if (search == g_debugOpcodes.end()) {
				printf("%X\n", uint16_t(opcode));
				g_debugOpcodes.insert(opcode);
			}*/
			m_isUndefinedState = true;
			std::ostringstream ss;
			ss << "Invalid opcode: " << (isIX ? "DD" : "FD") << " " << std::hex << uint16_t(opcode);
			throw std::runtime_error(ss.str());
		}
	}

	return regValue;
}

void Z80::doExecutionDDFDCB(uint16_t address, bool isIX) {
	uint8_t opcode = read8(m_PC);
	m_PC++;
	switch (opcode) {
		case 0x00:
			LOG_OPCODE(isIX ? "RLC B, (IX, d)" : "RLC B, (IY + d)");
			setRegB(doRLC(true, read8(address)));
			write8(address, regB());
			m_tStates += 2;
			break;

		case 0x01:
			LOG_OPCODE(isIX ? "RLC C, (IX, d)" : "RLC C, (IY + d)");
			setRegC(doRLC(true, read8(address)));
			write8(address, regC());
			m_tStates += 2;
			break;

		case 0x02:
			LOG_OPCODE(isIX ? "RLC D, (IX, d)" : "RLC D, (IY + d)");
			setRegD(doRLC(true, read8(address)));
			write8(address, regD());
			m_tStates += 2;
			break;

		case 0x03:
			LOG_OPCODE(isIX ? "RLC E, (IX, d)" : "RLC E, (IY + d)");
			setRegE(doRLC(true, read8(address)));
			write8(address, regE());
			m_tStates += 2;
			break;

		case 0x04:
			LOG_OPCODE(isIX ? "RLC H, (IX, d)" : "RLC H, (IY + d)");
			setRegH(doRLC(true, read8(address)));
			write8(address, regH());
			m_tStates += 2;
			break;

		case 0x05:
			LOG_OPCODE(isIX ? "RLC L, (IX, d)" : "RLC L, (IY + d)");
			setRegL(doRLC(true, read8(address)));
			write8(address, regL());
			m_tStates += 2;
			break;

		case 0x06:
			LOG_OPCODE(isIX ? "RLC (IX + d)" : "RLC (IY + d)");
			write8(address, doRLC(true, read8(address)));
			m_tStates += 2;
			break;

		case 0x07:
			LOG_OPCODE(isIX ? "RLC A, (IX, d)" : "RLC A, (IY + d)");
			setRegA(doRLC(true, read8(address)));
			write8(address, regA());
			break;

		case 0x08:
			LOG_OPCODE(isIX ? "RRC B, (IX, d)" : "RRC B, (IY + d)");
			setRegB(doRRC(true, read8(address)));
			write8(address, regB());
			m_tStates += 2;
			break;

		case 0x09:
			LOG_OPCODE(isIX ? "RRC C, (IX, d)" : "RRC C, (IY + d)");
			setRegC(doRRC(true, read8(address)));
			write8(address, regC());
			m_tStates += 2;
			break;

		case 0x0a:
			LOG_OPCODE(isIX ? "RRC D, (IX, d)" : "RRC D, (IY + d)");
			setRegD(doRRC(true, read8(address)));
			write8(address, regD());
			m_tStates += 2;
			break;

		case 0x0b:
			LOG_OPCODE(isIX ? "RRC E, (IX, d)" : "RRC E, (IY + d)");
			setRegE(doRRC(true, read8(address)));
			write8(address, regE());
			m_tStates += 2;
			break;

		case 0x0c:
			LOG_OPCODE(isIX ? "RRC H, (IX, d)" : "RRC H, (IY + d)");
			setRegH(doRRC(true, read8(address)));
			write8(address, regH());
			m_tStates += 2;
			break;

		case 0x0d:
			LOG_OPCODE(isIX ? "RRC L, (IX, d)" : "RRC L, (IY + d)");
			setRegL(doRRC(true, read8(address)));
			write8(address, regL());
			m_tStates += 2;
			break;

		case 0x0e:
			LOG_OPCODE(isIX ? "RRC (IX + d)" : "RRC (IY + d)");
			write8(address, doRRC(true, read8(address)));
			m_tStates += 2;
			break;

		case 0x0f:
			LOG_OPCODE(isIX ? "RRC A, (IX, d)" : "RRC A, (IY + d)");
			setRegA(doRRC(true, read8(address)));
			write8(address, regA());
			break;

		case 0x10:
			LOG_OPCODE(isIX ? "RL B, (IX, d)" : "RL B, (IY + d)");
			setRegB(doRL(true, read8(address)));
			write8(address, regB());
			m_tStates += 2;
			break;

		case 0x11:
			LOG_OPCODE(isIX ? "RL C, (IX, d)" : "RL C, (IY + d)");
			setRegC(doRL(true, read8(address)));
			write8(address, regC());
			m_tStates += 2;
			break;

		case 0x12:
			LOG_OPCODE(isIX ? "RL D, (IX, d)" : "RL D, (IY + d)");
			setRegD(doRL(true, read8(address)));
			write8(address, regD());
			m_tStates += 2;
			break;

		case 0x13:
			LOG_OPCODE(isIX ? "RL E, (IX, d)" : "RL E, (IY + d)");
			setRegE(doRL(true, read8(address)));
			write8(address, regE());
			m_tStates += 2;
			break;

		case 0x14:
			LOG_OPCODE(isIX ? "RL H, (IX, d)" : "RL H, (IY + d)");
			setRegH(doRL(true, read8(address)));
			write8(address, regH());
			m_tStates += 2;
			break;

		case 0x15:
			LOG_OPCODE(isIX ? "RL L, (IX, d)" : "RL L, (IY + d)");
			setRegL(doRL(true, read8(address)));
			write8(address, regL());
			m_tStates += 2;
			break;

		case 0x16:
			LOG_OPCODE(isIX ? "RL (IX + d)" : "RL (IY + d)");
			write8(address, doRL(true, read8(address)));
			m_tStates += 2;
			break;

		case 0x17:
			LOG_OPCODE(isIX ? "RL A, (IX, d)" : "RL A, (IY + d)");
			setRegA(doRL(true, read8(address)));
			write8(address, regA());
			break;

		case 0x18:
			LOG_OPCODE(isIX ? "RR B, (IX, d)" : "RR B, (IY + d)");
			setRegB(doRR(true, read8(address)));
			write8(address, regB());
			m_tStates += 2;
			break;

		case 0x19:
			LOG_OPCODE(isIX ? "RR C, (IX, d)" : "RR C, (IY + d)");
			setRegC(doRR(true, read8(address)));
			write8(address, regC());
			m_tStates += 2;
			break;

		case 0x1a:
			LOG_OPCODE(isIX ? "RR D, (IX, d)" : "RR D, (IY + d)");
			setRegD(doRR(true, read8(address)));
			write8(address, regD());
			m_tStates += 2;
			break;

		case 0x1b:
			LOG_OPCODE(isIX ? "RR E, (IX, d)" : "RR E, (IY + d)");
			setRegE(doRR(true, read8(address)));
			write8(address, regE());
			m_tStates += 2;
			break;

		case 0x1c:
			LOG_OPCODE(isIX ? "RR H, (IX, d)" : "RR H, (IY + d)");
			setRegH(doRR(true, read8(address)));
			write8(address, regH());
			m_tStates += 2;
			break;

		case 0x1d:
			LOG_OPCODE(isIX ? "RR L, (IX, d)" : "RR L, (IY + d)");
			setRegL(doRR(true, read8(address)));
			write8(address, regL());
			m_tStates += 2;
			break;

		case 0x1e:
			LOG_OPCODE(isIX ? "RR (IX + d)" : "RR (IY + d)");
			write8(address, doRR(true, read8(address)));
			m_tStates += 2;
			break;

		case 0x1f:
			LOG_OPCODE(isIX ? "RR A, (IX, d)" : "RR A, (IY + d)");
			setRegA(doRR(true, read8(address)));
			write8(address, regA());
			break;
			
		case 0x20:
			LOG_OPCODE(isIX ? "SLA B, (IX, d)" : "SLA B, (IY + d)");
			setRegB(doSL(read8(address), true));
			write8(address, regB());
			m_tStates += 2;
			break;

		case 0x21:
			LOG_OPCODE(isIX ? "SLA C, (IX, d)" : "SLA C, (IY + d)");
			setRegC(doSL(read8(address), true));
			write8(address, regC());
			m_tStates += 2;
			break;

		case 0x22:
			LOG_OPCODE(isIX ? "SLA D, (IX, d)" : "SLA D, (IY + d)");
			setRegD(doSL(read8(address), true));
			write8(address, regD());
			m_tStates += 2;
			break;

		case 0x23:
			LOG_OPCODE(isIX ? "SLA E, (IX, d)" : "SLA E, (IY + d)");
			setRegE(doSL(read8(address), true));
			write8(address, regE());
			m_tStates += 2;
			break;

		case 0x24:
			LOG_OPCODE(isIX ? "SLA H, (IX, d)" : "SLA H, (IY + d)");
			setRegH(doSL(read8(address), true));
			write8(address, regH());
			m_tStates += 2;
			break;

		case 0x25:
			LOG_OPCODE(isIX ? "SLA L, (IX, d)" : "SLA L, (IY + d)");
			setRegL(doSL(read8(address), true));
			write8(address, regL());
			m_tStates += 2;
			break;

		case 0x26:
			LOG_OPCODE(isIX ? "SLA (IX + d)" : "SLA (IY + d)");
			write8(address, doSL(read8(address), true));
			m_tStates += 2;
			break;

		case 0x27:
			LOG_OPCODE(isIX ? "SLA A, (IX, d)" : "SLA A, (IY + d)");
			setRegA(doSL(read8(address), true));
			write8(address, regA());
			break;

		case 0x28:
			LOG_OPCODE(isIX ? "SRA B, (IX, d)" : "SRA B, (IY + d)");
			setRegB(doSR(read8(address), true));
			write8(address, regB());
			m_tStates += 2;
			break;

		case 0x29:
			LOG_OPCODE(isIX ? "SRA C, (IX, d)" : "SRA C, (IY + d)");
			setRegC(doSR(read8(address), true));
			write8(address, regC());
			m_tStates += 2;
			break;

		case 0x2a:
			LOG_OPCODE(isIX ? "SRA D, (IX, d)" : "SRA D, (IY + d)");
			setRegD(doSR(read8(address), true));
			write8(address, regD());
			m_tStates += 2;
			break;

		case 0x2b:
			LOG_OPCODE(isIX ? "SRA E, (IX, d)" : "SRA E, (IY + d)");
			setRegE(doSR(read8(address), true));
			write8(address, regE());
			m_tStates += 2;
			break;

		case 0x2c:
			LOG_OPCODE(isIX ? "SRA H, (IX, d)" : "SRA H, (IY + d)");
			setRegH(doSR(read8(address), true));
			write8(address, regH());
			m_tStates += 2;
			break;

		case 0x2d:
			LOG_OPCODE(isIX ? "SRA L, (IX, d)" : "SRA L, (IY + d)");
			setRegL(doSR(read8(address), true));
			write8(address, regL());
			m_tStates += 2;
			break;

		case 0x2e:
			LOG_OPCODE(isIX ? "SRA (IX + d)" : "SRA (IY + d)");
			write8(address, doSR(read8(address), true));
			m_tStates += 2;
			break;

		case 0x2f:
			LOG_OPCODE(isIX ? "SRA A, (IX, d)" : "SRA A, (IY + d)");
			setRegA(doSR(read8(address), true));
			write8(address, regA());
			break;

		case 0x30:
			LOG_OPCODE(isIX ? "SLL B, (IX, d)" : "SLL B, (IY + d)");
			setRegB(doSL(read8(address), false));
			write8(address, regB());
			m_tStates += 2;
			break;

		case 0x31:
			LOG_OPCODE(isIX ? "SLL C, (IX, d)" : "SLL C, (IY + d)");
			setRegC(doSL(read8(address), false));
			write8(address, regC());
			m_tStates += 2;
			break;

		case 0x32:
			LOG_OPCODE(isIX ? "SLL D, (IX, d)" : "SLL D, (IY + d)");
			setRegD(doSL(read8(address), false));
			write8(address, regD());
			m_tStates += 2;
			break;

		case 0x33:
			LOG_OPCODE(isIX ? "SLL E, (IX, d)" : "SLL E, (IY + d)");
			setRegE(doSL(read8(address), false));
			write8(address, regE());
			m_tStates += 2;
			break;

		case 0x34:
			LOG_OPCODE(isIX ? "SLL H, (IX, d)" : "SLL H, (IY + d)");
			setRegH(doSL(read8(address), false));
			write8(address, regH());
			m_tStates += 2;
			break;

		case 0x35:
			LOG_OPCODE(isIX ? "SLL L, (IX, d)" : "SLL L, (IY + d)");
			setRegL(doSL(read8(address), false));
			write8(address, regL());
			m_tStates += 2;
			break;

		case 0x36:
			LOG_OPCODE(isIX ? "SLL (IX + d)" : "SLL (IY + d)");
			write8(address, doSL(read8(address), false));
			m_tStates += 2;
			break;

		case 0x37:
			LOG_OPCODE(isIX ? "SLL A, (IX, d)" : "SLL A, (IY + d)");
			setRegA(doSL(read8(address), false));
			write8(address, regA());
			break;

		case 0x38:
			LOG_OPCODE(isIX ? "SRL B, (IX, d)" : "SRL B, (IY + d)");
			setRegB(doSR(read8(address), false));
			write8(address, regB());
			m_tStates += 2;
			break;

		case 0x39:
			LOG_OPCODE(isIX ? "SRL C, (IX, d)" : "SRL C, (IY + d)");
			setRegC(doSR(read8(address), false));
			write8(address, regC());
			m_tStates += 2;
			break;

		case 0x3a:
			LOG_OPCODE(isIX ? "SRL D, (IX, d)" : "SRL D, (IY + d)");
			setRegD(doSR(read8(address), false));
			write8(address, regD());
			m_tStates += 2;
			break;

		case 0x3b:
			LOG_OPCODE(isIX ? "SRL E, (IX, d)" : "SRL E, (IY + d)");
			setRegE(doSR(read8(address), false));
			write8(address, regE());
			m_tStates += 2;
			break;

		case 0x3c:
			LOG_OPCODE(isIX ? "SRL H, (IX, d)" : "SRL H, (IY + d)");
			setRegH(doSR(read8(address), false));
			write8(address, regH());
			m_tStates += 2;
			break;

		case 0x3d:
			LOG_OPCODE(isIX ? "SRL L, (IX, d)" : "SRL L, (IY + d)");
			setRegL(doSR(read8(address), false));
			write8(address, regL());
			m_tStates += 2;
			break;

		case 0x3e:
			LOG_OPCODE(isIX ? "SRL (IX + d)" : "SRL (IY + d)");
			write8(address, doSR(read8(address), false));
			m_tStates += 2;
			break;

		case 0x3f:
			LOG_OPCODE(isIX ? "SRL A, (IX, d)" : "SRL A, (IY + d)");
			setRegA(doSR(read8(address), false));
			write8(address, regA());
			break;

		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
			LOG_OPCODE(isIX ? "BIT 0, (IX + d)" : "BIT 0, (IY + d)");
			doBIT_indexed(0, address);
			m_tStates += 2;
			break;

		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f:
			LOG_OPCODE(isIX ? "BIT 1, (IX + d)" : "BIT 1, (IY + d)");
			doBIT_indexed(1, address);
			m_tStates += 2;
			break;

		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
			LOG_OPCODE(isIX ? "BIT 2, (IX + d)" : "BIT 2, (IY + d)");
			doBIT_indexed(2, address);
			m_tStates += 2;
			break;

		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f:
			LOG_OPCODE(isIX ? "BIT 3, (IX + d)" : "BIT 3, (IY + d)");
			doBIT_indexed(3, address);
			m_tStates += 2;
			break;

		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
			LOG_OPCODE(isIX ? "BIT 4, (IX + d)" : "BIT 4, (IY + d)");
			doBIT_indexed(4, address);
			m_tStates += 2;
			break;

		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:
			LOG_OPCODE(isIX ? "BIT 5, (IX + d)" : "BIT 5, (IY + d)");
			doBIT_indexed(5, address);
			m_tStates += 2;
			break;

		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
			LOG_OPCODE(isIX ? "BIT 6, (IX + d)" : "BIT 6, (IY + d)");
			doBIT_indexed(6, address);
			m_tStates += 2;
			break;

		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:
			LOG_OPCODE(isIX ? "BIT 7, (IX + d)" : "BIT 7, (IY + d)");
			doBIT_indexed(7, address);
			m_tStates += 2;
			break;

		case 0x86:
			LOG_OPCODE(isIX ? "RES 0, (IX + d)" : "RES 0, (IY + d)");
			write8(address, doSetRes(false, 0, read8(address)));
			m_tStates += 2;
			break;

		case 0x8e:
			LOG_OPCODE(isIX ? "RES 1, (IX + d)" : "RES 1, (IY + d)");
			write8(address, doSetRes(false, 1, read8(address)));
			m_tStates += 2;
			break;

		case 0x96:
			LOG_OPCODE(isIX ? "RES 2, (IX + d)" : "RES 2, (IY + d)");
			write8(address, doSetRes(false, 2, read8(address)));
			m_tStates += 2;
			break;

		case 0x9e:
			LOG_OPCODE(isIX ? "RES 3, (IX + d)" : "RES 3, (IY + d)");
			write8(address, doSetRes(false, 3, read8(address)));
			m_tStates += 2;
			break;

		case 0xa6:
			LOG_OPCODE(isIX ? "RES 4, (IX + d)" : "RES 4, (IY + d)");
			write8(address, doSetRes(false, 4, read8(address)));
			m_tStates += 2;
			break;

		case 0xae:
			LOG_OPCODE(isIX ? "RES 5, (IX + d)" : "RES 5, (IY + d)");
			write8(address, doSetRes(false, 5, read8(address)));
			m_tStates += 2;
			break;

		case 0xb6:
			LOG_OPCODE(isIX ? "RES 6, (IX + d)" : "RES 6, (IY + d)");
			write8(address, doSetRes(false, 6, read8(address)));
			m_tStates += 2;
			break;

		case 0xbe:
			LOG_OPCODE(isIX ? "RES 7, (IX + d)" : "RES 7, (IY + d)");
			write8(address, doSetRes(false, 7, read8(address)));
			m_tStates += 2;
			break;

		case 0xc6:
			LOG_OPCODE(isIX ? "SET 0, (IX + d)" : "SET 0, (IY + d)");
			write8(address, doSetRes(true, 0, read8(address)));
			m_tStates += 2;
			break;

		case 0xce:
			LOG_OPCODE(isIX ? "SET 1, (IX + d)" : "SET 1, (IY + d)");
			write8(address, doSetRes(true, 1, read8(address)));
			m_tStates += 2;
			break;

		case 0xd6:
			LOG_OPCODE(isIX ? "SET 2, (IX + d)" : "SET 2, (IY + d)");
			write8(address, doSetRes(true, 2, read8(address)));
			m_tStates += 2;
			break;

		case 0xde:
			LOG_OPCODE(isIX ? "SET 3, (IX + d)" : "SET 3, (IY + d)");
			write8(address, doSetRes(true, 3, read8(address)));
			m_tStates += 2;
			break;

		case 0xe6:
			LOG_OPCODE(isIX ? "SET 4, (IX + d)" : "SET 4, (IY + d)");
			write8(address, doSetRes(true, 4, read8(address)));
			m_tStates += 2;
			break;

		case 0xee:
			LOG_OPCODE(isIX ? "SET 5, (IX + d)" : "SET 5, (IY + d)");
			write8(address, doSetRes(true, 5, read8(address)));
			m_tStates += 2;
			break;

		case 0xf6:
			LOG_OPCODE(isIX ? "SET 6, (IX + d)" : "SET 6, (IY + d)");
			write8(address, doSetRes(true, 6, read8(address)));
			m_tStates += 2;
			break;

		case 0xfe:
			LOG_OPCODE(isIX ? "SET 7, (IX + d)" : "SET 7, (IY + d)");
			write8(address, doSetRes(true, 7, read8(address)));
			m_tStates += 2;
			break;

		default: {
			m_isUndefinedState = true;
			std::ostringstream ss;
			ss << "Invalid opcode: " << (isIX ? "DD" : "FD") << " CB " << std::hex << uint16_t(opcode);
			throw std::runtime_error(ss.str());
		}
	}
}

void Z80::unhalt() {
	m_isHalted = false;
}

void Z80::doPush(uint16_t value) {
	m_SP -= 2;
	write16(m_SP, value);
}

uint16_t Z80::doPop() {
	uint16_t value = read16(m_SP);
	m_SP += 2;
	return value;
}

void Z80::doOR(uint8_t value) {
	setRegA(regA() | value);
	adjustLogicFlag(false);
}

void Z80::doXOR(uint8_t value) {
	setRegA(regA() ^ value);
	adjustLogicFlag(false);
}

void Z80::doAND(uint8_t value) {
	setRegA(regA() & value);
	adjustLogicFlag(true);
}

void Z80::setFlag(int mask, bool flag) {
	uint8_t flags = regF();

	if (flag) {
		flags |= mask;
	} else {
		flags &= ~mask;
	}

	setRegF(flags);
}

bool Z80::isFlagSet(int mask) {
	return (regF() & mask) != 0;
}

void Z80::adjustLogicFlag(bool flagH) {
	uint8_t a = regA();

	setFlag(FLAG_S_MASK, (a & 0x80) != 0);
	setFlag(FLAG_Z_MASK, a == 0);
	setFlag(FLAG_H_MASK, flagH);
	setFlag(FLAG_N_MASK, false);
	setFlag(FLAG_C_MASK, false);
	setFlag(FLAG_PV_MASK, parityBit[a] == 1);

	adjustFlags(a);
}

void Z80::adjustFlags(uint8_t value) {
	setFlag(FLAG_5_MASK, (value & FLAG_5_MASK) != 0);
	setFlag(FLAG_3_MASK, (value & FLAG_3_MASK) != 0);
}

void Z80::adjustFlagSZP(uint8_t value) {
	setFlag(FLAG_S_MASK, (value & 0x80) != 0);
	setFlag(FLAG_Z_MASK, value == 0);
	setFlag(FLAG_PV_MASK, parityBit[value] == 1);
}

void Z80::adjust53Flags(uint16_t address) {
	uint8_t sz5h3pnFlags = regF();
	sz5h3pnFlags = uint8_t((sz5h3pnFlags & (FLAG_S_MASK | FLAG_Z_MASK | FLAG_H_MASK | FLAG_PV_MASK)) | ((address >> 8) & (FLAG_5_MASK | FLAG_3_MASK)));
	setFlag(FLAG_S_MASK, (sz5h3pnFlags & FLAG_S_MASK) != 0);
	setFlag(FLAG_Z_MASK, (sz5h3pnFlags & FLAG_Z_MASK) != 0);
	setFlag(FLAG_5_MASK, (sz5h3pnFlags & FLAG_5_MASK) != 0);
	setFlag(FLAG_H_MASK, (sz5h3pnFlags & FLAG_H_MASK) != 0);
	setFlag(FLAG_3_MASK, (sz5h3pnFlags & FLAG_3_MASK) != 0);
	setFlag(FLAG_PV_MASK, (sz5h3pnFlags & FLAG_PV_MASK) != 0);
	setFlag(FLAG_N_MASK, (sz5h3pnFlags & FLAG_N_MASK) != 0);
}

uint8_t Z80::doArithmetic(uint8_t value, bool withCarry, bool isSub) {
	int16_t res; /* To detect carry */

	uint8_t a = regA();
	if (isSub) {
		setFlag(FLAG_N_MASK, true);
		res = a - value;
		if (withCarry) {
			if (isFlagSet(FLAG_C_MASK)) {
				res--;
			}
			setFlag(FLAG_H_MASK, ((a ^ value ^ (res & 0xff)) & 0x10) != 0);
		} else {
			setFlag(FLAG_H_MASK, (res & 0x0f) > (a & 0x0f));
		}
		setFlag(FLAG_PV_MASK, ((a ^ value) & (a ^ (res & 0xff))) > 0x7f);
	} else {
		setFlag(FLAG_N_MASK, false);
		res = a + value;
		if (withCarry) {
			if (isFlagSet(FLAG_C_MASK)) {
				res++;
			}
			setFlag(FLAG_H_MASK, ((a ^ value ^ (res & 0xff)) & 0x10) != 0);
		} else {
			setFlag(FLAG_H_MASK, (res & 0x0f) < (a & 0x0f));
		}
		setFlag(FLAG_PV_MASK, ((a ^ ~value) & (a ^ (res & 0xff))) > 0x7f);
	}
	setFlag(FLAG_S_MASK, (res & 0x80) != 0);
	setFlag(FLAG_C_MASK, (res & 0x100) != 0);
	setFlag(FLAG_Z_MASK, (res & 0xff) == 0);

	adjustFlags(res);

	return res;
}

uint16_t Z80::doAddWord(uint16_t a1, uint16_t a2, bool withCarry, bool isSub) {
	if(withCarry && isFlagSet(FLAG_C_MASK)) {
		a2++;
	}
	int sum = a1;
	if (isSub) {
		sum -= a2;
		setFlag(FLAG_H_MASK, (((a1 & 0x0fff) - (a2 & 0x0fff)) & 0x1000) != 0);
	} else {
		sum += a2;
		setFlag(FLAG_H_MASK, (((a1 & 0x0fff) + (a2 & 0x0fff)) & 0x1000) != 0);
	}
	setFlag(FLAG_C_MASK, (sum & 0x10000) != 0);
	if (withCarry || isSub) {
		int minuend_sign = a1 & 0x8000;
		int subtrahend_sign = a2 & 0x8000;
		int result_sign = sum & 0x8000;
		bool overflow;
		if (isSub) {
			overflow = minuend_sign != subtrahend_sign && result_sign != minuend_sign;
		} else {
			overflow = minuend_sign == subtrahend_sign && result_sign != minuend_sign;
		}
		setFlag(FLAG_PV_MASK, overflow);
		setFlag(FLAG_S_MASK, (sum & 0x8000) != 0);
		setFlag(FLAG_Z_MASK, (sum & 0xffff) == 0);
	}
	setFlag(FLAG_N_MASK, isSub);
	adjustFlags(sum >> 8);
	return sum;
}

uint8_t Z80::doIncDec(uint8_t value, bool isDec) {
	if (isDec) {
		setFlag(FLAG_PV_MASK, (value & 0x80) && !((value - 1) & 0x80));
		value--;
		setFlag(FLAG_H_MASK, (value & 0x0F) == 0x0F);
	} else {
		setFlag(FLAG_PV_MASK, !(value & 0x80) && ((value + 1) & 0x80));
		value++;
		setFlag(FLAG_H_MASK, !(value & 0x0F));
	}

	setFlag(FLAG_S_MASK, (value & 0x80) != 0);
	setFlag(FLAG_Z_MASK, value == 0);
	setFlag(FLAG_N_MASK, isDec);

	adjustFlags(value);

	return value;
}

uint8_t Z80::doRLC(bool adjFlags, uint8_t value) {
	setFlag(FLAG_C_MASK, (value & 0x80) != 0);
	value <<= 1;
	value |= isFlagSet(FLAG_C_MASK) ? FLAG_C_MASK : 0;

	adjustFlags(value);
	setFlag(FLAG_H_MASK | FLAG_N_MASK, false);

	if (adjFlags) {
		adjustFlagSZP(value);
	}

	return value;
}

uint8_t Z80::doRRC(bool adjFlags, uint8_t value) {
	setFlag(FLAG_C_MASK, (value & 0x01) != 0);
	value >>= 1;
	if (isFlagSet(FLAG_C_MASK)) {
		value |= 0x80;
	} else {
		value &= 0x7f;
	}

	adjustFlags(value);
	setFlag(FLAG_H_MASK | FLAG_N_MASK, false);

	if (adjFlags) {
		adjustFlagSZP(value);
	}

	return value;
}

uint8_t Z80::doRL(bool adjFlags, uint8_t value) {
	bool carryFlagWasSet = isFlagSet(FLAG_C_MASK);
	setFlag(FLAG_C_MASK, (value & 0x80) != 0);
	value <<= 1;
	if (carryFlagWasSet) {
		value |= 0x01;
	}

	adjustFlags(value);
	setFlag(FLAG_H_MASK | FLAG_N_MASK, false);

	if (adjFlags) {
		adjustFlagSZP(value);
	}

	return value;
}

uint8_t Z80::doRR(bool adjFlags, uint8_t value) {
	bool carryFlagWasSet = isFlagSet(FLAG_C_MASK);
	setFlag(FLAG_C_MASK, (value & 0x01) != 0);
	value >>= 1;
	if (carryFlagWasSet) {
		value |= 0x80;
	}

	adjustFlags(value);
	setFlag(FLAG_H_MASK | FLAG_N_MASK, false);

	if (adjFlags) {
		adjustFlagSZP(value);
	}

	return value;
}

uint8_t Z80::doSL(uint8_t value, int isArith) {
	setFlag(FLAG_C_MASK, (value & 0x80) != 0);
	value <<= 1;

	if (!isArith) {
		value |= 0x01;
	}

	adjustFlags(value);
	setFlag(FLAG_H_MASK | FLAG_N_MASK, false);
	adjustFlagSZP(value);

	return value;
}

uint8_t Z80::doSR(uint8_t value, int isArith) {
	uint8_t b = value & 0x80;

	setFlag(FLAG_C_MASK, (value & 0x01) != 0);
	value >>= 1;

	if (isArith) {
		value |= b;
	}

	adjustFlags(value);
	setFlag(FLAG_H_MASK | FLAG_N_MASK, false);
	adjustFlagSZP(value);

	return value;
}

void Z80::doBIT(int b, uint8_t value) {
	setFlag(FLAG_Z_MASK | FLAG_PV_MASK, (value & (1 << b)) == 0);

	setFlag(FLAG_H_MASK, true);
	setFlag(FLAG_N_MASK, false);

	setFlag(FLAG_S_MASK, false);
	if ((b == 7) && !isFlagSet(FLAG_Z_MASK)) {
		setFlag(FLAG_S_MASK, true);
	}
}

void Z80::doBIT_r(int b2, uint8_t value) {
	uint8_t mask = 1 << b2;
	bool zeroFlag = (mask & value) == 0;

	uint8_t sz5h3pnFlags = uint8_t((m_sz53n_addTable[value] & ~(FLAG_S_MASK | FLAG_Z_MASK | FLAG_PV_MASK)) | FLAG_H_MASK);

	if (zeroFlag) {
		sz5h3pnFlags |= (FLAG_PV_MASK | FLAG_Z_MASK);
	}

	if (mask == FLAG_S_MASK && !zeroFlag) {
		sz5h3pnFlags |= FLAG_S_MASK;
	}

	setFlag(FLAG_S_MASK, (sz5h3pnFlags & FLAG_S_MASK) != 0);
	setFlag(FLAG_Z_MASK, (sz5h3pnFlags & FLAG_Z_MASK) != 0);
	setFlag(FLAG_5_MASK, (sz5h3pnFlags & FLAG_5_MASK) != 0);
	setFlag(FLAG_H_MASK, (sz5h3pnFlags & FLAG_H_MASK) != 0);
	setFlag(FLAG_3_MASK, (sz5h3pnFlags & FLAG_3_MASK) != 0);
	setFlag(FLAG_PV_MASK, (sz5h3pnFlags & FLAG_PV_MASK) != 0);
	setFlag(FLAG_N_MASK, (sz5h3pnFlags & FLAG_N_MASK) != 0);
}

void Z80::doBIT_indexed(int b, uint16_t address) {
	uint8_t value = read8(address);
	uint8_t mask = 1 << b;
	bool zeroFlag = (mask & value) == 0;

	uint8_t sz5h3pnFlags = uint8_t((m_sz53n_addTable[value] & ~(FLAG_S_MASK | FLAG_Z_MASK | FLAG_PV_MASK)) | FLAG_H_MASK);

	if (zeroFlag) {
		sz5h3pnFlags |= (FLAG_PV_MASK | FLAG_Z_MASK);
	}

	if (mask == FLAG_S_MASK && !zeroFlag) {
		sz5h3pnFlags |= FLAG_S_MASK;
	}

	sz5h3pnFlags = uint8_t((sz5h3pnFlags & (FLAG_S_MASK | FLAG_Z_MASK | FLAG_H_MASK | FLAG_PV_MASK)) | ((address >> 8) & (FLAG_5_MASK | FLAG_3_MASK)));

	setFlag(FLAG_S_MASK, (sz5h3pnFlags & FLAG_S_MASK) != 0);
	setFlag(FLAG_Z_MASK, (sz5h3pnFlags & FLAG_Z_MASK) != 0);
	setFlag(FLAG_5_MASK, (sz5h3pnFlags & FLAG_5_MASK) != 0);
	setFlag(FLAG_H_MASK, (sz5h3pnFlags & FLAG_H_MASK) != 0);
	setFlag(FLAG_3_MASK, (sz5h3pnFlags & FLAG_3_MASK) != 0);
	setFlag(FLAG_PV_MASK, (sz5h3pnFlags & FLAG_PV_MASK) != 0);
	setFlag(FLAG_N_MASK, (sz5h3pnFlags & FLAG_N_MASK) != 0);
}

uint8_t Z80::doCP_HL() {
	uint8_t value = read8(m_HL);
	uint8_t result = doArithmetic(value, false, true);
	adjustFlags(value);
	return result;
}

void Z80::doCPD() {
	bool carry = isFlagSet(FLAG_C_MASK);
	uint8_t value = doCP_HL();
	if (isFlagSet(FLAG_H_MASK)) {
		value--;
	}
	m_HL--;
	m_BC--;
	setFlag(FLAG_PV_MASK, m_BC != 0);
	setFlag(FLAG_C_MASK, carry);
	setFlag(FLAG_5_MASK, (value & (1 << 1)) != 0);
	setFlag(FLAG_3_MASK, (value & (1 << 3)) != 0);
	m_tStates += 5;
}

void Z80::doCPI() {
	bool carry = isFlagSet(FLAG_C_MASK);
	uint8_t value = doCP_HL();
	if (isFlagSet(FLAG_H_MASK)) {
		value--;
	}
	m_HL++;
	m_BC--;
	setFlag(FLAG_PV_MASK, m_BC != 0);
	setFlag(FLAG_C_MASK, carry);
	setFlag(FLAG_5_MASK, (value & (1 << 1)) != 0);
	setFlag(FLAG_3_MASK, (value & (1 << 3)) != 0);
	m_tStates += 5;
}

void Z80::doDAA() {
	int correction_factor = 0x00;
	bool carry = false;
	if (regA() > 0x99 || isFlagSet(FLAG_C_MASK)) {
		correction_factor |= 0x60;
		carry = true;
	}
	if ((regA() & 0x0f) > 9 || isFlagSet(FLAG_H_MASK)) {
		correction_factor |= 0x06;
	}

	int a_before = regA();
	if (isFlagSet(FLAG_N_MASK)) {
		setRegA(regA() - correction_factor);
	} else {
		setRegA(regA() + correction_factor);
	}

	setFlag(FLAG_H_MASK, ((a_before ^ regA()) & 0x10) != 0);
	setFlag(FLAG_C_MASK, carry);
	setFlag(FLAG_S_MASK, (regA() & 0x80) != 0);
	setFlag(FLAG_Z_MASK, regA() == 0);
	setFlag(FLAG_PV_MASK, parityBit[regA()]);
	adjustFlags(regA());
}

void Z80::doLDD() {
	uint8_t value = read8(m_HL);
	write8(m_DE, value);
	m_DE--;
	m_HL--;
	m_BC--;
	setFlag(FLAG_5_MASK, ((regA() + value) & 0x02) != 0);
	setFlag(FLAG_3_MASK, ((regA() + value) & FLAG_3_MASK) != 0);
	setFlag(FLAG_H_MASK | FLAG_N_MASK, false);
	setFlag(FLAG_PV_MASK, m_BC != 0);
	m_tStates += 2;
}

void Z80::doLDI() {
	uint8_t value = read8(m_HL);
	write8(m_DE, value);
	m_DE++;
	m_HL++;
	m_BC--;
	setFlag(FLAG_5_MASK, ((regA() + value) & 0x02) != 0);
	setFlag(FLAG_3_MASK, ((regA() + value) & FLAG_3_MASK) != 0);
	setFlag(FLAG_H_MASK | FLAG_N_MASK, false);
	setFlag(FLAG_PV_MASK, m_BC != 0);
	m_tStates += 2;
}

uint8_t Z80::doSetRes(bool isSet, int pos, uint8_t value) {
	if (isSet) {
		value |= (1 << pos);
	} else {
		value &= ~(1 << pos);
	}

	return value;
}
