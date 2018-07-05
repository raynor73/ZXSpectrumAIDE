package ru.ilapin.zxspectrum;

import java.util.HashMap;
import java.util.Map;

public class Keyboard {

	private static final int [] KEYBOARD_PORT_ADDRESSES = new int[] {
			0xfefe,
			0xfdfe,
			0xfbfe,
			0xf7fe,
			0xeffe,
			0xdffe,
			0xbffe,
			0x7ffe
	};

	private static final int[] KEY_SHIFT_PORT_VALUES = new int[]{ 0x1e, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_Z_PORT_VALUES     = new int[]{ 0x1d, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_X_PORT_VALUES     = new int[]{ 0x1b, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_C_PORT_VALUES     = new int[]{ 0x17, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_V_PORT_VALUES     = new int[]{ 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };

	private static final int[] KEY_A_PORT_VALUES     = new int[]{ 0x1f, 0x1e, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_S_PORT_VALUES     = new int[]{ 0x1f, 0x1d, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_D_PORT_VALUES     = new int[]{ 0x1f, 0x1b, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_F_PORT_VALUES     = new int[]{ 0x1f, 0x17, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_G_PORT_VALUES     = new int[]{ 0x1f, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };

	private static final int[] KEY_Q_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1e, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_W_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1d, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_E_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1b, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_R_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x17, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_T_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f };

	private static final int[] KEY_1_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1e, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_2_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1d, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_3_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1b, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_4_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x17, 0x1f, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_5_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f };

	private static final int[] KEY_0_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1e, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_9_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1d, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_8_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1b, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_7_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x17, 0x1f, 0x1f, 0x1f };
	private static final int[] KEY_6_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x0f, 0x1f, 0x1f, 0x1f };

	private static final int[] KEY_P_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1e, 0x1f, 0x1f };
	private static final int[] KEY_O_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1d, 0x1f, 0x1f };
	private static final int[] KEY_I_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1b, 0x1f, 0x1f };
	private static final int[] KEY_U_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x17, 0x1f, 0x1f };
	private static final int[] KEY_Y_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0f, 0x1f, 0x1f };

	private static final int[] KEY_ENTER_PORT_VALUES = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1e, 0x1f };
	private static final int[] KEY_L_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1d, 0x1f };
	private static final int[] KEY_K_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1b, 0x1f };
	private static final int[] KEY_J_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x17, 0x1f };
	private static final int[] KEY_H_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0f, 0x1f };

	private static final int[] KEY_SPACE_PORT_VALUES = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1e };
	private static final int[] KEY_SYM_PORT_VALUES   = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1d };
	private static final int[] KEY_M_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1b };
	private static final int[] KEY_N_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x17 };
	private static final int[] KEY_B_PORT_VALUES     = new int[]{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0f };

	private static final Map<Integer, int[]> mKeyCodesOnPortValuesMap = new HashMap<Integer, int[]>(){{
		put(KEY_CODE_0, KEY_0_PORT_VALUES);
		put(KEY_CODE_1, KEY_1_PORT_VALUES);
		put(KEY_CODE_2, KEY_2_PORT_VALUES);
		put(KEY_CODE_3, KEY_3_PORT_VALUES);
		put(KEY_CODE_4, KEY_4_PORT_VALUES);
		put(KEY_CODE_5, KEY_5_PORT_VALUES);
		put(KEY_CODE_6, KEY_6_PORT_VALUES);
		put(KEY_CODE_7, KEY_7_PORT_VALUES);
		put(KEY_CODE_8, KEY_8_PORT_VALUES);
		put(KEY_CODE_9, KEY_9_PORT_VALUES);

		put(KEY_CODE_A, KEY_A_PORT_VALUES);
		put(KEY_CODE_B, KEY_B_PORT_VALUES);
		put(KEY_CODE_C, KEY_C_PORT_VALUES);
		put(KEY_CODE_D, KEY_D_PORT_VALUES);
		put(KEY_CODE_E, KEY_E_PORT_VALUES);
		put(KEY_CODE_F, KEY_F_PORT_VALUES);
		put(KEY_CODE_G, KEY_G_PORT_VALUES);
		put(KEY_CODE_H, KEY_H_PORT_VALUES);
		put(KEY_CODE_I, KEY_I_PORT_VALUES);
		put(KEY_CODE_J, KEY_J_PORT_VALUES);
		put(KEY_CODE_K, KEY_K_PORT_VALUES);
		put(KEY_CODE_L, KEY_L_PORT_VALUES);
		put(KEY_CODE_M, KEY_M_PORT_VALUES);
		put(KEY_CODE_N, KEY_N_PORT_VALUES);
		put(KEY_CODE_O, KEY_O_PORT_VALUES);
		put(KEY_CODE_P, KEY_P_PORT_VALUES);
		put(KEY_CODE_Q, KEY_Q_PORT_VALUES);
		put(KEY_CODE_R, KEY_R_PORT_VALUES);
		put(KEY_CODE_S, KEY_S_PORT_VALUES);
		put(KEY_CODE_T, KEY_T_PORT_VALUES);
		put(KEY_CODE_U, KEY_U_PORT_VALUES);
		put(KEY_CODE_V, KEY_V_PORT_VALUES);
		put(KEY_CODE_W, KEY_W_PORT_VALUES);
		put(KEY_CODE_X, KEY_X_PORT_VALUES);
		put(KEY_CODE_Y, KEY_Y_PORT_VALUES);
		put(KEY_CODE_Z, KEY_Z_PORT_VALUES);

		put(KEY_CODE_SHIFT, KEY_SHIFT_PORT_VALUES);
		put(KEY_CODE_ENTER, KEY_ENTER_PORT_VALUES);
		put(KEY_CODE_SPACE, KEY_SPACE_PORT_VALUES);
		put(KEY_CODE_SYMBOL, KEY_SYM_PORT_VALUES);
	}};

	public static final int KEY_CODE_0 = 0;
	public static final int KEY_CODE_1 = 1;
	public static final int KEY_CODE_2 = 2;
	public static final int KEY_CODE_3 = 3;
	public static final int KEY_CODE_4 = 4;
	public static final int KEY_CODE_5 = 5;
	public static final int KEY_CODE_6 = 6;
	public static final int KEY_CODE_7 = 7;
	public static final int KEY_CODE_8 = 8;
	public static final int KEY_CODE_9 = 38;

	public static final int KEY_CODE_A = 9;
	public static final int KEY_CODE_B = 10;
	public static final int KEY_CODE_C = 11;
	public static final int KEY_CODE_D = 12;
	public static final int KEY_CODE_E = 13;
	public static final int KEY_CODE_F = 14;
	public static final int KEY_CODE_G = 15;
	public static final int KEY_CODE_H = 16;
	public static final int KEY_CODE_I = 17;
	public static final int KEY_CODE_J = 18;
	public static final int KEY_CODE_K = 19;
	public static final int KEY_CODE_L = 20;
	public static final int KEY_CODE_M = 21;
	public static final int KEY_CODE_N = 22;
	public static final int KEY_CODE_O = 23;
	public static final int KEY_CODE_P = 24;
	public static final int KEY_CODE_Q = 25;
	public static final int KEY_CODE_R = 26;
	public static final int KEY_CODE_S = 27;
	public static final int KEY_CODE_T = 28;
	public static final int KEY_CODE_U = 29;
	public static final int KEY_CODE_V = 30;
	public static final int KEY_CODE_W = 31;
	public static final int KEY_CODE_X = 32;
	public static final int KEY_CODE_Y = 33;
	public static final int KEY_CODE_Z = 39;

	public static final int KEY_CODE_SHIFT = 34;
	public static final int KEY_CODE_ENTER = 35;
	public static final int KEY_CODE_SPACE = 36;
	public static final int KEY_CODE_SYMBOL = 37;

	private final byte[] mIoPorts;

	public Keyboard(final byte[] IoPorts) {
		mIoPorts = IoPorts;
		for (final int port : KEYBOARD_PORT_ADDRESSES) {
			mIoPorts[port] = 0x1f;
		}
	}

	public void onKeyPressed(final int keyCode) {
		final int[] portValues = mKeyCodesOnPortValuesMap.get(keyCode);
		for (int i = 0; i < KEYBOARD_PORT_ADDRESSES.length; i++) {
			final int portValue = portValues[i];
			if (portValue != 0x1f) {
				mIoPorts[KEYBOARD_PORT_ADDRESSES[i]] &= (byte) portValue;
			}
		}
	}

	public void onKeyReleased(final int keyCode) {
		final int[] portValues = mKeyCodesOnPortValuesMap.get(keyCode);
		for (int i = 0; i < KEYBOARD_PORT_ADDRESSES.length; i++) {
			final int portValue = portValues[i];
			if (portValue != 0x1f) {
				mIoPorts[KEYBOARD_PORT_ADDRESSES[i]] |= (byte) ((~portValue) & 0x1f);
			}
		}
	}
}
