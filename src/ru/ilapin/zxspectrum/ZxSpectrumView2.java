package ru.ilapin.zxspectrum;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;

public class ZxSpectrumView2 extends View {

	public static final int SCREEN_WIDTH = 256;
	public static final int SCREEN_HEIGHT = 192;

	private static final float ASPECT_RATIO = (float) 4 / 3;
	private static final int FLASH_DELAY = 781; // ms

	private BitmapDataProvider mBitmapDataProvider;
	private VerticalRefreshListener mVerticalRefreshListener;
	private final Bitmap mBitmap;
	private final Paint mPaint;
	private final Rect mSrcRect;
	private final Rect mDstRect;

	private Runnable mFlashRoutine;
	private boolean mIsFlash;

	public ZxSpectrumView2(final Context context) {
		this(context, null, 0);
	}

	public ZxSpectrumView2(final Context context, final AttributeSet attrs) {
		this(context, attrs, 0);
	}

	public ZxSpectrumView2(final Context context, final AttributeSet attrs, final int defStyleAttr) {
		super(context, attrs, defStyleAttr);

		mBitmap = Bitmap.createBitmap(SCREEN_WIDTH, SCREEN_HEIGHT, Bitmap.Config.ARGB_8888);

		mPaint = new Paint();
		mPaint.setAntiAlias(false);

		mSrcRect = new Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		mDstRect = new Rect();

		mFlashRoutine = new Runnable() {

			@Override
			public void run() {
				mIsFlash = !mIsFlash;
				postDelayed(mFlashRoutine, FLASH_DELAY);
			}
		};
		postDelayed(mFlashRoutine, FLASH_DELAY);
	}

	@Override
	protected void onMeasure(final int widthMeasureSpec, final int heightMeasureSpec) {
		super.onMeasure(widthMeasureSpec, heightMeasureSpec);

		final ViewGroup.LayoutParams lp = getLayoutParams();

		final int width;
		final int height;
		if (lp.width == ViewGroup.LayoutParams.WRAP_CONTENT) {
			height = getMeasuredHeight();
			width = (int) (ASPECT_RATIO * height);
		} else if (lp.height == ViewGroup.LayoutParams.WRAP_CONTENT) {
			width = getMeasuredWidth();
			height = (int) (width / ASPECT_RATIO);
		} else {
			width = getMeasuredWidth();
			height = getMeasuredHeight();
		}

		mDstRect.set(0, 0, width, height);

		setMeasuredDimension(width, height);
	}

	/*private long mTimestamp = -1;
	private long mFpsTimestamp = -1;
	private float mFps = -1;*/

	@SuppressLint("NewApi")
	@Override
	protected void onDraw(final Canvas canvas) {
		/*final long currentTimestamp = SystemClock.elapsedRealtimeNanos();
		if (mTimestamp >= 0) {
			mFps = 1e9f / (currentTimestamp - mTimestamp);
		}
		mTimestamp = currentTimestamp;

		if (mFpsTimestamp < 0) {
			mFpsTimestamp = currentTimestamp;
		} else if (currentTimestamp - mFpsTimestamp >= 1e9f) {
			Log.d("!@#", "FPS: " + mFps);
			mFpsTimestamp = currentTimestamp;
		}*/

		if (mBitmapDataProvider != null) {
			mBitmap.setPixels(
					mBitmapDataProvider.getData(mIsFlash), 0, SCREEN_WIDTH, 0, 0,SCREEN_WIDTH, SCREEN_HEIGHT);
		}

		mDstRect.set(0, 0, getWidth(), getHeight());
		canvas.drawBitmap(mBitmap, mSrcRect, mDstRect, mPaint);

		if (mVerticalRefreshListener != null) {
			mVerticalRefreshListener.onVerticalRefresh();
		}

		invalidate();
	}

	public void setBitmapDataProvider(final BitmapDataProvider bitmapDataProvider) {
		mBitmapDataProvider = bitmapDataProvider;
	}

	public void setVerticalRefreshListener(final VerticalRefreshListener verticalRefreshListener) {
		mVerticalRefreshListener = verticalRefreshListener;
	}

	public interface BitmapDataProvider {

		int[] getData(boolean isFlash);
	}

	public interface VerticalRefreshListener {

		void onVerticalRefresh();
	}
}
