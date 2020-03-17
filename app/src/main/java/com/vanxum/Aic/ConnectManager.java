package com.vanxum.Aic;

import android.Manifest;
import android.app.Activity;
import android.os.Bundle;


public class ConnectManager extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
       setContentView(R.layout.connectivity_main);

        this.requestPermissions(new String[]{
            Manifest.permission.INTERNET
        }, 1);




    }
}
