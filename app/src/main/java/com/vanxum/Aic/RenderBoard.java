package com.vanxum.Aic;

import android.animation.LayoutTransition;
import android.animation.ObjectAnimator;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;
import androidx.annotation.NonNull;
import java.util.Timer;
import java.util.TimerTask;

public class RenderBoard extends Activity implements examReportInterface {

    public MainRender mainRender;
    BroadcastReceiver bdReceiver;
    private IntentFilter ifilter;

    private TextView netDelay,netFps,netSpeed;

    long speedMs;

    Timer timer;

    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == 1){
                NetworkJni.getInstance().sendExam();
                speedMs = System.currentTimeMillis();
            }
            if(msg.what ==2)
            {
                  String t=String.valueOf(msg.arg1)+ " ms";
                  netDelay.setText(t);
            }
            if(msg.what==3)
            {
                String t=String.valueOf(msg.arg1)+ " fps";
                netFps.setText(t);
                t = String.valueOf(msg.arg2/1000)+" KB/s";
                netSpeed.setText(t);
            }
            super.handleMessage(msg);
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {


        super.onCreate(savedInstanceState);
        setContentView(R.layout.render_main);

        mainRender = (MainRender) findViewById(R.id.main_background1);

        mainRender.setNet(getIntent().getStringExtra("ipaddr"),getIntent().getIntExtra("port",-1));
        mainRender.setDecodeType(getIntent().getIntExtra("type",0));

        NetworkJni.getInstance().setOnExamReportListener(this);


   /*     LayoutTransition transition = new LayoutTransition();
        ObjectAnimator animIn = ObjectAnimator.ofFloat(null, "alpha", 0, 1);
        transition.setAnimator(LayoutTransition.APPEARING, animIn);
        transition.setDuration(500);
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
*/
        netDelay=(TextView)findViewById(R.id.net_delay);
        netFps=(TextView)findViewById(R.id.net_fps);
        netSpeed=(TextView)findViewById(R.id.net_speed);
        bdReceiver=new BroadcastReceiver()
        {

            @Override
            public void onReceive(Context context, Intent intent)
            {

                String bd_type=intent.getAction();

                if(bd_type.equals("render.message"))
                {
                    if(intent.getBooleanExtra("isClose", false))
                    {
                          //  mainRender.releaseNetwork();
                         //   finish();
                    }
                    if(intent.getBooleanExtra("networkerr", false))
                    {
                       onNetworkError();
                    }

                }

            }
        };
        ifilter= new IntentFilter();
        ifilter.addAction("render.message");
        registerReceiver(bdReceiver,ifilter);

        /*InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.showSoftInput(mainRender,InputMethodManager.SHOW_FORCED);*/

    }

    @Override
    protected void onPause() {

        timer.cancel();
        super.onPause();
    }



    @Override
    protected void onResume() {

        timer = new Timer();
        TimerTask timerTask = new TimerTask() {
            @Override
            public void run() {
                Message message = new Message();
                message.what = 1;
                handler.sendMessage(message);
            }
        };
        timer.schedule(timerTask,0,1000);
        super.onResume();
    }

    @Override
    public void onBackPressed()

    {
        Intent intent = new Intent();
        intent.setClass(RenderBoard.this, RemoteControl.class);

        startActivityForResult(intent,0);

    }


    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if(requestCode==0)
        {
            switch (resultCode)
            {
                case 5:
                    mainRender.sendPower();
                    break;
                case 4:
                    mainRender.sendBack();
                    break;
                case 1:
                    mainRender.sendHome();
                    break;
                case 2:
                    mainRender.sendMenu();
                    break;
                case 3:
                    mainRender.disconnect();
                    finish();
                    break;
            }
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    protected void onDestroy() {

        unregisterReceiver(bdReceiver);
        timer.cancel();
        super.onDestroy();

    }
    public void onNetworkError()
    {
        final Toast toast = Toast.makeText(RenderBoard.this, "网络连接失败", Toast.LENGTH_SHORT);
        toast.show();
        finish();
    }

    public void updateNetinfo(int fps,int data)
    {
        Message message = new Message();
        message.what = 3;
        message.arg1 = fps;
        message.arg2 = data;
        handler.sendMessage(message);
    }


    public void onExam()
    {
        Message message = new Message();
        message.what = 2;
        message.arg1 = (int)(System.currentTimeMillis()-speedMs);
        handler.sendMessage(message);
    }


    @Override
    public void reportExamFeedback() {
        onExam();
    }
}
