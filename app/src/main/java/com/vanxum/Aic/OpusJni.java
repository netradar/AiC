package com.vanxum.Aic;

public class OpusJni
{
	private static OpusJni mInstance;

	public synchronized static OpusJni getInstance() {
		if (mInstance == null)
			mInstance = new OpusJni();
		return mInstance;
	}

	 static
	 {
	     System.loadLibrary("opus_coder");
	 }
	public native int OpusInit();

	public native int Opusdecode(byte[] src, short[] out, int size);

}