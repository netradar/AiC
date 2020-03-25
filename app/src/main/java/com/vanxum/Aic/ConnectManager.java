package com.vanxum.Aic;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;


public class ConnectManager extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
       setContentView(R.layout.connectivity_main);

        this.requestPermissions(new String[]{
            Manifest.permission.INTERNET
        }, 1);




    }

    public void onClick(View v)
    {
        if(v==this.findViewById(R.id.ip))
        {
            Intent intent = new Intent();
            intent.setClass(ConnectManager.this, Ipconnect.class);
            startActivity(intent);
        }
    }


}
