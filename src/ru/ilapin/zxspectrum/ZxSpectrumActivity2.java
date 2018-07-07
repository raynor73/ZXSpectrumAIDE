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
import android.view.View.*;
import android.view.*;
import java.util.concurrent.*;
import android.media.*;
import java.util.*;
import java.io.*;
import java.text.*;
import android.content.*;
import android.widget.*;
import android.net.*;
import android.database.*;
import java.net.*;

public class ZxSpectrumActivity2 extends Activity {

    private static final String TAG = "ZxSpectrumActivity2";
	
	private static final int SAMPLE_RATE = 44100;
	private static final int FILE_SELECT_CODE = 123;
	
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
        System.loadLibrary("hello-jni");
    }
	
	private final SimpleDateFormat mSaveFilenameFormat = new SimpleDateFormat("'zx48k'_ddMMyyyy_HHmmss.'raw'");

    ZxSpectrumView2 mScreenView;
    TextView mInstructionsPerSecondView;
    TextView mInterruptsPerSecondView;
    TextView mExceededInstructionsView;
    PressReleaseButton mCapsShiftButton;
    PressReleaseButton mSymbolShiftButton;

    private final int[] mScreenData = new int[ZxSpectrumView2.SCREEN_WIDTH * ZxSpectrumView2.SCREEN_HEIGHT * 4];

    private Thread mZxSpectrumThread;
	private Thread mSoundThread;
	private Thread mAudioTrackThread;
    private Runnable mUpdateStatsRoutine;
	
	private int mBufferSize;
	
	private final BlockingQueue<short[]> mSoundDataQueue = new LinkedBlockingQueue<>(1);

	private boolean mShouldShowStats;
	
    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_zx_spectrum2);

		mBufferSize = AudioTrack.getMinBufferSize(SAMPLE_RATE, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT);
		
		mScreenView = findViewById(R.id.zx_spectrum_screen);
		mInstructionsPerSecondView = findViewById(R.id.ips);
		mInterruptsPerSecondView = findViewById(R.id.interruptsPerSecond);
		mExceededInstructionsView = findViewById(R.id.exceededInstructions);
		mCapsShiftButton = findViewById(R.id.capsShiftButton);
		mSymbolShiftButton = findViewById(R.id.symbolShiftButton);
		
		mScreenView.setBitmapDataProvider(new ZxSpectrumView2.BitmapDataProvider() {

				@Override
				public int[] getData(boolean isFlash) {
					getZxSpectrumScreen(mScreenData, isFlash);
					return mScreenData;
				}
				
				@Override
				public int getBorder() {
					return getZxSpectrumScreenBorder();
				}
		});
        
        final File logFile = new File(
                Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS),
                "z80_1000cpp.log"
        );

		mScreenView.setVerticalRefreshListener(new ZxSpectrumView2.VerticalRefreshListener() {

			@Override
			public void onVerticalRefresh() {
				ZxSpectrumActivity2.this.onVerticalRefresh();
			}
		});
		
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
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.zx_spectrum_menu, menu);
		return true;
	}

	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		boolean zxSpectrumIsRunning = mZxSpectrumThread != null && mZxSpectrumThread.isAlive();
		
		menu.findItem(R.id.start_item).setVisible(!zxSpectrumIsRunning);
		menu.findItem(R.id.stop_item).setVisible(zxSpectrumIsRunning);
		menu.findItem(R.id.save_item).setVisible(!zxSpectrumIsRunning);
		menu.findItem(R.id.restore_item).setVisible(!zxSpectrumIsRunning);
		
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
			case R.id.reset_item:
				resetZxSpectrum();
				return true;
				
			case R.id.toggle_stats_item:
				mShouldShowStats = !mShouldShowStats;
				updateStatsVisibility();
				return true;
				
			case R.id.start_item:
				doStartZxSpectrum();
				return true;
				
			case R.id.stop_item:
				doStopZxSpectrum();
				return true;
				
			case R.id.save_item:
				doSaveZxSpectrum();
				return true;
				
			case R.id.restore_item:
				doRestoreZxSpectrum();
				return true;
				
			default:
				return super.onOptionsItemSelected(item);
		}
	}
	
    @Override
    protected void onResume() {
        super.onResume();

		mUpdateStatsRoutine = new Runnable() {

			@Override
			public void run() {
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
			}
		};
		mScreenView.postDelayed(mUpdateStatsRoutine, 1000);
		
		updateStatsVisibility();
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
			mScreenView.postDelayed(new Runnable() {

				@Override
				public void run() {
					onKeyReleased(mKeyCodesMap.get(keyCode));
				}
			}, 250);
        }

        return super.onKeyUp(keyCode, event);
    }

    @Override
    protected void onPause() {
        super.onPause();

        mScreenView.removeCallbacks(mUpdateStatsRoutine);
    }
	
	private static String getPath(Context context, Uri uri) throws URISyntaxException {
		if ("content".equalsIgnoreCase(uri.getScheme())) {
			String[] projection = { "_data" };
			Cursor cursor = null;

			try {
				cursor = context.getContentResolver().query(uri, projection, null, null, null);
				int column_index = cursor.getColumnIndexOrThrow("_data");
				if (cursor.moveToFirst()) {
					return cursor.getString(column_index);
				}
			} catch (Exception e) {
				// Eat it
			}
		}
		else if ("file".equalsIgnoreCase(uri.getScheme())) {
			return uri.getPath();
		}

		return null;
	} 
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		switch (requestCode) {
			case FILE_SELECT_CODE:
				if (resultCode == RESULT_OK) {
					File saveFile;
					try {
						saveFile = new File(ZxSpectrumActivity2.getPath(this, data.getData()));
					} catch (URISyntaxException e) {
						throw new RuntimeException(e);
					}
					DataInputStream is;
					try {
						is = new DataInputStream(new BufferedInputStream(new FileInputStream(saveFile)));
					} catch (FileNotFoundException e) {
						throw new RuntimeException(e);
					}
					restoreZxSpectrumState(is);
					try {
						is.close();
					} catch (IOException e) {
						throw new RuntimeException(e);
					}
				}
				break;
				
			default:
				super.onActivityResult(requestCode, resultCode, data);
		}
	}
	
    @Override
    protected void onDestroy() {
        super.onDestroy();
		
		mAudioTrackThread.interrupt();
        doStopZxSpectrum();
        try {
            mZxSpectrumThread.join();
        } catch (final InterruptedException e) {
            Log.e(TAG, "Error stopping ZX Spectrum thread", e);
        }
    }
	
	private void doSaveZxSpectrum() {
		final String saveFilename = mSaveFilenameFormat.format(new Date());
		final File saveFile = new File(
			Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS),
			saveFilename
		);
		final DataOutputStream os;
		try {
			os = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(saveFile)));
		} catch (FileNotFoundException e) {
			//Log.e(TAG, "Can't save state", e);
			throw new RuntimeException(e);
		}
		
		saveZxSpectrumState(os);
		
		try {
			os.close();
		} catch (IOException e) {
			throw new RuntimeException(e);
		}
	}

	private void doRestoreZxSpectrum() {
		Intent intent = new Intent(Intent.ACTION_GET_CONTENT); 
		intent.setType("*/*"); 
		intent.addCategory(Intent.CATEGORY_OPENABLE);

		try {
			startActivityForResult(
                Intent.createChooser(intent, "Select saved state"),
                FILE_SELECT_CODE
			);
		} catch (ActivityNotFoundException e) {
			Toast.makeText(this, "Please install a File Manager", 
						   Toast.LENGTH_SHORT).show();
		}
	}
	
	private void updateStatsVisibility() {
		int visibility = mShouldShowStats ? View.VISIBLE : View.GONE;
		mInstructionsPerSecondView.setVisibility(visibility);
		mInterruptsPerSecondView.setVisibility(visibility);
		mExceededInstructionsView.setVisibility(visibility);
	}

	private void doStartZxSpectrum() {
		mZxSpectrumThread = new Thread(new Runnable() {

				@Override
				public void run() {
					ZxSpectrumActivity2.this.runZxSpectrum();
				}
			});
		mZxSpectrumThread.start();

		mSoundThread = new Thread(new Runnable() {

				@Override
				public void run() {
					ZxSpectrumActivity2.this.runSound();
				}
			});
		mSoundThread.setPriority(Thread.MAX_PRIORITY);
		mSoundThread.start();
		
		invalidateOptionsMenu();
	}
	
	private void doStopZxSpectrum() {
		stopZxSpectrum();
		
		invalidateOptionsMenu();
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
                initZxSpectrum(program, SAMPLE_RATE, mBufferSize, mSoundDataQueue, params[1]);

                is.close();
            } catch (final IOException e) {
                throw new RuntimeException(e);
            }

            return null;
        }

        @Override
        protected void onPostExecute(final Void aVoid) {
			doStartZxSpectrum();
			
			mAudioTrackThread = new Thread(new Runnable() {
				
				private final AudioTrack mAudioTrack;
				private final short[] mSurrogateBuffer = new short[1];
				private short mLastSample;
					
				{
					mAudioTrack = new AudioTrack(
						AudioManager.STREAM_MUSIC,
						SAMPLE_RATE,
						AudioFormat.CHANNEL_CONFIGURATION_MONO,
						AudioFormat.ENCODING_PCM_16BIT, 
						mBufferSize * 2, 
						AudioTrack.MODE_STREAM
					);
					mAudioTrack.play();
				}
				
				@Override
				public void run() {
					while (!mAudioTrackThread.isInterrupted()) {
						short[] data;
						data = mSoundDataQueue.poll();
						if (data == null) {
							data = mSurrogateBuffer;
							//Arrays.fill(data, mLastSample);
							data[0] = mLastSample;
						} else {
							mLastSample = data[data.length - 1];
						}
						/*try {
							data = mSoundDataQueue.take();
						} catch (InterruptedException e) {
							throw new RuntimeException(e);
						}*/
						mAudioTrack.write(data, 0, data.length);
					}
					mAudioTrack.stop();
				}
			});
			mAudioTrackThread.setPriority(Thread.MAX_PRIORITY);
            mAudioTrackThread.start();
		}
    }

    private native void initZxSpectrum(byte[] program, int sampleRate, int bufferSize, BlockingQueue<short[]> queue, String logFilePath);
    private native void runZxSpectrum();
    private native void stopZxSpectrum();
    private native void resetZxSpectrum();
	private native void runSound();
    private native void getZxSpectrumScreen(int[] outData, boolean isFlash);
	private native int getZxSpectrumScreenBorder();
    private native void onVerticalRefresh();
    private native void onKeyPressed(int keyCode);
    private native void onKeyReleased(int keyCode);
    private native float getExceededInstructionsPercent();
    private native int getInterruptCount();
    private native int getInstructionsCount();
	private native void saveZxSpectrumState(DataOutputStream os);
	private native void restoreZxSpectrumState(DataInputStream is);
}
