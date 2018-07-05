package ru.ilapin.zxspectrum;
import android.app.*;
import android.os.*;
import android.media.*;
import android.util.*;
import java.util.concurrent.*;
import java.util.*;

public class AudioTrackResearchActivity extends Activity {

	private static final int SAMPLE_RATE = 44100;
	
	private final BlockingQueue<short[]> mQueue = new LinkedBlockingQueue<>();
	private SoundRenderingThread mRenderingThread;
	private PlaybackThread mPlaybackThread;
	private CombinedThread mCombinedThread;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_audio_track_research);
		int minBufferSize = AudioTrack.getMinBufferSize(SAMPLE_RATE, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT);
		//Log.d("!#@", "minBufferSize: " + minBufferSize);
		mRenderingThread = new SoundRenderingThread(SAMPLE_RATE, minBufferSize, mQueue);
		mRenderingThread.start();
		mPlaybackThread = new PlaybackThread(SAMPLE_RATE, minBufferSize, mQueue);
		mPlaybackThread.start();
		/*mCombinedThread = new CombinedThread(SAMPLE_RATE, minBufferSize);
		mCombinedThread.start();*/
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		//mCombinedThread.interrupt();
		mPlaybackThread.interrupt();
		try {
			mPlaybackThread.join();
		} catch (InterruptedException e) {
			throw new RuntimeException(e);
		}
		mRenderingThread.interrupt();
	}
	
	private static class CombinedThread extends Thread {

		private static final int FREQUENCY = 500;
		
		private final AudioTrack mAudioTrack;
		private final short[] mData;
		private final int mSampleRate;
		
		public CombinedThread(int sampleRate, int bufferSize) {
			mSampleRate = sampleRate;
			mData = new short[bufferSize];
			mAudioTrack = new AudioTrack(
				AudioManager.STREAM_MUSIC,
				sampleRate,
				AudioFormat.CHANNEL_CONFIGURATION_MONO,
				AudioFormat.ENCODING_PCM_16BIT, 
				bufferSize, 
				AudioTrack.MODE_STREAM
			);
			mAudioTrack.play();
		}

		@Override
		public void run() {
			float t = 0;
			float dt = 1.0f / mSampleRate;
			float angularFrequency = (float) (2 * Math.PI * FREQUENCY);
			while (!isInterrupted()) {
				for (int i = 0; i < mData.length; i++) {
					mData[i] = (short) (Math.sin(t * angularFrequency) * Short.MAX_VALUE);
					t += dt;
				}
				mAudioTrack.write(mData, 0, mData.length);
			}
			mAudioTrack.stop();
		}
	}
	
	private static class PlaybackThread extends Thread {
		
		//private static final int FREQUENCY = 500;
		
		private final AudioTrack mAudioTrack;
		private final int mSampleRate;
		private final int mBufferSize;
		private final BlockingQueue<short[]> mQueue;
		
		private short mLastLevel;
		
		public PlaybackThread(int sampleRate, int bufferSize, BlockingQueue<short[]> queue) {
			mSampleRate = sampleRate;
			mBufferSize = bufferSize;
			mQueue = queue;
			mAudioTrack = new AudioTrack(
				AudioManager.STREAM_MUSIC,
				sampleRate,
				AudioFormat.CHANNEL_CONFIGURATION_MONO,
				AudioFormat.ENCODING_PCM_16BIT, 
				bufferSize, 
				AudioTrack.MODE_STREAM
			);
			mAudioTrack.play();
		}
		
		@Override
		public void run() {
			/*float t = 0;
			float dt = 1.0f / mSampleRate;
			float angularFrequency = (float) (2 * Math.PI * FREQUENCY);*/
			
			while (!isInterrupted()) {
				short[] data;
				if (mQueue.size() != 0) {
					try {
						data = /*new short[mBufferSize]; //*/mQueue.take();
					} catch (InterruptedException e) {
						throw new RuntimeException(e);
					}
					mLastLevel = data[data.length - 1];
				} else {
					data = new short[mBufferSize];
					Arrays.fill(data, mLastLevel);
				}
				

				/*for (int i = 0; i < data.length; i++) {
					data[i] = (short) (Math.sin(t * angularFrequency) * Short.MAX_VALUE);
					t += dt;
				}*/
				
				mAudioTrack.write(data, 0, data.length);
			}
			mAudioTrack.stop();
		}
	}
	
	private static class SoundRenderingThread extends Thread {

		private static final int FREQUENCY = 500;
		
		private final int mSampleRate;
		private final int mBufferSize;
		private final BlockingQueue<short[]> mQueue;
		
		/*private final short[] mBuffer0;
		private final short[] mBuffer1;
		
		private short[] mCurrentBuffer;
		private short[] mReadyBuffer;*/
		
		public SoundRenderingThread(int sampleRate, int bufferSize, BlockingQueue<short[]> queue) {
			mSampleRate = sampleRate;
			mBufferSize = bufferSize;
			mQueue = queue;
			/*mBuffer0 = new short[bufferSize];
			mBuffer1 = new short[bufferSize];
			
			mCurrentBuffer = mBuffer0;
			mReadyBuffer = mBuffer1;*/
		}
		
		@Override
		public void run() {
			float t = 0;
			float dt = 1.0f / mSampleRate;
			float angularFrequency = (float) (2 * Math.PI * FREQUENCY);
			
			while (!isInterrupted()) {
				short[] data = new short[mBufferSize];
				t %= 1.0 / FREQUENCY;
				for (int i = 0; i < data.length; i++) {
					data[i] = 0;//(short) (Math.sin(t * angularFrequency) * Short.MAX_VALUE);
					t += dt;
				}
				
				if (mQueue.size() == 0) {
					try {
						mQueue.put(data);
					} catch (InterruptedException e) {
						throw new RuntimeException(e);
					}
				}
				//swapBuffers();
			}
		}
		
		/*public short[] getSoundData() {
			short[] data = new short[mBuffer0.length];
			
			System.arraycopy(mReadyBuffer, 0, data, 0, data.length);
			
			return data;
		}
		
		private void swapBuffers() {
			if (mCurrentBuffer == mBuffer0) {
				mCurrentBuffer = mBuffer1;
				mReadyBuffer = mBuffer0;
			} else {
				mCurrentBuffer = mBuffer0;
				mReadyBuffer = mBuffer1;
			}
		}*/
	}
}
