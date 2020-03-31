package com.vanxum.Aic;

public class NetworkJni
{
	private static NetworkJni mInstance;

	MainRender mr;

	public synchronized static NetworkJni getInstance() {
		if (mInstance == null)
			mInstance = new NetworkJni();
		return mInstance;
	}

	 static
	 {
	     System.loadLibrary("network_vmtl");
	 }
	public native int vmtlInit(String local_ip,String remote_ip,int local_port,int remote_port);

	public native int startVmtl(boolean isStart);
	public native void stopVmtl();
	public native void sendInputEvent(byte[] event,int len);


	public void setMr(MainRender render)
	{
		mr = render;
	}

	public int networkVmtlInit(String local_ip,String remote_ip,int local_port,int remote_port)
	{
		return vmtlInit(local_ip,remote_ip,local_port,remote_port);

	}
	public void networkPutMsg(byte[] msg)
	{

	}


	void reportVideoData(byte[] data,int len)
	{
		mr.reportVideoData(data,len);
	}
	void reportAudioData(byte[] data,int len)
	{
		mr.reportAudioData(data,len);
	}
	void releaseVmtl()
	{
		stopVmtl();
	}
	void sendInputEventToJni(byte[] data,int len)
	{
		sendInputEvent(data,len);
	}
}