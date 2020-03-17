package com.vanxum.Aic;

import android.content.Context;
import android.content.Intent;
import android.graphics.Point;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.media.MediaCodec;
import android.media.MediaFormat;
import android.os.AsyncTask;

import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;

import java.io.IOException;

import java.util.concurrent.ArrayBlockingQueue;

import static java.lang.Thread.sleep;


public class MainRender extends SurfaceView implements SurfaceHolder.Callback,Runnable,  View.OnTouchListener {
    public SurfaceHolder surfaceHolder;
    private final static String TAG = "MainBackground";
    private final static int VIDEO_WIDTH = 1920;
    private final static int VIDEO_HEIGHT = 1080;

    private final static int EV_SYN = 0x00;
    private final static int EV_KEY = 0x01;
    private final static int EV_ABS = 0x03;

    private final static int ABS_X = 0x00;
    private final static int ABS_Y = 0x01;
    private final static int ABS_MT_SLOT = 0x2f;
    private final static int ABS_MT_TRACKING_ID = 0x39;
    private final static int ABS_MT_POSITION_X = 0x35;
    private final static int ABS_MT_POSITION_Y = 0x36;
    private final static int BTN_TOOL_FINGER = 0x145;
    private final static int BTN_TOUCH = 0x14a;
    private final static int SYN_REPORT = 0x00;





    float xRatio,yRatio;
    NetworkManager network;
    String ipaddr;
    int port;




    private ArrayBlockingQueue<byte[]> videoQueue = new ArrayBlockingQueue<>(10000);
    private ArrayBlockingQueue<byte[]> audioQueue = new ArrayBlockingQueue<>(100);



    class ConnectTask extends AsyncTask {
        @Override
        protected Object doInBackground(Object[] objects) {

            try {
                sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            if(network.connect(ipaddr,port)<0) {
                Intent i=new Intent("render.message");
                i.putExtra("networkerr", true);
                rbAct.sendBroadcast(i);
            }
            return 0;

        }
    }

    public void setNet(String ip,int p)
    {
        ipaddr = ip;
        port = p;
    }


    public MainRender(Context context) {
        super(context);
        init(context);
    }

    int count = 0;
    boolean videoThreahFlag = true;
    boolean audioThreadFlag = true;

    RenderBoard rbAct;

    public class VideoThread implements Runnable {
        @Override
        public void run() {

            MediaCodec codec = null;
            MediaFormat mediaFormat = MediaFormat.createVideoFormat("video/avc",1920,1080);
            mediaFormat.setString("mime", "video/avc");

            mediaFormat.setInteger(MediaFormat.KEY_MAX_HEIGHT, 1080);
            mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, 8000000);
            mediaFormat.setInteger(MediaFormat.KEY_PRIORITY,0);
            mediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE,1920*1080);
            mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE,30);
            mediaFormat.setInteger(MediaFormat.KEY_CHANNEL_COUNT,2);
            mediaFormat.setInteger(MediaFormat.KEY_MAX_WIDTH, 1920);

            try {
               codec = MediaCodec.createDecoderByType("video/avc");

            } catch (IOException e) {
                Log.d("lichao", "codec failed %s" + e.toString());
            }

            codec.configure(mediaFormat, MainRender.this.surfaceHolder.getSurface(), null, 0);Log.d("lichao","VideoThread started3333333333333");
            codec.start();

            byte[] tmpPkt = null;


            MediaCodec.BufferInfo outBufferInfo = new MediaCodec.BufferInfo();

           while (videoThreahFlag) {

                int inIndex = codec.dequeueInputBuffer(100);
                if (inIndex >= 0) {


                    count++;
                    if(count>65500)
                        count=0;
                    try {

                        tmpPkt = videoQueue.take();

                        if(!videoThreahFlag)
                            break;

                        codec.queueInputBuffer(inIndex, 0, tmpPkt.length, 0, 0);
                    } catch (InterruptedException e) {
                        Log.d("lichao", "InterruptedException is "+e.toString());
                    }


                }
                else
                {
                   Log.e("lichao", "input buffer full,indx is"+inIndex);


                }

                int outIndex;
                {
                    outIndex = codec.dequeueOutputBuffer(outBufferInfo, 100);
                    if (outIndex >= 0) {
                        codec.releaseOutputBuffer(outIndex, true);
                    }


                }


            }
            codec.reset();
            codec.stop();
            codec.release();


            Intent i=new Intent("render.message");
            i.putExtra("isClose", true);
            rbAct.sendBroadcast(i);

        }
    }





    public MainRender(Context context, AttributeSet attributeSet) {
        super(context, attributeSet);
        init(context);
    }
    private boolean init = false;

    private void init(Context context) {
        surfaceHolder = getHolder();
        surfaceHolder.addCallback(this);



        setOnTouchListener(this);

      //  renderHandler = new RenderHandler(this);

        rbAct = (RenderBoard)context;

        network = new NetworkManager(rbAct);




        Point outSize = new Point();
        WindowManager wm = (WindowManager)context.getSystemService(Context.WINDOW_SERVICE);
        wm.getDefaultDisplay().getRealSize(outSize);
        //getWindowManager().getDefaultDisplay().getRealSize(outSize);
        int x = outSize.x;
        int y = outSize.y;

        xRatio = (float)VIDEO_WIDTH/x;
        yRatio = (float)VIDEO_HEIGHT/y;
      //  BandMaster.getInstance().setCallBackListener(this);
    }

    public void surfaceCreated(SurfaceHolder holder) {
        if(!init) {


            videoThread = new Thread(new VideoThread());//
            videoThread.start();

            audioThread = new Thread(this);//
            audioThread.start();

            new ConnectTask().execute();
            init = true;


        }
    }
    public void releaseNetwork()
    {
     ;
        if(network!=null)
         network.closeAll();
    }
    public void disconnect()
    {

        videoThreahFlag = false;
        audioThreadFlag = false;

        try {
            videoQueue.put(new byte[4]);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

   //     videoThread.interrupt();
   //
    }
    boolean cs = false;
    long startMs;
    int frame = 0;
    int lent = 0;
    int x=0;
    public void reportVideoData(byte[] data,int len)
    {

        if(!cs)
        {
             startMs = System.currentTimeMillis();
             cs = true;
             frame=1;
             lent=0;
        }
        else
        {
            frame++;
            lent+=len;

            startMs = System.currentTimeMillis();


        }



        try {
            videoQueue.put(data);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

    }
    public void reportAudioData(byte[] data,int len)
    {


        try {
            audioQueue.put(data);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

    }

    Thread videoThread;
    Thread audioThread;

    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        disconnect();

        init = false;

    }

    AudioTrack audioTrack=null;
    public void run()
    {
        byte[] take;
        int channel = 1;

        int minBufferSize = AudioTrack.getMinBufferSize(48000,
                 AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_16BIT);

        audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,
                48000,
                AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_16BIT,
                minBufferSize,
                AudioTrack.MODE_STREAM);
        audioTrack.play();
        OpusJni.getInstance().OpusInit();
        short[] s_buf = new short[10000];
        int s_len = 0;

        while (audioThreadFlag)
        {


            try {
                take = audioQueue.take();
                s_len = OpusJni.getInstance().Opusdecode(take,s_buf,take.length);
                byte[] tmp = ShortByteUtil.shortArray2ByteArray(s_buf,s_len);
                if(audioTrack!=null)
                {

                    audioTrack.write(tmp,0,tmp.length);
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

        }


    }





    final static int DOWN = MotionEvent.ACTION_DOWN;
    final static int MOVE = MotionEvent.ACTION_MOVE;
    final static int UP = MotionEvent.ACTION_UP;


    public void sendOneInputEvent(int type,int code,int value)
    {
        MyInputEvent event = new MyInputEvent();

      event.type = type;
      event.code = code;
      event.value = value;
      network.putMsg(event);


    }





    @Override
    public boolean onTouch(View view, MotionEvent event) {
        int action = event.getActionMasked();
        int x = (int)(event.getX()*xRatio);
        int y = (int)(event.getY()*yRatio);
        switch (action){

            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_UP:
                int id1 = event.getPointerId(event.getActionIndex());

                sendInputEvent(action,id1,x,y);
                break;
              //     Log.d("lichao1","event is "+action+" x is"+x+" y is "+y);


            case  MotionEvent.ACTION_POINTER_DOWN:
            case MotionEvent.ACTION_POINTER_UP:

                int id = event.getPointerId(event.getActionIndex());

                sendPointInputEvent(action,id,x,y);

                break;

            case MotionEvent.ACTION_MOVE:
                int count = event.getPointerCount();
                int id2 = event.getPointerId(event.getActionIndex());
                if(count == 1)
                    sendInputEvent(action,id2,x,y);
                else
                {
                    for(int i=0;i<count;i++)
                    {
                        sendPointInputEvent(action,i,(int)(event.getX(i)*xRatio),(int)(event.getY(i)*yRatio));
                    }
                }

                break;


        }
        return true;
    }

    private void sendMoveInputEvent(int action, int count,int x, int y) {
        if(count ==0)
        {
            sendOneInputEvent(EV_ABS,ABS_MT_POSITION_X,x);
            sendOneInputEvent(EV_ABS,ABS_MT_POSITION_Y,y);
            sendOneInputEvent(EV_SYN,SYN_REPORT,0x00);
        }
        else
        {
            for(int i=0;i<count;i++)
            {
                sendOneInputEvent(EV_ABS,ABS_MT_POSITION_X,x);
                sendOneInputEvent(EV_ABS,ABS_MT_POSITION_Y,y);
                sendOneInputEvent(EV_SYN,SYN_REPORT,0x00);
            }
        }
    }

    public void sendInputEvent(int event,int id,int x,int y)
    {

        if(event == DOWN)
        {
            sendOneInputEvent(EV_ABS,ABS_MT_SLOT,id);
            sendOneInputEvent(EV_ABS,ABS_MT_TRACKING_ID,0x01);
            sendOneInputEvent(EV_KEY,BTN_TOUCH,0x01);
            sendOneInputEvent(EV_ABS,ABS_MT_POSITION_X,x);
            sendOneInputEvent(EV_ABS,ABS_MT_POSITION_Y,y);
            sendOneInputEvent(EV_SYN,SYN_REPORT,0x00);
        }

        if(event == MOVE)
        {
            sendOneInputEvent(EV_ABS,ABS_MT_SLOT,id);

            sendOneInputEvent(EV_ABS,ABS_MT_POSITION_X,x);
            sendOneInputEvent(EV_ABS,ABS_MT_POSITION_Y,y);
            sendOneInputEvent(EV_SYN,SYN_REPORT,0x00);
        }

        if(event == UP)
        {
            sendOneInputEvent(EV_ABS,ABS_MT_SLOT,id);
            sendOneInputEvent(EV_ABS,ABS_MT_TRACKING_ID,0xffffffff);
            sendOneInputEvent(EV_SYN,SYN_REPORT,0x00);
        }

    }
    private void sendPointInputEvent(int event, int actionIndex, int x, int y) {
        if(event == MotionEvent.ACTION_POINTER_DOWN)
        {
            sendOneInputEvent(EV_ABS,ABS_MT_SLOT,actionIndex);
            sendOneInputEvent(EV_ABS,ABS_MT_TRACKING_ID,0x01);
            sendOneInputEvent(EV_KEY,BTN_TOUCH,0x01);
            sendOneInputEvent(EV_ABS,ABS_MT_POSITION_X,x);
            sendOneInputEvent(EV_ABS,ABS_MT_POSITION_Y,y);
            sendOneInputEvent(EV_SYN,SYN_REPORT,0x00);
        }

        if(event == MotionEvent.ACTION_MOVE)
        {
            sendOneInputEvent(EV_ABS,ABS_MT_SLOT,actionIndex);
            sendOneInputEvent(EV_ABS,ABS_MT_POSITION_X,x);
            sendOneInputEvent(EV_ABS,ABS_MT_POSITION_Y,y);
            sendOneInputEvent(EV_SYN,SYN_REPORT,0x00);
        }

        if(event == MotionEvent.ACTION_POINTER_UP)
        {
            sendOneInputEvent(EV_ABS,ABS_MT_SLOT,actionIndex);
            sendOneInputEvent(EV_ABS,ABS_MT_TRACKING_ID,0xffffffff);
            sendOneInputEvent(EV_SYN,SYN_REPORT,0x00);
        }
    }
}

