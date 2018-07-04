package ru.ilapin.zxspectrum;

import android.annotation.SuppressLint;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import ru.ilapin.zxspectrum.R;
import android.app.*;
import android.util.*;
import android.view.View.*;
import android.view.*;

public class Z80ExerciserActivity extends Activity {

    static {
        //System.loadLibrary("zx-spectrum-emulator");
		System.loadLibrary("hello-jni");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_z80_exerciser);

		findViewById(R.id.test_button).setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View p1) {
				new Thread(new Runnable() {

					@Override
					public void run() {
						Z80ExerciserActivity.this.runTest();
					}
				}).start();
			}
		});
		
        final File logFile = new File(
            Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS),
            "z80_test.log"
        );

        new LoadRomTask().execute("zexall.bin", logFile.getAbsolutePath());
        //new LoadRomTask().execute("8080PRE.COM", logFile.getAbsolutePath());
        //new LoadRomTask().execute("TEST.COM", logFile.getAbsolutePath());
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
				//Log.d("!#$", "bytesRead :" + bytesRead);
                initExerciser(program, params[1]);

                is.close();
            } catch (final IOException e) {
                throw new RuntimeException(e);
            }

            return null;
        }
    }

    private native void initExerciser(byte[] program, String logFilePath);
    private native void runTest();
}
