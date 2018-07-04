//
// Created by Igor Lapin on 18.06.18.
//

#ifndef ZX_SPECTRUM_KEYBOARD_H
#define ZX_SPECTRUM_KEYBOARD_H


#include <cstdint>
#include <unordered_map>

class Keyboard {
public:
    static const int KEY_CODE_0;
    static const int KEY_CODE_1;
    static const int KEY_CODE_2;
    static const int KEY_CODE_3;
    static const int KEY_CODE_4;
    static const int KEY_CODE_5;
    static const int KEY_CODE_6;
    static const int KEY_CODE_7;
    static const int KEY_CODE_8;
    static const int KEY_CODE_9;
    
    static const int KEY_CODE_A;
    static const int KEY_CODE_B;
    static const int KEY_CODE_C;
    static const int KEY_CODE_D;
    static const int KEY_CODE_E;
    static const int KEY_CODE_F;
    static const int KEY_CODE_G;
    static const int KEY_CODE_H;
    static const int KEY_CODE_I;
    static const int KEY_CODE_J;
    static const int KEY_CODE_K;
    static const int KEY_CODE_L;
    static const int KEY_CODE_M;
    static const int KEY_CODE_N;
    static const int KEY_CODE_O;
    static const int KEY_CODE_P;
    static const int KEY_CODE_Q;
    static const int KEY_CODE_R;
    static const int KEY_CODE_S;
    static const int KEY_CODE_T;
    static const int KEY_CODE_U;
    static const int KEY_CODE_V;
    static const int KEY_CODE_W;
    static const int KEY_CODE_X;
    static const int KEY_CODE_Y;
    static const int KEY_CODE_Z;
    
    static const int KEY_CODE_SHIFT;
    static const int KEY_CODE_ENTER;
    static const int KEY_CODE_SPACE;
    static const int KEY_CODE_SYMBOL;

    Keyboard(uint8_t *ioPorts);

    void onKeyPressed(const int keyCode);
    void onKeyReleased(const int keyCode);

private:
    static const uint16_t KEYBOARD_PORT_ADDRESSES[];

    static const uint8_t KEY_SHIFT_PORT_VALUES[];
    static const uint8_t KEY_Z_PORT_VALUES[];
    static const uint8_t KEY_X_PORT_VALUES[];
    static const uint8_t KEY_C_PORT_VALUES[];
    static const uint8_t KEY_V_PORT_VALUES[];

    static const uint8_t KEY_A_PORT_VALUES[];
    static const uint8_t KEY_S_PORT_VALUES[];
    static const uint8_t KEY_D_PORT_VALUES[];
    static const uint8_t KEY_F_PORT_VALUES[];
    static const uint8_t KEY_G_PORT_VALUES[];

    static const uint8_t KEY_Q_PORT_VALUES[];
    static const uint8_t KEY_W_PORT_VALUES[];
    static const uint8_t KEY_E_PORT_VALUES[];
    static const uint8_t KEY_R_PORT_VALUES[];
    static const uint8_t KEY_T_PORT_VALUES[];

    static const uint8_t KEY_1_PORT_VALUES[];
    static const uint8_t KEY_2_PORT_VALUES[];
    static const uint8_t KEY_3_PORT_VALUES[];
    static const uint8_t KEY_4_PORT_VALUES[];
    static const uint8_t KEY_5_PORT_VALUES[];

    static const uint8_t KEY_0_PORT_VALUES[];
    static const uint8_t KEY_9_PORT_VALUES[];
    static const uint8_t KEY_8_PORT_VALUES[];
    static const uint8_t KEY_7_PORT_VALUES[];
    static const uint8_t KEY_6_PORT_VALUES[];

    static const uint8_t KEY_P_PORT_VALUES[];
    static const uint8_t KEY_O_PORT_VALUES[];
    static const uint8_t KEY_I_PORT_VALUES[];
    static const uint8_t KEY_U_PORT_VALUES[];
    static const uint8_t KEY_Y_PORT_VALUES[];

    static const uint8_t KEY_ENTER_PORT_VALUES[];
    static const uint8_t KEY_L_PORT_VALUES[];
    static const uint8_t KEY_K_PORT_VALUES[];
    static const uint8_t KEY_J_PORT_VALUES[];
    static const uint8_t KEY_H_PORT_VALUES[];

    static const uint8_t KEY_SPACE_PORT_VALUES[];
    static const uint8_t KEY_SYM_PORT_VALUES[];
    static const uint8_t KEY_M_PORT_VALUES[];
    static const uint8_t KEY_N_PORT_VALUES[];
    static const uint8_t KEY_B_PORT_VALUES[];

    std::unordered_map<int, const uint8_t*> m_keyCodesOnPortValuesMap;

    uint8_t *m_ioPorts;
};


#endif //ZX_SPECTRUM_KEYBOARD_H
