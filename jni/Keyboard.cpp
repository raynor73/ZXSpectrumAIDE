//
// Created by Igor Lapin on 18.06.18.
//

#include "Keyboard.h"

const uint16_t Keyboard::KEYBOARD_PORT_ADDRESSES[] = {
        0xfefe,
        0xfdfe,
        0xfbfe,
        0xf7fe,
        0xeffe,
        0xdffe,
        0xbffe,
        0x7ffe
};

const int Keyboard::KEY_CODE_0 = 0;
const int Keyboard::KEY_CODE_1 = 1;
const int Keyboard::KEY_CODE_2 = 2;
const int Keyboard::KEY_CODE_3 = 3;
const int Keyboard::KEY_CODE_4 = 4;
const int Keyboard::KEY_CODE_5 = 5;
const int Keyboard::KEY_CODE_6 = 6;
const int Keyboard::KEY_CODE_7 = 7;
const int Keyboard::KEY_CODE_8 = 8;
const int Keyboard::KEY_CODE_9 = 38;

const int Keyboard::KEY_CODE_A = 9;
const int Keyboard::KEY_CODE_B = 10;
const int Keyboard::KEY_CODE_C = 11;
const int Keyboard::KEY_CODE_D = 12;
const int Keyboard::KEY_CODE_E = 13;
const int Keyboard::KEY_CODE_F = 14;
const int Keyboard::KEY_CODE_G = 15;
const int Keyboard::KEY_CODE_H = 16;
const int Keyboard::KEY_CODE_I = 17;
const int Keyboard::KEY_CODE_J = 18;
const int Keyboard::KEY_CODE_K = 19;
const int Keyboard::KEY_CODE_L = 20;
const int Keyboard::KEY_CODE_M = 21;
const int Keyboard::KEY_CODE_N = 22;
const int Keyboard::KEY_CODE_O = 23;
const int Keyboard::KEY_CODE_P = 24;
const int Keyboard::KEY_CODE_Q = 25;
const int Keyboard::KEY_CODE_R = 26;
const int Keyboard::KEY_CODE_S = 27;
const int Keyboard::KEY_CODE_T = 28;
const int Keyboard::KEY_CODE_U = 29;
const int Keyboard::KEY_CODE_V = 30;
const int Keyboard::KEY_CODE_W = 31;
const int Keyboard::KEY_CODE_X = 32;
const int Keyboard::KEY_CODE_Y = 33;
const int Keyboard::KEY_CODE_Z = 39;

const int Keyboard::KEY_CODE_SHIFT = 34;
const int Keyboard::KEY_CODE_ENTER = 35;
const int Keyboard::KEY_CODE_SPACE = 36;
const int Keyboard::KEY_CODE_SYMBOL = 37;


const uint8_t Keyboard::KEY_SHIFT_PORT_VALUES[] = { 0x1e, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_Z_PORT_VALUES[]     = { 0x1d, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_X_PORT_VALUES[]     = { 0x1b, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_C_PORT_VALUES[]     = { 0x17, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_V_PORT_VALUES[]     = { 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };

const uint8_t Keyboard::KEY_A_PORT_VALUES[]     = { 0x1f, 0x1e, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_S_PORT_VALUES[]     = { 0x1f, 0x1d, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_D_PORT_VALUES[]     = { 0x1f, 0x1b, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_F_PORT_VALUES[]     = { 0x1f, 0x17, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_G_PORT_VALUES[]     = { 0x1f, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };

const uint8_t Keyboard::KEY_Q_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1e, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_W_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1d, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_E_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1b, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_R_PORT_VALUES[]     = { 0x1f, 0x1f, 0x17, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_T_PORT_VALUES[]     = { 0x1f, 0x1f, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };

const uint8_t Keyboard::KEY_1_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1e, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_2_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1d, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_3_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1b, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_4_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x17, 0x1f, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_5_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f };

const uint8_t Keyboard::KEY_0_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1e, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_9_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1d, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_8_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1b, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_7_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x17, 0x1f, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_6_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x0f, 0x1f, 0x1f, 0x1f };

const uint8_t Keyboard::KEY_P_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1e, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_O_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1d, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_I_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1b, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_U_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x17, 0x1f, 0x1f };
const uint8_t Keyboard::KEY_Y_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0f, 0x1f, 0x1f };

const uint8_t Keyboard::KEY_ENTER_PORT_VALUES[] = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1e, 0x1f };
const uint8_t Keyboard::KEY_L_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1d, 0x1f };
const uint8_t Keyboard::KEY_K_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1b, 0x1f };
const uint8_t Keyboard::KEY_J_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x17, 0x1f };
const uint8_t Keyboard::KEY_H_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0f, 0x1f };

const uint8_t Keyboard::KEY_SPACE_PORT_VALUES[] = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1e };
const uint8_t Keyboard::KEY_SYM_PORT_VALUES[]   = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1d };
const uint8_t Keyboard::KEY_M_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1b };
const uint8_t Keyboard::KEY_N_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x17 };
const uint8_t Keyboard::KEY_B_PORT_VALUES[]     = { 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0f };

Keyboard::Keyboard(uint8_t *ioPorts) : m_ioPorts(ioPorts) {
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_0, KEY_0_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_1, KEY_1_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_2, KEY_2_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_3, KEY_3_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_4, KEY_4_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_5, KEY_5_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_6, KEY_6_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_7, KEY_7_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_8, KEY_8_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_9, KEY_9_PORT_VALUES });

    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_A, KEY_A_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_B, KEY_B_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_C, KEY_C_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_D, KEY_D_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_E, KEY_E_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_F, KEY_F_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_G, KEY_G_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_H, KEY_H_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_I, KEY_I_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_J, KEY_J_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_K, KEY_K_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_L, KEY_L_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_M, KEY_M_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_N, KEY_N_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_O, KEY_O_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_P, KEY_P_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_Q, KEY_Q_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_R, KEY_R_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_S, KEY_S_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_T, KEY_T_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_U, KEY_U_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_V, KEY_V_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_W, KEY_W_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_X, KEY_X_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_Y, KEY_Y_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_Z, KEY_Z_PORT_VALUES });

    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_SHIFT, KEY_SHIFT_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_ENTER, KEY_ENTER_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_SPACE, KEY_SPACE_PORT_VALUES });
    m_keyCodesOnPortValuesMap.insert({ KEY_CODE_SYMBOL, KEY_SYM_PORT_VALUES });

    for (int i = 0; i < sizeof(KEYBOARD_PORT_ADDRESSES) / sizeof(uint16_t); i++) {
        m_ioPorts[KEYBOARD_PORT_ADDRESSES[i]] = 0x1f;
    }
}

void Keyboard::onKeyPressed(const int keyCode) {
    const uint8_t *portValues = m_keyCodesOnPortValuesMap[keyCode];
    for (int i = 0; i < sizeof(KEYBOARD_PORT_ADDRESSES) / sizeof(uint16_t); i++) {
        uint8_t portValue = portValues[i];
        if (portValue != 0x1f) {
            m_ioPorts[KEYBOARD_PORT_ADDRESSES[i]] &= portValue;
        }
    }

}

void Keyboard::onKeyReleased(const int keyCode) {
    const uint8_t *portValues = m_keyCodesOnPortValuesMap[keyCode];
    for (int i = 0; i < sizeof(KEYBOARD_PORT_ADDRESSES) / sizeof(uint16_t); i++) {
        uint8_t portValue = portValues[i];
        if (portValue != 0x1f) {
            m_ioPorts[KEYBOARD_PORT_ADDRESSES[i]] |= ((~portValue) & 0x1f);
        }
    }
}
