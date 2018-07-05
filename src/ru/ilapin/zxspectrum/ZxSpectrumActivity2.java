package ru.ilapin.zxspectrum;

import android.annotation.SuppressLint;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.TextView;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

import ru.ilapin.zxspectrum.R;
import ru.ilapin.common.android.widgets.PressReleaseButton;
import android.app.*;

public class ZxSpectrumActivity2 extends Activity {

    private static final String TAG = "ZxSpectrumActivity2";

    private final Map<Integer, Integer> mKeyCodesMap = new HashMap<Integer, Integer>(){{
        put(KeyEvent.KEYCODE_0, Keyboard.KEY_CODE_0);
        put(KeyEvent.KEYCODE_1, Keyboard.KEY_CODE_1);
        put(KeyEvent.KEYCODE_2, Keyboard.KEY_CODE_2);
        put(KeyEvent.KEYCODE_3, Keyboard.KEY_CODE_3);
        put(KeyEvent.KEYCODE_4, Keyboard.KEY_CODE_4);
        put(KeyEvent.KEYCODE_5, Keyboard.KEY_CODE_5);
        put(KeyEvent.KEYCODE_6, Keyboard.KEY_CODE_6);
        put(KeyEvent.KEYCODE_7, Keyboard.KEY_CODE_7);
        put(KeyEvent.KEYCODE_8, Keyboard.KEY_CODE_8);
        put(KeyEvent.KEYCODE_9, Keyboard.KEY_CODE_9);

        put(KeyEvent.KEYCODE_A, Keyboard.KEY_CODE_A);
        put(KeyEvent.KEYCODE_B, Keyboard.KEY_CODE_B);
        put(KeyEvent.KEYCODE_C, Keyboard.KEY_CODE_C);
        put(KeyEvent.KEYCODE_D, Keyboard.KEY_CODE_D);
        put(KeyEvent.KEYCODE_E, Keyboard.KEY_CODE_E);
        put(KeyEvent.KEYCODE_F, Keyboard.KEY_CODE_F);
        put(KeyEvent.KEYCODE_G, Keyboard.KEY_CODE_G);
        put(KeyEvent.KEYCODE_H, Keyboard.KEY_CODE_H);
        put(KeyEvent.KEYCODE_I, Keyboard.KEY_CODE_I);
        put(KeyEvent.KEYCODE_J, Keyboard.KEY_CODE_J);
        put(KeyEvent.KEYCODE_K, Keyboard.KEY_CODE_K);
        put(KeyEvent.KEYCODE_L, Keyboard.KEY_CODE_L);
        put(KeyEvent.KEYCODE_M, Keyboard.KEY_CODE_M);
        put(KeyEvent.KEYCODE_N, Keyboard.KEY_CODE_N);
        put(KeyEvent.KEYCODE_O, Keyboard.KEY_CODE_O);
        put(KeyEvent.KEYCODE_P, Keyboard.KEY_CODE_P);
        put(KeyEvent.KEYCODE_Q, Keyboard.KEY_CODE_Q);
        put(KeyEvent.KEYCODE_R, Keyboard.KEY_CODE_R);
        put(KeyEvent.KEYCODE_S, Keyboard.KEY_CODE_S);
        put(KeyEvent.KEYCODE_T, Keyboard.KEY_CODE_T);
        put(KeyEvent.KEYCODE_U, Keyboard.KEY_CODE_U);
        put(KeyEvent.KEYCODE_V, Keyboard.KEY_CODE_V);
        put(KeyEvent.KEYCODE_W, Keyboard.KEY_CODE_W);
        put(KeyEvent.KEYCODE_X, Keyboard.KEY_CODE_X);
        put(KeyEvent.KEYCODE_Y, Keyboard.KEY_CODE_Y);
        put(KeyEvent.KEYCODE_Z, Keyboard.KEY_CODE_Z);

        put(KeyEvent.KEYCODE_ENTER, Keyboard.KEY_CODE_ENTER);
        put(KeyEvent.KEYCODE_SPACE, Keyboard.KEY_CODE_SPACE);
    }};

    static {
        System.loadLibrary("zx-spectrum-emulator");
    }

    //@BindView(R.id.zx_spectrum_screen)
    ZxSpectrumView2 mScreenView;
    //@BindView(R.id.ips)
    TextView mInstructionsPerSecondView;
    //@BindView(R.id.interruptsPerSecond)
    TextView mInterruptsPerSecondView;
    //@BindView(R.id.exceededInstructions)
    TextView mExceededInstructionsView;
    //@BindView(R.id.capsShiftButton)
    PressReleaseButton mCapsShiftButton;
    //@BindView(R.id.symbolShiftButton)
    PressReleaseButton mSymbolShiftButton;

    private final int[] mScreenData = new int[ZxSpectrumView2.SCREEN_WIDTH * ZxSpectrumView2.SCREEN_HEIGHT * 4];

    private Thread mZxSpectrumThread;
    private Runnable mUpdateStatsRoutine;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_zx_spectrum2);

		mScreenView.setBitmapDataProvider(new ZxSpectrumView2.BitmapDataProvider() {

				@Override
				public int[] getData(boolean isFlash) {
					getZxSpectrumScreen(mScreenData, isFlash);
					return mScreenData;
				}
		});
        
        final File logFile = new File(
                Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS),
                "z80_1000cpp.log"
        );

        mCapsShiftButton.setListener(new PressReleaseButton.Listener() {

            @Override
            public void onPress() {
                onKeyPressed(Keyboard.KEY_CODE_SHIFT);
            }

            @Override
            public void onRelease() {
                onKeyReleased(Keyboard.KEY_CODE_SHIFT);
            }
        });
        mSymbolShiftButton.setListener(new PressReleaseButton.Listener() {

            @Override
            public void onPress() {
                onKeyPressed(Keyboard.KEY_CODE_SYMBOL);
            }

            @Override
            public void onRelease() {
                onKeyReleased(Keyboard.KEY_CODE_SYMBOL);
            }
        });

        new LoadRomTask().execute("48.rom", logFile.getAbsolutePath());
    }

    @Override
    protected void onResume() {
        super.onResume();

        mUpdateStatsRoutine = () -> {
            final float instructionsPerSecond = getInstructionsCount();
            if (instructionsPerSecond > 0) {
                mInstructionsPerSecondView.setText(
                        getString(R.string.zx_instructions_per_second, String.valueOf(instructionsPerSecond)));
            } else {
                mInstructionsPerSecondView.setText(
                        getString(R.string.zx_instructions_per_second, getString(R.string.not_available)));
            }

            final float interruptsPerSecond = getInterruptCount();
            if (interruptsPerSecond > 0) {
                mInterruptsPerSecondView.setText(
                        getString(R.string.zx_interrupts_per_second, String.valueOf(interruptsPerSecond)));
            } else {
                mInterruptsPerSecondView.setText(
                        getString(R.string.zx_interrupts_per_second, getString(R.string.not_available))
                );
            }

            final float exceededInstructions = getExceededInstructionsPercent();
            if (exceededInstructions > 0) {
                mExceededInstructionsView.setText(
                        getString(R.string.zx_exceeded_instruction_percent, String.valueOf(exceededInstructions)));
            } else {
                mExceededInstructionsView.setText(
                        getString(R.string.zx_exceeded_instruction_percent, getString(R.string.not_available))
                );
            }

            mScreenView.postDelayed(mUpdateStatsRoutine, 1000);
        };
        mScreenView.postDelayed(mUpdateStatsRoutine, 1000);
    }

    @Override
    public boolean onKeyDown(final int keyCode, final KeyEvent event) {
        if (mKeyCodesMap.containsKey(keyCode)) {
            onKeyPressed(mKeyCodesMap.get(keyCode));
        }

        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(final int keyCode, final KeyEvent event) {
        if (mKeyCodesMap.containsKey(keyCode)) {
            mScreenView.postDelayed(() -> onKeyReleased(mKeyCodesMap.get(keyCode)), 250);
        }

        return super.onKeyUp(keyCode, event);
    }

    @Override
    protected void onPause() {
        super.onPause();

        mScreenView.removeCallbacks(mUpdateStatsRoutine);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        stopZxSpectrum();
        try {
            mZxSpectrumThread.join();
        } catch (final InterruptedException e) {
            Log.e(TAG, "Error stopping ZX Spectrum thread", e);
        }
    }

    @OnClick(R.id.resetButton)
    public void onResetButtonClick() {
        resetZxSpectrum();
    }

    @SuppressLint("StaticFieldLeak")
    private class LoadRomTask extends AsyncTask<String, Void, Void> {

        @Override
        protected Void doInBackground(final String... params) {
            try {
                final InputStream is = getAssets().open(params[0]);

                final byte[] buffer = new byte[65536];
                final int bytesRead = is.read(buffer);
                final byte[] program = new byte[bytesRead];
                System.arraycopy(buffer, 0, program, 0, bytesRead);
                initZxSpectrum(program, params[1]);

                is.close();
            } catch (final IOException e) {
                throw new RuntimeException(e);
            }

            return null;
        }

        @Override
        protected void onPostExecute(final Void aVoid) {
            mZxSpectrumThread = new Thread(ZxSpectrumActivity2.this::runZxSpectrum);
            mZxSpectrumThread.start();

            mScreenView.setVerticalRefreshListener(ZxSpectrumActivity2.this::onVerticalRefresh);
        }
    }

    private native void nativeGetScreenData(final byte[] memory, final int[] outData, boolean isFlash);
    private native void runNativeThread();

    private native void initZxSpectrum(byte[] program, String logFilePath);
    private native void runZxSpectrum();
    private native void stopZxSpectrum();
    private native void resetZxSpectrum();
    private native void getZxSpectrumScreen(int[] outData, boolean isFlash);
    private native void onVerticalRefresh();
    private native void onKeyPressed(int keyCode);
    private native void onKeyReleased(int keyCode);
    private native void writeToPort(int port, byte value);
    private native float getExceededInstructionsPercent();
    private native int getInterruptCount();
    private native int getInstructionsCount();
}