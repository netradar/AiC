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
import android.view.View;
import android.widget.Toast;




public class RenderBoard extends Activity{
  //  private final static String TAG = "com.vanxum.Aic.RenderBoard";
    public MainRender mainRender;
    BroadcastReceiver bdReceiver;
    private IntentFilter ifilter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.render_main);

        mainRender = (MainRender)findViewById(R.id.main_background1);

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
                            mainRender.releaseNetwork();
                            finish();
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


        final SharedPreferences preferences = getApplicationContext().getSharedPreferences("setting", MODE_PRIVATE);

        /*String contacts = settings.getString("contacts","[]");
        try {
            JSONArray array = new JSONArray(contacts);
            JSONObject object;
            for (int i = 0; i < array.length(); i++) {
                object = array.getJSONObject(i);
                Log.e(TAG,"contacts address:" + object.getString("address") + " note:" + object.getString("note"));
            }

        } catch (JSONException e) {
            e.printStackTrace();
        }*/




    }


    @Override
    public void onBackPressed()
    {
        mainRender.disconnect();
    }

    @Override
    protected void onDestroy() {
        unregisterReceiver(bdReceiver);
        super.onDestroy();

    }
    public void onNetworkError()
    {
        final Toast toast = Toast.makeText(RenderBoard.this, "网络连接失败", Toast.LENGTH_SHORT);
        toast.show();
        this.onBackPressed();
    }
}
