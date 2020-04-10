package com.vanxum.Aic;

public class NetworkJni
{
	private static NetworkJni mInstance=null;

	MainRender mr;
	RenderBoard rb;

	private NetworkJni(){

	}

	public synchronized static NetworkJni getInstance() {
		if (mInstance == null)
			mInstance = new NetworkJni();
		return mInstance;
	}

	 static
	 {
	     System.loadLibrary("network_vmtl");
	 }
	public native int vmtlInit(String local_ip,String remote_ip,int local_port,int remote_port,int decodeType);

	public native int startVmtl(boolean isStart);
	public native void stopVmtl();
	public native void sendInputEvent(byte[] event,int len);
	public native void sendExamPacket();



	public void setMr(MainRender render)
	{
		mr = render;
	}
	public void setRb(RenderBoard render)
	{
		rb = render;
	}

	public int networkVmtlInit(String local_ip,String remote_ip,int local_port,int remote_port,int decodeType)
	{
		return vmtlInit(local_ip,remote_ip,local_port,remote_port,decodeType);

	}
	public void networkPutMsg(byte[] msg)
	{

	}
	public void sendExam()
	{
		sendExamPacket();
	}

	void reportVideoData(byte[] data,int len)
	{
		mr.reportVideoData(data,len);
	}
	void reportAudioData(byte[] data,int len)
	{
		mr.reportAudioData(data,len);
	}
	void reportExam(){ rb.onExam();}
	void reportConnected() {mr.onConnected();}
	void releaseVmtl()
	{
		stopVmtl();
	}
	void sendInputEventToJni(byte[] data,int len)
	{
		sendInputEvent(data,len);
	}
}