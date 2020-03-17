package com.vanxum.Aic;

import android.util.Log;

import com.vanxum.Aic.MyInputEvent;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Arrays;
import java.util.concurrent.ArrayBlockingQueue;

public class NetworkManager {
	
	boolean isConnected = false;

	Socket socket =null;
	Socket socketAudio = null;
	
	boolean recvFlag,sendFlag;
	RenderBoard render;


	InputStream inputStream, inputStreamAudio;
	
	OutputStream outputStream;
	
	Thread	recvThread = null,sendThread =null,recvAudioThread = null;


	private ArrayBlockingQueue<MyInputEvent> sendBuffer = new ArrayBlockingQueue<>(100);

	public class SendRunnable implements Runnable
	{

		@Override
		public void run() 
		{

			while(sendFlag)
			{

				MyInputEvent info = new MyInputEvent();
			//	synchronized (sendBuffer)
				{

					try {
						info = sendBuffer.take();
						if(!sendFlag)
							break;
						if(info!=null)
							sendData(info);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}



				}

		//		Log.d("lichao","sendBuffer 5");

				
				
			}

			Log.d("lichao","sendMsg thread exit");
		}
			
		
		
	}

	public class RecvRunnable implements Runnable
	{

		@Override
		public void run() 
		{
			
			
			
				byte[] headLen = new byte[4];
				int length = 0;
				boolean isClose=false;
				

					isClose = socket.isClosed();

				//	Log.d("lichao","recv  date ");
					
					while(recvFlag&&!isClose&&!socket.isInputShutdown())
					{
					//	Log.d("lichao","recv  date 1");

						if(readData(headLen,4)==0)
						{
							int dataLen = byteToInt(headLen);
					//		Log.d("lichao","date len "+dataLen);
							byte[] data = new byte[dataLen];
							Arrays.fill(data, (byte) 0);
							if(readData(data,dataLen)==0)
							{
							/*	Intent i=new Intent("render.message");
								i.putExtra("isClose", false);
								i.putExtra("buffer", data);
								render.sendBroadcast(i);*/
								render.mainRender.reportVideoData(data,dataLen);
							}
						}



						
						
					}


			Log.d("lichao","recv thread exit");

		}
			
		
		
	}
	public class RecvAudioRunnable implements Runnable
	{

		@Override
		public void run()
		{
			byte[] headLen = new byte[4];
			int length = 0;
			boolean isClose=false;


			isClose = socketAudio.isClosed();


			while(recvFlag&&!isClose&&!socketAudio.isInputShutdown())
			{


				if(readAudioData(headLen,4)==0)
				{
					int dataLen = byteToInt(headLen);
						Log.d("lichao","audio date len "+dataLen);
					byte[] data = new byte[dataLen];
					if(readAudioData(data,dataLen)==0)
					{
						Log.d("lichao","audio recv  date 1");
						render.mainRender.reportAudioData(data,dataLen);
					}
				}





				if (length > 0)
				{
					// act.onReceive(buffer);
					      /*  Intent i=new Intent("car.bd.message");
					        i.putExtra("isClose", false);
							i.putExtra("buffer", buffer);
							act.sendBroadcast(i);*/
				}
				else
				{

				}


			}



			if(recvFlag)
			{
				closeAll();

			}
		}



	}

	public int readData(byte[] buf,int len)
	{

		int offset = 0;
		int ret;
		while (offset < len) {

			try {

				ret = inputStream.read(buf, offset, len-offset);
				if(ret<=0)
					return -1;
				offset+=ret;

			} catch (IOException e) {
				return -1;
			}

		}
		return 0;
	}
	public int readAudioData(byte[] buf,int len)
	{

		int offset = 0;
		int ret;
		while (offset < len) {

			try {

				ret = inputStreamAudio.read(buf, offset, len-offset);
				if(ret<=0)
					return -1;
				offset+=ret;

			} catch (IOException e) {
				e.printStackTrace();
			}

		}
		return 0;
	}
	public int byteToInt(byte[] b)
	{


		int t = 0;
		int s1 = b[0],s2=b[1],s3=b[2],s4=b[3];
		t = (b[0]&0xff)<<24;
		t+= (b[1]&0xff)<<16;
		t+= (b[2]&0xff)<<8;
		t+= (b[3]&0xff);

		return t;
	}
	public NetworkManager(RenderBoard render_)
	{
		render = render_;
		isConnected = false;



	}
	public int connect(String ip,int port)
	{

		Log.d("lichao","ip is "+ip+" port is "+port);
		try {
			try {
			//	socket = new Socket("172.16.200.236", 8181);
			//	socketAudio = new Socket("172.16.200.236", 8182);

				socket = new Socket(ip, port);
				socketAudio = new Socket(ip, port+1);


				if(socket==null||socketAudio==null)
				{
					Log.d("lichao","socket error");
					return -1;
				}
			} catch (IOException e) {
				e.printStackTrace();
				Log.d("lichao",e.toString());
				return -1;
			}

			socket.setSoTimeout(0);
			
			inputStream = socket.getInputStream();
			inputStreamAudio = socketAudio.getInputStream();

			outputStream = socket.getOutputStream();

			recvFlag = true;
			sendFlag = true;

			recvThread = new Thread(new RecvRunnable());
			sendThread = new Thread(new SendRunnable());
			recvAudioThread = new Thread((new RecvAudioRunnable()));
			

			
			recvThread.start();
			sendThread.start();
			recvAudioThread.start();
			
			//putMsg(setModeCommand);
			//switchLed(0,0);
			
			
		} catch (UnknownHostException e)
		{
			return -1;
			
		} catch (IOException e) {
			return -1;
			
		}
		
		
		return 0;
	}
	public void putMsg(MyInputEvent data)
	{

	//	synchronized (sendBuffer)
		{

			try {
				sendBuffer.put(data);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			//		sendBuffer.notify();
		}
	}
	public int sendData(MyInputEvent event)
	{
		byte[] msg = new byte[8];

		//Log.d("lichao2","send data ty is 1    "+event.type);
		msg[0] = (byte)(event.type&0xff);
		msg[1] = (byte)((event.type>>8)&0xff);
	//	Log.d("lichao2","send data ty is  2   "+msg[0]);
		msg[2] = (byte)(event.code&0xff);
		msg[3] = (byte)((event.code>>8)&0xff);

		msg[4] = (byte)(event.value&0xff);
		msg[5] = (byte)((event.value>>8)&0xff);
		msg[6] = (byte)((event.value>>16)&0xff);
		msg[7] = (byte)((event.value>>24)&0xff);

		if(!socket.isClosed()&&socket.isConnected())
		{
			try {
		//		Log.d("lichao2","send data ");
				outputStream.write(msg,0,8);
			//	outputStream.flush();
	//			socket.
	//			Log.d("lichao","send data");
			//	outputStream.flush();
			} catch (UnsupportedEncodingException e) {
				return -1;
			} catch (IOException e) {
				return -1;
			}
		}
		return 0;
	}
	public void closeAll()
	{
		recvFlag = false;
		sendFlag = false;
		
		isConnected = false;

		if(recvThread!=null)
			recvThread.interrupt();
		if(sendThread!=null)
		{
			try {
				sendBuffer.put(new MyInputEvent());
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		if(socket!=null&&socket.isConnected())
		{
			
			try {
				socket.close();
			} catch (IOException e) {
				
			}
		}
		
	//	 Intent i=new Intent("car.bd.message");
	  //   i.putExtra("isClose", true);
		
	//	 act.sendBroadcast(i);
	}

}
