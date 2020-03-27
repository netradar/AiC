package com.vanxum.Aic;

import android.animation.LayoutTransition;
import android.animation.ObjectAnimator;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Toast;

import androidx.annotation.NonNull;

import static androidx.core.content.ContextCompat.getSystemService;


public class RenderBoard extends Activity{
  //  private final static String TAG = "com.vanxum.Aic.RenderBoard";
    public MainRender mainRender;
    BroadcastReceiver bdReceiver;
    private IntentFilter ifilter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        Log.d("lichao","renderboard onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.render_main);

        mainRender = (MainRender) findViewById(R.id.main_background1);

        mainRender.setNet(getIntent().getStringExtra("ipaddr"),getIntent().getIntExtra("port",-1));

        LayoutTransition transition = new LayoutTransition();
        ObjectAnimator animIn = ObjectAnimator.ofFloat(null, "alpha", 0, 1);
        transition.setAnimator(LayoutTransition.APPEARING, animIn);
        transition.setDuration(500);
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);

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

        InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.showSoftInput(mainRender,InputMethodManager.SHOW_FORCED);







    }

    @Override
    protected void onPause() {
        Log.d("lichao","renderboard onPause");
        super.onPause();
    }

    @Override
    protected void onStop() {
        Log.d("lichao","renderboard onStop");
        super.onStop();
    }

    @Override
    public void onBackPressed()

    {
        Intent intent = new Intent();
        intent.setClass(RenderBoard.this, RemoteControl.class);

        startActivityForResult(intent,0);
      //  mainRender.disconnect();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if(requestCode==0)
        {
            switch (resultCode)
            {
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
        Log.d("lichao","renderboard destroyed");
        unregisterReceiver(bdReceiver);
        super.onDestroy();

    }
    public void onNetworkError()
    {
        final Toast toast = Toast.makeText(RenderBoard.this, "网络连接失败", Toast.LENGTH_SHORT);
        toast.show();
        finish();
    }
}
