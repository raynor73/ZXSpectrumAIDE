//
// Created by Игорь on 20.06.2018.
//

#ifndef Z80STATE_H
#define Z80STATE_H


#include <cstdint>

class Z80State {
public:
	uint16_t regPC() const { return m_PC; }

	uint16_t regAF() const { return m_AF; }
	uint16_t regBC() const { return m_BC; }
	uint16_t regDE() const { return m_DE; }
	uint16_t regHL() const { return m_HL; }
	uint16_t regIX() const { return m_IX; }
	uint16_t regIY() const { return m_IY; }
	uint16_t regSP() const { return m_SP; }

	uint16_t regAFalt() const { return m_AFalt; }
	uint16_t regBCalt() const { return m_BCalt; }
	uint16_t regDEalt() const { return m_DEalt; }
	uint16_t regHLalt() const { return m_HLalt; }
	uint16_t regIXalt() const { return m_IXalt; }
	uint16_t regIYalt() const { return m_IYalt; }
	uint16_t regSPalt() const { return m_SPalt; }
	
	uint8_t regI() const { return m_I; }
	uint8_t regR() const { return m_R; }
	
	uint64_t tStates() const { return m_tStates; }
	
	bool isHalted() const { return m_isHalted; }
	bool isUndefinedState() const { return m_isUndefinedState; }
	bool IFF1() const { return m_IFF1; }
	bool IFF2() const { return m_IFF2; }
	uint8_t IM() const { return m_IM; }
	bool isNmiRequested() const { return m_isNmiRequested; }
	bool isIntRequested() const { return m_isIntRequested; }
	bool shouldDeferInt() const { return m_shouldDeferInt; }
	uint8_t intVector() const { return m_intVector; }
	bool shouldExecuteIntVector() const { return m_shouldExecuteIntVector; }
	
	void setRegPC(uint16_t value) { m_PC = value; }
	
	void setRegAF(uint16_t value) { m_AF = value; }
	void setRegBC(uint16_t value) { m_BC = value; }
	void setRegDE(uint16_t value) { m_DE = value; }
	void setRegHL(uint16_t value) { m_HL = value; }
	void setRegIX(uint16_t value) { m_IX = value; }
	void setRegIY(uint16_t value) { m_IY = value; }
	void setRegSP(uint16_t value) { m_SP = value; }
	
	void setRegAFalt(uint16_t value) { m_AFalt = value; }
	void setRegBCalt(uint16_t value) { m_BCalt = value; }
	void setRegDEalt(uint16_t value) { m_DEalt = value; }
	void setRegHLalt(uint16_t value) { m_HLalt = value; }
	void setRegIXalt(uint16_t value) { m_IXalt = value; }
	void setRegIYalt(uint16_t value) { m_IYalt = value; }
	void setRegSPalt(uint16_t value) { m_SPalt = value; }
	
	void setI(uint8_t value) { m_I = value; }
	void setR(uint8_t value) { m_R = value; }
	
	void setTstates(uint64_t value) { m_tStates = value; }

	void setIsHalted(bool value) { m_isHalted = value; }
	void setIsUndefinedState(bool value) { m_isUndefinedState = value; }
	
	void setIFF1(bool value) { m_IFF1 = value; }
	void setIFF2(bool value) { m_IFF2 = value; }
	void setIM(uint8_t value) { m_IM = value; }
	void setIsNmiRequested(bool value) { m_isNmiRequested = value; }
	void setIsIntRequested(bool value) { m_isIntRequested = value; }
	void setShouldDeferInt(bool value) { m_shouldDeferInt = value; }
	void setIntVector(uint8_t value) { m_intVector = value; }
	void setShouldExecuteIntVector(bool value) { m_shouldExecuteIntVector = value; }
	
private:
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
};


#endif //Z80STATE_H

